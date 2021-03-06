//===- PTACallGraph.cpp -- Call graph used internally in SVF------------------//
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
 * PTACallGraph.cpp
 *
 *  Created on: Nov 7, 2013
 *      Author: Yulei Sui
 */

#include "Graphs/PTACallGraph.h"
#include "SVF-FE/LLVMUtil.h"
#include "SVF-FE/SVFProject.h"
#include "Util/SVFModule.h"
#include <llvm/Demangle/Demangle.h>

using namespace SVF;
using namespace SVFUtil;

// PTACallGraph::CallSiteToIdMap PTACallGraph::csToIdMap;
// PTACallGraph::IdToCallSiteMap PTACallGraph::idToCSMap;
// CallSiteID PTACallGraph::totalCallSiteNum = 1;

/// Add direct and indirect callsite
//@{
void PTACallGraphEdge::addDirectCallSite(const CallBlockNode *call) {
    const Instruction *callInst = call->getCallSite();
    llvm::ImmutableCallSite cs(callInst);
    // handle pointer cast
    // same as in getCallee
    auto *sf = cs.getCalledValue()->stripPointerCasts();
    assert((cs.getCalledFunction() || llvm::isa<Function>(sf)) &&
           "not a direct callsite??");

    directCalls.insert(call);
}

void PTACallGraphEdge::addInDirectCallSite(const CallBlockNode *call,
                                           SVFProject *proj) {
    const Instruction *callInst = call->getCallSite();
    llvm::ImmutableCallSite cs(callInst);
    assert((nullptr == cs.getCalledFunction() ||
            nullptr == llvm::dyn_cast<Function>(
                           proj->getForkedFun(call->getCallSite()))) &&
           "not an indirect callsite??");
    indirectCalls.insert(call);
}
//@}

const std::string PTACallGraphEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "CallSite ID: " << getCallSiteID();
    if (isDirectCallEdge()) {
        rawstr << "direct call";
    } else {
        rawstr << "indirect call";
    }
    rawstr << "[" << getDstID() << "<--" << getSrcID() << "]\t";
    return rawstr.str();
}

const std::string PTACallGraphNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "CallGraphNode ID: " << getId()
           << " {fun: " << llvm::demangle(fun->getName()) << "}";
    return rawstr.str();
}

bool PTACallGraphNode::isReachableFromProgEntry() const {
    std::stack<const PTACallGraphNode *> nodeStack;
    NodeBS visitedNodes;
    nodeStack.push(this);
    visitedNodes.set(getId());

    while (nodeStack.empty() == false) {
        auto *node = const_cast<PTACallGraphNode *>(nodeStack.top());
        nodeStack.pop();

        if (SVFUtil::isProgEntryFunction(node->getFunction())) {
            return true;
        }

        for (auto it = node->InEdgeBegin(), eit = node->InEdgeEnd(); it != eit;
             ++it) {
            PTACallGraphEdge *edge = *it;
            if (visitedNodes.test_and_set(edge->getSrcID())) {
                nodeStack.push(edge->getSrcNode());
            }
        }
    }

    return false;
}

/// Constructor
PTACallGraph::PTACallGraph(SVFProject *proj, CGEK k)
    : kind(k), pag(proj->getPAG()), proj(proj) {
    numOfResolvedIndCallEdge = 0;
}

/*!
 *  Memory has been cleaned up at GenericGraph
 */
void PTACallGraph::destroy() {}

/*!
 * Add call graph node
 */
void PTACallGraph::addCallGraphNode(const SVFFunction *fun) {
    auto *callGraphNode = new PTACallGraphNode(getNextNodeId(), fun);
    addGNode(callGraphNode);
    funToCallGraphNodeMap[fun] = callGraphNode;
}

/*!
 * Add direct call edges
 */
void PTACallGraph::addDirectCallGraphEdge(const CallBlockNode *cs,
                                          const SVFFunction *callerFun,
                                          const SVFFunction *calleeFun) {

    PTACallGraphNode *caller = getCallGraphNode(callerFun);
    PTACallGraphNode *callee = getCallGraphNode(calleeFun);

    CallSiteID csId = addCallSite(cs, callee->getFunction());

    auto flag = PTACallGraphEdge::makeEdgeFlagWithAuxInfo(
        PTACallGraphEdge::CallRetEdge, csId);
    if (getGEdge(caller, callee, flag) == nullptr) {
        auto *edge = new PTACallGraphEdge(caller, callee, getNextEdgeId(),
                                          PTACallGraphEdge::CallRetEdge, csId);
        edge->addDirectCallSite(cs);
        addGEdge(edge);
        callinstToCallGraphEdgesMap[cs].insert(edge);
    }
}

/*!
 * Add indirect call edge to update call graph
 */
void PTACallGraph::addIndirectCallGraphEdge(const CallBlockNode *cs,
                                            const SVFFunction *callerFun,
                                            const SVFFunction *calleeFun) {

    PTACallGraphNode *caller = getCallGraphNode(callerFun);
    PTACallGraphNode *callee = getCallGraphNode(calleeFun);

    numOfResolvedIndCallEdge++;

    CallSiteID csId = addCallSite(cs, callee->getFunction());
    auto flag = PTACallGraphEdge::makeEdgeFlagWithAuxInfo(
        PTACallGraphEdge::CallRetEdge, csId);
    if (getGEdge(caller, callee, flag) == nullptr) {
        auto *edge = new PTACallGraphEdge(caller, callee, getNextEdgeId(),
                                          PTACallGraphEdge::CallRetEdge, csId);
        edge->addInDirectCallSite(cs, proj);
        addGEdge(edge);
        callinstToCallGraphEdgesMap[cs].insert(edge);
    }
}

/*!
 * Get all callsite invoking this callee
 */
