//===- VFG.cpp -- Sparse value-flow graph-----------------------------------//
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
 * VFG.cpp
 *
 *  Created on: Sep 11, 2018
 *      Author: Yulei Sui
 */

#include "Graphs/VFG.h"
#include "Graphs/SVFG.h"
#include "SVF-FE/LLVMUtil.h"
#include "Util/Options.h"
#include "Util/SVFModule.h"
#include <llvm-10/llvm/Demangle/Demangle.h>

using namespace SVF;
using namespace SVFUtil;

bool VFGNode::classof(const GenericVFGNodeTy *node) {
    return node->getNodeKind() == VFGNode::Vfg || StmtVFGNode::classof(node) ||
           CmpVFGNode::classof(node) || BinaryOPVFGNode::classof(node) ||
           UnaryOPVFGNode::classof(node) || PHIVFGNode::classof(node) ||
           ArgumentVFGNode::classof(node) || NullPtrVFGNode::classof(node);
}

bool StmtVFGNode::classof(const GenericVFGNodeTy *node) {
    return node->getNodeKind() == VFGNode::Stmt || LoadVFGNode::classof(node) ||
           StoreVFGNode::classof(node) || CopyVFGNode::classof(node) ||
           GepVFGNode::classof(node) || AddrVFGNode::classof(node);
}

bool PHIVFGNode::classof(const GenericVFGNodeTy *node) {
    return node->getNodeKind() == TPhi || IntraPHIVFGNode::classof(node) ||
           InterPHIVFGNode::classof(node);
}

bool ArgumentVFGNode::classof(const GenericVFGNodeTy *node) {
    return node->getNodeKind() == VFGNode::Argument ||
           ActualParmVFGNode::classof(node) ||
           FormalParmVFGNode::classof(node) ||
           ActualRetVFGNode::classof(node) || FormalRetVFGNode::classof(node);
}

bool VFGEdge::classof(const GenericVFGEdgeTy *edge) {
    return edge->getEdgeKind() == VFGEdge::VF ||
           DirectSVFGEdge::classof(edge) || IndirectSVFGEdge::classof(edge);
}

bool DirectSVFGEdge::classof(const GenericVFGEdgeTy *edge) {
    return edge->getEdgeKind() == VFGEdge::DirectVF ||
           IntraDirSVFGEdge::classof(edge) || CallDirSVFGEdge::classof(edge) ||
           RetDirSVFGEdge::classof(edge);
}

const std::string VFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "VFGNode ID: " << getId() << " ";
    return rawstr.str();
}

const std::string StmtVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "StmtVFGNode ID: " << getId() << " ";
    rawstr << getPAGEdge()->toString();
    return rawstr.str();
}

const std::string LoadVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "LoadVFGNode ID: " << getId() << " ";
    rawstr << getPAGEdge()->toString();
    return rawstr.str();
}

const std::string StoreVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "StoreVFGNode ID: " << getId() << " ";
    rawstr << getPAGEdge()->toString();
    return rawstr.str();
}

const std::string CopyVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "CopyVFGNode ID: " << getId() << " ";
    rawstr << getPAGEdge()->toString();
    return rawstr.str();
}

const std::string CmpVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "CmpVFGNode ID: " << getId() << " ";
    rawstr << "PAGEdge: [" << res->getId() << " = cmp(";
    for (CmpVFGNode::OPVers::const_iterator it = opVerBegin(), eit = opVerEnd();
         it != eit; it++) {
        rawstr << it->second->getId() << ", ";
    }
    rawstr << ")]\n";
    if (res->hasValue()) {
        rawstr << " " << *res->getValue();
        rawstr << SVFUtil::getSourceLoc(res->getValue());
    }
    return rawstr.str();
}

const std::string BinaryOPVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "BinaryOPVFGNode ID: " << getId() << " ";
    rawstr << "PAGEdge: [" << res->getId() << " = Binary(";
    for (BinaryOPVFGNode::OPVers::const_iterator it = opVerBegin(),
                                                 eit = opVerEnd();
         it != eit; it++) {
        rawstr << it->second->getId() << ", ";
    }
    rawstr << ")]\t";
    if (res->hasValue()) {
        rawstr << " " << *res->getValue() << " ";
        rawstr << SVFUtil::getSourceLoc(res->getValue());
    }
    return rawstr.str();
}

const std::string UnaryOPVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "UnaryOPVFGNode ID: " << getId() << " ";
    rawstr << "PAGEdge: [" << res->getId() << " = Unary(";
    for (UnaryOPVFGNode::OPVers::const_iterator it = opVerBegin(),
                                                eit = opVerEnd();
         it != eit; it++) {
        rawstr << it->second->getId() << ", ";
    }
    rawstr << ")]\t";
    if (res->hasValue()) {
        rawstr << " " << *res->getValue() << " ";
        rawstr << SVFUtil::getSourceLoc(res->getValue());
    }
    return rawstr.str();
}

const std::string GepVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "GepVFGNode ID: " << getId() << " ";
    rawstr << getPAGEdge()->toString();
    return rawstr.str();
}

