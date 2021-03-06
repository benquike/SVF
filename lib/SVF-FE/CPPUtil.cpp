//===- CPPUtil.cpp -- Base class of pointer analyses -------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2017>  <Yulei Sui>
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
 * CPPUtil.cpp
 *
 *  Created on: Apr 13, 2016
 *      Author: Xiaokang Fan
 */

#include "SVF-FE/CPPUtil.h"
#include "SVF-FE/LLVMUtil.h"
#include "Util/SVFUtil.h"

#include <cxxabi.h> // for demangling

using namespace std;
using namespace SVF;

// label for global vtbl value before demangle
const string vtblLabelBeforeDemangle = "_ZTV";

// label for global vtbl value before demangle
const string vtblLabelAfterDemangle = "vtable for ";

// label for virtual functions
const string vfunPreLabel = "_Z";

// label for multi inheritance virtual function
const string NVThunkFunLabel = "non-virtual thunk to ";
const string VThunkFuncLabel = "virtual thunk to ";

const string clsName = "class.";
const string structName = "struct.";

static bool isOperOverload(const string &name) {
    s32_t leftnum = 0, rightnum = 0;
    string subname = name;
    size_t leftpos, rightpos;
    leftpos = subname.find("<");
    while (leftpos != string::npos) {
        subname = subname.substr(leftpos + 1);
        leftpos = subname.find("<");
        leftnum++;
    }
    subname = name;
    rightpos = subname.find(">");
    while (rightpos != string::npos) {
        subname = subname.substr(rightpos + 1);
        rightpos = subname.find(">");
        rightnum++;
    }
    if (leftnum != rightnum) {
        return true;
    }
    return false;
}

static string getBeforeParenthesis(const string &name) {
    size_t lastRightParen = name.rfind(")");
    assert(lastRightParen > 0);

    s32_t paren_num = 1, pos;
    for (pos = lastRightParen - 1; pos >= 0; pos--) {
        if (name[pos] == ')') {
            paren_num++;
        }
        if (name[pos] == '(') {
            paren_num--;
        }
        if (paren_num == 0) {
            break;
        }
    }
    return name.substr(0, pos);
}

string cppUtil::getBeforeBrackets(const string &name) {
    if (name[name.size() - 1] != '>') {
        return name;
    }
    s32_t bracket_num = 1, pos;
    for (pos = name.size() - 2; pos >= 0; pos--) {
        if (name[pos] == '>') {
            bracket_num++;
        }
        if (name[pos] == '<') {
            bracket_num--;
        }
        if (bracket_num == 0) {
            break;
        }
    }
    return name.substr(0, pos);
}

bool cppUtil::isValVtbl(const Value *val) {
    if (!llvm::isa<GlobalVariable>(val)) {
        return false;
    }
    string valName = val->getName().str();
    if (valName.compare(0, vtblLabelBeforeDemangle.size(),
                        vtblLabelBeforeDemangle) == 0) {
        return true;
    }
    return false;
}

static void handleThunkFunction(cppUtil::DemangledName &dname) {
    // when handling multi-inheritance,
    // the compiler may generate thunk functions
    // to perform `this` pointer adjustment
    // they are indicated with `virtual thunk to `
    // and `nun-virtual thunk to`.
    // if the classname starts with part of a
    // demangled name starts with
    // these prefixes, we need to remove the prefix
    // to get the real class name

    static vector<string> thunkPrefixes = {VThunkFuncLabel, NVThunkFunLabel};
    for (const auto &prefix : thunkPrefixes) {
        if (dname.className.size() > prefix.size() &&
            dname.className.compare(0, prefix.size(), prefix) == 0) {
            dname.className = dname.className.substr(prefix.size());
            dname.isThunkFunc = true;
            return;
        }
    }
}

/*
 * input: _ZN****
 * after abi::__cxa_demangle:
 * namespace::A<...::...>::f<...::...>(...)
 *                       ^
 *                    delimiter
 *
 * step1: getBeforeParenthesis
 * namespace::A<...::...>::f<...::...>
 *
 * step2: getBeforeBrackets
 * namespace::A<...::...>::f
 *
 * step3: find delimiter
 * namespace::A<...::...>::
 *                       ^
 *
 * className: namespace::A<...::...>
 * functionName: f<...::...>
 */

