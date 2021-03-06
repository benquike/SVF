//===- FlowSensitive.h -- Flow-sensitive pointer analysis---------------------//
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
 * FlowSensitiveAnalysis.h
 *
 *  Created on: Oct 28, 2013
 *      Author: Yulei Sui
 *
 *  Updated by:
 *     Hui Peng <peng124@purdue.edu>
 *     2021-04-18
 */

#ifndef FLOWSENSITIVEANALYSIS_H_
#define FLOWSENSITIVEANALYSIS_H_

#include "Graphs/SVFGOPT.h"
#include "MSSA/SVFGBuilder.h"
#include "WPA/WPAFSSolver.h"

namespace SVF {

class AndersenWaveDiff;
class SVFModule;

/*!
 * Flow sensitive whole program pointer analysis
 */
using WPASVFGFSSolver = WPAFSSolver<SVFG *>;
class FlowSensitive : public WPASVFGFSSolver, public BVDataPTAImpl {
    friend class FlowSensitiveStat;

  protected:
    using SVFGEdgeSetTy = SVFG::SVFGEdgeSetTy;

  public:
    using MutDFPTDataTy = BVDataPTAImpl::MutDFPTDataTy;
    using DFInOutMap = BVDataPTAImpl::MutDFPTDataTy::DFPtsMap;
    using PtsMap = BVDataPTAImpl::MutDFPTDataTy::PtsMap;

    /// Constructor
    explicit FlowSensitive(SVFProject *proj, PTATY type = FSSPARSE_WPA,
                           bool alias_check = true,
                           bool enableVirtualCallAnalysis = false,
                           bool threadCallGraph = false)
        : WPASVFGFSSolver(),
          BVDataPTAImpl(proj, type, alias_check, enableVirtualCallAnalysis,
                        threadCallGraph) {
        svfg = nullptr;
        solveTime = sccTime = processTime = propagationTime = updateTime = 0;
        addrTime = copyTime = gepTime = loadTime = storeTime = phiTime = 0;
        updateCallGraphTime = directPropaTime = indirectPropaTime = 0;
        numOfProcessedAddr = numOfProcessedCopy = numOfProcessedGep = 0;
        numOfProcessedLoad = numOfProcessedStore = 0;
        numOfProcessedPhi = numOfProcessedActualParam =
            numOfProcessedFormalRet = 0;
        numOfProcessedMSSANode = 0;
        maxSCCSize = numOfSCC = numOfNodesInSCC = 0;
        iterationForPrintStat = OnTheFlyIterBudgetForStat;
    }

    /// Destructor
    ~FlowSensitive() override;

    /// Create signle instance of flow-sensitive pointer analysis
    static FlowSensitive *createFSWPA(SVFProject *proj,
                                      bool virtualCallAnalaysis = false,
                                      bool threadCallGraph = false) {
        FlowSensitive *fspta = new FlowSensitive(proj);
        if (virtualCallAnalaysis) {
            fspta->enableVirtualCallAnalysis();
        }

        if (threadCallGraph) {
            fspta->buildThreadCallGraph();
        }

        fspta->analyze();
        return fspta;
    }

    /// We start from here
    virtual bool runOnModule(SVFModule *) { return false; }

    /// Flow sensitive analysis
    void analyze() override;

    /// Initialize analysis
    void initialize() override;

    /// Finalize analysis
    void finalize() override;

