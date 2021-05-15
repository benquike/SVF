//===- PointerAnalysis.cpp -- Base class of pointer analyses------------------//
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
 * PointerAnalysis.cpp
 *
 *  Created on: May 14, 2013
 *      Author: Yulei Sui
 *
 *  Updated by:
 *     Hui Peng <peng124@purdue.edu>
 *     2021-03-19
 */

#include "SVF-FE/CHG.h"
#include "SVF-FE/CPPUtil.h"
#include "SVF-FE/CallGraphBuilder.h"
#include "SVF-FE/DCHG.h"
#include "SVF-FE/LLVMUtil.h"
#include "Util/Options.h"
#include "Util/SVFModule.h"
#include "Util/SVFUtil.h"

#include "Graphs/ExternalPAG.h"
#include "Graphs/ICFG.h"
#include "Graphs/ThreadCallGraph.h"
#include "MemoryModel/PAGBuilderFromFile.h"
#include "MemoryModel/PTAStat.h"
#include "MemoryModel/PTAType.h"
#include "MemoryModel/PointerAnalysisImpl.h"
#include "WPA/FlowSensitiveTBHC.h"
#include <fstream>
#include <llvm/Demangle/Demangle.h>
#include <sstream>

using namespace SVF;
using namespace SVFUtil;
using namespace cppUtil;

const std::string PointerAnalysis::aliasTestMayAlias = "MAYALIAS";
const std::string PointerAnalysis::aliasTestMayAliasMangled = "_Z8MAYALIASPvS_";
const std::string PointerAnalysis::aliasTestNoAlias = "NOALIAS";
const std::string PointerAnalysis::aliasTestNoAliasMangled = "_Z7NOALIASPvS_";
const std::string PointerAnalysis::aliasTestPartialAlias = "PARTIALALIAS";
const std::string PointerAnalysis::aliasTestPartialAliasMangled =
    "_Z12PARTIALALIASPvS_";
const std::string PointerAnalysis::aliasTestMustAlias = "MUSTALIAS";
const std::string PointerAnalysis::aliasTestMustAliasMangled =
    "_Z9MUSTALIASPvS_";
const std::string PointerAnalysis::aliasTestFailMayAlias =
    "EXPECTEDFAIL_MAYALIAS";
const std::string PointerAnalysis::aliasTestFailMayAliasMangled =
    "_Z21EXPECTEDFAIL_MAYALIASPvS_";
const std::string PointerAnalysis::aliasTestFailNoAlias =
    "EXPECTEDFAIL_NOALIAS";
const std::string PointerAnalysis::aliasTestFailNoAliasMangled =
    "_Z20EXPECTEDFAIL_NOALIASPvS_";

/*!
 * Constructor
 */
PointerAnalysis::PointerAnalysis(SVFProject *proj, PTATY ty, bool alias_check,
                                 bool enableVirtualCallAnalysis,
                                 bool threadCallGraph)
    : ptaTy(ty), stat(nullptr), ptaCallGraph(nullptr), callGraphSCC(nullptr),
      typeSystem(nullptr), proj(proj), vcall_cha(enableVirtualCallAnalysis),
      threadCallGraph(threadCallGraph) {
    OnTheFlyIterBudgetForStat = Options::StatBudget;
    print_stat = Options::PStat;
    ptaImplTy = BaseImpl;
    alias_validation = (alias_check && Options::EnableAliasCheck);
}

/*!
 * Destructor
 */
PointerAnalysis::~PointerAnalysis() {
    destroy();
    // do not delete the PAG for now
    // delete pag;
}

void PointerAnalysis::destroy() {
    delete chgraph;
    chgraph = nullptr;

    delete ptaCallGraph;
    ptaCallGraph = nullptr;

    delete callGraphSCC;
    callGraphSCC = nullptr;

    // delete stat;
    // stat = nullptr;

    delete typeSystem;
    typeSystem = nullptr;
}

/*!
 *
 * Initialization of pointer analysis
 * 1. build class hierarchy graph
 * 2. build PTA callgraph
 * 3. initialize the PTACallGraphSCCDetector
 */