struct cppUtil::DemangledName cppUtil::demangle(const string &name) {
    struct cppUtil::DemangledName dname;
    dname.isThunkFunc = false;

    s32_t status;
    char *realname = abi::__cxa_demangle(name.c_str(), 0, 0, &status);
    if (realname == nullptr) {
        dname.className = "";
        dname.funcName = "";
    } else {
        string realnameStr = string(realname);
        string beforeParenthesis = getBeforeParenthesis(realnameStr);
        if (beforeParenthesis.find("::") == string::npos ||
            isOperOverload(beforeParenthesis)) {
            dname.className = "";
            dname.funcName = "";
        } else {
            string beforeBracket = getBeforeBrackets(beforeParenthesis);
            size_t colon = beforeBracket.rfind("::");
            if (colon == string::npos) {
                dname.className = "";
                dname.funcName = "";
            } else {
                dname.className = beforeParenthesis.substr(0, colon);
                dname.funcName = beforeParenthesis.substr(colon + 2);
            }
        }
    }

    handleThunkFunction(dname);

    return dname;
}

bool cppUtil::isLoadVtblInst(const LoadInst *loadInst) {
    const Value *loadSrc = loadInst->getPointerOperand();
    const Type *valTy = loadSrc->getType();
    const Type *elemTy = valTy;
    for (s32_t i = 0; i < 3; ++i) {
        if (const auto *ptrTy = llvm::dyn_cast<PointerType>(elemTy)) {
            elemTy = ptrTy->getElementType();
        } else {
            return false;
        }
    }
    if (const auto *functy = llvm::dyn_cast<FunctionType>(elemTy)) {
        const Type *paramty = functy->getParamType(0);
        string className = cppUtil::getClassNameFromType(paramty);
        if (!className.empty()) {
            return true;
        }
    }
    return false;
}

/*
 * a virtual callsite follows the following instruction sequence pattern:
 * %vtable = load this
 * %vfn = getelementptr %vtable, idx
 * %x = load %vfn
 * call %x (this)
 */
bool cppUtil::isVirtualCallSite(CallSite cs, LLVMModuleSet *modSet) {
    // the callsite must be an indirect one with at least one argument (this
    // ptr)
    if (cs.getCalledFunction() != nullptr || cs.arg_empty()) {
        return false;
    }

    // When compiled with ctir, we'd be using the DCHG which has its own
    // virtual annotations.
    if (modSet && modSet->allCTir()) {
        return cs.getInstruction()->getMetadata(cppUtil::ctir::derefMDName) !=
               nullptr;
    }

    const Value *vfunc = cs.getCalledValue();
    if (const auto *vfuncloadinst = llvm::dyn_cast<LoadInst>(vfunc)) {
        const Value *vfuncptr = vfuncloadinst->getPointerOperand();
        if (const auto *vfuncptrgepinst =
                llvm::dyn_cast<GetElementPtrInst>(vfuncptr)) {
            if (vfuncptrgepinst->getNumIndices() != 1) {
                return false;
            }
            const Value *vtbl = vfuncptrgepinst->getPointerOperand();
            if (llvm::isa<LoadInst>(vtbl)) {
                return true;
            }
        }
    }
    return false;
}

bool cppUtil::isCPPThunkFunction(const Function *F) {
    cppUtil::DemangledName dname = cppUtil::demangle(F->getName());
    return dname.isThunkFunc;
}

const Function *cppUtil::getThunkTarget(const Function *F) {
    const Function *ret = nullptr;

    assert(F != nullptr && "F should not be nullptr");

    if (F->isDeclaration()) {
        return F;
    }

    for (const auto &bb : *F) {
        for (const auto &inst : bb) {
            if (llvm::isa<CallInst>(inst) || llvm::isa<InvokeInst>(inst) ||
                llvm::isa<CallBrInst>(inst)) {
                llvm::ImmutableCallSite cs(&inst);
                assert(ret == nullptr && "multiple callsites in thunk func");

                const auto *v = cs.getCalledValue();
                if (const auto *alias = llvm::dyn_cast<llvm::GlobalAlias>(v)) {
                    v = alias->getAliasee();
                }

                if (auto *CE = llvm::dyn_cast<llvm::ConstantExpr>(v)) {
                    auto *AsI = CE->getAsInstruction();
                    if (auto *bc = llvm::dyn_cast<llvm::BitCastInst>(AsI)) {
                        v = bc->getOperand(0);
                    }
                    AsI->deleteValue();
                }

                assert(llvm::isa<llvm::Function>(v) &&
                       "Indirect call detected in thunk function");

                ret = llvm::dyn_cast<llvm::Function>(v);
            }
        }
    }

    assert(ret != nullptr && "no callsites detected in thunk function");
    return ret;
}

