//===- SVFG.cpp -- Sparse value-flow graph-----------------------------------//
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
 * SVFG.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Yulei Sui
 */

#include "Graphs/SVFG.h"
#include "Graphs/SVFGOPT.h"
#include "Graphs/SVFGStat.h"
#include "Graphs/VFG.h"
#include "SVF-FE/LLVMUtil.h"
#include "Util/SVFModule.h"
#include <llvm/Demangle/Demangle.h>

using namespace SVF;
using namespace SVFUtil;

bool IndirectSVFGEdge::classof(const GenericVFGEdgeTy *edge) {
    return edge->getEdgeKind() == VFGEdge::IndirectVF ||
           IntraIndSVFGEdge::classof(edge) || CallIndSVFGEdge::classof(edge) ||
           RetIndSVFGEdge::classof(edge) || ThreadMHPIndSVFGEdge::classof(edge);
}

bool MRSVFGNode::classof(const GenericVFGNodeTy *node) {
    return node->getNodeKind() == MRS || FormalINSVFGNode::classof(node) ||
           FormalOUTSVFGNode::classof(node) ||
           ActualINSVFGNode::classof(node) ||
           ActualOUTSVFGNode::classof(node) || MSSAPHISVFGNode::classof(node);
}

bool MSSAPHISVFGNode::classof(const GenericVFGNodeTy *node) {
    return node->getNodeKind() == MPhi || IntraMSSAPHISVFGNode::classof(node) ||
           InterMSSAPHISVFGNode::classof(node);
}

const std::string MRSVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "MRSVFGNode ID: " << getId();
    return rawstr.str();
}

const std::string FormalINSVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "FormalINSVFGNode ID: " << getId()
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    rawstr << getEntryChi()->getMR()->getMRID() << "V_"
           << getEntryChi()->getResVer()->getSSAVersion() << " = ENCHI(MR_"
           << getEntryChi()->getMR()->getMRID() << "V_"
           << getEntryChi()->getOpVer()->getSSAVersion() << ")\n";
    rawstr << getEntryChi()->getMR()->dumpStr() << "\n";
    return rawstr.str();
}

const std::string FormalOUTSVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "FormalOUTSVFGNode ID: " << getId()
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    rawstr << "RETMU(" << getRetMU()->getMR()->getMRID() << "V_"
           << getRetMU()->getVer()->getSSAVersion() << ")\n";
    rawstr << getRetMU()->getMR()->dumpStr() << "\n";
    return rawstr.str();
}

const std::string ActualINSVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "ActualINSVFGNode ID: " << getId()
           << " at callsite: " << *getCallSite()->getCallSite()
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    rawstr << "CSMU(" << getCallMU()->getMR()->getMRID() << "V_"
           << getCallMU()->getVer()->getSSAVersion() << ")\n";
    rawstr << getCallMU()->getMR()->dumpStr() << "\n";
    rawstr << "CS[" << getSourceLoc(getCallSite()->getCallSite()) << "]";
    return rawstr.str();
}

const std::string ActualOUTSVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "ActualOUTSVFGNode ID: " << getId()
           << " at callsite: " << *getCallSite()->getCallSite()
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    rawstr << getCallCHI()->getMR()->getMRID() << "V_"
           << getCallCHI()->getResVer()->getSSAVersion() << " = CSCHI(MR_"
           << getCallCHI()->getMR()->getMRID() << "V_"
           << getCallCHI()->getOpVer()->getSSAVersion() << ")\n";
    rawstr << getCallCHI()->getMR()->dumpStr() << "\n";
    rawstr << "CS[" << getSourceLoc(getCallSite()->getCallSite()) << "]";
    return rawstr.str();
}

const std::string MSSAPHISVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "MSSAPHISVFGNode ID: " << getId()
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    rawstr << "MR_" << getRes()->getMR()->getMRID() << "V_"
           << getRes()->getResVer()->getSSAVersion() << " = PHI(";
    for (auto it = opVerBegin(), eit = opVerEnd(); it != eit; it++) {
        rawstr << "MR_" << it->second->getMR()->getMRID() << "V_"
               << it->second->getSSAVersion() << ", ";
    }
    rawstr << ")\n";

    rawstr << getRes()->getMR()->dumpStr();
    rawstr << getSourceLoc(&getICFGNode()->getBB()->back());
    return rawstr.str();
}

const std::string IntraMSSAPHISVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "IntraMSSAPHISVFGNode ID: " << getId()
           << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    rawstr << MSSAPHISVFGNode::toString();
    return rawstr.str();
}