void PTACallGraph::getAllCallSitesInvokingCallee(
    const SVFFunction *callee, PTACallGraphEdge::CallInstSet &csSet) {
    PTACallGraphNode *callGraphNode = getCallGraphNode(callee);
    for (auto it = callGraphNode->InEdgeBegin(),
              eit = callGraphNode->InEdgeEnd();
         it != eit; ++it) {
        for (auto cit = (*it)->directCallsBegin(),
                  ecit = (*it)->directCallsEnd();
             cit != ecit; ++cit) {
            csSet.insert((*cit));
        }
        for (auto cit = (*it)->indirectCallsBegin(),
                  ecit = (*it)->indirectCallsEnd();
             cit != ecit; ++cit) {
            csSet.insert((*cit));
        }
    }
}

/*!
 * Get direct callsite invoking this callee
 */
void PTACallGraph::getDirCallSitesInvokingCallee(
    const SVFFunction *callee, PTACallGraphEdge::CallInstSet &csSet) {
    PTACallGraphNode *callGraphNode = getCallGraphNode(callee);
    for (auto it = callGraphNode->InEdgeBegin(),
              eit = callGraphNode->InEdgeEnd();
         it != eit; ++it) {
        for (auto cit = (*it)->directCallsBegin(),
                  ecit = (*it)->directCallsEnd();
             cit != ecit; ++cit) {
            csSet.insert((*cit));
        }
    }
}

/*!
 * Get indirect callsite invoking this callee
 */
void PTACallGraph::getIndCallSitesInvokingCallee(
    const SVFFunction *callee, PTACallGraphEdge::CallInstSet &csSet) {
    PTACallGraphNode *callGraphNode = getCallGraphNode(callee);
    for (auto it = callGraphNode->InEdgeBegin(),
              eit = callGraphNode->InEdgeEnd();
         it != eit; ++it) {
        for (auto cit = (*it)->indirectCallsBegin(),
                  ecit = (*it)->indirectCallsEnd();
             cit != ecit; ++cit) {
            csSet.insert((*cit));
        }
    }
}

/*!
 * Issue a warning if the function which has indirect call sites can not be
 * reached from program entry.
 */
void PTACallGraph::verifyCallGraph() {
    auto it = indirectCallMap.begin();
    auto eit = indirectCallMap.end();
    for (const auto &it : indirectCallMap) {
        const FunctionSet &targets = it.second;
        if (!targets.empty()) {
            const CallBlockNode *cs = it.first;
            const SVFFunction *func = cs->getCaller();
            if (getCallGraphNode(func)->isReachableFromProgEntry() == false) {
                writeWrnMsg(
                    func->getName().str() +
                    " has indirect call site but not reachable from main");
            }
        }
    }
}

/*!
 * Whether its reachable between two functions
 */
bool PTACallGraph::isReachableBetweenFunctions(const SVFFunction *srcFn,
                                               const SVFFunction *dstFn) const {
    PTACallGraphNode *dstNode = getCallGraphNode(dstFn);

    std::stack<const PTACallGraphNode *> nodeStack;
    NodeBS visitedNodes;
    nodeStack.push(dstNode);
    visitedNodes.set(dstNode->getId());

    while (nodeStack.empty() == false) {
        auto *node = const_cast<PTACallGraphNode *>(nodeStack.top());
        nodeStack.pop();

        if (node->getFunction() == srcFn) {
            return true;
        }

        for (auto it = node->InEdgeBegin(), eit = node->InEdgeEnd(); it != eit;
             ++it) {
            PTACallGraphEdge *edge = *it;
            if (visitedNodes.test_and_set(edge->getSrcID())) {
                nodeStack.push(edge->getSrcNode());
            }
        }
    }

    return false;
}

/*!
 * Dump call graph into dot file
 */
void PTACallGraph::dump(const std::string &filename) {
    GraphPrinter::WriteGraphToFile(outs(), filename, this);
}

namespace llvm {

/*!
 * Write value flow graph into dot file for debugging
 */
template <>
struct DOTGraphTraits<PTACallGraph *> : public DefaultDOTGraphTraits {

    using NodeType = PTACallGraphNode;
    using ChildIteratorType = NodeType::iterator;
    DOTGraphTraits(bool isSimple = false) : DefaultDOTGraphTraits(isSimple) {}

    /// Return name of the graph
    static std::string getGraphName(PTACallGraph *) { return "PTA Call Graph"; }
    /// Return function name;
    static std::string getNodeLabel(PTACallGraphNode *node, PTACallGraph *) {
        return node->toString();
    }

    static std::string getNodeAttributes(PTACallGraphNode *node,
                                         PTACallGraph *) {
        const SVFFunction *fun = node->getFunction();
        if (!SVFUtil::isExtCall(fun)) {
            return "shape=box";
        }

        return "shape=Mrecord";
    }

    template <class EdgeIter>
    static std::string getEdgeAttributes(PTACallGraphNode *, EdgeIter EI,
                                         PTACallGraph *) {

        // TODO: mark indirect call of Fork with different color
        PTACallGraphEdge *edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");

        std::string color;

        if (edge->getEdgeKind() == PTACallGraphEdge::TDJoinEdge) {
            color = "color=green";
        } else if (edge->getEdgeKind() == PTACallGraphEdge::TDForkEdge) {
            color = "color=blue";
        } else {
            color = "color=black";
        }
        if (0 != edge->getIndirectCalls().size()) {
            color = "color=red";
        }
        return color;
    }

    template <class EdgeIter>
    static std::string getEdgeSourceLabel(NodeType *, EdgeIter EI) {
        PTACallGraphEdge *edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");

        std::string str;
        raw_string_ostream rawstr(str);
        rawstr << edge->getCallSiteID();

        return rawstr.str();
    }
};
} // End namespace llvm
