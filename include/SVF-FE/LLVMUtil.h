//===- SVFUtil.h -- Analysis helper functions----------------------------//
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
 * SVFUtil.h
 *
 *  Created on: Apr 11, 2013
 *      Author: Yulei Sui
 */

#ifndef INCLUDE_SVF_FE_LLVMUTIL_H_
#define INCLUDE_SVF_FE_LLVMUTIL_H_

#include "Util/BasicTypes.h"
#include "Util/ExtAPI.h"
#include "Util/SVFUtil.h"
#include "Util/ThreadAPI.h"
#include "llvm/Support/JSON.h"

namespace SVF::SVFUtil {

/// This function servers a allocation wrapper detector
inline bool isAnAllocationWraper(const Instruction *) { return false; }

/// Return LLVM function if this value is
inline const Function *getLLVMFunction(const Value *val) {
    const auto *fun = llvm::dyn_cast<Function>(val->stripPointerCasts());
    return fun;
}

///
/// Return true if the call is an external function (external library in
/// function summary table) or it is just a declaration without a definition. If
/// the libary function is redefined in the application code (e.g., memcpy), it
/// will return false and will not be treated as an external call.
///
///  ExtAPI::getExtAPI()->is_ext(fun) will return true if fun is
///  a declaration.
///
///  So, no worries
//@{
inline bool isExtCall(const SVFFunction *fun) {
    return fun && ExtAPI::getExtAPI()->is_ext(fun);
}

inline bool isExtCall(const CallSite cs, SVFModule *svfMod) {
    return isExtCall(getCallee(svfMod->getLLVMModSet(), cs));
}

inline bool isExtCall(const Instruction *inst, SVFModule *svfMod) {
    return isExtCall(getCallee(svfMod->getLLVMModSet(), inst));
}
//@}

/// Return true if the call is a heap allocator/reallocator
//@{
/// note that these two functions are not suppose to be used externally
inline bool isHeapAllocExtFunViaRet(const SVFFunction *fun) {
    return fun && (ExtAPI::getExtAPI()->is_alloc(fun) ||
                   ExtAPI::getExtAPI()->is_realloc(fun));
}
inline bool isHeapAllocExtFunViaArg(const SVFFunction *fun) {
    return fun && ExtAPI::getExtAPI()->is_arg_alloc(fun);
}

/// interfaces to be used externally
inline bool isHeapAllocExtCallViaRet(const CallSite cs, SVFModule *svfMod) {
    bool isPtrTy = cs.getInstruction()->getType()->isPointerTy();
    return isPtrTy &&
           isHeapAllocExtFunViaRet(getCallee(svfMod->getLLVMModSet(), cs));
}

inline bool isHeapAllocExtCallViaRet(const Instruction *inst,
                                     SVFModule *svfMod) {
    bool isPtrTy = inst->getType()->isPointerTy();
    return isPtrTy &&
           isHeapAllocExtFunViaRet(getCallee(svfMod->getLLVMModSet(), inst));
}

inline bool isHeapAllocExtCallViaArg(const CallSite cs, SVFModule *svfMod) {
    return isHeapAllocExtFunViaArg(getCallee(svfMod->getLLVMModSet(), cs));
}

inline bool isHeapAllocExtCallViaArg(const Instruction *inst,
                                     SVFModule *svfMod) {
    return isHeapAllocExtFunViaArg(getCallee(svfMod->getLLVMModSet(), inst));
}

inline bool isHeapAllocExtCall(const CallSite cs, SVFModule *svfMod) {
    return isHeapAllocExtCallViaRet(cs, svfMod) ||
           isHeapAllocExtCallViaArg(cs, svfMod);
}

inline bool isHeapAllocExtCall(const Instruction *inst, SVFModule *svfMod) {
    return isHeapAllocExtCallViaRet(inst, svfMod) ||
           isHeapAllocExtCallViaArg(inst, svfMod);
}
//@}

/// Get the position of argument that holds an allocated heap object.
//@{
inline int getHeapAllocHoldingArgPosition(const SVFFunction *fun) {
    return ExtAPI::getExtAPI()->get_alloc_arg_pos(fun);
}

inline int getHeapAllocHoldingArgPosition(const CallSite cs,
                                          SVFModule *svfMod) {
    return getHeapAllocHoldingArgPosition(
        getCallee(svfMod->getLLVMModSet(), cs));
}

inline int getHeapAllocHoldingArgPosition(const Instruction *inst,
                                          SVFModule *svfMod) {
    return getHeapAllocHoldingArgPosition(
        getCallee(svfMod->getLLVMModSet(), inst));
}
//@}

/// Return true if the call is a heap reallocator
//@{
/// note that this function is not suppose to be used externally
inline bool isReallocExtFun(const SVFFunction *fun) {
    return fun && (ExtAPI::getExtAPI()->is_realloc(fun));
}

inline bool isReallocExtCall(const CallSite cs, SVFModule *svfMod) {
    bool isPtrTy = cs.getInstruction()->getType()->isPointerTy();
    return isPtrTy && isReallocExtFun(getCallee(svfMod->getLLVMModSet(), cs));
}

inline bool isReallocExtCall(const Instruction *inst, SVFModule *svfMod) {
    bool isPtrTy = inst->getType()->isPointerTy();
    return isPtrTy && isReallocExtFun(getCallee(svfMod->getLLVMModSet(), inst));
}
//@}

/// Return true if the call is a heap dealloc or not
//@{
/// note that this function is not suppose to be used externally
inline bool isDeallocExtFun(const SVFFunction *fun) {
    return fun && (ExtAPI::getExtAPI()->is_dealloc(fun));
}

inline bool isDeallocExtCall(const CallSite cs, SVFModule *svfMod) {
    return isDeallocExtFun(getCallee(svfMod->getLLVMModSet(), cs));
}

inline bool isDeallocExtCall(const Instruction *inst, SVFModule *svfMod) {
    return isDeallocExtFun(getCallee(svfMod->getLLVMModSet(), inst));
}
//@}

/// Return true if the call is a static global call
//@{
/// note that this function is not suppose to be used externally
inline bool isStaticExtFun(const SVFFunction *fun) {
    return fun && ExtAPI::getExtAPI()->has_static(fun);
}

inline bool isStaticExtCall(const CallSite cs, SVFModule *svfMod) {
    bool isPtrTy = cs.getInstruction()->getType()->isPointerTy();
    return isPtrTy && isStaticExtFun(getCallee(svfMod->getLLVMModSet(), cs));
}

inline bool isStaticExtCall(const Instruction *inst, SVFModule *svfMod) {
    bool isPtrTy = inst->getType()->isPointerTy();
    return isPtrTy && isStaticExtFun(getCallee(svfMod->getLLVMModSet(), inst));
}
//@}

/// Return true if the call is a static global call
//@{
inline bool isHeapAllocOrStaticExtCall(const CallSite cs, SVFModule *svfMod) {
    return isStaticExtCall(cs, svfMod) || isHeapAllocExtCall(cs, svfMod);
}

inline bool isHeapAllocOrStaticExtCall(const Instruction *inst,
                                       SVFModule *svfMod) {
    return isStaticExtCall(inst, svfMod) || isHeapAllocExtCall(inst, svfMod);
}
//@}

/// Return external call type
inline ExtAPI::extf_t extCallTy(const SVFFunction *fun) {
    return ExtAPI::getExtAPI()->get_type(fun);
}

/// Get the reference type of heap/static object from an allocation site.
//@{
inline const PointerType *getRefTypeOfHeapAllocOrStatic(const CallSite cs,
                                                        SVFModule *svfMod) {
    const PointerType *refType = nullptr;
    // Case 1: heap object held by *argument, we should get its element type.
    if (isHeapAllocExtCallViaArg(cs, svfMod)) {
        int argPos = getHeapAllocHoldingArgPosition(cs, svfMod);
        const Value *arg = cs.getArgument(argPos);
        if (const PointerType *argType =
                llvm::dyn_cast<PointerType>(arg->getType())) {
            refType = llvm::dyn_cast<PointerType>(argType->getElementType());
        }
    }
    // Case 2: heap/static object held by return value.
    else {
        assert((isStaticExtCall(cs, svfMod) ||
                isHeapAllocExtCallViaRet(cs, svfMod)) &&
               "Must be heap alloc via ret, or static allocation site");
        refType = llvm::dyn_cast<PointerType>(cs.getType());
    }
    assert(refType &&
           "Allocated object must be held by a pointer-typed value.");
    return refType;
}

inline const PointerType *getRefTypeOfHeapAllocOrStatic(const Instruction *inst,
                                                        SVFModule *svfMod) {
    CallSite cs(const_cast<Instruction *>(inst));
    return getRefTypeOfHeapAllocOrStatic(cs, svfMod);
}
//@}

/// Return true if this value refers to a object
bool isObject(const Value *ref, SVFModule *svfMod);

/// Return true if the value refers to constant data, e.g., i32 0
inline bool isConstantData(const Value *val) {
    return llvm::isa<ConstantData>(val) || llvm::isa<ConstantAggregate>(val) ||
           llvm::isa<MetadataAsValue>(val) || llvm::isa<BlockAddress>(val);
}

/// Method for dead function, which does not have any possible caller
/// function address is not taken and never be used in call or invoke
/// instruction
//@{
/// whether this is a function without any possible caller?
bool isDeadFunction(const Function *fun, SVFModule *svfMod);

/// whether this is an argument in dead function
inline bool ArgInDeadFunction(const Value *val, SVFModule *svfMod) {
    return llvm::isa<Argument>(val) &&
           isDeadFunction(llvm::cast<Argument>(val)->getParent(), svfMod);
}
//@}

/// Program entry function e.g. main
//@{
/// Return true if this is a program entry function (e.g. main)
inline bool isProgEntryFunction(const SVFFunction *fun) {
    return (fun != nullptr) && fun->getName().str() == "main";
}

inline bool isProgEntryFunction(const Function *fun) {
    return fun && fun->getName().str() == "main";
}

/// Get program entry function from module.
inline const SVFFunction *getProgEntryFunction(SVFModule *svfModule) {
    for (const auto *fun : *svfModule) {
        if (isProgEntryFunction(fun)) {
            return fun;
        }
    }

    return nullptr;
}

/// Return true if this is an argument of a program entry function (e.g. main)
inline bool ArgInProgEntryFunction(const Value *val) {
    return llvm::isa<Argument>(val) &&
           isProgEntryFunction(llvm::cast<Argument>(val)->getParent());
}
/// Return true if this is value in a dead function (function without any
/// caller)
bool isPtrInDeadFunction(const Value *value, SVFModule *svfMod);
//@}

/// Return true if this is a program exit function call
//@{
inline bool isProgExitFunction(const SVFFunction *fun) {
    return fun && (fun->getName().str() == "exit" ||
                   fun->getName().str() == "__assert_rtn" ||
                   fun->getName().str() == "__assert_fail");
}

inline bool isProgExitCall(const CallSite cs, SVFModule *svfMod) {
    return isProgExitFunction(getCallee(svfMod->getLLVMModSet(), cs));
}

inline bool isProgExitCall(const Instruction *inst, SVFModule *svfMod) {
    return isProgExitFunction(getCallee(svfMod->getLLVMModSet(), inst));
}
//@}

/// Function does not have any possible caller in the call graph
//@{
/// Return true if the function does not have a caller (either it is a main
/// function or a dead function)
inline bool isNoCallerFunction(const Function *fun, SVFModule *svfMod) {
    return isDeadFunction(fun, svfMod) || isProgEntryFunction(fun);
}

/// Return true if the argument in a function does not have a caller
inline bool ArgInNoCallerFunction(const Value *val, SVFModule *svfMod) {
    return llvm::isa<Argument>(val) &&
           isNoCallerFunction(llvm::cast<Argument>(val)->getParent(), svfMod);
}
//@}

/// Return true if the function has a return instruction reachable from function
/// entry
bool functionDoesNotRet(const Function *fun);

/// Get reachable basic block from function entry
void getFunReachableBBs(const Function *fun, DominatorTree *dt,
                        std::vector<const BasicBlock *> &bbs);

/// Get function exit basic block
/// FIXME: this back() here is only valid when UnifyFunctionExitNodes pass is
/// invoked
inline const BasicBlock *getFunExitBB(const Function *fun) {
    return &fun->back();
}
/// Strip off the constant casts
const Value *stripConstantCasts(const Value *val);

/// Strip off the all casts
Value *stripAllCasts(Value *val);

/// Get the type of the heap allocation
const Type *getTypeOfHeapAlloc(const llvm::Instruction *inst,
                               SVFModule *svfMod);

/// Return corresponding constant expression, otherwise return nullptr
//@{
inline const ConstantExpr *isGepConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if (constExpr->getOpcode() == Instruction::GetElementPtr) {
            return constExpr;
        }
    }
    return nullptr;
}