const std::string InterMSSAPHISVFGNode::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    if (isFormalINPHI()) {
        rawstr << "FormalINPHISVFGNode ID: " << getId()
               << " {fun: " << getFun()->getName() << "}";
    } else {
        rawstr << "ActualOUTPHISVFGNode ID: " << getId()
               << " at callsite: " << *getCallSite()->getCallSite()
               << " {fun: " << llvm::demangle(getFun()->getName().str()) << "}";
    }
    rawstr << MSSAPHISVFGNode::toString();
    return rawstr.str();
}

const std::string IndirectSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "IndirectSVFGEdge: " << getDstID() << "<--" << getSrcID() << "\n";
    return rawstr.str();
}

const std::string IntraIndSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "IntraIndSVFGEdge: " << getDstID() << "<--" << getSrcID() << "\n";
    return rawstr.str();
}

const std::string CallIndSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "CallIndSVFGEdge CallSite ID: " << getCallSiteId() << " ";
    rawstr << getDstID() << "<--" << getSrcID() << "\n";
    return rawstr.str();
}

const std::string RetIndSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "RetIndSVFGEdge CallSite ID: " << getCallSiteId() << " ";
    rawstr << getDstID() << "<--" << getSrcID() << "\n";
    return rawstr.str();
}

const std::string ThreadMHPIndSVFGEdge::toString() const {
    std::string str;
    raw_string_ostream rawstr(str);
    rawstr << "ThreadMHPIndSVFGEdge: " << getDstID() << "<--" << getSrcID()
           << "\n";
    return rawstr.str();
}

FormalOUTSVFGNode::FormalOUTSVFGNode(NodeID id, const MemSSA::RETMU *exit)
    : MRSVFGNode(id, FPOUT), mu(exit) {
    cpts = exit->getMR()->getPointsTo();
}

/*!
 * Constructor
 */
SVFG::SVFG(MemSSA *_mssa, PAG *pag, VFGK k)
    : VFG(_mssa->getPTA()->getPTACallGraph(), pag, k), mssa(_mssa),
      pta(mssa->getPTA()) {
    stat = new SVFGStat(this);
}

/*!
 * Memory has been cleaned up at GenericGraph
 */
void SVFG::destroy() {
    delete stat;
    stat = nullptr;
    clearMSSA();
}

/*!
 * Build SVFG
 * 1) build SVFG nodes
 *    a) statements for top level pointers (PAGEdges)
 *    b) operators of address-taken variables (MSSAPHI and MSSACHI)
 * 2) connect SVFG edges
 *    a) between two statements (PAGEdges)
 *    b) between two memory SSA operators (MSSAPHI MSSAMU and MSSACHI)
 */

void SVFG::buildSVFG() {
    DBOUT(DGENERAL, outs() << pasMsg("Build Sparse Value-Flow Graph \n"));
    stat->startClk();

    DBOUT(DGENERAL, outs() << pasMsg("\tCreate SVFG Addr-taken Node\n"));

    stat->ATVFNodeStart();
    addSVFGNodesForAddrTakenVars();
    stat->ATVFNodeEnd();

    DBOUT(DGENERAL, outs() << pasMsg("\tCreate SVFG Indirect Edge\n"));

    stat->indVFEdgeStart();
    connectIndirectSVFGEdges();
    stat->indVFEdgeEnd();
}

/*
 * Create SVFG nodes for address-taken variables
 */
void SVFG::addSVFGNodesForAddrTakenVars() {

    // set defs for address-taken vars defined at store statements
    PAGEdge::PAGEdgeSetTy &stores = getPAGEdgeSet(PAGEdge::Store);
    for (auto *iter : stores) {
        auto *store = llvm::cast<StorePE>(iter);
        const StmtSVFGNode *sNode = getStmtVFGNode(store);
        for (auto *pi : mssa->getCHISet(store)) {
            setDef(pi->getResVer(), sNode);
        }
    }

    /// set defs for address-taken vars defined at phi/chi/call
    /// create corresponding def and use nodes for address-taken vars (a.k.a
    /// MRVers) initialize memory SSA phi nodes (phi of address-taken variables)
    for (auto &it : mssa->getBBToPhiSetMap()) {
        for (auto *pi : it.second) {
            addIntraMSSAPHISVFGNode(pi);
        }
    }
    /// initialize memory SSA entry chi nodes
    for (auto &it : mssa->getFunToEntryChiSetMap()) {
        for (auto *pi : it.second) {
            addFormalINSVFGNode(llvm::cast<ENTRYCHI>(pi));
        }
    }
    /// initialize memory SSA return mu nodes
    for (auto &it : mssa->getFunToRetMuSetMap()) {
        for (auto *pi : it.second) {
            addFormalOUTSVFGNode(llvm::cast<RETMU>(pi));
        }
    }
    /// initialize memory SSA callsite mu nodes
    for (auto &it : mssa->getCallSiteToMuSetMap()) {
        for (auto *pi : it.second) {
            addActualINSVFGNode(llvm::cast<CALLMU>(pi));
        }
    }
    /// initialize memory SSA callsite chi nodes
    for (auto &it : mssa->getCallSiteToChiSetMap()) {
        for (auto *pi : it.second) {
            addActualOUTSVFGNode(llvm::cast<CALLCHI>(pi));
        }
    }
}