void PointerAnalysis::initialize() {

    auto pag = getPAG();
    assert(pag && "PAG has not been built!");

    if (chgraph == nullptr) {
        if (getSVFModule()->getLLVMModSet()->allCTir()) {
            DCHGraph *dchg = new DCHGraph(proj->getSymbolTableInfo());
            // TODO: we might want to have an option for extending.
            dchg->buildCHG(true);
            chgraph = dchg;
        } else {
            CHGraph *chg = new CHGraph(proj->getSymbolTableInfo());
            chg->buildCHG();
            chgraph = chg;
        }
    }

    // dump PAG
    if (dumpGraph()) {
        pag->dump("pag_initial");
    }

    // dump ICFG

    if (Options::DumpICFG) {
        getSVFProject()->getICFG()->dump("icfg_initial");
    }

    // print to command line of the PAG graph
    if (Options::PAGPrint) {
        pag->print();
    }

    /// initialise pta call graph
    /// for every pointer analysis instance
    bool includeThreadCall = Options::EnableThreadCallGraph || threadCallGraph;
    if (includeThreadCall) {
        ptaCallGraph = new ThreadCallGraph(proj);
    } else {
        ptaCallGraph = new PTACallGraph(proj);
    }
    /// build the callgraph with direct calls now
    /// indirect calls will be resolved in analyze method
    CallGraphBuilder bd(proj, ptaCallGraph, includeThreadCall);
    ptaCallGraph = bd.buildCallGraph();

    /// initialize the  Strong-Connected-Component
    /// module for building the callgraph
    callGraphSCCDetection();

    // dump callgraph
    if (Options::CallGraphDotGraph) {
        getPTACallGraph()->dump("callgraph_initial");
    }
}

/*!
 * Return TRUE if this node is a local variable of recursive function.
 */
bool PointerAnalysis::isLocalVarInRecursiveFun(NodeID id) const {

    const MemObj *obj = getPAG()->getObject(id);
    assert(obj && "object not found!!");
    if (obj->isStack()) {
        if (const auto *local = llvm::dyn_cast<AllocaInst>(obj->getRefVal())) {
            auto *llvmModSet = getSVFModule()->getLLVMModSet();
            const SVFFunction *fun =
                llvmModSet->getSVFFunction(local->getFunction());
            return callGraphSCC->isInCycle(
                getPTACallGraph()->getCallGraphNode(fun)->getId());
        }
    }
    return false;
}

/*!
 * Reset field sensitivity
 */
void PointerAnalysis::resetObjFieldSensitive() {
    auto pag = getPAG();
    for (auto &nIter : *pag) {
        if (auto *node = llvm::dyn_cast<ObjPN>(nIter.second)) {
            const_cast<MemObj *>(node->getMemObj())->setFieldSensitive();
        }
    }
}

/*!
 * Flag in order to dump graph
 */
bool PointerAnalysis::dumpGraph() { return Options::PAGDotGraph; }

/*
 * Dump statistics
 */

void PointerAnalysis::dumpStat() {

    if (print_stat && stat) {
        stat->performStat();
    }
}

/*!
 * Finalize the analysis after solving
 * Given the alias results, verify whether it is correct or not using alias
 * check functions
 */
void PointerAnalysis::finalize() {

    auto pag = getPAG();

    /// Print statistics
    dumpStat();

    // dump PAG
    if (dumpGraph()) {
        pag->dump("pag_final");
    }

    // dump ICFG
    if (Options::DumpICFG) {
        pag->getICFG()->updateCallGraph(ptaCallGraph);
        pag->getICFG()->dump("icfg_final");
    }

    if (!DumpPAGFunctions.empty()) {
        pag->dumpFunctions(DumpPAGFunctions);
    }

    /// Dump results
    if (Options::PTSPrint) {
        dumpTopLevelPtsTo();
        // dumpAllPts();
        // dumpCPts();
    }

    if (Options::TypePrint) {
        dumpAllTypes();
    }

    if (Options::PTSAllPrint) {
        dumpAllPts();
    }

    if (Options::FuncPointerPrint) {
        printIndCSTargets();
    }

    getPTACallGraph()->verifyCallGraph();

    if (Options::CallGraphDotGraph) {
        getPTACallGraph()->dump("callgraph_final");
    }

    // FSTBHC has its own TBHC-specific test validation.
    if (!pag->isBuiltFromFile() && alias_validation &&
        !llvm::isa<FlowSensitiveTBHC>(this)) {
        validateTests();
    }

    if (!Options::UsePreCompFieldSensitive) {
        resetObjFieldSensitive();
    }
}

