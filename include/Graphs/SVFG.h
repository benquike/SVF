//===- SVFG.h -- Sparse value-flow graph--------------------------------------//
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
 * SVFG.h
 *
 *  Created on: Oct 28, 2013
 *      Author: Yulei Sui
 */

#ifndef SVFG_H_
#define SVFG_H_

#include "Graphs/SVFGNode.h"
#include "Graphs/VFG.h"

namespace SVF {

class PointerAnalysis;
class SVFGStat;

using SVFGEdge = VFGEdge;
using SVFGNode = VFGNode;
using ActualParmSVFGNode = ActualParmVFGNode;
using ActualRetSVFGNode = ActualRetVFGNode;
using FormalParmSVFGNode = FormalParmVFGNode;
using FormalRetSVFGNode = FormalRetVFGNode;

using NullPtrSVFGNode = NullPtrVFGNode;
using StmtSVFGNode = StmtVFGNode;
using AddrSVFGNode = AddrVFGNode;
using CopySVFGNode = CopyVFGNode;
using StoreSVFGNode = StoreVFGNode;
using LoadSVFGNode = LoadVFGNode;
using GepSVFGNode = GepVFGNode;
using PHISVFGNode = PHIVFGNode;
using IntraPHISVFGNode = IntraPHIVFGNode;
using InterPHISVFGNode = InterPHIVFGNode;

/*!
 * Sparse value flow graph
 * Each node stands for a definition, each edge stands for value flow relations
 */
class SVFG : public VFG {
    friend class SVFGBuilder;
    friend class SaberSVFGBuilder;
    friend class TaintSVFGBuilder;
    friend class DDASVFGBuilder;
    friend class MTASVFGBuilder;
    friend class RcSvfgBuilder;

  public:
    using PAGNodeToDefMapTy = Map<PAGNodeID, NodeID>;
    using MSSAVarToDefMapTy = Map<MRVerSPtr, NodeID>;
    using ActualINSVFGNodeSet = NodeBS;
    using ActualOUTSVFGNodeSet = NodeBS;
    using FormalINSVFGNodeSet = NodeBS;
    using FormalOUTSVFGNodeSet = NodeBS;
    using CallSiteToActualINsMapTy = Map<ICFGCallBlockID, ActualINSVFGNodeSet>;
    using CallSiteToActualOUTsMapTy =
        Map<ICFGCallBlockID, ActualOUTSVFGNodeSet>;
    using FunctionToFormalINsMapTy =
        Map<const SVFFunction *, FormalINSVFGNodeSet>;
    using FunctionToFormalOUTsMapTy =
        Map<const SVFFunction *, FormalOUTSVFGNodeSet>;
    using MUSet = MemSSA::MUSet;
    using CHISet = MemSSA::CHISet;
    using PHISet = MemSSA::PHISet;
    using MU = MemSSA::MU;
    using CHI = MemSSA::CHI;
    using LOADMU = MemSSA::LOADMU;
    using STORECHI = MemSSA::STORECHI;
    using RETMU = MemSSA::RETMU;
    using ENTRYCHI = MemSSA::ENTRYCHI;
    using CALLCHI = MemSSA::CALLCHI;
    using CALLMU = MemSSA::CALLMU;

  protected:
    MSSAVarToDefMapTy MSSAVarToDefMap; ///< map a memory SSA operator to its
                                       ///< definition SVFG node
    CallSiteToActualINsMapTy callSiteToActualINMap;
    CallSiteToActualOUTsMapTy callSiteToActualOUTMap;
    FunctionToFormalINsMapTy funToFormalINMap;
    FunctionToFormalOUTsMapTy funToFormalOUTMap;
    SVFGStat *stat = nullptr;
    MemSSA *mssa = nullptr;
    PointerAnalysis *pta = nullptr;

    /// Clean up memory
    void destroy();

    /// Start building SVFG
    virtual void buildSVFG();

  public:
    /// Constructor
    SVFG(MemSSA *mssa, PAG *pag, VFGK k);
    SVFG() = default;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFG>(*this);
        ar &MSSAVarToDefMap;
        ar &callSiteToActualINMap;
        ar &callSiteToActualOUTMap;
        boost::serialization::save_map(ar, funToFormalINMap);
        boost::serialization::save_map(ar, funToFormalOUTMap);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFG>(*this);
        ar &MSSAVarToDefMap;
        ar &callSiteToActualINMap;
        ar &callSiteToActualOUTMap;
        boost::serialization::load_map(ar, funToFormalINMap);
        boost::serialization::load_map(ar, funToFormalOUTMap);
    }
    /// @}