inline const ConstantExpr *isInt2PtrConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if (constExpr->getOpcode() == Instruction::IntToPtr) {
            return constExpr;
        }
    }
    return nullptr;
}

inline const ConstantExpr *isPtr2IntConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if (constExpr->getOpcode() == Instruction::PtrToInt) {
            return constExpr;
        }
    }
    return nullptr;
}

inline const ConstantExpr *isCastConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if (constExpr->getOpcode() == Instruction::BitCast) {
            return constExpr;
        }
    }
    return nullptr;
}

inline const ConstantExpr *isSelectConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if (constExpr->getOpcode() == Instruction::Select) {
            return constExpr;
        }
    }
    return nullptr;
}

inline const ConstantExpr *isTruncConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if (constExpr->getOpcode() == Instruction::Trunc ||
            constExpr->getOpcode() == Instruction::FPTrunc ||
            constExpr->getOpcode() == Instruction::ZExt ||
            constExpr->getOpcode() == Instruction::SExt ||
            constExpr->getOpcode() == Instruction::FPExt) {
            return constExpr;
        }
    }
    return nullptr;
}

inline const ConstantExpr *isCmpConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if (constExpr->getOpcode() == Instruction::ICmp ||
            constExpr->getOpcode() == Instruction::FCmp) {
            return constExpr;
        }
    }
    return nullptr;
}

