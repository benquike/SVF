//===- ThreadCallGraph.cpp -- Call graph considering thread fork/join---------//
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
 * ThreadCallGraph.cpp
 *
 *  Created on: Jul 12, 2014
 *      Author: Yulei Sui, Peng Di, Ding Ye
 */

#include "Graphs/ThreadCallGraph.h"
#include "Util/SVFModule.h"
#include "Util/ThreadAPI.h"

using namespace SVF;
using namespace SVFUtil;

/*!
 * Constructor
 */
ThreadCallGraph::ThreadCallGraph(SVFProject *proj)
    : PTACallGraph(proj, ThdCallGraph), tdAPI(proj->getThreadAPI()) {
    DBOUT(DGENERAL, outs() << SVFUtil::pasMsg("Building ThreadCallGraph\n"));
}

/*
 * Update call graph using pointer analysis results
 * (1) resolve function pointers for non-fork calls
 * (2) resolve function pointers for fork sites
 * (3) resolve function pointers for parallel_for sites
 */
void ThreadCallGraph::updateCallGraph(PointerAnalysis *pta) {
    for (const auto &iter : pta->getIndCallMap()) {
        const CallBlockNode *cs = iter.first;
        const PTACallGraph::FunctionSet &functions = iter.second;
        for (const auto *callee : functions) {
            this->addIndirectCallGraphEdge(cs, cs->getCaller(), callee);
        }
    }

    // Fork sites
    for (auto it = forksitesBegin(), eit = forksitesEnd(); it != eit; ++it) {
        const Value *forkedval = tdAPI->getForkedFun((*it)->getCallSite());
        if (llvm::dyn_cast<Function>(forkedval) == nullptr) {
            PAG *pag = pta->getPAG();
            const PointsTo &targets = pta->getPts(pag->getValueNode(forkedval));
            for (auto ii : targets) {
                if (auto *objPN = llvm::dyn_cast<ObjPN>(pag->getGNode(ii))) {
                    const MemObj *obj = pag->getObject(objPN);
                    if (obj->isFunction()) {
                        const auto *callee =
                            llvm::cast<Function>(obj->getRefVal());
                        LLVMModuleSet *modSet =
                            getPAG()->getModule()->getLLVMModSet();
                        const SVFFunction *svfCallee =
                            modSet->getSVFFunction(callee);
                        this->addIndirectForkEdge(*it, svfCallee);
                    }
                }
            }
        }
    }

    // parallel_for sites
    for (auto it = parForSitesBegin(), eit = parForSitesEnd(); it != eit;
         ++it) {
        const Value *forkedval =
            tdAPI->getTaskFuncAtHareParForSite((*it)->getCallSite());
        if (llvm::dyn_cast<Function>(forkedval) == nullptr) {
            PAG *pag = pta->getPAG();
            const PointsTo &targets = pta->getPts(pag->getValueNode(forkedval));
            for (auto ii : targets) {
                if (auto *objPN = llvm::dyn_cast<ObjPN>(pag->getGNode(ii))) {
                    const MemObj *obj = pag->getObject(objPN);
                    if (obj->isFunction()) {
                        const auto *callee =
                            llvm::cast<Function>(obj->getRefVal());
                        LLVMModuleSet *modSet =
                            getPAG()->getModule()->getLLVMModSet();
                        const SVFFunction *svfCallee =
                            modSet->getSVFFunction(callee);
                        this->addIndirectForkEdge(*it, svfCallee);
                    }
                }
            }
        }
    }
}

/*!
 * Update join edge using pointer analysis results
 */
void ThreadCallGraph::updateJoinEdge(PointerAnalysis *pta) {

    for (auto it = joinsitesBegin(), eit = joinsitesEnd(); it != eit; ++it) {
        const Value *jointhread = tdAPI->getJoinedThread((*it)->getCallSite());
        // find its corresponding fork sites first
        CallSiteSet forkset;
        for (auto it = forksitesBegin(), eit = forksitesEnd(); it != eit;
             ++it) {
            const Value *forkthread =
                tdAPI->getForkedThread((*it)->getCallSite());
            if (pta->alias(jointhread, forkthread)) {
                forkset.insert(*it);
            }
        }
        assert(!forkset.empty() && "Can't find a forksite for this join!!");
        addDirectJoinEdge(*it, forkset);
    }
}

/*!
 * Add direct fork edges
 */
void ThreadCallGraph::addDirectForkEdge(const CallBlockNode *cs) {

    PTACallGraphNode *caller = getCallGraphNode(cs->getCaller());
    const auto *forkee =
        llvm::dyn_cast<Function>(tdAPI->getForkedFun(cs->getCallSite()));
    assert(forkee && "callee does not exist");
    LLVMModuleSet *modSet = getPAG()->getModule()->getLLVMModSet();
    PTACallGraphNode *callee =
        getCallGraphNode(getDefFunForMultipleModule(modSet, forkee));
    CallSiteID csId = addCallSite(cs, callee->getFunction());

    auto flag = PTACallGraphEdge::makeEdgeFlagWithAuxInfo(
        PTACallGraphEdge::TDForkEdge, csId);
    if (getGEdge(caller, callee, flag) == nullptr) {
        assert(cs->getCaller() == caller->getFunction() &&
               "callee instruction not inside caller??");

        auto *edge = new ThreadForkEdge(caller, callee, getNextEdgeId(), csId);
        edge->addDirectCallSite(cs);

        addGEdge(edge);
        addThreadForkEdgeSetMap(cs, edge);
    }
}