    /// Get PTA name
    const std::string PTAName() const override { return "FlowSensitive"; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast
    //@{
    static inline bool classof(const PointerAnalysis *pta) {
        return pta->getAnalysisTy() == FSSPARSE_WPA;
    }
    //@}

    /// Return SVFG
    inline SVFG *getSVFG() const { return svfg; }

  protected:
    /// SCC detection
    NodeStack &SCCDetect() override;

    /// Propagation
    //@{
    /// Propagate points-to information from an edge's src node to its dst node.
    bool propFromSrcToDst(SVFGEdge *edge) override;
    /// Propagate points-to information along a DIRECT SVFG edge.
    virtual bool propAlongDirectEdge(const DirectSVFGEdge *edge);
    /// Propagate points-to information along an INDIRECT SVFG edge.
    virtual bool propAlongIndirectEdge(const IndirectSVFGEdge *edge);
    /// Propagate points-to information of a certain variable from src to dst.
    virtual bool propVarPtsFromSrcToDst(NodeID var, const SVFGNode *src,
                                        const SVFGNode *dst);
    /// Propagate points-to information from an actual-param to a formal-param.
    /// Not necessary if SVFGOPT is used instead of original SVFG.
    virtual bool propagateFromAPToFP(const ActualParmSVFGNode *ap,
                                     const SVFGNode *dst);
    /// Propagate points-to information from a formal-ret to an actual-ret.
    /// Not necessary if SVFGOPT is used instead of original SVFG.
    virtual bool propagateFromFRToAR(const FormalRetSVFGNode *fr,
                                     const SVFGNode *dst);
    /// Handle weak updates
    virtual bool weakUpdateOutFromIn(const SVFGNode *node) {
        return getDFPTDataTy()->updateAllDFOutFromIn(node->getId(), 0, false);
    }
    /// Handle strong updates
    virtual bool strongUpdateOutFromIn(const SVFGNode *node, NodeID singleton) {
        return getDFPTDataTy()->updateAllDFOutFromIn(node->getId(), singleton,
                                                     true);
    }
    //@}

    /// Propagation between newly connected SVFG nodes during updateCallGraph.
    /// Can only be used during updateCallGraph.
    //@{
    bool propVarPtsAfterCGUpdated(NodeID var, const SVFGNode *src,
                                  const SVFGNode *dst);

    virtual inline bool propDFOutToIn(const SVFGNode *srcStmt, NodeID srcVar,
                                      const SVFGNode *dstStmt, NodeID dstVar) {
        return getDFPTDataTy()->updateAllDFInFromOut(srcStmt->getId(), srcVar,
                                                     dstStmt->getId(), dstVar);
    }
    virtual inline bool propDFInToIn(const SVFGNode *srcStmt, NodeID srcVar,
                                     const SVFGNode *dstStmt, NodeID dstVar) {
        return getDFPTDataTy()->updateAllDFInFromIn(srcStmt->getId(), srcVar,
                                                    dstStmt->getId(), dstVar);
    }
    //@}

    /// Update data-flow points-to data
    //@{
    inline bool updateOutFromIn(const SVFGNode *srcStmt, NodeID srcVar,
                                const SVFGNode *dstStmt, NodeID dstVar) {
        return getDFPTDataTy()->updateDFOutFromIn(srcStmt->getId(), srcVar,
                                                  dstStmt->getId(), dstVar);
    }
    virtual inline bool updateInFromIn(const SVFGNode *srcStmt, NodeID srcVar,
                                       const SVFGNode *dstStmt, NodeID dstVar) {
        return getDFPTDataTy()->updateDFInFromIn(srcStmt->getId(), srcVar,
                                                 dstStmt->getId(), dstVar);
    }
    virtual inline bool updateInFromOut(const SVFGNode *srcStmt, NodeID srcVar,
                                        const SVFGNode *dstStmt,
                                        NodeID dstVar) {
        return getDFPTDataTy()->updateDFInFromOut(srcStmt->getId(), srcVar,
                                                  dstStmt->getId(), dstVar);
    }

    virtual inline bool unionPtsFromIn(const SVFGNode *stmt, NodeID srcVar,
                                       NodeID dstVar) {
        return getDFPTDataTy()->updateTLVPts(stmt->getId(), srcVar, dstVar);
    }
    virtual inline bool unionPtsFromTop(const SVFGNode *stmt, NodeID srcVar,
                                        NodeID dstVar) {
        return getDFPTDataTy()->updateATVPts(srcVar, stmt->getId(), dstVar);
    }

    inline void clearAllDFOutVarFlag(const SVFGNode *stmt) {
        getDFPTDataTy()->clearAllDFOutUpdatedVar(stmt->getId());
    }
    //@}

    /// Handle various constraints
    //@{
    void processNode(NodeID nodeId) override;
    bool processSVFGNode(SVFGNode *node);
    virtual bool processAddr(const AddrSVFGNode *addr);
    virtual bool processCopy(const CopySVFGNode *copy);
    virtual bool processPhi(const PHISVFGNode *phi);
    virtual bool processGep(const GepSVFGNode *edge);
    virtual bool processLoad(const LoadSVFGNode *load);
    virtual bool processStore(const StoreSVFGNode *store);
    //@}

    /// Update call graph
    //@{
    /// Update call graph.
    bool updateCallGraph(const CallSiteToFunPtrMap &callsites) override;
    /// Connect nodes in SVFG.
    void connectCallerAndCallee(const CallEdgeMap &newEdges,
                                SVFGEdgeSetTy &edges);
    /// Update nodes connected during updating call graph.
    virtual void updateConnectedNodes(const SVFGEdgeSetTy &edges);
    //@}

    /// Return TRUE if this is a strong update STORE statement.
    bool isStrongUpdate(const SVFGNode *node, NodeID &singleton);

    /// Prints some easily parseable stats on aliasing of relevant CTir TL PTS.
    /// Format: eval-ctir-aliases #TOTAL_TESTS #MAY_ALIAS #NO_ALIAS
    virtual void printCTirAliasStats(void);

    /// Fills may/noAliases for the location/pointer pairs in cmp.
    virtual void countAliases(Set<std::pair<NodeID, NodeID>> cmp,
                              unsigned *mayAliases, unsigned *noAliases);

    SVFG *svfg = nullptr;
    /// Get points-to set for a node from data flow IN/OUT set at a statement.
    //@{
    inline const PointsTo &getDFInPtsSet(const SVFGNode *stmt,
                                         const NodeID node) {
        return getDFPTDataTy()->getDFInPtsSet(stmt->getId(), node);
    }
    inline const PointsTo &getDFOutPtsSet(const SVFGNode *stmt,
                                          const NodeID node) {
        return getDFPTDataTy()->getDFOutPtsSet(stmt->getId(), node);
    }
    //@}

    /// Get IN/OUT data flow map.
    /// May only be called when the backing is MUTABLE.
    ///@{
    inline const DFInOutMap &getDFInputMap() const {
        return getMutDFPTDataTy()->getDFIn();
    }
    inline const DFInOutMap &getDFOutputMap() const {
        return getMutDFPTDataTy()->getDFOut();
    }
    ///@}

    SVFGBuilder svfgBuilder;
    AndersenWaveDiff *ander = nullptr;

    /// Statistics.
    //@{
    Size_t numOfProcessedAddr;        /// Number of processed Addr node
    Size_t numOfProcessedCopy;        /// Number of processed Copy node
    Size_t numOfProcessedGep;         /// Number of processed Gep node
    Size_t numOfProcessedPhi;         /// Number of processed Phi node
    Size_t numOfProcessedLoad;        /// Number of processed Load node
    Size_t numOfProcessedStore;       /// Number of processed Store node
    Size_t numOfProcessedActualParam; /// Number of processed actual param node
    Size_t numOfProcessedFormalRet;   /// Number of processed formal ret node
    Size_t numOfProcessedMSSANode;    /// Number of processed mssa node

    Size_t maxSCCSize;
    Size_t numOfSCC;
    Size_t numOfNodesInSCC;

    double solveTime;       ///< time of solve.
    double sccTime;         ///< time of SCC detection.
    double processTime;     ///< time of processNode.
    double propagationTime; ///< time of points-to propagation.
    double directPropaTime; ///< time of points-to propagation of address-taken
                            ///< objects
    double indirectPropaTime;   ///< time of points-to propagation of top-level
                                ///< pointers
    double updateTime;          ///< time of strong/weak updates.
    double addrTime;            ///< time of handling address edges
    double copyTime;            ///< time of handling copy edges
    double gepTime;             ///< time of handling gep edges
    double loadTime;            ///< time of load edges
    double storeTime;           ///< time of store edges
    double phiTime;             ///< time of phi nodes.
    double updateCallGraphTime; ///< time of updating call graph

    NodeBS svfgHasSU;
    //@}

    void svfgStat();
};

} // End namespace SVF

#endif /* FLOWSENSITIVEANALYSIS_H_ */