const std::string PHIVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "PHIVFGNode ID: " << getId() << " ";
    rawstr << "PAGEdge: [" << res->getId() << " = PHI(";
    for (PHIVFGNode::OPVers::const_iterator it = opVerBegin(), eit = opVerEnd();
         it != eit; it++) {
        rawstr << it->second->getId() << ", ";
    }
    rawstr << ")]\t";
    if (res->hasValue()) {
        rawstr << " " << *res->getValue();
        rawstr << SVFUtil::getSourceLoc(res->getValue());
    }
    return rawstr.str();
}

const std::string IntraPHIVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "IntraPHIVFGNode ID: " << getId() << " ";
    rawstr << "PAGEdge: [" << res->getId() << " = PHI(";
    for (PHIVFGNode::OPVers::const_iterator it = opVerBegin(), eit = opVerEnd();
         it != eit; it++) {
        rawstr << it->second->getId() << ", ";
    }
    rawstr << ")]\t";
    if (res->hasValue()) {
        rawstr << " " << *res->getValue();
        rawstr << SVFUtil::getSourceLoc(res->getValue());
    }
    return rawstr.str();
}

const std::string AddrVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "AddrVFGNode ID: " << getId() << " ";
    rawstr << getPAGEdge()->toString();
    return rawstr.str();
}

const std::string ArgumentVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "ArgumentVFGNode ID: " << getId() << " ";
    rawstr << param->toString();
    return rawstr.str();
}

const std::string ActualParmVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "ActualParmVFGNode ID: " << getId() << " ";
    rawstr << "CS[" << getSourceLoc(getCallSite()->getCallSite()) << "]";
    rawstr << param->toString();
    return rawstr.str();
}

const std::string FormalParmVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "FormalParmVFGNode ID: " << getId() << " ";
    rawstr << "Fun[" << llvm::demangle(getFun()->getName().str()) << "]";
    rawstr << param->toString();
    return rawstr.str();
}

const std::string ActualRetVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "ActualRetVFGNode ID: " << getId() << " ";
    rawstr << "CS[" << getSourceLoc(getCallSite()->getCallSite()) << "]";
    rawstr << param->toString();
    return rawstr.str();
}

const std::string FormalRetVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "FormalRetVFGNode ID: " << getId() << " ";
    rawstr << "Fun[" << llvm::demangle(getFun()->getName().str()) << "]";
    rawstr << param->toString();
    return rawstr.str();
}

const std::string InterPHIVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    if (isFormalParmPHI()) {
        rawstr << "FormalParmPHI ID: " << getId()
               << " PAGNode ID: " << res->getId() << "\n"
               << *res->getValue();
    } else {
        rawstr << "ActualRetPHI ID: " << getId()
               << " PAGNode ID: " << res->getId() << "\n"
               << *res->getValue();
    }
    return rawstr.str();
}

const std::string NullPtrVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "NullPtrVFGNode ID: " << getId();
    rawstr << " PAGNode ID: " << node->getId() << "\n";
    return rawstr.str();
}

const std::string VFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "VFGEdge: [" << getDstID() << "<--" << getSrcID() << "]\t";
    return rawstr.str();
}

const std::string DirectSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "DirectVFGEdge: [" << getDstID() << "<--" << getSrcID() << "]\t";
    return rawstr.str();
}

const std::string IntraDirSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "IntraDirSVFGEdge: [" << getDstID() << "<--" << getSrcID()
           << "]\t";
    return rawstr.str();
}

const std::string CallDirSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "CallDirSVFGEdge CallSite ID: " << getCallSiteId() << " [";
    rawstr << getDstID() << "<--" << getSrcID() << "]\t";
    return rawstr.str();
}

const std::string RetDirSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "RetDirSVFGEdge CallSite ID: " << getCallSiteId() << " [";
    rawstr << getDstID() << "<--" << getSrcID() << "]\t";
    return rawstr.str();
}

FormalRetVFGNode::FormalRetVFGNode(NodeID id, const PAGNode *n,
                                   const SVFFunction *f)
    : ArgumentVFGNode(id, n, FRet), fun(f) {}

PHIVFGNode::PHIVFGNode(NodeID id, const PAGNode *r, VFGNodeK k)
    : VFGNode(id, k), res(r) {}

/*!
 * Constructor
 *  * Build VFG
 * 1) build VFG nodes
 *    statements for top level pointers (PAGEdges)
 * 2) connect VFG edges
 *    between two statements (PAGEdges)
 */
VFG::VFG(PTACallGraph *cg, PAG *pag, VFGK k)
    : callgraph(cg), pag(pag), kind(k) {

    DBOUT(DGENERAL, outs() << pasMsg("\tCreate VFG Top Level Node\n"));
    addVFGNodes();

    DBOUT(DGENERAL, outs() << pasMsg("\tCreate SVFG Direct Edge\n"));
    connectDirectVFGEdges();
}

/*!
 * Memory has been cleaned up at GenericGraph
 */
void VFG::destroy() { pag = nullptr; }

/*!
 * \brief: create nodes for the VFG
 *
 * Create VFG nodes for top level pointers
 */