const Value *cppUtil::getVCallThisPtr(CallSite cs) {
    if (cs.paramHasAttr(0, llvm::Attribute::StructRet)) {
        return cs.getArgument(1);
    }

    return cs.getArgument(0);
}

/*!
 * Given a inheritance relation B is a child of A
 * We assume B::B(thisPtr1){ A::A(thisPtr2) } such that thisPtr1 == thisPtr2
 * In the following code thisPtr1 is "%class.B1* %this" and thisPtr2 is
"%class.A* %0".
 *
define linkonce_odr dso_local void @B1::B1()(%class.B1* %this) unnamed_addr #6
comdat %this.addr = alloca %class.B1*, align 8 store %class.B1* %this,
%class.B1** %this.addr, align 8 %this1 = load %class.B1*, %class.B1**
%this.addr, align 8 %0 = bitcast %class.B1* %this1 to %class.A* call void
@A::A()(%class.A* %0)
 */
bool cppUtil::isSameThisPtrInConstructor(const Argument *thisPtr1,
                                         const Value *thisPtr2) {
    if (thisPtr1 == thisPtr2) {
        return true;
    }

    for (const User *thisU : thisPtr1->users()) {
        if (const auto *store = llvm::dyn_cast<StoreInst>(thisU)) {
            for (const User *storeU : store->getPointerOperand()->users()) {
                if (const auto *load = llvm::dyn_cast<LoadInst>(storeU)) {
                    if (load->getNextNode() &&
                        llvm::isa<CastInst>(load->getNextNode())) {
                        return llvm::cast<CastInst>(load->getNextNode()) ==
                               (thisPtr2->stripPointerCasts());
                    }
                }
            }
        }
    }
    return false;
}

const Argument *cppUtil::getConstructorThisPtr(const Function *fun) {
    assert((isConstructor(fun) || isDestructor(fun)) && "not a constructor?");
    assert(fun->arg_size() >= 1 && "argument size >= 1?");
    const Argument *thisPtr = &*(fun->arg_begin());
    return thisPtr;
}

/*
 * get the ptr "vtable" for a given virtual callsite:
 * %vtable = load ...
 * %vfn = getelementptr %vtable, idx
 * %x = load %vfn
 * call %x (...)
 */
const Value *cppUtil::getVCallVtblPtr(CallSite cs) {
    const LoadInst *loadInst = llvm::dyn_cast<LoadInst>(cs.getCalledValue());
    assert(loadInst != nullptr);
    const Value *vfuncptr = loadInst->getPointerOperand();
    const auto *gepInst = llvm::dyn_cast<GetElementPtrInst>(vfuncptr);
    assert(gepInst != nullptr);
    const Value *vtbl = gepInst->getPointerOperand();
    return vtbl;
}

u64_t cppUtil::getVCallIdx(CallSite cs) {
    const LoadInst *vfuncloadinst =
        llvm::dyn_cast<LoadInst>(cs.getCalledValue());
    assert(vfuncloadinst != nullptr);
    const Value *vfuncptr = vfuncloadinst->getPointerOperand();
    const auto *vfuncptrgepinst = llvm::dyn_cast<GetElementPtrInst>(vfuncptr);
    User::const_op_iterator oi = vfuncptrgepinst->idx_begin();
    const auto *idx = llvm::dyn_cast<ConstantInt>(&(*oi));
    u64_t idx_value;
    if (idx == nullptr) {
        SVFUtil::errs() << "vcall gep idx not constantint\n";
        idx_value = 0;
    } else {
        idx_value = idx->getSExtValue();
    }
    return idx_value;
}

string cppUtil::getClassNameFromType(const Type *ty) {
    string className = "";
    if (const auto *ptrType = llvm::dyn_cast<PointerType>(ty)) {
        const Type *elemType = ptrType->getElementType();
        if (llvm::isa<StructType>(elemType) &&
            !((llvm::cast<StructType>(elemType))->isLiteral())) {
            string elemTypeName = elemType->getStructName().str();
            if (elemTypeName.compare(0, clsName.size(), clsName) == 0) {
                className = elemTypeName.substr(clsName.size());
            } else if (elemTypeName.compare(0, structName.size(), structName) ==
                       0) {
                className = elemTypeName.substr(structName.size());
            }
        }
    }
    return className;
}