/*
 * Connect def-use chains for indirect value-flow, (value-flow of address-taken
 * variables)
 */
void SVFG::connectIndirectSVFGEdges() {

    for (auto it = begin(), eit = end(); it != eit; ++it) {
        NodeID nodeId = it->first;
        const SVFGNode *node = it->second;
        if (const auto *loadNode = llvm::dyn_cast<LoadSVFGNode>(node)) {
            MUSet &muSet =
                mssa->getMUSet(llvm::cast<LoadPE>(loadNode->getPAGEdge()));
            for (auto *it : muSet) {
                if (auto *mu = llvm::dyn_cast<LOADMU>(it)) {
                    NodeID def = getDef(mu->getVer());
                    addIntraIndirectVFEdge(
                        def, nodeId, mu->getVer()->getMR()->getPointsTo());
                }
            }
        } else if (const auto *storeNode =
                       llvm::dyn_cast<StoreSVFGNode>(node)) {
            CHISet &chiSet =
                mssa->getCHISet(llvm::cast<StorePE>(storeNode->getPAGEdge()));
            for (auto *it : chiSet) {
                if (STORECHI *chi = llvm::dyn_cast<STORECHI>(it)) {
                    NodeID def = getDef(chi->getOpVer());
                    addIntraIndirectVFEdge(
                        def, nodeId, chi->getOpVer()->getMR()->getPointsTo());
                }
            }
        } else if (const auto *formalIn =
                       llvm::dyn_cast<FormalINSVFGNode>(node)) {
            PTACallGraphEdge::CallInstSet callInstSet;
            auto *ptaCG = mssa->getPTA()->getPTACallGraph();
            ptaCG->getDirCallSitesInvokingCallee(
                formalIn->getEntryChi()->getFunction(), callInstSet);
            for (const auto *cs : callInstSet) {
                if (!mssa->hasMU(cs)) {
                    continue;
                }
                ActualINSVFGNodeSet &actualIns = getActualINSVFGNodes(cs);
                for (auto ait : actualIns) {
                    const auto *actualIn =
                        llvm::cast<ActualINSVFGNode>(getSVFGNode(ait));
                    addInterIndirectVFCallEdge(
                        actualIn, formalIn,
                        getCallSiteID(cs, formalIn->getFun()));
                }
            }
        } else if (const auto *formalOut =
                       llvm::dyn_cast<FormalOUTSVFGNode>(node)) {
            PTACallGraphEdge::CallInstSet callInstSet;
            const MemSSA::RETMU *retMu = formalOut->getRetMU();
            mssa->getPTA()->getPTACallGraph()->getDirCallSitesInvokingCallee(
                retMu->getFunction(), callInstSet);
            for (const auto *cs : callInstSet) {
                if (!mssa->hasCHI(cs)) {
                    continue;
                }
                ActualOUTSVFGNodeSet &actualOuts = getActualOUTSVFGNodes(cs);
                for (auto ait : actualOuts) {
                    const ActualOUTSVFGNode *actualOut =
                        llvm::cast<ActualOUTSVFGNode>(getSVFGNode(ait));
                    addInterIndirectVFRetEdge(
                        formalOut, actualOut,
                        getCallSiteID(cs, formalOut->getFun()));
                }
            }
            NodeID def = getDef(retMu->getVer());
            addIntraIndirectVFEdge(def, nodeId,
                                   retMu->getVer()->getMR()->getPointsTo());
        } else if (const auto *actualIn =
                       llvm::dyn_cast<ActualINSVFGNode>(node)) {
            auto ver = actualIn->getCallMU()->getVer();
            NodeID def = getDef(ver);
            addIntraIndirectVFEdge(def, nodeId, ver->getMR()->getPointsTo());
        } else if (llvm::isa<ActualOUTSVFGNode>(node)) {
            /// There's no need to connect actual out node to its definition
            /// site in the same function.
        } else if (const auto *phiNode =
                       llvm::dyn_cast<MSSAPHISVFGNode>(node)) {
            for (auto it = phiNode->opVerBegin(), eit = phiNode->opVerEnd();
                 it != eit; it++) {
                auto op = it->second;
                NodeID def = getDef(op);
                addIntraIndirectVFEdge(def, nodeId, op->getMR()->getPointsTo());
            }
        }
    }

    connectFromGlobalToProgEntry();
}