void VFG::addVFGNodes() {

    // initialize dummy definition  null pointers in order to uniform the
    // construction to be noted for black hole pointer it has already has
    // address edge connected, and its definition will be set when processing
    // addr PAG edge.
    addNullPtrVFGNode(pag->getGNode(pag->getNullPtr()));

    ///
    ///  Create am AddrVFGNode for each AddrEdge in PAG
    ///
    PAGEdge::PAGEdgeSetTy &addrs = getPAGEdgeSet(PAGEdge::Addr);
    for (auto *addr : addrs) {
        addAddrVFGNode(llvm::cast<AddrPE>(addr));
    }

    ///
    ///  create a CopyVFGNode for each CopyEdge in PAG
    ///
    PAGEdge::PAGEdgeSetTy &copys = getPAGEdgeSet(PAGEdge::Copy);
    for (auto *copy : copys) {
        const CopyPE *edge = llvm::cast<CopyPE>(copy);
        if (!isPhiCopyEdge(edge)) {
            addCopyVFGNode(edge);
        }
    }

    ///
    /// Create a GepVFG node for each NormalGep Edge in PAG
    ///
    PAGEdge::PAGEdgeSetTy &ngeps = getPAGEdgeSet(PAGEdge::NormalGep);
    for (auto *ngep : ngeps) {
        addGepVFGNode(llvm::cast<NormalGepPE>(ngep));
    }

    ///
    /// Create a GepVFG node for each VariantGep edge in PAG
    ///
    PAGEdge::PAGEdgeSetTy &vgeps = getPAGEdgeSet(PAGEdge::VariantGep);
    for (auto *vgep : vgeps) {
        addGepVFGNode(llvm::cast<VariantGepPE>(vgep));
    }

    ///
    /// Create a LoadVFGNode for each Load Edge in PAG
    ///
    PAGEdge::PAGEdgeSetTy &loads = getPAGEdgeSet(PAGEdge::Load);
    for (auto *load : loads) {
        addLoadVFGNode(llvm::cast<LoadPE>(load));
    }

    ///
    /// Create StoreVFGNode for each store edge in PAG
    ///
    PAGEdge::PAGEdgeSetTy &stores = getPAGEdgeSet(PAGEdge::Store);
    for (auto *store : stores) {
        addStoreVFGNode(llvm::cast<StorePE>(store));
    }

    ///
    /// Create an ActualParmVFGNode for each
    /// ThreadFork edge in PAG
    ///
    PAGEdge::PAGEdgeSetTy &forks = getPAGEdgeSet(PAGEdge::ThreadFork);
    for (auto *fork : forks) {
        auto *forkedge = llvm::cast<TDForkPE>(fork);
        addActualParmVFGNode(forkedge->getSrcNode(), forkedge->getCallSite());
    }

    ///
    /// create an ActualParmVFGNode for
    /// each arg in every callsite
    ///
    /// Note: only interesting PAGNodes
    /// are considered
    ///
    for (auto &it : pag->getCallSiteArgsMap()) {
        for (auto pit = it.second.begin(), epit = it.second.end(); pit != epit;
             ++pit) {
            const PAGNode *argPAGNode = *pit;
            if (isInterestedPAGNode(argPAGNode)) {
                addActualParmVFGNode(argPAGNode, it.first);
            }
        }
    }

    ///
    /// initialize actual return nodes (callsite return)
    for (auto &it : pag->getCallSiteRets()) {

        /// for external function we do not create acutalRet VFGNode
        /// they are in the formal of AddrVFGNode if the external function
        /// returns an allocated memory if fun has body, it may also exist in
        /// isExtCall, e.g., xmalloc() in bzip2, spec2000.
        if (isInterestedPAGNode(it.second) == false || hasDef(it.second)) {
            continue;
        }

        addActualRetVFGNode(it.second, it.first->getCallBlockNode());
    }

    // initialize formal parameter nodes
    for (auto &it : pag->getFunArgsMap()) {
        const SVFFunction *func = it.first;

        for (const auto *param : it.second) {
            if (!isInterestedPAGNode(param) ||
                hasBlackHoleConstObjAddrAsDef(param)) {
                continue;
            }

            CallPESet callPEs;
            if (param->hasIncomingEdges(PAGEdge::Call)) {
                for (auto cit = param->getIncomingEdgesBegin(PAGEdge::Call),
                          ecit = param->getIncomingEdgesEnd(PAGEdge::Call);
                     cit != ecit; ++cit) {
                    auto *callPE = llvm::cast<CallPE>(*cit);
                    if (isInterestedPAGNode(callPE->getSrcNode())) {
                        callPEs.insert(callPE);
                    }
                }
            }

            addFormalParmVFGNode(param, func, callPEs);
        }

        if (func->getLLVMFun()->getFunctionType()->isVarArg()) {
            const PAGNode *varParam = pag->getGNode(pag->getVarargNode(func));
            if (isInterestedPAGNode(varParam) == false ||
                hasBlackHoleConstObjAddrAsDef(varParam)) {
                continue;
            }

            CallPESet callPEs;
            if (varParam->hasIncomingEdges(PAGEdge::Call)) {
                for (auto cit = varParam->getIncomingEdgesBegin(PAGEdge::Call),
                          ecit = varParam->getIncomingEdgesEnd(PAGEdge::Call);
                     cit != ecit; ++cit) {
                    auto *callPE = llvm::cast<CallPE>(*cit);
                    if (isInterestedPAGNode(callPE->getSrcNode())) {
                        callPEs.insert(callPE);
                    }
                }
            }
            addFormalParmVFGNode(varParam, func, callPEs);
        }
    }

    // initialize formal return nodes (callee return)
    for (auto &it : pag->getFunRets()) {
        const SVFFunction *func = it.first;

        const PAGNode *uniqueFunRetNode = it.second;

        RetPESet retPEs;
        if (uniqueFunRetNode->hasOutgoingEdges(PAGEdge::Ret)) {
            for (auto
                     cit =
                         uniqueFunRetNode->getOutgoingEdgesBegin(PAGEdge::Ret),
                     ecit = uniqueFunRetNode->getOutgoingEdgesEnd(PAGEdge::Ret);
                 cit != ecit; ++cit) {
                const RetPE *retPE = llvm::cast<RetPE>(*cit);
                if (isInterestedPAGNode(retPE->getDstNode())) {
                    retPEs.insert(retPE);
                }
            }
        }

        if (isInterestedPAGNode(uniqueFunRetNode)) {
            addFormalRetVFGNode(uniqueFunRetNode, func, retPEs);
        }
    }

    // initialize llvm phi nodes (phi of top level pointers)
    PAG::PHINodeMap &phiNodeMap = pag->getPhiNodeMap();
    for (auto &pit : phiNodeMap) {
        if (isInterestedPAGNode(pit.first)) {
            addIntraPHIVFGNode(pit.first, pit.second);
        }
    }
    // initialize llvm binary nodes (binary operators)
    PAG::BinaryNodeMap &binaryNodeMap = pag->getBinaryNodeMap();
    for (auto &pit : binaryNodeMap) {
        if (isInterestedPAGNode(pit.first)) {
            addBinaryOPVFGNode(pit.first, pit.second);
        }
    }
    // initialize llvm unary nodes (unary operators)
    PAG::UnaryNodeMap &unaryNodeMap = pag->getUnaryNodeMap();
    for (auto &pit : unaryNodeMap) {
        if (isInterestedPAGNode(pit.first)) {
            addUnaryOPVFGNode(pit.first, pit.second);
        }
    }
    // initialize llvm cmp nodes (comparision)
    PAG::CmpNodeMap &cmpNodeMap = pag->getCmpNodeMap();
    for (auto &pit : cmpNodeMap) {
        if (isInterestedPAGNode(pit.first)) {
            addCmpVFGNode(pit.first, pit.second);
        }
    }
}