/*!
 * Add indirect fork edge to update call graph
 */
void ThreadCallGraph::addIndirectForkEdge(const CallBlockNode *cs,
                                          const SVFFunction *calleefun) {
    PTACallGraphNode *caller = getCallGraphNode(cs->getCaller());
    PTACallGraphNode *callee = getCallGraphNode(calleefun);

    CallSiteID csId = addCallSite(cs, callee->getFunction());
    auto flag = PTACallGraphEdge::makeEdgeFlagWithAuxInfo(
        PTACallGraphEdge::TDForkEdge, csId);

    if (getGEdge(caller, callee, flag) == nullptr) {
        assert(cs->getCaller() == caller->getFunction() &&
               "callee instruction not inside caller??");

        auto *edge = new ThreadForkEdge(caller, callee, getNextEdgeId(), csId);
        edge->addInDirectCallSite(cs, proj);

        addGEdge(edge);
        addThreadForkEdgeSetMap(cs, edge);
    }
}

/*!
 * Add direct fork edges
 * As join edge is a special return which is back to join site(s) rather than
 * its fork site A ThreadJoinEdge is created from the functions where join sites
 * reside in to the start routine function But we don't invoke addEdge() method
 * to add the edge to src and dst, otherwise it makes a scc cycle
 */
void ThreadCallGraph::addDirectJoinEdge(const CallBlockNode *cs,
                                        const CallSiteSet &forkset) {

    PTACallGraphNode *joinFunNode = getCallGraphNode(cs->getCaller());

    for (const auto *it : forkset) {

        const auto *threadRoutineFun =
            llvm::dyn_cast<Function>(tdAPI->getForkedFun(it->getCallSite()));
        assert(threadRoutineFun && "thread routine function does not exist");
        LLVMModuleSet *modSet = getPAG()->getModule()->getLLVMModSet();
        const SVFFunction *svfRoutineFun =
            modSet->getSVFFunction(threadRoutineFun);
        PTACallGraphNode *threadRoutineFunNode =
            getCallGraphNode(svfRoutineFun);
        CallSiteID csId = addCallSite(cs, svfRoutineFun);

        if (!hasThreadJoinEdge(cs, joinFunNode, threadRoutineFunNode, csId)) {
            assert(cs->getCaller() == joinFunNode->getFunction() &&
                   "callee instruction not inside caller??");
            auto *edge = new ThreadJoinEdge(joinFunNode, threadRoutineFunNode,
                                            getNextEdgeId(), csId);
            edge->addDirectCallSite(cs);

            addThreadJoinEdgeSetMap(cs, edge);
        }
    }
}

/*!
 * Add a direct ParFor edges
 */
void ThreadCallGraph::addDirectParForEdge(const CallBlockNode *cs) {

    PTACallGraphNode *caller = getCallGraphNode(cs->getCaller());
    const auto *taskFunc = llvm::dyn_cast<Function>(
        tdAPI->getTaskFuncAtHareParForSite(cs->getCallSite()));
    assert(taskFunc && "callee does not exist");
    LLVMModuleSet *modSet = getPAG()->getModule()->getLLVMModSet();
    const SVFFunction *svfTaskFun = modSet->getSVFFunction(taskFunc);

    PTACallGraphNode *callee = getCallGraphNode(svfTaskFun);

    CallSiteID csId = addCallSite(cs, callee->getFunction());

    auto flag = PTACallGraphEdge::makeEdgeFlagWithAuxInfo(
        PTACallGraphEdge::TDForkEdge, csId);
    if (getGEdge(caller, callee, flag) == nullptr) {
        assert(cs->getCaller() == caller->getFunction() &&
               "callee instruction not inside caller??");

        auto *edge = new HareParForEdge(caller, callee, getNextEdgeId(), csId);
        edge->addDirectCallSite(cs);

        addGEdge(edge);
        addHareParForEdgeSetMap(cs, edge);
    }
}

/*!
 * Add an indirect ParFor edge to update call graph
 */
void ThreadCallGraph::addIndirectParForEdge(const CallBlockNode *cs,
                                            const SVFFunction *calleefun) {

    PTACallGraphNode *caller = getCallGraphNode(cs->getCaller());
    PTACallGraphNode *callee = getCallGraphNode(calleefun);

    CallSiteID csId = addCallSite(cs, callee->getFunction());

    auto flag = PTACallGraphEdge::makeEdgeFlagWithAuxInfo(
        PTACallGraphEdge::HareParForEdge, csId);
    if (getGEdge(caller, callee, flag) == nullptr) {
        assert(cs->getCaller() == caller->getFunction() &&
               "callee instruction not inside caller??");

        auto *edge = new HareParForEdge(caller, callee, getNextEdgeId(), csId);
        edge->addInDirectCallSite(cs, proj);

        addGEdge(edge);
        addHareParForEdgeSetMap(cs, edge);
    }
}