string cppUtil::getClassNameFromVtblObj(const Value *value) {
    string className = "";

    string vtblName = value->getName().str();
    s32_t status;
    char *realname = abi::__cxa_demangle(vtblName.c_str(), 0, 0, &status);
    if (realname != nullptr) {
        string realnameStr = string(realname);
        if (realnameStr.compare(0, vtblLabelAfterDemangle.size(),
                                vtblLabelAfterDemangle) == 0) {
            className = realnameStr.substr(vtblLabelAfterDemangle.size());
        }
    }
    return className;
}

bool cppUtil::isConstructor(const Function *F) {
    if (F->isDeclaration()) {
        return false;
    }
    string funcName = F->getName().str();
    if (funcName.compare(0, vfunPreLabel.size(), vfunPreLabel) != 0) {
        return false;
    }
    struct cppUtil::DemangledName dname = demangle(funcName.c_str());
    dname.funcName = getBeforeBrackets(dname.funcName);
    dname.className = getBeforeBrackets(dname.className);
    size_t colon = dname.className.rfind("::");
    if (colon == string::npos) {
        dname.className = getBeforeBrackets(dname.className);
    } else {
        dname.className = getBeforeBrackets(dname.className.substr(colon + 2));
    }
    if (!dname.className.empty() && (dname.className == dname.funcName)) {
        /// TODO: on mac os function name is an empty string after demangling
        return true;
    }

    return false;
}

bool cppUtil::isDestructor(const Function *F) {
    if (F->isDeclaration()) {
        return false;
    }
    string funcName = F->getName().str();
    if (funcName.compare(0, vfunPreLabel.size(), vfunPreLabel) != 0) {
        return false;
    }
    struct cppUtil::DemangledName dname = demangle(funcName.c_str());
    dname.funcName = getBeforeBrackets(dname.funcName);
    dname.className = getBeforeBrackets(dname.className);
    size_t colon = dname.className.rfind("::");
    if (colon == string::npos) {
        dname.className = getBeforeBrackets(dname.className);
    } else {
        dname.className = getBeforeBrackets(dname.className.substr(colon + 2));
    }
    if (!dname.className.empty() && !dname.funcName.empty() &&
        dname.className.size() + 1 == dname.funcName.size() &&
        dname.funcName.compare(0, 1, "~") == 0 &&
        dname.className == dname.funcName.substr(1)) {
        return true;
    }

    return false;
}

string cppUtil::getClassNameOfThisPtr(CallSite cs) {
    string thisPtrClassName;
    Instruction *inst = cs.getInstruction();
    if (const MDNode *N = inst->getMetadata("VCallPtrType")) {
        const auto *mdstr = llvm::cast<MDString>(N->getOperand(0).get());
        thisPtrClassName = mdstr->getString().str();
    }
    if (thisPtrClassName.empty()) {
        const Value *thisPtr = getVCallThisPtr(cs);
        thisPtrClassName = getClassNameFromType(thisPtr->getType());
    }

    size_t found = thisPtrClassName.find_last_not_of("0123456789");
    if (found != string::npos) {
        if (found != thisPtrClassName.size() - 1 &&
            thisPtrClassName[found] == '.') {
            return thisPtrClassName.substr(0, found);
        }
    }

    return thisPtrClassName;
}

string cppUtil::getFunNameOfVCallSite(CallSite cs) {
    string funName;
    Instruction *inst = cs.getInstruction();
    if (const MDNode *N = inst->getMetadata("VCallFunName")) {
        const auto *mdstr = llvm::cast<MDString>(N->getOperand(0).get());
        funName = mdstr->getString().str();
    }
    return funName;
}

/*
 * Is this virtual call inside its own constructor or destructor?
 */
bool cppUtil::VCallInCtorOrDtor(CallSite cs) {
    std::string classNameOfThisPtr = getClassNameOfThisPtr(cs);
    const Function *func = cs.getCaller();
    if (isConstructor(func) || isDestructor(func)) {
        struct DemangledName dname = demangle(func->getName().str());
        if (classNameOfThisPtr == dname.className) {
            return true;
        }
    }
    return false;
}
