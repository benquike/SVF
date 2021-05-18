//===- SVFUtil.cpp -- Analysis helper functions----------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013->  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

/*
 * SVFUtil.cpp
 *
 *  Created on: Apr 11, 2013
 *      Author: Yulei Sui
 */

#include "boost/algorithm/string/trim.hpp"
#include "boost/filesystem.hpp"

#include "SVF-FE/LLVMUtil.h"
#include "llvm/Support/JSON.h"

using namespace SVF;

namespace SVF {
namespace SVFUtil {

/*!
 * A value represents an object if it is
 * 1) function,
 * 2) global
 * 3) stack
 * 4) heap
 */
bool isObject(const Value *ref, SVFModule *svfMod) {
    bool createobj = false;
    if (llvm::isa<Instruction>(ref) &&
        isStaticExtCall(llvm::cast<Instruction>(ref), svfMod)) {
        /// a call to external function
        createobj = true;
    } else if (llvm::isa<Instruction>(ref) &&
               isHeapAllocExtCallViaRet(llvm::cast<Instruction>(ref), svfMod)) {
        // a call to heap allocation function
        createobj = true;
    } else if (llvm::isa<GlobalVariable>(ref)) {
        // global variable
        createobj = true;
    } else if (llvm::isa<Function>(ref) || llvm::isa<AllocaInst>(ref)) {
        /// function or Alloca instruction
        createobj = true;
    }

    return createobj;
}

/*!
 * Return reachable bbs from function entry
 */
void getFunReachableBBs(const Function *fun, DominatorTree *dt,
                        std::vector<const BasicBlock *> &reachableBBs) {
    Set<const BasicBlock *> visited;
    std::vector<const BasicBlock *> bbVec;
    bbVec.push_back(&fun->getEntryBlock());
    while (!bbVec.empty()) {
        const BasicBlock *bb = bbVec.back();
        bbVec.pop_back();
        reachableBBs.push_back(bb);
        if (DomTreeNode *dtNode = dt->getNode(const_cast<BasicBlock *>(bb))) {
            for (auto &DI : *dtNode) {
                const BasicBlock *succbb = DI->getBlock();
                if (visited.find(succbb) == visited.end())
                    visited.insert(succbb);
                else
                    continue;
                bbVec.push_back(succbb);
            }
        }
    }
}

/*!
 * Return true if the function has a return instruction reachable from function
 * entry
 */
bool functionDoesNotRet(const Function *fun) {

    std::vector<const BasicBlock *> bbVec;
    Set<const BasicBlock *> visited;
    bbVec.push_back(&fun->getEntryBlock());
    while (!bbVec.empty()) {
        const BasicBlock *bb = bbVec.back();
        bbVec.pop_back();
        for (const auto &it : *bb) {
            if (llvm::isa<ReturnInst>(it))
                return false;
        }

        for (succ_const_iterator sit = succ_begin(bb), esit = succ_end(bb);
             sit != esit; ++sit) {
            const BasicBlock *succbb = (*sit);
            if (visited.find(succbb) == visited.end())
                visited.insert(succbb);
            else
                continue;
            bbVec.push_back(succbb);
        }
    }
    //    if(isProgEntryFunction(fun)==false) {
    //        writeWrnMsg(fun->getName().str() + " does not have return");
    //    }
    return true;
}

/*!
 * Return true if this is a function without any possible caller
 */
bool isDeadFunction(const Function *fun, SVFModule *svfMod) {
    if (fun->hasAddressTaken())
        return false;
    if (isProgEntryFunction(fun))
        return false;
    for (Value::const_user_iterator i = fun->user_begin(), e = fun->user_end();
         i != e; ++i) {
        if (isCallSite(*i))
            return false;
    }
    if (svfMod->getLLVMModSet()->hasDeclaration(fun)) {
        const SVFModule::FunctionSetType &decls =
            svfMod->getLLVMModSet()->getDeclaration(fun);
        for (const auto *it : decls) {
            const Function *decl = it->getLLVMFun();
            if (decl->hasAddressTaken())
                return false;
            for (Value::const_user_iterator i = decl->user_begin(),
                                            e = decl->user_end();
                 i != e; ++i) {
                if (isCallSite(*i))
                    return false;
            }
        }
    }
    return true;
}

/*!
 * Return true if this is a value in a dead function (function without any
 * caller)
 */
bool isPtrInDeadFunction(const Value *value, SVFModule *svfMod) {
    if (const auto *inst = llvm::dyn_cast<Instruction>(value)) {
        if (isDeadFunction(inst->getParent()->getParent(), svfMod))
            return true;
    } else if (const auto *arg = llvm::dyn_cast<Argument>(value)) {
        if (isDeadFunction(arg->getParent(), svfMod))
            return true;
    }
    return false;
}

/*!
 * Strip constant casts
 */
const Value *stripConstantCasts(const Value *val) {
    if (llvm::isa<GlobalValue>(val) || isInt2PtrConstantExpr(val))
        return val;
    else if (const auto *CE = llvm::dyn_cast<ConstantExpr>(val)) {
        if (Instruction::isCast(CE->getOpcode()))
            return stripConstantCasts(CE->getOperand(0));
    }
    return val;
}

/*!
 * Strip all casts
 */
Value *stripAllCasts(Value *val) {
    while (true) {
        if (auto *ci = llvm::dyn_cast<CastInst>(val)) {
            val = ci->getOperand(0);
        } else if (auto *ce = llvm::dyn_cast<ConstantExpr>(val)) {
            if (ce->isCast())
                val = ce->getOperand(0);
        } else {
            return val;
        }
    }
    return nullptr;
}

/// Get the next instructions following control flow
void getNextInsts(const Instruction *curInst,
                  std::vector<const Instruction *> &instList) {
    if (!curInst->isTerminator()) {
        const Instruction *nextInst = curInst->getNextNode();
        if (isIntrinsicInst(nextInst))
            getNextInsts(nextInst, instList);
        else
            instList.push_back(nextInst);
    } else {
        const BasicBlock *BB = curInst->getParent();
        // Visit all successors of BB in the CFG
        for (succ_const_iterator it = succ_begin(BB), ie = succ_end(BB);
             it != ie; ++it) {
            const Instruction *nextInst = &((*it)->front());
            if (isIntrinsicInst(nextInst))
                getNextInsts(nextInst, instList);
            else
                instList.push_back(nextInst);
        }
    }
}

/// Get the previous instructions following control flow
void getPrevInsts(const Instruction *curInst,
                  std::vector<const Instruction *> &instList) {
    if (curInst != &(curInst->getParent()->front())) {
        const Instruction *prevInst = curInst->getPrevNode();
        if (isIntrinsicInst(prevInst))
            getPrevInsts(prevInst, instList);
        else
            instList.push_back(prevInst);
    } else {
        const BasicBlock *BB = curInst->getParent();
        // Visit all successors of BB in the CFG
        for (const_pred_iterator it = pred_begin(BB), ie = pred_end(BB);
             it != ie; ++it) {
            const Instruction *prevInst = &((*it)->back());
            if (isIntrinsicInst(prevInst))
                getPrevInsts(prevInst, instList);
            else
                instList.push_back(prevInst);
        }
    }
}

/*!
 * Return the type of the object from a heap allocation
 */
const Type *getTypeOfHeapAlloc(const Instruction *inst, SVFModule *svfMod) {
    const PointerType *type = llvm::dyn_cast<PointerType>(inst->getType());

    if (isHeapAllocExtCallViaRet(inst, svfMod)) {
        const Instruction *nextInst = inst->getNextNode();
        if (nextInst && nextInst->getOpcode() == Instruction::BitCast)
            // we only consider bitcast instructions and ignore others (e.g.,
            // IntToPtr and ZExt)
            type = llvm::dyn_cast<PointerType>(inst->getNextNode()->getType());
    } else if (isHeapAllocExtCallViaArg(inst, svfMod)) {
        CallSite cs = getLLVMCallSite(inst);
        int arg_pos = getHeapAllocHoldingArgPosition(
            getCallee(svfMod->getLLVMModSet(), cs));
        const Value *arg = cs.getArgument(arg_pos);
        type = llvm::dyn_cast<PointerType>(arg->getType());
    } else {
        assert(false && "not a heap allocation instruction?");
    }

    assert(type && "not a pointer type?");
    return type->getElementType();
}

/*!
 * Get position of a successor basic block
 */
u32_t getBBSuccessorPos(const BasicBlock *BB, const BasicBlock *Succ) {
    u32_t i = 0;
    for (const BasicBlock *SuccBB : successors(BB)) {
        if (SuccBB == Succ)
            return i;
        i++;
    }
    assert(false && "Didn't find succesor edge?");
    return 0;
}

/*!
 * Return a position index from current bb to it successor bb
 */
u32_t getBBPredecessorPos(const BasicBlock *bb, const BasicBlock *succbb) {
    u32_t pos = 0;
    for (const_pred_iterator it = pred_begin(succbb), et = pred_end(succbb);
         it != et; ++it, ++pos) {
        if (*it == bb)
            return pos;
    }
    assert(false && "Didn't find predecessor edge?");
    return pos;
}

/*!
 *  Get the num of BB's successors
 */
u32_t getBBSuccessorNum(const BasicBlock *BB) {
    return BB->getTerminator()->getNumSuccessors();
}

/*!
 * Get the num of BB's predecessors
 */
u32_t getBBPredecessorNum(const BasicBlock *BB) {
    u32_t num = 0;
    for (const_pred_iterator it = pred_begin(BB), et = pred_end(BB); it != et;
         ++it)
        num++;
    return num;
}

/*
 * Reference functions:
 * llvm::parseIRFile (lib/IRReader/IRReader.cpp)
 * llvm::parseIR (lib/IRReader/IRReader.cpp)
 */
bool isIRFile(const std::string &filename) {
    llvm::ErrorOr<std::unique_ptr<llvm::MemoryBuffer>> FileOrErr =
        llvm::MemoryBuffer::getFileOrSTDIN(filename);
    if (FileOrErr.getError())
        return false;
    llvm::MemoryBufferRef Buffer = FileOrErr.get()->getMemBufferRef();
    const auto *bufferStart = (const unsigned char *)Buffer.getBufferStart();
    const auto *bufferEnd = (const unsigned char *)Buffer.getBufferEnd();
    return llvm::isBitcode(bufferStart, bufferEnd)
               ? true
               : Buffer.getBuffer().startswith("; ModuleID =");
}

/// Get the names of all modules into a vector
/// And process arguments
void processArguments(int argc, char **argv, int &arg_num, char **arg_value,
                      std::vector<std::string> &moduleNameVec) {
    bool first_ir_file = true;
    for (s32_t i = 0; i < argc; ++i) {
        std::string argument(argv[i]);
        if (isIRFile(argument)) {
            if (find(moduleNameVec.begin(), moduleNameVec.end(), argument) ==
                moduleNameVec.end())
                moduleNameVec.push_back(argument);
            if (first_ir_file) {
                arg_value[arg_num] = argv[i];
                arg_num++;
                first_ir_file = false;
            }
        } else {
            arg_value[arg_num] = argv[i];
            arg_num++;
        }
    }
}

llvm::DILocation *getDILocation(const Instruction *inst) {
    if (auto *MN = inst->getMetadata(LLVMContext::MD_dbg)) {
        return llvm::dyn_cast<llvm::DILocation>(MN);
    }
    return nullptr;
}

llvm::DbgVariableIntrinsic *getDbgVarIntrinsic(const llvm::Value *V) {
    if (auto *VAM =
            llvm::ValueAsMetadata::getIfExists(const_cast<llvm::Value *>(V))) {
        if (auto *MDV =
                llvm::MetadataAsValue::getIfExists(V->getContext(), VAM)) {
            for (auto *U : MDV->users()) {
                if (auto *DBGIntr =
                        llvm::dyn_cast<llvm::DbgVariableIntrinsic>(U)) {
                    return DBGIntr;
                }
            }
        }
    } else if (const auto *Arg = llvm::dyn_cast<llvm::Argument>(V)) {
        /* If mem2reg is not activated, formal parameters will be stored in
         * registers at the beginning of function call. Debug info will be
         * linked to those alloca's instead of the arguments itself. */
        for (const auto *User : Arg->users()) {
            if (const auto *Store = llvm::dyn_cast<llvm::StoreInst>(User)) {
                if (Store->getValueOperand() == Arg &&
                    llvm::isa<llvm::AllocaInst>(Store->getPointerOperand())) {
                    return getDbgVarIntrinsic(Store->getPointerOperand());
                }
            }
        }
    }
    return nullptr;
}

llvm::DILocalVariable *getDILocalVariable(const llvm::Value *V) {
    if (auto *DbgIntr = getDbgVarIntrinsic(V)) {
        if (auto *DDI = llvm::dyn_cast<llvm::DbgDeclareInst>(DbgIntr)) {
            return DDI->getVariable();
        }
        if (auto *DVI = llvm::dyn_cast<llvm::DbgValueInst>(DbgIntr)) {
            return DVI->getVariable();
        }
    }
    return nullptr;
}

llvm::DIGlobalVariable *getDIGlobalVariable(const llvm::Value *V) {
    if (const auto *GV = llvm::dyn_cast<llvm::GlobalVariable>(V)) {
        if (auto *MN = GV->getMetadata(llvm::LLVMContext::MD_dbg)) {
            if (auto *DIGVExp =
                    llvm::dyn_cast<llvm::DIGlobalVariableExpression>(MN)) {
                return DIGVExp->getVariable();
            }
        }
    }
    return nullptr;
}

llvm::DISubprogram *getDISubprogram(const llvm::Value *V) {
    if (const auto *F = llvm::dyn_cast<llvm::Function>(V)) {
        return F->getSubprogram();
    }
    return nullptr;
}

llvm::DILocation *getDILocation(const llvm::Value *V) {
    // Arguments and Instruction such as AllocaInst
    if (auto *DbgIntr = getDbgVarIntrinsic(V)) {
        if (auto *MN = DbgIntr->getMetadata(llvm::LLVMContext::MD_dbg)) {
            return llvm::dyn_cast<llvm::DILocation>(MN);
        }
    } else if (const auto *I = llvm::dyn_cast<llvm::Instruction>(V)) {
        if (auto *MN = I->getMetadata(llvm::LLVMContext::MD_dbg)) {
            return llvm::dyn_cast<llvm::DILocation>(MN);
        }
    }
    return nullptr;
}

llvm::DIFile *getDIFile(const llvm::Value *V) {
    if (const auto *GO = llvm::dyn_cast<llvm::GlobalObject>(V)) {
        if (auto *MN = GO->getMetadata(llvm::LLVMContext::MD_dbg)) {
            if (auto *Subpr = llvm::dyn_cast<llvm::DISubprogram>(MN)) {
                return Subpr->getFile();
            }
            if (auto *GVExpr =
                    llvm::dyn_cast<llvm::DIGlobalVariableExpression>(MN)) {
                return GVExpr->getVariable()->getFile();
            }
        }
    } else if (const auto *Arg = llvm::dyn_cast<llvm::Argument>(V)) {
        if (auto *LocVar = getDILocalVariable(Arg)) {
            return LocVar->getFile();
        }
    } else if (const auto *I = llvm::dyn_cast<llvm::Instruction>(V)) {
        if (I->isUsedByMetadata()) {
            if (auto *LocVar = getDILocalVariable(I)) {
                return LocVar->getFile();
            }
        } else if (I->getMetadata(llvm::LLVMContext::MD_dbg)) {
            return I->getDebugLoc()->getFile();
        }
    }
    return nullptr;
}

std::string getVarNameFromIR(const llvm::Value *V) {
    if (auto *LocVar = getDILocalVariable(V)) {
        return LocVar->getName().str();
    } else if (auto *GlobVar = getDIGlobalVariable(V)) {
        return GlobVar->getName().str();
    }
    return "";
}

std::string getFunctionNameFromIR(const llvm::Value *V) {
    // We can return unmangled function names w/o checking debug info
    if (const auto *F = llvm::dyn_cast<llvm::Function>(V)) {
        return F->getName().str();
    } else if (const auto *Arg = llvm::dyn_cast<llvm::Argument>(V)) {
        return Arg->getParent()->getName().str();
    } else if (const auto *I = llvm::dyn_cast<llvm::Instruction>(V)) {
        return I->getFunction()->getName().str();
    }
    return "";
}

std::string getFilePathFromIR(const llvm::Value *V) {
    if (auto *DIF = getDIFile(V)) {
        boost::filesystem::path File(DIF->getFilename().str());
        boost::filesystem::path Dir(DIF->getDirectory().str());
        if (!File.empty()) {
            // try to concatenate file path and dir to get absolut path
            if (!File.has_root_directory() && !Dir.empty()) {
                File = Dir / File;
            }
            return File.string();
        }
    } else {
        /* As a fallback solution, we will return 'source_filename' info from
         * module. However, it is not guaranteed to contain the absoult path,
         * and it will return 'llvm-link' for linked modules. */
        if (const auto *F = llvm::dyn_cast<llvm::Function>(V)) {
            return F->getParent()->getSourceFileName();
        } else if (const auto *Arg = llvm::dyn_cast<llvm::Argument>(V)) {
            return Arg->getParent()->getParent()->getSourceFileName();
        } else if (const auto *I = llvm::dyn_cast<llvm::Instruction>(V)) {
            return I->getFunction()->getParent()->getSourceFileName();
        }
    }
    return "";
}

unsigned int getLineFromIR(const llvm::Value *V) {
    // Argument and Instruction
    if (auto *DILoc = getDILocation(V)) {
        return DILoc->getLine();
    } else if (auto *DISubpr = getDISubprogram(V)) { // Function
        return DISubpr->getLine();
    } else if (auto *DIGV = getDIGlobalVariable(V)) { // Globals
        return DIGV->getLine();
    }
    return 0;
}

std::string getDirectoryFromIR(const llvm::Value *V) {
    // Argument and Instruction
    if (auto *DILoc = getDILocation(V)) {
        return DILoc->getDirectory();
    } else if (auto *DISubpr = getDISubprogram(V)) { // Function
        return DISubpr->getDirectory();
    } else if (auto *DIGV = getDIGlobalVariable(V)) { // Globals
        return DIGV->getDirectory();
    }
    return nullptr;
}

unsigned int getColumnFromIR(const llvm::Value *V) {
    // Globals and Function have no column info
    if (auto *DILoc = getDILocation(V)) {
        return DILoc->getColumn();
    }
    return 0;
}

std::string getSrcCodeFromIR(const llvm::Value *V) {
    unsigned int LineNr = getLineFromIR(V);
    if (LineNr > 0) {
        boost::filesystem::path Path(getFilePathFromIR(V));
        if (boost::filesystem::exists(Path) &&
            !boost::filesystem::is_directory(Path)) {
            std::ifstream Ifs(Path.string(), std::ios::binary);
            if (Ifs.is_open()) {
                Ifs.seekg(std::ios::beg);
                std::string SrcLine;
                for (unsigned int I = 0; I < LineNr - 1; ++I) {
                    Ifs.ignore(std::numeric_limits<std::streamsize>::max(),
                               '\n');
                }
                std::getline(Ifs, SrcLine);
                boost::algorithm::trim(SrcLine);
                return SrcLine;
            }
        }
    }
    return "";
}

std::string getModuleIDFromIR(const llvm::Value *V) {
    if (const auto *GO = llvm::dyn_cast<llvm::GlobalObject>(V)) {
        return GO->getParent()->getModuleIdentifier();
    } else if (const auto *Arg = llvm::dyn_cast<llvm::Argument>(V)) {
        return Arg->getParent()->getParent()->getModuleIdentifier();
    } else if (const auto *I = llvm::dyn_cast<llvm::Instruction>(V)) {
        return I->getFunction()->getParent()->getModuleIdentifier();
    }
    return "";
}

} // end of namespace SVFUtil
} // end of namespace SVF