/*!
 * Add def-use edges for top level pointers
 */
VFGEdge *VFG::addIntraDirectVFEdge(NodeID srcId, NodeID dstId) {
    VFGNode *srcNode = getGNode(srcId);
    VFGNode *dstNode = getGNode(dstId);
    checkIntraEdgeParents(srcNode, dstNode);
    if (VFGEdge *edge =
            hasIntraVFGEdge(srcNode, dstNode, VFGEdge::IntraDirectVF)) {
        assert(edge->isDirectVFGEdge() &&
               "this should be a direct value flow edge!");
        return nullptr;
    }

    if (srcNode != dstNode) {
        auto *directEdge =
            new IntraDirSVFGEdge(srcNode, dstNode, getNextEdgeId());
        return (addGEdge(directEdge) ? directEdge : nullptr);
    }

    return nullptr;
}

/*!
 * Add interprocedural call edges for top level pointers
 */
VFGEdge *VFG::addCallEdge(NodeID srcId, NodeID dstId, CallSiteID csId) {
    VFGNode *srcNode = getGNode(srcId);
    VFGNode *dstNode = getGNode(dstId);
    if (VFGEdge *edge =
            hasInterVFGEdge(srcNode, dstNode, VFGEdge::CallDirVF, csId)) {
        assert(edge->isCallDirectVFGEdge() &&
               "this should be a direct value flow edge!");
        return nullptr;
    }

    auto *callEdge =
        new CallDirSVFGEdge(srcNode, dstNode, getNextEdgeId(), csId);
    return (addGEdge(callEdge) ? callEdge : nullptr);
}

/*!
 * Add interprocedural return edges for top level pointers
 */
VFGEdge *VFG::addRetEdge(NodeID srcId, NodeID dstId, CallSiteID csId) {
    VFGNode *srcNode = getGNode(srcId);
    VFGNode *dstNode = getGNode(dstId);
    if (VFGEdge *edge =
            hasInterVFGEdge(srcNode, dstNode, VFGEdge::RetDirVF, csId)) {
        assert(edge->isRetDirectVFGEdge() &&
               "this should be a direct value flow edge!");
        return nullptr;
    }

    auto *retEdge = new RetDirSVFGEdge(srcNode, dstNode, getNextEdgeId(), csId);
    return (addGEdge(retEdge) ? retEdge : nullptr);
}

