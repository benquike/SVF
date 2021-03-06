//===- ICFG.cpp -- Sparse value-flow graph-----------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2018>  <Yulei Sui>
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
 * ICFG.cpp
 *
 *  Created on: Sep 11, 2018
 *      Author: Yulei Sui
 *
 *  Updated by:
 *     Hui Peng <peng124@purdue.edu>
 *     2021-03-21
 */

#include "Graphs/ICFG.h"
#include "Graphs/PAG.h"
#include "Graphs/PTACallGraph.h"
#include "SVF-FE/ICFGBuilder.h"
#include "SVF-FE/LLVMUtil.h"
#include "Util/SVFModule.h"
#include <Util/Options.h>
#include <llvm/Demangle/Demangle.h>

using namespace SVF;
using namespace SVFUtil;

bool ICFGNode::classof(const GenericICFGNodeTy *node) {
    return node->getNodeKind() == AbstractNode ||
           GlobalBlockNode::classof(node) || IntraBlockNode::classof(node) ||
           InterBlockNode::classof(node);
}

bool InterBlockNode::classof(const GenericICFGNodeTy *node) {
    return node->getNodeKind() == InterBlock ||
           FunEntryBlockNode::classof(node) ||
           FunExitBlockNode::classof(node) || CallBlockNode::classof(node) ||
           RetBlockNode::classof(node);
}

bool ICFGEdge::classof(const GenericICFGEdgeTy *edge) {
    return edge->getEdgeKind() == AbstractEdge || IntraCFGEdge::classof(edge) ||
           CallCFGEdge::classof(edge) || RetCFGEdge::classof(edge);
}

const std::string ICFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "ICFGNode ID: " << getId();
    return rawstr.str();
}

void ICFGNode::dump() const { outs() << this->toString() << "\n"; }

const std::string GlobalBlockNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "GlobalBlockNode ID: " << getId();
    return rawstr.str();
}

const std::string IntraBlockNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "IntraBlockNode ID: " << getId();
    rawstr << value2String(getInst())
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    return rawstr.str();
}

const std::string FunEntryBlockNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "FunEntryBlockNode ID: " << getId();
    if (isExtCall(getFun())) {
        rawstr << " Entry("
               << ")\n";
    } else {
        rawstr << " Entry(" << getSourceLoc(getFun()->getLLVMFun()) << ")\n";
    }
    rawstr << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    return rawstr.str();
}

const std::string FunExitBlockNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "FunExitBlockNode ID: " << getId();
    if (isExtCall(getFun())) {
        rawstr << " Exit("
               << ")\n";
    } else {
        rawstr << " Exit("
               << getSourceLoc(
                      getFunExitBB(getFun()->getLLVMFun())->getFirstNonPHI())
               << ")\n";
    }
    rawstr << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    return rawstr.str();
}

const std::string CallBlockNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "CallBlockNode ID: " << getId();
    rawstr << value2String(getCallSite())
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    return rawstr.str();
}

const std::string RetBlockNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "RetBlockNode ID: " << getId();
    rawstr << value2String(getCallSite())
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    return rawstr.str();
}

const std::string ICFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "ICFGEdge: [" << getDstID() << "<--" << getSrcID() << "]\t";
    return rawstr.str();
}

const std::string IntraCFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    if (brCondition.first == nullptr) {
        rawstr << "IntraCFGEdge: [" << getDstID() << "<--" << getSrcID()
               << "]\t";
    } else {
        rawstr << "IntraCFGEdge: [" << getDstID() << "<--" << getSrcID()
               << "] with condition (" << *brCondition.first
               << "==" << brCondition.second << ") \t";
    }

    return rawstr.str();
}

const std::string CallCFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "CallCFGEdge "
           << " [";
    rawstr << getDstID() << "<--" << getSrcID() << "]\t CallSite: " << *cs
           << "\t";
    return rawstr.str();
}

const std::string RetCFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "RetCFGEdge "
           << " [";
    rawstr << getDstID() << "<--" << getSrcID() << "]\t CallSite: " << *cs
           << "\t";
    return rawstr.str();
}

/*!
 * Constructor: create ICFG and build it
 */
ICFG::ICFG(PAG *pag) : pag(pag) {
    DBOUT(DGENERAL, outs() << pasMsg("\tCreate ICFG ...\n"));
    globalBlockNode = new GlobalBlockNode(getNextNodeId());
    addGNode(globalBlockNode);
    buildICFG();
}

/*!
 * Build the ICFG using ICFGBuilder
 */
void ICFG::buildICFG() {
    ICFGBuilder builder(this);
    builder.build();
}