/*!
 * Connect indirect SVFG edges from global initializers (store) to main function
 * entry
 */
void SVFG::connectFromGlobalToProgEntry() {
    SVFModule *svfModule = mssa->getPTA()->getModule();
    const SVFFunction *mainFunc = SVFUtil::getProgEntryFunction(svfModule);
    FormalINSVFGNodeSet &formalIns = getFormalINSVFGNodes(mainFunc);
    if (formalIns.empty()) {
        return;
    }

    for (const auto *globalVFGNode : globalVFGNodes) {
        if (const auto *store = llvm::dyn_cast<StoreSVFGNode>(globalVFGNode)) {
            /// connect this store to main function entry
            const PointsTo &storePts =
                mssa->getPTA()->getPts(store->getPAGDstNodeID());

            for (auto fiIt : formalIns) {
                NodeID formalInID = fiIt;
                PointsTo formalInPts =
                    ((FormalINSVFGNode *)getSVFGNode(formalInID))
                        ->getPointsTo();

                formalInPts &= storePts;
                if (formalInPts.empty()) {
                    continue;
                }

                /// add indirect value flow edge
                addIntraIndirectVFEdge(store->getId(), formalInID, formalInPts);
            }
        }
    }
}

/*
 *  Add def-use edges of a memory region between two statements
 */
SVFGEdge *SVFG::addIntraIndirectVFEdge(NodeID srcId, NodeID dstId,
                                       const PointsTo &cpts) {
    SVFGNode *srcNode = getSVFGNode(srcId);
    SVFGNode *dstNode = getSVFGNode(dstId);
    checkIntraEdgeParents(srcNode, dstNode);
    if (SVFGEdge *edge =
            hasIntraVFGEdge(srcNode, dstNode, SVFGEdge::IntraIndirectVF)) {
        assert(llvm::isa<IndirectSVFGEdge>(edge) &&
               "this should be a indirect value flow edge!");
        return (llvm::cast<IndirectSVFGEdge>(edge)->addPointsTo(cpts)
                    ? edge
                    : nullptr);
    }

    auto *indirectEdge =
        new IntraIndSVFGEdge(srcNode, dstNode, getNextEdgeId());
    indirectEdge->addPointsTo(cpts);
    return (addSVFGEdge(indirectEdge) ? indirectEdge : nullptr);
}

/*!
 * Add def-use edges of a memory region between two may-happen-in-parallel
 * statements for multithreaded program
 */
SVFGEdge *SVFG::addThreadMHPIndirectVFEdge(NodeID srcId, NodeID dstId,
                                           const PointsTo &cpts) {
    SVFGNode *srcNode = getSVFGNode(srcId);
    SVFGNode *dstNode = getSVFGNode(dstId);
    if (SVFGEdge *edge =
            hasThreadVFGEdge(srcNode, dstNode, SVFGEdge::TheadMHPIndirectVF)) {
        assert(llvm::isa<IndirectSVFGEdge>(edge) &&
               "this should be a indirect value flow edge!");
        return (llvm::cast<IndirectSVFGEdge>(edge)->addPointsTo(cpts)
                    ? edge
                    : nullptr);
    }

    auto *indirectEdge =
        new ThreadMHPIndSVFGEdge(srcNode, dstNode, getNextEdgeId());
    indirectEdge->addPointsTo(cpts);
    return (addSVFGEdge(indirectEdge) ? indirectEdge : nullptr);
}

/*
 *  Add def-use call edges of a memory region between two statements
 */