/*!
 * Connect def-use chains for direct value-flow, (value-flow of top level
 * pointers)
 */
void VFG::connectDirectVFGEdges() {
    for (iterator it = begin(); it != end(); ++it) {
        NodeID nodeId = it->first;
        VFGNode *node = it->second;

        if (auto *stmtNode = llvm::dyn_cast<StmtVFGNode>(node)) {
            /// do not handle AddrSVFG node, as it is already
            /// the source of a definition
            if (llvm::isa<AddrVFGNode>(stmtNode)) {
                continue;
            }

            /// for all other cases, like copy/gep/load/ret, connect the RHS
            /// pointer to its def
            if (!stmtNode->getPAGSrcNode()->isConstantData()) {
                addIntraDirectVFEdge(getDef(stmtNode->getPAGSrcNode()), nodeId);
            }

            /// for store, connect the RHS/LHS pointer to its def
            if (llvm::isa<StoreVFGNode>(stmtNode) &&
                (stmtNode->getPAGDstNode()->isConstantData() == false)) {
                addIntraDirectVFEdge(getDef(stmtNode->getPAGDstNode()), nodeId);
            }
        } else if (auto *phiNode = llvm::dyn_cast<PHIVFGNode>(node)) {
            for (auto it = phiNode->opVerBegin(), eit = phiNode->opVerEnd();
                 it != eit; it++) {
                if (!it->second->isConstantData()) {
                    addIntraDirectVFEdge(getDef(it->second), nodeId);
                }
            }
        } else if (auto *binaryNode = llvm::dyn_cast<BinaryOPVFGNode>(node)) {
            for (auto it = binaryNode->opVerBegin(),
                      eit = binaryNode->opVerEnd();
                 it != eit; it++) {
                if (it->second->isConstantData() == false) {
                    addIntraDirectVFEdge(getDef(it->second), nodeId);
                }
            }
        } else if (auto *unaryNode = llvm::dyn_cast<UnaryOPVFGNode>(node)) {

            for (auto it = unaryNode->opVerBegin(), eit = unaryNode->opVerEnd();
                 it != eit; it++) {
                if (!it->second->isConstantData()) {
                    addIntraDirectVFEdge(getDef(it->second), nodeId);
                }
            }

        } else if (auto *cmpNode = llvm::dyn_cast<CmpVFGNode>(node)) {

            for (auto it = cmpNode->opVerBegin(), eit = cmpNode->opVerEnd();
                 it != eit; it++) {
                if (!it->second->isConstantData()) {
                    addIntraDirectVFEdge(getDef(it->second), nodeId);
                }
            }
        } else if (auto *actualParm = llvm::dyn_cast<ActualParmVFGNode>(node)) {

            if (!actualParm->getParam()->isConstantData()) {
                addIntraDirectVFEdge(getDef(actualParm->getParam()), nodeId);
            }

        } else if (auto *formalParm = llvm::dyn_cast<FormalParmVFGNode>(node)) {

            for (auto it = formalParm->callPEBegin(),
                      eit = formalParm->callPEEnd();
                 it != eit; ++it) {
                const CallBlockNode *cs = (*it)->getCallSite();
                ActualParmVFGNode *acutalParm =
                    getActualParmVFGNode((*it)->getSrcNode(), cs);

                addInterEdgeFromAPToFP(acutalParm, formalParm,
                                       getCallSiteID(cs, formalParm->getFun()));
            }
        } else if (auto *calleeRet = llvm::dyn_cast<FormalRetVFGNode>(node)) {
            /// connect formal ret to its definition node
            addIntraDirectVFEdge(getDef(calleeRet->getRet()), nodeId);

            /// connect formal ret to actual ret
            for (auto it = calleeRet->retPEBegin(), eit = calleeRet->retPEEnd();
                 it != eit; ++it) {

                ActualRetVFGNode *callsiteRev =
                    getActualRetVFGNode((*it)->getDstNode());
                const CallBlockNode *retBlockNode = (*it)->getCallSite();
                CallBlockNode *callBlockNode = pag->getICFG()->getCallBlockNode(
                    retBlockNode->getCallSite());

                addInterEdgeFromFRToAR(
                    calleeRet, callsiteRev,
                    getCallSiteID(callBlockNode, calleeRet->getFun()));
            }
        }
        /// Do not process FormalRetVFGNode, as they are connected by copy
        /// within callee We assume one procedure only has unique return
    }

    /// connect direct value-flow edges (parameter passing) for thread fork/join
    /// add fork edge
    PAGEdge::PAGEdgeSetTy &forks = getPAGEdgeSet(PAGEdge::ThreadFork);
    for (auto *fork : forks) {
        auto *forkedge = llvm::cast<TDForkPE>(fork);
        ActualParmVFGNode *acutalParm = getActualParmVFGNode(
            forkedge->getSrcNode(), forkedge->getCallSite());
        FormalParmVFGNode *formalParm =
            getFormalParmVFGNode(forkedge->getDstNode());
        addInterEdgeFromAPToFP(
            acutalParm, formalParm,
            getCallSiteID(forkedge->getCallSite(), formalParm->getFun()));
    }
    /// add join edge
    PAGEdge::PAGEdgeSetTy &joins = getPAGEdgeSet(PAGEdge::ThreadJoin);
    for (auto *join : joins) {
        auto *joinedge = llvm::cast<TDJoinPE>(join);
        NodeID callsiteRev = getDef(joinedge->getDstNode());
        FormalRetVFGNode *calleeRet =
            getFormalRetVFGNode(joinedge->getSrcNode());
        addRetEdge(calleeRet->getId(), callsiteRev,
                   getCallSiteID(joinedge->getCallSite(), calleeRet->getFun()));
    }
}