/*!
 * Validate test cases
 */
void PointerAnalysis::validateTests() {
    validateSuccessTests(aliasTestMayAlias);
    validateSuccessTests(aliasTestNoAlias);
    validateSuccessTests(aliasTestMustAlias);
    validateSuccessTests(aliasTestPartialAlias);
    validateExpectedFailureTests(aliasTestFailMayAlias);
    validateExpectedFailureTests(aliasTestFailNoAlias);

    validateSuccessTests(aliasTestMayAliasMangled);
    validateSuccessTests(aliasTestNoAliasMangled);
    validateSuccessTests(aliasTestMustAliasMangled);
    validateSuccessTests(aliasTestPartialAliasMangled);
    validateExpectedFailureTests(aliasTestFailMayAliasMangled);
    validateExpectedFailureTests(aliasTestFailNoAliasMangled);
}

void PointerAnalysis::dumpAllTypes() {
    for (auto nIter : this->getAllValidPtrs()) {
        const PAGNode *node = getPAG()->getGNode(nIter);
        if (llvm::isa<DummyObjPN>(node) || llvm::isa<DummyValPN>(node)) {
            continue;
        }

        outs() << "##<" << node->getValue()->getName() << "> ";
        outs() << "Source Loc: " << getSourceLoc(node->getValue());
        outs() << "\nNodeID " << node->getId() << "\n";

        Type *type = node->getValue()->getType();
        SymbolTableInfo *symbolTableInfo = getPAG()->getSymbolTableInfo();
        symbolTableInfo->printFlattenFields(type);
        if (auto *ptType = llvm::dyn_cast<PointerType>(type)) {
            symbolTableInfo->printFlattenFields(ptType->getElementType());
        }
    }
}

/*!
 * Dump points-to of top-level pointers (ValPN)
 */
void PointerAnalysis::dumpPts(NodeID ptr, const PointsTo &pts) {

    auto pag = getPAG();

    const PAGNode *node = pag->getGNode(ptr);
    /// print the points-to set of node which has the maximum pts size.
    if (llvm::isa<DummyObjPN>(node)) {
        outs() << "##<Dummy Obj > id:" << node->getId();
    } else if (!llvm::isa<DummyValPN>(node) && !SVFModule::pagReadFromTXT()) {
        if (node->hasValue()) {
            outs() << "##<" << node->getValue()->getName() << "> ";
            outs() << "Source Loc: " << getSourceLoc(node->getValue());
        }
    }
    outs() << "\nPtr " << node->getId() << " ";

    if (pts.empty()) {
        outs() << "\t\tPointsTo: {empty}\n\n";
    } else {
        outs() << "\t\tPointsTo: { ";
        for (auto it : pts) {
            outs() << it << " ";
        }
        outs() << "}\n\n";
    }

    outs() << "";

    for (auto it : pts) {
        const PAGNode *node = pag->getGNode(it);
        if (llvm::isa<ObjPN>(node) == false) {
            continue;
        }
        NodeID ptd = node->getId();
        outs() << "!!Target NodeID " << ptd << "\t [";
        const PAGNode *pagNode = pag->getGNode(ptd);
        if (llvm::isa<DummyValPN>(node)) {
            outs() << "DummyVal\n";
        } else if (llvm::isa<DummyObjPN>(node)) {
            outs() << "Dummy Obj id: " << node->getId() << "]\n";
        } else {
            if (!SVFModule::pagReadFromTXT()) {
                if (node->hasValue()) {
                    outs() << "<" << pagNode->getValue()->getName() << "> ";
                    outs() << "Source Loc: "
                           << getSourceLoc(pagNode->getValue()) << "] \n";
                }
            }
        }
    }
}

/*!
 * Print indirect call targets at an indirect callsite
 */