SVFGEdge *SVFG::addCallIndirectVFEdge(NodeID srcId, NodeID dstId,
                                      const PointsTo &cpts, CallSiteID csId) {
    SVFGNode *srcNode = getSVFGNode(srcId);
    SVFGNode *dstNode = getSVFGNode(dstId);
    if (SVFGEdge *edge =
            hasInterVFGEdge(srcNode, dstNode, SVFGEdge::CallIndVF, csId)) {
        assert(llvm::isa<CallIndSVFGEdge>(edge) &&
               "this should be a indirect value flow edge!");
        return (llvm::cast<CallIndSVFGEdge>(edge)->addPointsTo(cpts) ? edge
                                                                     : nullptr);
    }

    auto *callEdge =
        new CallIndSVFGEdge(srcNode, dstNode, getNextEdgeId(), csId);
    callEdge->addPointsTo(cpts);
    return (addSVFGEdge(callEdge) ? callEdge : nullptr);
}

/*
 *  Add def-use return edges of a memory region between two statements
 */
SVFGEdge *SVFG::addRetIndirectVFEdge(NodeID srcId, NodeID dstId,
                                     const PointsTo &cpts, CallSiteID csId) {
    SVFGNode *srcNode = getSVFGNode(srcId);
    SVFGNode *dstNode = getSVFGNode(dstId);
    if (SVFGEdge *edge =
            hasInterVFGEdge(srcNode, dstNode, SVFGEdge::RetIndVF, csId)) {
        assert(llvm::isa<RetIndSVFGEdge>(edge) &&
               "this should be a indirect value flow edge!");
        return (llvm::cast<RetIndSVFGEdge>(edge)->addPointsTo(cpts) ? edge
                                                                    : nullptr);
    }

    auto *retEdge = new RetIndSVFGEdge(srcNode, dstNode, getNextEdgeId(), csId);
    retEdge->addPointsTo(cpts);
    return (addSVFGEdge(retEdge) ? retEdge : nullptr);
}

/*!
 *
 */
SVFGEdge *SVFG::addInterIndirectVFCallEdge(const ActualINSVFGNode *src,
                                           const FormalINSVFGNode *dst,
                                           CallSiteID csId) {
    PointsTo cpts1 = src->getPointsTo();
    PointsTo cpts2 = dst->getPointsTo();
    if (cpts1.intersects(cpts2)) {
        cpts1 &= cpts2;
        return addCallIndirectVFEdge(src->getId(), dst->getId(), cpts1, csId);
    }
    return nullptr;
}

/*!
 * Add inter VF edge from function exit mu to callsite chi
 */
SVFGEdge *SVFG::addInterIndirectVFRetEdge(const FormalOUTSVFGNode *src,
                                          const ActualOUTSVFGNode *dst,
                                          CallSiteID csId) {

    PointsTo cpts1 = src->getPointsTo();
    PointsTo cpts2 = dst->getPointsTo();
    if (cpts1.intersects(cpts2)) {
        cpts1 &= cpts2;
        return addRetIndirectVFEdge(src->getId(), dst->getId(), cpts1, csId);
    }
    return nullptr;
}

/*!
 * Dump SVFG
 */
void SVFG::dump(const std::string &file, bool simple) {
    GraphPrinter::WriteGraphToFile(outs(), file, this, simple);
}

/**
 * Get all inter value flow edges at this indirect call site, including call and
 * return edges.
 */
void SVFG::getInterVFEdgesForIndirectCallSite(
    const CallBlockNode *callBlockNode, const SVFFunction *callee,
    SVFGEdgeSetTy &edges) {
    CallSiteID csId = getCallSiteID(callBlockNode, callee);
    RetBlockNode *retBlockNode =
        pag->getICFG()->getRetBlockNode(callBlockNode->getCallSite());

    // Find inter direct call edges between actual param and formal param.
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
                getInterVFEdgeAtIndCSFromAPToFP(cs_arg, fun_arg, callBlockNode,
                                                csId, edges);
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
                        getInterVFEdgeAtIndCSFromAPToFP(
                            cs_arg, varFunArgNode, callBlockNode, csId, edges);
                    }
                }
            }
        }
    }

    // Find inter direct return edges between actual return and formal return.
    if (pag->funHasRet(callee) && pag->callsiteHasRet(retBlockNode)) {
        const PAGNode *cs_return = pag->getCallSiteRet(retBlockNode);
        const PAGNode *fun_return = pag->getFunRet(callee);
        if (cs_return->isPointer() && fun_return->isPointer()) {
            getInterVFEdgeAtIndCSFromFRToAR(fun_return, cs_return, csId, edges);
        }
    }

    // Find inter indirect call edges between actual-in and formal-in svfg
    // nodes.
    if (hasFuncEntryChi(callee) && hasCallSiteMu(callBlockNode)) {
        SVFG::ActualINSVFGNodeSet &actualInNodes =
            getActualINSVFGNodes(callBlockNode);
        for (auto ai_it : actualInNodes) {
            auto *actualIn = llvm::cast<ActualINSVFGNode>(getSVFGNode(ai_it));
            getInterVFEdgeAtIndCSFromAInToFIn(actualIn, callee, edges);
        }
    }

    // Find inter indirect return edges between actual-out and formal-out svfg
    // nodes.
    if (hasFuncRetMu(callee) && hasCallSiteChi(callBlockNode)) {
        SVFG::ActualOUTSVFGNodeSet &actualOutNodes =
            getActualOUTSVFGNodes(callBlockNode);
        for (auto ao_it : actualOutNodes) {
            auto *actualOut = llvm::cast<ActualOUTSVFGNode>(getSVFGNode(ao_it));
            getInterVFEdgeAtIndCSFromFOutToAOut(actualOut, callee, edges);
        }
    }
}