/// Get a basic block ICFGNode
ICFGNode *ICFG::getBlockICFGNode(const Instruction *inst) {
    ICFGNode *node;
    if (SVFUtil::isNonInstricCallSite(inst)) {
        node = getCallBlockNode(inst);
    } else if (SVFUtil::isIntrinsicInst(inst)) {
        node = getIntraBlockNode(inst);
        //			assert (false && "associating an intrinsic instruction with
        // an
        // ICFGNode!");
    } else {
        node = getIntraBlockNode(inst);
    }

    assert(node != nullptr && "no ICFGNode for this instruction?");
    return node;
}

CallBlockNode *ICFG::getCallBlockNode(const Instruction *inst) {
    if (SVFUtil::isCallSite(inst) == false) {
        outs() << *inst << "\n";
    }
    assert(SVFUtil::isCallSite(inst) && "not a call instruction?");
    assert(SVFUtil::isNonInstricCallSite(inst) &&
           "associating an intrinsic debug instruction with an ICFGNode!");
    CallBlockNode *node = getCallICFGNode(inst);
    if (node == nullptr) {
        node = addCallICFGNode(inst);
    }
    assert(node != nullptr && "no CallBlockNode for this instruction?");
    return node;
}

RetBlockNode *ICFG::getRetBlockNode(const Instruction *inst) {
    assert(SVFUtil::isCallSite(inst) && "not a call instruction?");
    assert(SVFUtil::isNonInstricCallSite(inst) &&
           "associating an intrinsic debug instruction with an ICFGNode!");
    RetBlockNode *node = getRetICFGNode(inst);
    if (node == nullptr) {
        node = addRetICFGNode(inst);
    }
    assert(node != nullptr && "no RetBlockNode for this instruction?");
    return node;
}

IntraBlockNode *ICFG::getIntraBlockNode(const Instruction *inst) {
    IntraBlockNode *node = getIntraBlockICFGNode(inst);
    if (node == nullptr) {
        node = addIntraBlockICFGNode(inst);
    }
    return node;
}

/// Add a function entry node
FunEntryBlockNode *ICFG::getFunEntryBlockNode(const SVFFunction *fun) {
    FunEntryBlockNode *b = getFunEntryICFGNode(fun);
    if (b == nullptr) {
        return addFunEntryICFGNode(fun);
    }

    return b;
}
/// Add a function exit node
FunExitBlockNode *ICFG::getFunExitBlockNode(const SVFFunction *fun) {
    FunExitBlockNode *b = getFunExitICFGNode(fun);
    if (b == nullptr) {
        return addFunExitICFGNode(fun);
    }

    return b;
}

/*!
 * Whether we has an intra ICFG edge
 *
 * FIXME: cleanup this code
 */
ICFGEdge *ICFG::hasIntraICFGEdge(ICFGNode *src, ICFGNode *dst,
                                 ICFGEdge::ICFGEdgeK kind) {
    ICFGEdge edge(src, dst, getDummyEdgeId(), kind);
    ICFGEdge *outEdge = src->hasOutgoingEdge(&edge);
    ICFGEdge *inEdge = dst->hasIncomingEdge(&edge);
    if (outEdge && inEdge) {
        assert(outEdge == inEdge && "edges not match");
        return outEdge;
    }

    return nullptr;
}

/*!
 * Whether we has an inter ICFG edge
 * FIXME: cleanup this code
 */
ICFGEdge *ICFG::hasInterICFGEdge(ICFGNode *src, ICFGNode *dst,
                                 ICFGEdge::ICFGEdgeK kind) {
    ICFGEdge edge(src, dst, getDummyEdgeId(), kind);
    ICFGEdge *outEdge = src->hasOutgoingEdge(&edge);
    ICFGEdge *inEdge = dst->hasIncomingEdge(&edge);
    if (outEdge && inEdge) {
        assert(outEdge == inEdge && "edges not match");
        return outEdge;
    }

    return nullptr;
}

/*!
 * Whether we has an thread ICFG edge
 */
ICFGEdge *ICFG::hasThreadICFGEdge(ICFGNode *src, ICFGNode *dst,
                                  ICFGEdge::ICFGEdgeK kind) {
    ICFGEdge edge(src, dst, getDummyEdgeId(), kind);
    ICFGEdge *outEdge = src->hasOutgoingEdge(&edge);
    ICFGEdge *inEdge = dst->hasIncomingEdge(&edge);
    if (outEdge && inEdge) {
        assert(outEdge == inEdge && "edges not match");
        return outEdge;
    }

    return nullptr;
}

/*!
 * Add intraprocedural edges between two nodes
 */