void PointerAnalysis::printIndCSTargets(const CallBlockNode *cs,
                                        const FunctionSet &targets) {
    outs() << "\nNodeID: " << getFunPtr(cs);
    outs() << "\nCallSite: ";
    cs->getCallSite()->print(outs());
    outs() << "\tLocation: " << SVFUtil::getSourceLoc(cs->getCallSite());
    outs() << "\t with Targets: ";

    if (!targets.empty()) {
        for (const auto *fit : targets) {
            const SVFFunction *callee = fit;
            outs() << "\n\t" << callee->getName();
        }
    } else {
        outs() << "\n\tNo Targets!";
    }

    outs() << "\n";
}

/*!
 * Print all indirect callsites
 */
void PointerAnalysis::printIndCSTargets() {
    outs() << "==================Function Pointer Targets==================\n";
    const CallEdgeMap &callEdges = getIndCallMap();

    for (const auto &it : callEdges) {
        const CallBlockNode *cs = it.first;
        const FunctionSet &targets = it.second;
        printIndCSTargets(cs, targets);
    }

    const CallSiteToFunPtrMap &indCS = getIndirectCallsites();
    for (const auto &csIt : indCS) {
        const CallBlockNode *cs = csIt.first;
        if (hasIndCSCallees(cs) == false) {
            outs() << "\nNodeID: " << csIt.second;
            outs() << "\nCallSite: ";
            cs->getCallSite()->print(outs());
            outs() << "\tLocation: "
                   << SVFUtil::getSourceLoc(cs->getCallSite());
            outs() << "\n\t!!!has no targets!!!\n";
        }
    }
}

/*!
 * Resolve indirect calls
 */
void PointerAnalysis::resolveIndCalls(const CallBlockNode *cs,
                                      const PointsTo &target,
                                      CallEdgeMap &newEdges, LLVMCallGraph *) {
    auto pag = getPAG();
    auto svfMod = getSVFModule();
    assert(pag->isIndirectCallSites(cs) && "not an indirect callsite?");
    /// discover indirect pointer target
    for (const auto &ii : target) {

        if (getNumOfResolvedIndCallEdge() >= Options::IndirectCallLimit) {
            wrnMsg("Resolved Indirect Call Edges are Out-Of-Budget, please "
                   "increase the limit");
            return;
        }

        if (auto *objPN = llvm::dyn_cast<ObjPN>(pag->getGNode(ii))) {
            const MemObj *obj = pag->getObject(objPN);

            if (obj->isFunction()) {
                const auto *calleefun = llvm::cast<Function>(obj->getRefVal());
                const SVFFunction *callee = getDefFunForMultipleModule(
                    svfMod->getLLVMModSet(), calleefun);

                /// if the arg size does not match then we do not need to
                /// connect this parameter even if the callee is a variadic
                /// function (the first parameter of variadic function is its
                /// paramter number)
                if (matchArgs(cs, callee) == false) {
                    continue;
                }

                if (0 == getIndCallMap()[cs].count(callee)) {
                    newEdges[cs].insert(callee);
                    getIndCallMap()[cs].insert(callee);

                    ptaCallGraph->addIndirectCallGraphEdge(cs, cs->getCaller(),
                                                           callee);
                    // FIXME: do we need to update llvm call graph here?
                    // The indirect call is maintained by ourself, We may update
                    // llvm's when we need to
                    // CallGraphNode* callgraphNode =
                    // callgraph->getOrInsertFunction(cs.getCaller());
                    // callgraphNode->addCalledFunction(cs,callgraph->getOrInsertFunction(callee));
                }
            }
        }
    }
}

/*!
 * Match arguments for callsite at caller and callee
 */
bool PointerAnalysis::matchArgs(const CallBlockNode *cs,
                                const SVFFunction *callee) {
    if (proj->getThreadAPI()->isTDFork(cs->getCallSite())) {
        return true;
    }

    return SVFUtil::getLLVMCallSite(cs->getCallSite()).arg_size() ==
           callee->arg_size();
}

/*
 * Get virtual functions "vfns" based on CHA
 */
void PointerAnalysis::getVFnsFromCHA(const CallBlockNode *cs, VFunSet &vfns) {
    CallSite llvmCS = SVFUtil::getLLVMCallSite(cs->getCallSite());
    if (chgraph->csHasVFnsBasedonCHA(llvmCS)) {
        vfns = chgraph->getCSVFsBasedonCHA(llvmCS);
    }
}