/**
 * Connect actual params/return to formal params/return for top-level variables.
 * Also connect indirect actual in/out and formal in/out.
 */
void SVFG::connectCallerAndCallee(const CallBlockNode *cs,
                                  const SVFFunction *callee,
                                  SVFGEdgeSetTy &edges) {
    VFG::connectCallerAndCallee(cs, callee, edges);

    CallSiteID csId = getCallSiteID(cs, callee);

    // connect actual in and formal in
    if (hasFuncEntryChi(callee) && hasCallSiteMu(cs)) {
        SVFG::ActualINSVFGNodeSet &actualInNodes = getActualINSVFGNodes(cs);
        const SVFG::FormalINSVFGNodeSet &formalInNodes =
            getFormalINSVFGNodes(callee);
        for (auto ai_it : actualInNodes) {
            const auto *actualIn =
                llvm::cast<ActualINSVFGNode>(getSVFGNode(ai_it));
            for (auto fi_it : formalInNodes) {
                const auto *formalIn =
                    llvm::cast<FormalINSVFGNode>(getSVFGNode(fi_it));
                connectAInAndFIn(actualIn, formalIn, csId, edges);
            }
        }
    }

    // connect actual out and formal out
    if (hasFuncRetMu(callee) && hasCallSiteChi(cs)) {
        // connect formal out and actual out
        const SVFG::FormalOUTSVFGNodeSet &formalOutNodes =
            getFormalOUTSVFGNodes(callee);
        SVFG::ActualOUTSVFGNodeSet &actualOutNodes = getActualOUTSVFGNodes(cs);
        for (auto fo_it : formalOutNodes) {
            const auto *formalOut =
                llvm::cast<FormalOUTSVFGNode>(getSVFGNode(fo_it));
            for (auto ao_it : actualOutNodes) {
                const auto *actualOut =
                    llvm::cast<ActualOUTSVFGNode>(getSVFGNode(ao_it));
                connectFOutAndAOut(formalOut, actualOut, csId, edges);
            }
        }
    }
}

/*!
 * Whether this is an function entry SVFGNode (formal parameter, formal In)
 */
const SVFFunction *SVFG::isFunEntrySVFGNode(const SVFGNode *node) const {
    if (const auto *fp = llvm::dyn_cast<FormalParmSVFGNode>(node)) {
        return fp->getFun();
    }

    if (const auto *phi = llvm::dyn_cast<InterPHISVFGNode>(node)) {
        if (phi->isFormalParmPHI()) {
            return phi->getFun();
        }
    } else if (const auto *fi = llvm::dyn_cast<FormalINSVFGNode>(node)) {
        return fi->getFun();
    }

    if (const auto *mphi = llvm::dyn_cast<InterMSSAPHISVFGNode>(node)) {
        if (mphi->isFormalINPHI()) {
            return mphi->getFun();
        }
    }

    return nullptr;
}

/*!
 * Whether this is an callsite return SVFGNode (actual return, actual out)
 */
const CallBlockNode *SVFG::isCallSiteRetSVFGNode(const SVFGNode *node) const {
    if (const auto *ar = llvm::dyn_cast<ActualRetSVFGNode>(node)) {
        return ar->getCallSite();
    }

    if (const auto *phi = llvm::dyn_cast<InterPHISVFGNode>(node)) {
        if (phi->isActualRetPHI()) {
            return phi->getCallSite();
        }
    } else if (const auto *ao = llvm::dyn_cast<ActualOUTSVFGNode>(node)) {
        return ao->getCallSite();
    }

    if (const auto *mphi = llvm::dyn_cast<InterMSSAPHISVFGNode>(node)) {
        if (mphi->isActualOUTPHI()) {
            return mphi->getCallSite();
        }
    }
    return nullptr;
}