/*!
 * Whether VFG has an intra VFG edge
 */
VFGEdge *VFG::hasIntraVFGEdge(VFGNode *src, VFGNode *dst,
                              VFGEdge::VFGEdgeK kind) {
    VFGEdge edge(src, dst, getDummyEdgeId(), kind);
    VFGEdge *outEdge = src->hasOutgoingEdge(&edge);
    VFGEdge *inEdge = dst->hasIncomingEdge(&edge);
    if (outEdge && inEdge) {
        assert(outEdge == inEdge && "edges not match");
        return outEdge;
    }

    return nullptr;
}

/*!
 * Whether we has an thread VFG edge
 */
VFGEdge *VFG::hasThreadVFGEdge(VFGNode *src, VFGNode *dst,
                               VFGEdge::VFGEdgeK kind) {
    VFGEdge edge(src, dst, getDummyEdgeId(), kind);
    VFGEdge *outEdge = src->hasOutgoingEdge(&edge);
    VFGEdge *inEdge = dst->hasIncomingEdge(&edge);
    if (outEdge && inEdge) {
        assert(outEdge == inEdge && "edges not match");
        return outEdge;
    }

    return nullptr;
}

/*!
 * Whether we has an inter VFG edge
 */
VFGEdge *VFG::hasInterVFGEdge(VFGNode *src, VFGNode *dst,
                              VFGEdge::VFGEdgeK kind, CallSiteID csId) {
    VFGEdge edge(src, dst, getDummyEdgeId(),
                 VFGEdge::makeEdgeFlagWithAuxInfo(kind, csId));
    VFGEdge *outEdge = src->hasOutgoingEdge(&edge);
    VFGEdge *inEdge = dst->hasIncomingEdge(&edge);
    if (outEdge && inEdge) {
        assert(outEdge == inEdge && "edges not match");
        return outEdge;
    }

    return nullptr;
}

/*!
 * Return the corresponding VFGEdge
 */
VFGEdge *VFG::getIntraVFGEdge(const VFGNode *src, const VFGNode *dst,
                              VFGEdge::VFGEdgeK kind) {
    return hasIntraVFGEdge(const_cast<VFGNode *>(src),
                           const_cast<VFGNode *>(dst), kind);
}

/*!
 * Dump VFG
 */
void VFG::dump(const std::string &file, bool simple) {
    GraphPrinter::WriteGraphToFile(outs(), file, this, simple);
}

void VFG::updateCallGraph(PointerAnalysis *pta) {
    VFGEdgeSetTy vfEdgesAtIndCallSite;

    for (const auto &iter : pta->getIndCallMap()) {
        const CallBlockNode *newcs = iter.first;
        assert(newcs->isIndirectCall() && "this is not an indirect call?");
        const PointerAnalysis::FunctionSet &functions = iter.second;
        for (const auto *func : functions) {
            connectCallerAndCallee(newcs, func, vfEdgesAtIndCallSite);
        }
    }
}

/**
 * Connect actual params/return to formal params/return for top-level variables.
 * Also connect indirect actual in/out and formal in/out.
 */
void VFG::connectCallerAndCallee(const CallBlockNode *callBlockNode,
                                 const SVFFunction *callee,
                                 VFGEdgeSetTy &edges) {
    ICFG *icfg = pag->getICFG();
    CallSiteID csId = getCallSiteID(callBlockNode, callee);
    RetBlockNode *retBlockNode =
        icfg->getRetBlockNode(callBlockNode->getCallSite());
    // connect actual and formal param
    if (pag->hasCallSiteArgsMap(callBlockNode) && pag->hasFunArgsList(callee)) {
        const PAG::PAGNodeList &csArgList =
            pag->getCallSiteArgsList(callBlockNode);
        const PAG::PAGNodeList &funArgList = pag->getFunArgsList(callee);
        auto csArgIt = csArgList.begin();
        auto csArgEit = csArgList.end();
        auto funArgIt = funArgList.begin();
        auto funArgEit = funArgList.end();
        for (; funArgIt != funArgEit && csArgIt != csArgEit;
             funArgIt++, csArgIt++) {
            const PAGNode *cs_arg = *csArgIt;
            const PAGNode *fun_arg = *funArgIt;
            if (fun_arg->isPointer() && cs_arg->isPointer()) {
                connectAParamAndFParam(cs_arg, fun_arg, callBlockNode, csId,
                                       edges);
            }
        }
        assert(funArgIt == funArgEit &&
               "function has more arguments than call site");
        if (callee->getLLVMFun()->isVarArg()) {
            NodeID varFunArg = pag->getVarargNode(callee);
            const PAGNode *varFunArgNode = pag->getGNode(varFunArg);
            if (varFunArgNode->isPointer()) {
                for (; csArgIt != csArgEit; csArgIt++) {
                    const PAGNode *cs_arg = *csArgIt;
                    if (cs_arg->isPointer()) {
                        connectAParamAndFParam(cs_arg, varFunArgNode,
                                               callBlockNode, csId, edges);
                    }
                }
            }
        }
    }

    // connect actual return and formal return
    if (pag->funHasRet(callee) && pag->callsiteHasRet(retBlockNode)) {
        const PAGNode *cs_return = pag->getCallSiteRet(retBlockNode);
        const PAGNode *fun_return = pag->getFunRet(callee);
        if (cs_return->isPointer() && fun_return->isPointer()) {
            connectFRetAndARet(fun_return, cs_return, csId, edges);
        }
    }
}