/*
 * Get virtual functions "vfns" from PoninsTo set "target" for callsite "cs"
 */
void PointerAnalysis::getVFnsFromPts(const CallBlockNode *cs,
                                     const PointsTo &target, VFunSet &vfns) {

    PAG *pag = proj->getPAG();
    CallSite llvmCS = SVFUtil::getLLVMCallSite(cs->getCallSite());
    if (chgraph->csHasVtblsBasedonCHA(llvmCS)) {
        Set<const GlobalValue *> vtbls;
        const VTableSet &chaVtbls = chgraph->getCSVtblsBasedonCHA(llvmCS);
        for (auto it : target) {
            const PAGNode *ptdnode = pag->getGNode(it);
            if (ptdnode->hasValue()) {
                if (const auto *vtbl =
                        llvm::dyn_cast<GlobalValue>(ptdnode->getValue())) {
                    if (chaVtbls.find(vtbl) != chaVtbls.end()) {
                        vtbls.insert(vtbl);
                    }
                }
            }
        }
        chgraph->getVFnsFromVtbls(SVFUtil::getLLVMCallSite(cs->getCallSite()),
                                  vtbls, vfns);
    }
}

/*
 * Connect callsite "cs" to virtual functions in "vfns"
 */
void PointerAnalysis::connectVCallToVFns(const CallBlockNode *cs,
                                         const VFunSet &vfns,
                                         CallEdgeMap &newEdges) {
    //// connect all valid functions
    LLVMModuleSet *modSet = getSVFModule()->getLLVMModSet();
    for (const auto *callee : vfns) {
        callee = getDefFunForMultipleModule(modSet, callee->getLLVMFun());
        if (getIndCallMap()[cs].count(callee) > 0) {
            continue;
        }

        CallSite llvmCS = SVFUtil::getLLVMCallSite(cs->getCallSite());

        // here we only check the number of args
        if (llvmCS.arg_size() == callee->arg_size() ||
            (llvmCS.getFunctionType()->isVarArg() && callee->isVarArg())) {

            // save callee to the output variable newEdges
            newEdges[cs].insert(callee);

            // insert the callee to the internal map
            getIndCallMap()[cs].insert(callee);

            // update the CallGraph
            ptaCallGraph->addIndirectCallGraphEdge(cs, cs->getCaller(), callee);
        }
    }
}

/// Resolve cpp indirect call edges
void PointerAnalysis::resolveCPPIndCalls(const CallBlockNode *cs,
                                         const PointsTo &target,
                                         CallEdgeMap &newEdges) {
    assert(isVirtualCallSite(SVFUtil::getLLVMCallSite(cs->getCallSite()),
                             getPAG()->getModule()->getLLVMModSet()) &&
           "not cpp virtual call");

    VFunSet vfns;
    if (Options::ConnectVCallOnCHA || vcall_cha) {
        getVFnsFromCHA(cs, vfns);
    } else {
        getVFnsFromPts(cs, target, vfns);
    }

    // update the intenral map and (pta) callgraph
    connectVCallToVFns(cs, vfns, newEdges);
}

/*!
 * Find the alias check functions annotated in the C files
 * check whether the alias analysis results consistent with the alias check
 * function itself
 */