  public:
    friend class SVFGStat;

    /// Destructor
    virtual ~SVFG() { destroy(); }

    /// Return statistics
    inline SVFGStat *getStat() const { return stat; }

    /// Clear MSSA
    inline void clearMSSA() {
        delete mssa;
        mssa = nullptr;
    }

    /// Get SVFG memory SSA
    inline MemSSA *getMSSA() const { return mssa; }

    /// Get Pointer Analysis
    inline PointerAnalysis *getPTA() const { return pta; }

    /// Get all inter value flow edges of a indirect call site
    void getInterVFEdgesForIndirectCallSite(const CallBlockNode *cs,
                                            const SVFFunction *callee,
                                            SVFGEdgeSetTy &edges);

    /// Dump graph into dot file
    void dump(const std::string &file, bool simple = false);

    /// Connect SVFG nodes between caller and callee for indirect call site
    void connectCallerAndCallee(const CallBlockNode *cs,
                                const SVFFunction *callee,
                                SVFGEdgeSetTy &edges) override;

    /// Given a pagNode, return its definition site
    inline const SVFGNode *getDefSVFGNode(const PAGNode *pagNode) const {
        return getGNode(VFG::getDef(pagNode));
    }

    /// Perform statistics
    void performStat();

    /// Has a SVFGNode
    //@{
    inline bool hasActualINSVFGNodes(const CallBlockNode *cs) const {
        return callSiteToActualINMap.find(cs->getId()) !=
               callSiteToActualINMap.end();
    }

    inline bool hasActualOUTSVFGNodes(const CallBlockNode *cs) const {
        return callSiteToActualOUTMap.find(cs->getId()) !=
               callSiteToActualOUTMap.end();
    }

    inline bool hasFormalINSVFGNodes(const SVFFunction *fun) const {
        return funToFormalINMap.find(fun) != funToFormalINMap.end();
    }

    inline bool hasFormalOUTSVFGNodes(const SVFFunction *fun) const {
        return funToFormalOUTMap.find(fun) != funToFormalOUTMap.end();
    }
    //@}

    /// Get SVFGNode set
    //@{
    inline ActualINSVFGNodeSet &getActualINSVFGNodes(const CallBlockNode *cs) {
        return callSiteToActualINMap[cs->getId()];
    }

    inline ActualOUTSVFGNodeSet &
    getActualOUTSVFGNodes(const CallBlockNode *cs) {
        return callSiteToActualOUTMap[cs->getId()];
    }

    inline FormalINSVFGNodeSet &getFormalINSVFGNodes(const SVFFunction *fun) {
        return funToFormalINMap[fun];
    }

    inline FormalOUTSVFGNodeSet &getFormalOUTSVFGNodes(const SVFFunction *fun) {
        return funToFormalOUTMap[fun];
    }
    //@}

    /// Whether a node is function entry SVFGNode
    const SVFFunction *isFunEntrySVFGNode(const SVFGNode *node) const;

    /// Whether a node is callsite return SVFGNode
    const CallBlockNode *isCallSiteRetSVFGNode(const SVFGNode *node) const;

  protected:
    /// Add indirect def-use edges of a memory region between two statements,
    //@{
    SVFGEdge *addIntraIndirectVFEdge(NodeID srcId, NodeID dstId,
                                     const PointsTo &cpts);
    SVFGEdge *addCallIndirectVFEdge(NodeID srcId, NodeID dstId,
                                    const PointsTo &cpts, CallSiteID csId);
    SVFGEdge *addRetIndirectVFEdge(NodeID srcId, NodeID dstId,
                                   const PointsTo &cpts, CallSiteID csId);
    SVFGEdge *addThreadMHPIndirectVFEdge(NodeID srcId, NodeID dstId,
                                         const PointsTo &cpts);
    //@}

    /// Add inter VF edge from callsite mu to function entry chi
    SVFGEdge *addInterIndirectVFCallEdge(const ActualINSVFGNode *src,
                                         const FormalINSVFGNode *dst,
                                         CallSiteID csId);

    /// Add inter VF edge from function exit mu to callsite chi
    SVFGEdge *addInterIndirectVFRetEdge(const FormalOUTSVFGNode *src,
                                        const ActualOUTSVFGNode *dst,
                                        CallSiteID csId);