/*!
 * Perform Statistics
 */
void SVFG::performStat() { stat->performStat(); }

/*!
 * GraphTraits specialization
 */
namespace llvm {
template <>
struct DOTGraphTraits<SVFG *> : public DOTGraphTraits<PAG *> {

    using NodeType = SVFGNode;
    DOTGraphTraits(bool isSimple = false) : DOTGraphTraits<PAG *>(isSimple) {}

    /// Return name of the graph
    static std::string getGraphName(SVFG *) { return "SVFG"; }

    std::string getNodeLabel(NodeType *node, SVFG *graph) {
        if (isSimple()) {
            return getSimpleNodeLabel(node, graph);
        }

        return getCompleteNodeLabel(node, graph);
    }

    /// Return label of a VFG node without MemSSA information
    static std::string getSimpleNodeLabel(NodeType *node, SVFG *) {
        std::string str;
        raw_string_ostream rawstr(str);

        if (auto *stmtNode = llvm::dyn_cast<StmtSVFGNode>(node)) {
            rawstr << stmtNode->toString();
        } else if (auto *tphi = llvm::dyn_cast<PHISVFGNode>(node)) {
            rawstr << tphi->toString();
        } else if (auto *fp = llvm::dyn_cast<FormalParmSVFGNode>(node)) {
            rawstr << fp->toString();
        } else if (auto *ap = llvm::dyn_cast<ActualParmSVFGNode>(node)) {
            rawstr << ap->toString();
        } else if (auto *ar = llvm::dyn_cast<ActualRetSVFGNode>(node)) {
            rawstr << ar->toString();
        } else if (auto *fr = llvm::dyn_cast<FormalRetSVFGNode>(node)) {
            rawstr << fr->toString();
        } else if (auto *fi = llvm::dyn_cast<FormalINSVFGNode>(node)) {
            rawstr << fi->toString();
        } else if (auto *fo = llvm::dyn_cast<FormalOUTSVFGNode>(node)) {
            rawstr << fo->toString();
        } else if (auto *ai = llvm::dyn_cast<ActualINSVFGNode>(node)) {
            rawstr << ai->toString();
        } else if (auto *ao = llvm::dyn_cast<ActualOUTSVFGNode>(node)) {
            rawstr << ao->toString();
        } else if (auto *mphi = llvm::dyn_cast<MSSAPHISVFGNode>(node)) {
            rawstr << mphi->toString();
        } else if (llvm::isa<NullPtrSVFGNode>(node)) {
            rawstr << "NullPtr";
        } else if (auto *bop = llvm::dyn_cast<BinaryOPVFGNode>(node)) {
            rawstr << bop->toString();
        } else if (auto *uop = llvm::dyn_cast<UnaryOPVFGNode>(node)) {
            rawstr << uop->toString();
        } else if (auto *cmp = llvm::dyn_cast<CmpVFGNode>(node)) {
            rawstr << cmp->toString();
        } else {
            assert(false && "what else kinds of nodes do we have??");
        }

        return rawstr.str();
    }