void PointerAnalysis::validateSuccessTests(std::string fun) {

    // check for must alias cases, whether our alias analysis produce the
    // correct results
    LLVMModuleSet *modSet = getSVFModule()->getLLVMModSet();
    if (const SVFFunction *checkFun = getFunction(modSet, fun)) {
        if (!checkFun->getLLVMFun()->use_empty()) {
            outs() << "[" << this->PTAName() << "] Checking " << fun << "\n";
        }

        for (Value::user_iterator i = checkFun->getLLVMFun()->user_begin(),
                                  e = checkFun->getLLVMFun()->user_end();
             i != e; ++i) {
            if (SVFUtil::isCallSite(*i)) {

                CallSite cs(*i);
                assert(cs.getNumArgOperands() == 2 &&
                       "arguments should be two pointers!!");
                Value *V1 = cs.getArgOperand(0);
                Value *V2 = cs.getArgOperand(1);
                AliasResult aliasRes = alias(V1, V2);

                bool checkSuccessful = false;
                if (fun == aliasTestMayAlias ||
                    fun == aliasTestMayAliasMangled) {
                    if (aliasRes == llvm::MayAlias ||
                        aliasRes == llvm::MustAlias) {
                        checkSuccessful = true;
                    }
                } else if (fun == aliasTestNoAlias ||
                           fun == aliasTestNoAliasMangled) {
                    if (aliasRes == llvm::NoAlias) {
                        checkSuccessful = true;
                    }
                } else if (fun == aliasTestMustAlias ||
                           fun == aliasTestMustAliasMangled) {
                    // change to must alias when our analysis support it
                    if (aliasRes == llvm::MayAlias ||
                        aliasRes == llvm::MustAlias) {
                        checkSuccessful = true;
                    }
                } else if (fun == aliasTestPartialAlias ||
                           fun == aliasTestPartialAliasMangled) {
                    // change to partial alias when our analysis support it
                    if (aliasRes == llvm::MayAlias) {
                        checkSuccessful = true;
                    }
                } else {
                    assert(false && "not supported alias check!!");
                }

                NodeID id1 = getPAG()->getValueNode(V1);
                NodeID id2 = getPAG()->getValueNode(V2);

                if (checkSuccessful) {
                    outs() << sucMsg("\t SUCCESS :") << fun
                           << " check <id:" << id1 << ", id:" << id2 << "> at ("
                           << getSourceLoc(*i) << ")\n";
                } else {
                    SVFUtil::errs() << errMsg("\t FAILURE :") << fun
                                    << " check <id:" << id1 << ", id:" << id2
                                    << "> at (" << getSourceLoc(*i) << ")\n";
                    assert(false && "test case failed!");
                }
            } else {
                assert(false &&
                       "alias check functions not only used at callsite??");
            }
        }
    }
}

/*!
 * Pointer analysis validator
 */
void PointerAnalysis::validateExpectedFailureTests(std::string fun) {

    LLVMModuleSet *modSet = getSVFModule()->getLLVMModSet();
    if (const SVFFunction *checkFun = getFunction(modSet, fun)) {
        if (!checkFun->getLLVMFun()->use_empty()) {
            outs() << "[" << this->PTAName() << "] Checking " << fun << "\n";
        }

        for (Value::user_iterator i = checkFun->getLLVMFun()->user_begin(),
                                  e = checkFun->getLLVMFun()->user_end();
             i != e; ++i) {
            if (CallInst *call = llvm::dyn_cast<CallInst>(*i)) {
                assert(call->getNumArgOperands() == 2 &&
                       "arguments should be two pointers!!");
                Value *V1 = call->getArgOperand(0);
                Value *V2 = call->getArgOperand(1);
                AliasResult aliasRes = alias(V1, V2);

                bool expectedFailure = false;
                if (fun == aliasTestFailMayAlias ||
                    fun == aliasTestFailMayAliasMangled) {
                    // change to must alias when our analysis support it
                    if (aliasRes == llvm::NoAlias) {
                        expectedFailure = true;
                    }
                } else if (fun == aliasTestFailNoAlias ||
                           fun == aliasTestFailNoAliasMangled) {
                    // change to partial alias when our analysis support it
                    if (aliasRes == llvm::MayAlias ||
                        aliasRes == llvm::PartialAlias ||
                        aliasRes == llvm::MustAlias) {
                        expectedFailure = true;
                    }
                } else {
                    assert(false && "not supported alias check!!");
                }

                NodeID id1 = getPAG()->getValueNode(V1);
                NodeID id2 = getPAG()->getValueNode(V2);

                if (expectedFailure) {
                    outs() << sucMsg("\t EXPECTED-FAILURE :") << fun
                           << " check <id:" << id1 << ", id:" << id2 << "> at ("
                           << getSourceLoc(call) << ")\n";
                } else {
                    SVFUtil::errs() << errMsg("\t UNEXPECTED FAILURE :") << fun
                                    << " check <id:" << id1 << ", id:" << id2
                                    << "> at (" << getSourceLoc(call) << ")\n";
                    assert(false && "test case failed!");
                }
            } else {
                assert(false &&
                       "alias check functions not only used at callsite??");
            }
        }
    }
}