    /// Connect SVFG nodes between caller and callee for indirect call site
    //@{
    /// Connect actual-in and formal-in
    virtual inline void connectAInAndFIn(const ActualINSVFGNode *actualIn,
                                         const FormalINSVFGNode *formalIn,
                                         CallSiteID csId,
                                         SVFGEdgeSetTy &edges) {
        SVFGEdge *edge = addInterIndirectVFCallEdge(actualIn, formalIn, csId);
        if (edge != nullptr) {
            edges.insert(edge);
        }
    }
    /// Connect formal-out and actual-out
    virtual inline void connectFOutAndAOut(const FormalOUTSVFGNode *formalOut,
                                           const ActualOUTSVFGNode *actualOut,
                                           CallSiteID csId,
                                           SVFGEdgeSetTy &edges) {
        SVFGEdge *edge = addInterIndirectVFRetEdge(formalOut, actualOut, csId);
        if (edge != nullptr) {
            edges.insert(edge);
        }
    }
    //@}

    /// Get inter value flow edges between indirect call site and callee.
    //@{
    virtual inline void getInterVFEdgeAtIndCSFromAPToFP(const PAGNode *cs_arg,
                                                        const PAGNode *fun_arg,
                                                        const CallBlockNode *,
                                                        CallSiteID csId,
                                                        SVFGEdgeSetTy &edges) {
        SVFGNode *actualParam = getGNode(VFG::getDef(cs_arg));
        SVFGNode *formalParam = getGNode(VFG::getDef(fun_arg));
        SVFGEdge *edge = hasInterVFGEdge(actualParam, formalParam,
                                         SVFGEdge::CallDirVF, csId);
        assert(edge != nullptr &&
               "Can not find inter value flow edge from aparam to fparam");
        edges.insert(edge);
    }

    virtual inline void getInterVFEdgeAtIndCSFromFRToAR(const PAGNode *fun_ret,
                                                        const PAGNode *cs_ret,
                                                        CallSiteID csId,
                                                        SVFGEdgeSetTy &edges) {
        SVFGNode *formalRet = getGNode(VFG::getDef(fun_ret));
        SVFGNode *actualRet = getGNode(VFG::getDef(cs_ret));
        SVFGEdge *edge =
            hasInterVFGEdge(formalRet, actualRet, SVFGEdge::RetDirVF, csId);
        assert(edge != nullptr &&
               "Can not find inter value flow edge from fret to aret");
        edges.insert(edge);
    }

    virtual inline void
    getInterVFEdgeAtIndCSFromAInToFIn(ActualINSVFGNode *actualIn,
                                      const SVFFunction *callee,
                                      SVFGEdgeSetTy &edges) {
        for (auto outIt = actualIn->OutEdgeBegin(),
                  outEit = actualIn->OutEdgeEnd();
             outIt != outEit; ++outIt) {
            SVFGEdge *edge = *outIt;
            if (edge->getDstNode()->getFun() == callee) {
                edges.insert(edge);
            }
        }
    }

    virtual inline void
    getInterVFEdgeAtIndCSFromFOutToAOut(ActualOUTSVFGNode *actualOut,
                                        const SVFFunction *callee,
                                        SVFGEdgeSetTy &edges) {
        for (auto inIt = actualOut->InEdgeBegin(),
                  inEit = actualOut->InEdgeEnd();
             inIt != inEit; ++inIt) {

            SVFGEdge *edge = *inIt;
            if (edge->getSrcNode()->getFun() == callee) {
                edges.insert(edge);
            }
        }
    }
    //@}

    /// Given a MSSADef, set/get its def SVFG node (definition of address-taken
    /// variables)
    //@{
    inline void setDef(MRVerSPtr mvar, const SVFGNode *node) {
        auto it = MSSAVarToDefMap.find(mvar);
        if (it == MSSAVarToDefMap.end()) {
            MSSAVarToDefMap[mvar] = node->getId();
            assert(hasGNode(node->getId()) && "not in the map!!");
        } else {
            assert((it->second == node->getId()) &&
                   "a PAG node can only have unique definition ");
        }
    }

    inline NodeID getDef(MRVerSPtr mvar) const {
        auto it = MSSAVarToDefMap.find(mvar);
        assert(it != MSSAVarToDefMap.end() &&
               "memory SSA does not have a definition??");
        return it->second;
    }
    //@}

    /// Create SVFG nodes for address-taken variables
    void addSVFGNodesForAddrTakenVars();
    /// Connect direct SVFG edges between two SVFG nodes (value-flow of top
    /// address-taken variables)
    void connectIndirectSVFGEdges();
    /// Connect indirect SVFG edges from global initializers (store) to main
    /// function entry
    void connectFromGlobalToProgEntry();