/*!
 * Given a VFG node, return its left hand side top level pointer
 */
const PAGNode *VFG::getLHSTopLevPtr(const VFGNode *node) const {

    if (const auto *addr = llvm::dyn_cast<AddrVFGNode>(node)) {
        return addr->getPAGDstNode();
    }

    if (const auto *copy = llvm::dyn_cast<CopyVFGNode>(node)) {
        return copy->getPAGDstNode();
    }

    if (const auto *gep = llvm::dyn_cast<GepVFGNode>(node)) {
        return gep->getPAGDstNode();
    }

    if (const auto *load = llvm::dyn_cast<LoadVFGNode>(node)) {
        return load->getPAGDstNode();
    }

    if (const auto *phi = llvm::dyn_cast<PHIVFGNode>(node)) {
        return phi->getRes();
    }

    if (const auto *cmp = llvm::dyn_cast<CmpVFGNode>(node)) {
        return cmp->getRes();
    }

    if (const auto *bop = llvm::dyn_cast<BinaryOPVFGNode>(node)) {
        return bop->getRes();
    }

    if (const auto *uop = llvm::dyn_cast<UnaryOPVFGNode>(node)) {
        return uop->getRes();
    }

    if (const auto *ap = llvm::dyn_cast<ActualParmVFGNode>(node)) {
        return ap->getParam();
    }

    if (const auto *fp = llvm::dyn_cast<FormalParmVFGNode>(node)) {
        return fp->getParam();
    }

    if (const auto *ar = llvm::dyn_cast<ActualRetVFGNode>(node)) {
        return ar->getRev();
    }

    if (const auto *fr = llvm::dyn_cast<FormalRetVFGNode>(node)) {
        return fr->getRet();
    }

    if (const auto *nullVFG = llvm::dyn_cast<NullPtrVFGNode>(node)) {
        return nullVFG->getPAGNode();
    }

    assert(false && "unexpected node kind!");
    return nullptr;
}

/*!
 * Whether this is an function entry VFGNode (formal parameter, formal In)
 */
const SVFFunction *VFG::isFunEntryVFGNode(const VFGNode *node) const {
    if (const FormalParmVFGNode *fp = llvm::dyn_cast<FormalParmVFGNode>(node)) {
        return fp->getFun();
    } else if (const InterPHIVFGNode *phi =
                   llvm::dyn_cast<InterPHIVFGNode>(node)) {
        if (phi->isFormalParmPHI()) {
            return phi->getFun();
        }
    }
    return nullptr;
}

/*!
 * GraphTraits specialization
 */
namespace llvm {
template <>
struct DOTGraphTraits<VFG *> : public DOTGraphTraits<PAG *> {

    using NodeType = VFGNode;
    DOTGraphTraits(bool isSimple = false) : DOTGraphTraits<PAG *>(isSimple) {}

    /// Return name of the graph
    static std::string getGraphName(VFG *) { return "VFG"; }

    std::string getNodeLabel(NodeType *node, VFG *graph) {
        if (isSimple()) {
            return getSimpleNodeLabel(node, graph);
        }

        return getCompleteNodeLabel(node, graph);
    }

    /// Return label of a VFG node without MemSSA information
    static std::string getSimpleNodeLabel(NodeType *node, VFG *) {
        std::string str;
        raw_string_ostream rawstr(str);
        if (auto *stmtNode = llvm::dyn_cast<StmtVFGNode>(node)) {
            rawstr << stmtNode->toString();
        } else if (auto *tphi = llvm::dyn_cast<PHIVFGNode>(node)) {
            rawstr << tphi->toString();
        } else if (auto *fp = llvm::dyn_cast<FormalParmVFGNode>(node)) {
            rawstr << fp->toString();
        } else if (auto *ap = llvm::dyn_cast<ActualParmVFGNode>(node)) {
            rawstr << ap->toString();
        } else if (auto *ar = llvm::dyn_cast<ActualRetVFGNode>(node)) {
            rawstr << ar->toString();
        } else if (auto *fr = llvm::dyn_cast<FormalRetVFGNode>(node)) {
            rawstr << fr->toString();
        } else if (llvm::isa<NullPtrVFGNode>(node)) {
            rawstr << "NullPtr";
        } else if (auto *bop = llvm::dyn_cast<BinaryOPVFGNode>(node)) {
            rawstr << bop->toString();
        } else if (auto *uop = llvm::dyn_cast<UnaryOPVFGNode>(node)) {
            rawstr << uop->toString();
        } else if (auto *cmp = llvm::dyn_cast<CmpVFGNode>(node)) {
            rawstr << cmp->toString();
            ;
        } else {
            assert(false && "what else kinds of nodes do we have??");
        }

        return rawstr.str();
    }