inline const ConstantExpr *isBinaryConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if ((constExpr->getOpcode() >= Instruction::BinaryOpsBegin) &&
            (constExpr->getOpcode() <= Instruction::BinaryOpsEnd)) {
            return constExpr;
        }
    }
    return nullptr;
}

inline const ConstantExpr *isUnaryConstantExpr(const Value *val) {
    if (const auto *constExpr = llvm::dyn_cast<ConstantExpr>(val)) {
        if ((constExpr->getOpcode() >= Instruction::UnaryOpsBegin) &&
            (constExpr->getOpcode() <= Instruction::UnaryOpsEnd)) {
            return constExpr;
        }
    }
    return nullptr;
}
//@}

/// Get the next instructions following control flow
void getNextInsts(const Instruction *curInst,
                  std::vector<const Instruction *> &instList);

/// Get the previous instructions following control flow
void getPrevInsts(const Instruction *curInst,
                  std::vector<const Instruction *> &instList);

/// Get basic block successor position
u32_t getBBSuccessorPos(const BasicBlock *BB, const BasicBlock *Succ);
/// Get num of BB's successors
u32_t getBBSuccessorNum(const BasicBlock *BB);
/// Get basic block predecessor positin
u32_t getBBPredecessorPos(const BasicBlock *BB, const BasicBlock *Pred);
/// Get num of BB's predecessors
u32_t getBBPredecessorNum(const BasicBlock *BB);

/// Check whether a file is an LLVM IR file
bool isIRFile(const std::string &filename);

/// Parse argument for multi-module analysis
void processArguments(int argc, char **argv, int &arg_num, char **arg_value,
                      std::vector<std::string> &moduleNameVec);

} // namespace SVF::SVFUtil

#endif /* INCLUDE_SVF_FE_LLVMUTIL_H_ */