    /// Add SVFG node
    virtual inline void addSVFGNode(SVFGNode *node, ICFGNode *icfgNode) {
        addVFGNode(node, icfgNode);
    }

    /// Add memory Function entry chi SVFG node
    inline void addFormalINSVFGNode(const MemSSA::ENTRYCHI *chi) {
        auto *sNode = new FormalINSVFGNode(getNextNodeId(), chi);
        addSVFGNode(sNode,
                    pag->getICFG()->getFunEntryBlockNode(chi->getFunction()));
        setDef(chi->getResVer(), sNode);
        funToFormalINMap[chi->getFunction()].set(sNode->getId());
    }
    /// Add memory Function return mu SVFG node
    inline void addFormalOUTSVFGNode(const MemSSA::RETMU *mu) {
        auto *sNode = new FormalOUTSVFGNode(getNextNodeId(), mu);
        addSVFGNode(sNode,
                    pag->getICFG()->getFunExitBlockNode(mu->getFunction()));
        funToFormalOUTMap[mu->getFunction()].set(sNode->getId());
    }
    /// Add memory callsite mu SVFG node
    inline void addActualINSVFGNode(const MemSSA::CALLMU *mu) {
        auto *sNode =
            new ActualINSVFGNode(getNextNodeId(), mu, mu->getCallSite());
        addSVFGNode(sNode, pag->getICFG()->getCallBlockNode(
                               mu->getCallSite()->getCallSite()));
        callSiteToActualINMap[mu->getCallSite()->getId()].set(sNode->getId());
    }
    /// Add memory callsite chi SVFG node
    inline void addActualOUTSVFGNode(const MemSSA::CALLCHI *chi) {
        auto *sNode =
            new ActualOUTSVFGNode(getNextNodeId(), chi, chi->getCallSite());
        addSVFGNode(sNode, pag->getICFG()->getRetBlockNode(
                               chi->getCallSite()->getCallSite()));
        setDef(chi->getResVer(), sNode);
        callSiteToActualOUTMap[chi->getCallSite()->getId()].set(sNode->getId());
    }
    /// Add memory SSA PHI SVFG node
    inline void addIntraMSSAPHISVFGNode(const MemSSA::PHI *phi) {
        auto *sNode = new IntraMSSAPHISVFGNode(getNextNodeId(), phi);
        addSVFGNode(sNode, pag->getICFG()->getBlockICFGNode(
                               &(phi->getBasicBlock()->front())));
        for (auto it = phi->opVerBegin(), eit = phi->opVerEnd(); it != eit;
             ++it) {
            sNode->setOpVer(it->first, it->second);
        }
        setDef(phi->getResVer(), sNode);
    }

    /// Has function for EntryCHI/RetMU/CallCHI/CallMU
    //@{
    inline bool hasFuncEntryChi(const SVFFunction *func) const {
        return (funToFormalINMap.find(func) != funToFormalINMap.end());
    }
    inline bool hasFuncRetMu(const SVFFunction *func) const {
        return (funToFormalOUTMap.find(func) != funToFormalOUTMap.end());
    }
    inline bool hasCallSiteChi(const CallBlockNode *cs) const {
        return (callSiteToActualOUTMap.find(cs->getId()) !=
                callSiteToActualOUTMap.end());
    }
    inline bool hasCallSiteMu(const CallBlockNode *cs) const {
        return (callSiteToActualINMap.find(cs->getId()) !=
                callSiteToActualINMap.end());
    }
    //@}
};

} // End namespace SVF

namespace llvm {
/* !
 * GraphTraits specializations for SVFG to be used for generic graph algorithms.
 * Provide graph traits for traversing from a SVFG node using standard graph
 * traversals.
 */
// template<> struct GraphTraits<SVF::SVFGNode*>: public
// GraphTraits<SVF::GenericNode<SVF::SVFGNode,SVF::SVFGEdge>*  > {
//};
//
///// Inverse GraphTraits specializations for Value flow node, it is used for
/// inverse traversal.
// template<>
// struct GraphTraits<Inverse<SVF::SVFGNode *> > : public
// GraphTraits<Inverse<SVF::GenericNode<SVF::SVFGNode,SVF::SVFGEdge>* > > {
//};

template <>
struct GraphTraits<SVF::SVFG *>
    : public GraphTraits<SVF::GenericGraph<SVF::SVFGNode, SVF::SVFGEdge> *> {
    using NodeRef = SVF::SVFGNode *;
};

} // End namespace llvm

#endif /* SVFG_H_ */