    /// Return label of a VFG node with MemSSA information
    static std::string getCompleteNodeLabel(NodeType *node, VFG *) {

        std::string str;
        raw_string_ostream rawstr(str);
        if (auto *stmtNode = llvm::dyn_cast<StmtVFGNode>(node)) {
            rawstr << stmtNode->toString();
        } else if (auto *bop = llvm::dyn_cast<BinaryOPVFGNode>(node)) {
            rawstr << bop->toString();
        } else if (auto *uop = llvm::dyn_cast<UnaryOPVFGNode>(node)) {
            rawstr << uop->toString();
        } else if (auto *cmp = llvm::dyn_cast<CmpVFGNode>(node)) {
            rawstr << cmp->toString();
        } else if (auto *phi = llvm::dyn_cast<PHIVFGNode>(node)) {
            rawstr << phi->toString();
        } else if (auto *fp = llvm::dyn_cast<FormalParmVFGNode>(node)) {
            rawstr << fp->toString();
        } else if (auto *ap = llvm::dyn_cast<ActualParmVFGNode>(node)) {
            rawstr << ap->toString();
        } else if (auto *nptr = llvm::dyn_cast<NullPtrVFGNode>(node)) {
            rawstr << nptr->toString();
        } else if (auto *ar = llvm::dyn_cast<ActualRetVFGNode>(node)) {
            rawstr << ar->toString();
        } else if (auto *fr = llvm::dyn_cast<FormalRetVFGNode>(node)) {
            rawstr << fr->toString();
        } else {
            assert(false && "what else kinds of nodes do we have??");
        }

        return rawstr.str();
    }

    static std::string getNodeAttributes(NodeType *node, VFG *) {
        std::string str;
        raw_string_ostream rawstr(str);

        if (auto *stmtNode = llvm::dyn_cast<StmtVFGNode>(node)) {
            const PAGEdge *edge = stmtNode->getPAGEdge();
            if (llvm::isa<AddrPE>(edge)) {
                rawstr << "color=green";
            } else if (llvm::isa<CopyPE>(edge)) {
                rawstr << "color=black";
            } else if (llvm::isa<RetPE>(edge)) {
                rawstr << "color=black,style=dotted";
            } else if (llvm::isa<GepPE>(edge)) {
                rawstr << "color=purple";
            } else if (llvm::isa<StorePE>(edge)) {
                rawstr << "color=blue";
            } else if (llvm::isa<LoadPE>(edge)) {
                rawstr << "color=red";
            } else {
                assert(0 && "No such kind edge!!");
            }
            rawstr << "";
        } else if (llvm::isa<CmpVFGNode>(node)) {
            rawstr << "color=grey";
        } else if (llvm::isa<BinaryOPVFGNode>(node)) {
            rawstr << "color=grey";
        } else if (llvm::isa<UnaryOPVFGNode>(node)) {
            rawstr << "color=grey";
        } else if (llvm::isa<PHIVFGNode>(node)) {
            rawstr << "color=black";
        } else if (llvm::isa<NullPtrVFGNode>(node)) {
            rawstr << "color=grey";
        } else if (llvm::isa<FormalParmVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<ActualParmVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<ActualRetVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<FormalRetVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else {
            assert(false && "no such kind of node!!");
        }

        rawstr << "";

        return rawstr.str();
    }

    template <class EdgeIter>
    static std::string getEdgeAttributes(NodeType *, EdgeIter EI, VFG *) {
        VFGEdge *edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");
        if (llvm::isa<DirectSVFGEdge>(edge)) {
            if (llvm::isa<CallDirSVFGEdge>(edge)) {
                return "style=solid,color=red";
            }

            if (llvm::isa<RetDirSVFGEdge>(edge)) {
                return "style=solid,color=blue";
            }

            return "style=solid";
        }

        assert(false && "what else edge we have?");
        return "";
    }

    template <class EdgeIter>
    static std::string getEdgeSourceLabel(NodeType *, EdgeIter EI) {
        VFGEdge *edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");

        std::string str;
        raw_string_ostream rawstr(str);
        if (auto *dirCall = llvm::dyn_cast<CallDirSVFGEdge>(edge)) {
            rawstr << dirCall->getCallSiteId();
        } else if (auto *dirRet = llvm::dyn_cast<RetDirSVFGEdge>(edge)) {
            rawstr << dirRet->getCallSiteId();
        }

        return rawstr.str();
    }
};
} // End namespace llvm