    /// Return label of a VFG node with MemSSA information
    static std::string getCompleteNodeLabel(NodeType *node, SVFG *) {

        std::string str;
        raw_string_ostream rawstr(str);
        if (auto *stmtNode = llvm::dyn_cast<StmtSVFGNode>(node)) {
            rawstr << stmtNode->toString();
        } else if (auto *bop = llvm::dyn_cast<BinaryOPVFGNode>(node)) {
            rawstr << bop->toString();
        } else if (auto *uop = llvm::dyn_cast<UnaryOPVFGNode>(node)) {
            rawstr << uop->toString();
        } else if (auto *cmp = llvm::dyn_cast<CmpVFGNode>(node)) {
            rawstr << cmp->toString();
        } else if (auto *mphi = llvm::dyn_cast<MSSAPHISVFGNode>(node)) {
            rawstr << mphi->toString();
        } else if (auto *tphi = llvm::dyn_cast<PHISVFGNode>(node)) {
            rawstr << tphi->toString();
        } else if (auto *fi = llvm::dyn_cast<FormalINSVFGNode>(node)) {
            rawstr << fi->toString();
        } else if (auto *fo = llvm::dyn_cast<FormalOUTSVFGNode>(node)) {
            rawstr << fo->toString();
        } else if (auto *fp = llvm::dyn_cast<FormalParmSVFGNode>(node)) {
            rawstr << fp->toString();
        } else if (auto *ai = llvm::dyn_cast<ActualINSVFGNode>(node)) {
            rawstr << ai->toString();
        } else if (auto *ao = llvm::dyn_cast<ActualOUTSVFGNode>(node)) {
            rawstr << ao->toString();
        } else if (auto *ap = llvm::dyn_cast<ActualParmSVFGNode>(node)) {
            rawstr << ap->toString();
        } else if (auto *nptr = llvm::dyn_cast<NullPtrSVFGNode>(node)) {
            rawstr << nptr->toString();
        } else if (auto *ar = llvm::dyn_cast<ActualRetSVFGNode>(node)) {
            rawstr << ar->toString();
        } else if (auto *fr = llvm::dyn_cast<FormalRetSVFGNode>(node)) {
            rawstr << fr->toString();
        } else {
            assert(false && "what else kinds of nodes do we have??");
        }

        return rawstr.str();
    }

    static std::string getNodeAttributes(NodeType *node, SVFG *graph) {
        std::string str;
        raw_string_ostream rawstr(str);

        if (auto *stmtNode = llvm::dyn_cast<StmtSVFGNode>(node)) {
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
        } else if (llvm::isa<MSSAPHISVFGNode>(node)) {
            rawstr << "color=black";
        } else if (llvm::isa<PHISVFGNode>(node)) {
            rawstr << "color=black";
        } else if (llvm::isa<NullPtrSVFGNode>(node)) {
            rawstr << "color=grey";
        } else if (llvm::isa<FormalINSVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<FormalOUTSVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<FormalParmSVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<ActualINSVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<ActualOUTSVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<ActualParmSVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<ActualRetSVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<FormalRetSVFGNode>(node)) {
            rawstr << "color=yellow,style=double";
        } else if (llvm::isa<BinaryOPVFGNode>(node)) {
            rawstr << "color=black,style=double";
        } else if (llvm::isa<CmpVFGNode>(node)) {
            rawstr << "color=black,style=double";
        } else if (llvm::isa<UnaryOPVFGNode>(node)) {
            rawstr << "color=black,style=double";
        } else {
            assert(false && "no such kind of node!!");
        }

        /// dump slice information
        if (graph->getStat()->isSource(node)) {
            rawstr << ",style=filled, fillcolor=red";
        } else if (graph->getStat()->isSink(node)) {
            rawstr << ",style=filled, fillcolor=blue";
        } else if (graph->getStat()->inBackwardSlice(node)) {
            rawstr << ",style=filled, fillcolor=yellow";
        } else if (graph->getStat()->inForwardSlice(node)) {
            rawstr << ",style=filled, fillcolor=gray";
        }

        rawstr << "";

        return rawstr.str();
    }

    template <class EdgeIter>
    static std::string getEdgeAttributes(NodeType *, EdgeIter EI, SVFG *) {
        SVFGEdge *edge = *(EI.getCurrent());
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

        if (llvm::isa<IndirectSVFGEdge>(edge)) {
            if (llvm::isa<CallIndSVFGEdge>(edge)) {
                return "style=dashed,color=red";
            }
            if (llvm::isa<RetIndSVFGEdge>(edge)) {
                return "style=dashed,color=blue";
            }

            return "style=dashed";
        }

        return "";
    }

    template <class EdgeIter>
    static std::string getEdgeSourceLabel(NodeType *, EdgeIter EI) {
        SVFGEdge *edge = *(EI.getCurrent());
        assert(edge && "No edge found!!");

        std::string str;
        raw_string_ostream rawstr(str);
        if (auto *dirCall = llvm::dyn_cast<CallDirSVFGEdge>(edge)) {
            rawstr << dirCall->getCallSiteId();
        } else if (auto *dirRet = llvm::dyn_cast<RetDirSVFGEdge>(edge)) {
            rawstr << dirRet->getCallSiteId();
        } else if (auto *indCall = llvm::dyn_cast<CallIndSVFGEdge>(edge)) {
            rawstr << indCall->getCallSiteId();
        } else if (auto *indRet = llvm::dyn_cast<RetIndSVFGEdge>(edge)) {
            rawstr << indRet->getCallSiteId();
        }

        return rawstr.str();
    }
};
} // End namespace llvm