ICFGEdge *ICFG::addIntraEdge(ICFGNode *srcNode, ICFGNode *dstNode) {
    checkIntraEdgeParents(srcNode, dstNode);
    if (ICFGEdge *edge =
            hasIntraICFGEdge(srcNode, dstNode, ICFGEdge::IntraCF)) {
        assert(edge->isIntraCFGEdge() && "this should be an intra CFG edge!");
        return nullptr;
    }

    auto *intraEdge = new IntraCFGEdge(srcNode, dstNode, getNextEdgeId());
    return (addGEdge(intraEdge) ? intraEdge : nullptr);
}

/*!
 * Add conditional intraprocedural edges between two nodes
 */
ICFGEdge *ICFG::addConditionalIntraEdge(ICFGNode *srcNode, ICFGNode *dstNode,
                                        const Value *condition,
                                        NodeID branchID) {

    checkIntraEdgeParents(srcNode, dstNode);
    if (ICFGEdge *edge =
            hasIntraICFGEdge(srcNode, dstNode, ICFGEdge::IntraCF)) {
        assert(edge->isIntraCFGEdge() && "this should be an intra CFG edge!");
        return nullptr;
    }

    auto *intraEdge = new IntraCFGEdge(srcNode, dstNode, getNextEdgeId());
    intraEdge->setBranchCondtion(condition, branchID);
    return (addGEdge(intraEdge) ? intraEdge : nullptr);
}

/*!
 * Add interprocedural call edges between two nodes
 */
ICFGEdge *ICFG::addCallEdge(ICFGNode *srcNode, ICFGNode *dstNode,
                            const Instruction *cs) {
    if (ICFGEdge *edge = hasInterICFGEdge(srcNode, dstNode, ICFGEdge::CallCF)) {
        assert(edge->isCallCFGEdge() && "this should be a call CFG edge!");
        return nullptr;
    }

    auto *callEdge = new CallCFGEdge(srcNode, dstNode, getNextEdgeId(), cs);
    return (addGEdge(callEdge) ? callEdge : nullptr);
}

/*!
 * Add interprocedural return edges between two nodes
 */
ICFGEdge *ICFG::addRetEdge(ICFGNode *srcNode, ICFGNode *dstNode,
                           const Instruction *cs) {
    if (ICFGEdge *edge = hasInterICFGEdge(srcNode, dstNode, ICFGEdge::RetCF)) {
        assert(edge->isRetCFGEdge() && "this should be a return CFG edge!");
        return nullptr;
    }

    auto *retEdge = new RetCFGEdge(srcNode, dstNode, getNextEdgeId(), cs);
    return (addGEdge(retEdge) ? retEdge : nullptr);
}

/*!
 * Dump ICFG
 */
void ICFG::dump(const std::string &file, bool simple) {
    GraphPrinter::WriteGraphToFile(outs(), file, this, simple);
}

/*!
 * View ICFG
 */
void ICFG::view() {
    llvm::ViewGraph(this, "Interprocedural Control-Flow Graph");
}

/*!
 * Update ICFG for indirect calls
 * using the PTACallGraph, which resolves function pointers
 * called at callsites
 */
void ICFG::updateCallGraph(PTACallGraph *callgraph) {

    for (auto &iter : callgraph->getIndCallMap()) {
        const CallBlockNode *callBlock = iter.first;
        const Instruction *cs = callBlock->getCallSite();
        assert(callBlock->isIndirectCall() && "this is not an indirect call?");
        const PTACallGraph::FunctionSet &functions = iter.second;
        for (const auto *callee : functions) {
            CallBlockNode *CallBlockNode = getCallBlockNode(cs);
            if (!isExtCall(callee)) {
                FunEntryBlockNode *calleeEntryNode =
                    getFunEntryICFGNode(callee);
                addCallEdge(CallBlockNode, calleeEntryNode, cs);
                RetBlockNode *retBlockNode = getRetBlockNode(cs);
                FunExitBlockNode *calleeExitNode = getFunExitICFGNode(callee);
                addRetEdge(calleeExitNode, retBlockNode, cs);
            }
        }
    }
}

IntraBlockNode *ICFG::addIntraBlockICFGNode(const Instruction *inst) {
    IntraBlockNode *sNode =
        new IntraBlockNode(getNextNodeId(), inst, pag->getModule());
    addGNode(sNode);
    InstToBlockNodeMap[inst] = sNode;
    return sNode;
}

CallBlockNode *ICFG::addCallICFGNode(const Instruction *cs) {
    CallBlockNode *sNode =
        new CallBlockNode(getNextNodeId(), cs, pag->getModule());
    addGNode(sNode);
    CSToCallNodeMap[cs] = sNode;
    return sNode;
}

RetBlockNode *ICFG::addRetICFGNode(const Instruction *cs) {
    CallBlockNode *callBlockNode = getCallBlockNode(cs);
    RetBlockNode *sNode =
        new RetBlockNode(getNextNodeId(), cs, callBlockNode, pag->getModule());

    callBlockNode->setRetBlockNode(sNode);
    addGNode(sNode);
    CSToRetNodeMap[cs] = sNode;
    return sNode;
}

/*!
 * GraphTraits specialization
 */
namespace llvm {
template <>
struct DOTGraphTraits<ICFG *> : public DOTGraphTraits<PAG *> {

    using NodeType = ICFGNode;
    DOTGraphTraits(bool isSimple = false) : DOTGraphTraits<PAG *>(isSimple) {}

    /// Return name of the graph
    static std::string getGraphName(ICFG *) { return "ICFG"; }

    std::string getNodeLabel(NodeType *node, ICFG *graph) {
        return getSimpleNodeLabel(node, graph);
    }

    /// Return the label of an ICFG node
    static std::string getSimpleNodeLabel(NodeType *node, ICFG *g) {
        std::string str;
        raw_string_ostream rawstr(str);
        rawstr << "NodeID: " << node->getId() << "\n";

        if (auto *bNode = llvm::dyn_cast<IntraBlockNode>(node)) {
            rawstr << "IntraBlockNode ID: " << bNode->getId() << " \t";

            PAG::PAGEdgeList &edges = g->getPAG()->getInstPTAPAGEdgeList(bNode);
            if (edges.empty()) {
                rawstr << value2String(bNode->getInst()) << " \t";
            } else {
                for (auto edge : edges) {
                    rawstr << edge->toString();
                }
            }
            rawstr << " {fun: " << bNode->getFun()->getName() << "}";
        } else if (auto *entry = llvm::dyn_cast<FunEntryBlockNode>(node)) {
            rawstr << entry->toString();
        } else if (auto *exit = llvm::dyn_cast<FunExitBlockNode>(node)) {
            rawstr << exit->toString();
        } else if (auto *call = llvm::dyn_cast<CallBlockNode>(node)) {
            rawstr << call->toString();
        } else if (auto *ret = llvm::dyn_cast<RetBlockNode>(node)) {
            rawstr << ret->toString();
        } else if (auto *glob = llvm::dyn_cast<GlobalBlockNode>(node)) {
            PAG::PAGEdgeList &edges = g->getPAG()->getInstPTAPAGEdgeList(glob);
            for (const auto *edge : edges) {
                rawstr << edge->toString();
            }
        } else {
            assert(false && "what else kinds of nodes do we have??");
        }

        return rawstr.str();
    }

    static std::string getNodeAttributes(NodeType *node, ICFG *) {
        std::string str;
        raw_string_ostream rawstr(str);

        if (llvm::isa<IntraBlockNode>(node)) {
            rawstr << "color=black";
        } else if (llvm::isa<FunEntryBlockNode>(node)) {
            rawstr << "color=yellow";
        } else if (llvm::isa<FunExitBlockNode>(node)) {
            rawstr << "color=green";
        } else if (llvm::isa<CallBlockNode>(node)) {
            rawstr << "color=red";
        } else if (llvm::isa<RetBlockNode>(node)) {
            rawstr << "color=blue";
        } else if (llvm::isa<GlobalBlockNode>(node)) {
            rawstr << "color=purple";
        } else {
            assert(false && "no such kind of node!!");
        }

        rawstr << "";

        return rawstr.str();
    }

    template <class EdgeIter>
    static std::string getEdgeAttributes(NodeType *, EdgeIter EI, ICFG *) {
        ICFGEdge *edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");
        if (llvm::isa<CallCFGEdge>(edge)) {
            return "style=solid,color=red";
        }

        if (llvm::isa<RetCFGEdge>(edge)) {
            return "style=solid,color=blue";
        }

        return "style=solid";

        // return "";
    }

    template <class EdgeIter>
    static std::string getEdgeSourceLabel(NodeType *, EdgeIter EI) {
        ICFGEdge *edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");

        std::string str;
        raw_string_ostream rawstr(str);
        if (auto *dirCall = llvm::dyn_cast<CallCFGEdge>(edge)) {
            rawstr << dirCall->getCallSite();
        } else if (auto *dirRet = llvm::dyn_cast<RetCFGEdge>(edge)) {
            rawstr << dirRet->getCallSite();
        }

        return rawstr.str();
    }
};
} // End namespace llvm
