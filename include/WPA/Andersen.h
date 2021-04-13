//===- Andersen.h -- Field-sensitive Andersen's pointer analysis-------------//
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
 * Andersen.h
 *
 *  Created on: Nov 12, 2013
 *      Author: Yulei Sui
 */

#ifndef ANDERSENPASS_H_
#define ANDERSENPASS_H_

#include "Graphs/ConsG.h"
#include "Graphs/OfflineConsG.h"
#include "Graphs/PAG.h"
#include "MemoryModel/PointerAnalysisImpl.h"
#include "WPA/WPASolver.h"
#include "WPA/WPAStat.h"

namespace SVF {

class PTAType;
class SVFModule;

/*!
 * Abstract class of inclusion-based Pointer Analysis
 */
using WPAConstraintSolver = WPASolver<ConstraintGraph *>;

class AndersenBase : public WPAConstraintSolver, public BVDataPTAImpl {
  public:
    /// Constructor
    explicit AndersenBase(SVFProject *proj, PTATY type = Andersen_BASE,
                          bool alias_check = true)
        : BVDataPTAImpl(proj, type, alias_check), consCG(nullptr) {
        iterationForPrintStat = OnTheFlyIterBudgetForStat;
    }

    /// Destructor
    ~AndersenBase() override {
        delete consCG;
        consCG = nullptr;
    }

    /// Andersen analysis
    void analyze() override;

    /// Initialize analysis
    void initialize() override;

    /// Finalize analysis
    void finalize() override;

    /// Implement it in child class to update call graph
    inline bool updateCallGraph(const CallSiteToFunPtrMap &) override {
        return false;
    }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const AndersenBase *) { return true; }
    static inline bool classof(const PointerAnalysis *pta) {
        return (pta->getAnalysisTy() == Andersen_BASE ||
                pta->getAnalysisTy() == Andersen_WPA ||
                pta->getAnalysisTy() == AndersenLCD_WPA ||
                pta->getAnalysisTy() == AndersenHCD_WPA ||
                pta->getAnalysisTy() == AndersenHLCD_WPA ||
                pta->getAnalysisTy() == AndersenWaveDiff_WPA ||
                pta->getAnalysisTy() == AndersenWaveDiffWithType_WPA ||
                pta->getAnalysisTy() == AndersenSCD_WPA ||
                pta->getAnalysisTy() == AndersenSFR_WPA ||
                pta->getAnalysisTy() == TypeCPP_WPA ||
                pta->getAnalysisTy() == Steensgaard_WPA);
    }
    //@}

    /// Get constraint graph
    ConstraintGraph *getConstraintGraph() { return consCG; }

    /// dump statistics
    inline void printStat() { PointerAnalysis::dumpStat(); }

    /// Statistics
    //@{
    static Size_t numOfProcessedAddr;  /// Number of processed Addr edge
    static Size_t numOfProcessedCopy;  /// Number of processed Copy edge
    static Size_t numOfProcessedGep;   /// Number of processed Gep edge
    static Size_t numOfProcessedLoad;  /// Number of processed Load edge
    static Size_t numOfProcessedStore; /// Number of processed Store edge
    static Size_t numOfSfrs;
    static Size_t numOfFieldExpand;

    static Size_t numOfSCCDetection;
    static double timeOfSCCDetection;
    static double timeOfSCCMerges;
    static double timeOfCollapse;
    static Size_t AveragePointsToSetSize;
    static Size_t MaxPointsToSetSize;
    static double timeOfProcessCopyGep;
    static double timeOfProcessLoadStore;
    static double timeOfUpdateCallGraph;
    //@}

  protected:
    /// Constraint Graph
    ConstraintGraph *consCG;
};

/*!
 * Inclusion-based Pointer Analysis
 */
class Andersen : public AndersenBase {

  public:
    using CGSCC = SCCDetection<ConstraintGraph *>;
    using CallSite2DummyValPN = OrderedMap<CallSite, NodeID>;

    /// Constructor
    explicit Andersen(SVFProject *proj, PTATY type = Andersen_WPA,
                      bool alias_check = true)
        : AndersenBase(proj, type, alias_check), pwcOpt(false), diffOpt(true) {}

    /// Destructor
    ~Andersen() override {}

    /// Initialize analysis
    void initialize() override;

    /// Finalize analysis
    void finalize() override;

    /// Reset data
    inline void resetData() {
        AveragePointsToSetSize = 0;
        MaxPointsToSetSize = 0;
        timeOfProcessCopyGep = 0;
        timeOfProcessLoadStore = 0;
    }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const Andersen *) { return true; }
    static inline bool classof(const PointerAnalysis *pta) {
        return (pta->getAnalysisTy() == Andersen_WPA ||
                pta->getAnalysisTy() == AndersenLCD_WPA ||
                pta->getAnalysisTy() == AndersenHCD_WPA ||
                pta->getAnalysisTy() == AndersenHLCD_WPA ||
                pta->getAnalysisTy() == AndersenWaveDiff_WPA ||
                pta->getAnalysisTy() == AndersenWaveDiffWithType_WPA ||
                pta->getAnalysisTy() == AndersenSCD_WPA ||
                pta->getAnalysisTy() == AndersenSFR_WPA);
    }
    //@}

    /// SCC methods
    //@{
    inline NodeID sccRepNode(NodeID id) const override {
        return consCG->sccRepNode(id);
    }

    inline NodeBS &sccSubNodes(NodeID repId) {
        return consCG->sccSubNodes(repId);
    }
    //@}

    /// Operation of points-to set
    inline const PointsTo &getPts(NodeID id) override {
        return getPTDataTy()->getPts(sccRepNode(id));
    }

    inline bool unionPts(NodeID id, const PointsTo &target) override {
        id = sccRepNode(id);
        return getPTDataTy()->unionPts(id, target);
    }

    inline bool unionPts(NodeID id, NodeID ptd) override {
        id = sccRepNode(id);
        ptd = sccRepNode(ptd);
        return getPTDataTy()->unionPts(id, ptd);
    }

    void dumpTopLevelPtsTo() override;

    void setPWCOpt(bool flag) {
        pwcOpt = flag;
        if (pwcOpt) {
            setSCCEdgeFlag(ConstraintNode::Direct);
        } else {
            setSCCEdgeFlag(ConstraintNode::Copy);
        }
    }

    bool mergePWC() const { return pwcOpt; }

    void setDiffOpt(bool flag) { diffOpt = flag; }

    bool enableDiff() const { return diffOpt; }

  protected:
    CallSite2DummyValPN
        callsite2DummyValPN; ///< Map an instruction to a dummy obj which
                             ///< created at an indirect callsite, which invokes
                             ///< a heap allocator
    void heapAllocatorViaIndCall(CallSite cs, NodePairSet &cpySrcNodes);

    bool pwcOpt;
    bool diffOpt;

    /// Handle diff points-to set.
    virtual inline void computeDiffPts(NodeID id) {
        if (enableDiff()) {
            NodeID rep = sccRepNode(id);
            getDiffPTDataTy()->computeDiffPts(rep,
                                              getDiffPTDataTy()->getPts(rep));
        }
    }
    virtual inline const PointsTo &getDiffPts(NodeID id) {
        NodeID rep = sccRepNode(id);
        if (enableDiff()) {
            return getDiffPTDataTy()->getDiffPts(rep);
        }

        return getPTDataTy()->getPts(rep);
    }

    /// Handle propagated points-to set.
    inline void updatePropaPts(NodeID dstId, NodeID srcId) {
        if (!enableDiff()) {
            return;
        }

        NodeID srcRep = sccRepNode(srcId);
        NodeID dstRep = sccRepNode(dstId);
        getDiffPTDataTy()->updatePropaPtsMap(srcRep, dstRep);
    }
    inline void clearPropaPts(NodeID src) {
        if (enableDiff()) {
            NodeID rep = sccRepNode(src);
            getDiffPTDataTy()->clearPropaPts(rep);
        }
    }

    void initWorklist() override {}

    virtual void setSCCEdgeFlag(ConstraintNode::SCCEdgeFlag f) {
        ConstraintNode::sccEdgeFlag = f;
    }

    /// Override WPASolver function in order to use the default solver
    void processNode(NodeID nodeId) override;

    /// handling various constraints
    //@{
    void processAllAddr();

    virtual bool processLoad(NodeID node, const ConstraintEdge *load);
    virtual bool processStore(NodeID node, const ConstraintEdge *load);
    virtual bool processCopy(NodeID node, const ConstraintEdge *edge);
    virtual bool processGep(NodeID node, const GepCGEdge *edge);
    virtual void handleCopyGep(ConstraintNode *node);
    virtual void handleLoadStore(ConstraintNode *node);
    virtual void processAddr(const AddrCGEdge *addr);
    virtual bool processGepPts(const PointsTo &pts, const GepCGEdge *edge);
    //@}

    /// Add copy edge on constraint graph
    virtual inline bool addCopyEdge(NodeID src, NodeID dst) {
        if (consCG->addCopyCGEdge(src, dst) != nullptr) {
            updatePropaPts(src, dst);
            return true;
        }
        return false;
    }

    /// Update call graph for the input indirect callsites
    bool updateCallGraph(const CallSiteToFunPtrMap &callsites) override;

    /// Connect formal and actual parameters for indirect callsites
    void connectCaller2CalleeParams(CallSite cs, const SVFFunction *F,
                                    NodePairSet &cpySrcNodes);

    /// Merge sub node to its rep
    virtual void mergeNodeToRep(NodeID nodeId, NodeID newRepId);

    virtual bool mergeSrcToTgt(NodeID srcId, NodeID tgtId);

    /// Merge sub node in a SCC cycle to their rep node
    //@{
    void mergeSccNodes(NodeID repNodeId, const NodeBS &subNodes);
    void mergeSccCycle();
    //@}
    /// Collapse a field object into its base for field insensitive anlaysis
    //@{
    void collapsePWCNode(NodeID nodeId) override;
    void collapseFields() override;
    bool collapseNodePts(NodeID nodeId);
    bool collapseField(NodeID nodeId);
    //@}

    /// Updates subnodes of its rep, and rep node of its subs
    void updateNodeRepAndSubs(NodeID nodeId, NodeID newRepId);

    /// SCC detection
    NodeStack &SCCDetect() override;

    /// Sanitize pts for field insensitive objects
    void sanitizePts() {
        for (ConstraintGraph::iterator it = consCG->begin(),
                                       eit = consCG->end();
             it != eit; ++it) {
            const PointsTo &pts = getPts(it->first);
            NodeBS fldInsenObjs;
            for (NodeBS::iterator pit = pts.begin(), epit = pts.end();
                 pit != epit; ++pit) {
                if (isFieldInsensitive(*pit)) {
                    fldInsenObjs.set(*pit);
                }
            }
            for (NodeBS::iterator pit = fldInsenObjs.begin(),
                     epit = fldInsenObjs.end();
                 pit != epit; ++pit) {
                unionPts(it->first, consCG->getAllFieldsObjNode(*pit));
            }
        }
    }

    /// Get PTA name
    const std::string PTAName() const override { return "AndersenWPA"; }

    /// match types for Gep Edges
    virtual bool matchType(NodeID, NodeID, const NormalGepCGEdge *) {
        return true;
    }
    /// add type for newly created GepObjNode
    virtual void addTypeForGepObjNode(NodeID, const NormalGepCGEdge *) {}
};

/**
 * Wave propagation with diff points-to set.
 */
class AndersenWaveDiff : public Andersen {

  private:
    static AndersenWaveDiff *diffWave; // static instance

  public:
    explicit
    AndersenWaveDiff(SVFProject *proj, PTATY type = AndersenWaveDiff_WPA,
                     bool alias_check = true)
        : Andersen(proj, type, alias_check) {}

    /// Create an singleton instance directly instead of invoking llvm pass
    /// manager
    /// FIXME: remove it
    static AndersenWaveDiff *createAndersenWaveDiff(SVFProject *proj) {
        if (diffWave == nullptr) {
            diffWave = new AndersenWaveDiff(proj, AndersenWaveDiff_WPA, false);
            diffWave->analyze();
            return diffWave;
        }
        return diffWave;
    }

    static void releaseAndersenWaveDiff() {
        if (diffWave != nullptr) {
            delete diffWave;
        }

        diffWave = nullptr;
    }

    void solveWorklist() override;
    void processNode(NodeID nodeId) override;
    virtual void postProcessNode(NodeID nodeId);
    void handleCopyGep(ConstraintNode *node) override;
    virtual bool handleLoad(NodeID id, const ConstraintEdge *load);
    virtual bool handleStore(NodeID id, const ConstraintEdge *store);
    bool processCopy(NodeID node, const ConstraintEdge *edge) override;

  protected:
    void mergeNodeToRep(NodeID nodeId, NodeID newRepId) override;

    /// process "bitcast" CopyCGEdge
    virtual void processCast(const ConstraintEdge *) {}
};

/**
 * Wave propagation with diff points-to set with type filter.
 */
class AndersenWaveDiffWithType : public AndersenWaveDiff {

  private:
    using TypeMismatchedObjToEdgeTy = Map<NodeID, Set<const GepCGEdge *>>;

    TypeMismatchedObjToEdgeTy typeMismatchedObjToEdges;

    void recordTypeMismatchedGep(NodeID obj, const GepCGEdge *gepEdge) {
        auto it = typeMismatchedObjToEdges.find(obj);
        if (it != typeMismatchedObjToEdges.end()) {
            Set<const GepCGEdge *> &edges = it->second;
            edges.insert(gepEdge);
        } else {
            Set<const GepCGEdge *> edges;
            edges.insert(gepEdge);
            typeMismatchedObjToEdges[obj] = edges;
        }
    }

    static AndersenWaveDiffWithType *diffWaveWithType; // static instance

    /// Handle diff points-to set.
    //@{
    inline void computeDiffPts(NodeID id) override {
        NodeID rep = sccRepNode(id);
        getDiffPTDataTy()->computeDiffPts(rep, getDiffPTDataTy()->getPts(rep));
    }
    inline const PointsTo &getDiffPts(NodeID id) override {
        NodeID rep = sccRepNode(id);
        return getDiffPTDataTy()->getDiffPts(rep);
    }
    //@}

  public:
    explicit
    AndersenWaveDiffWithType(SVFProject *proj,
                             PTATY type = AndersenWaveDiffWithType_WPA)
        : AndersenWaveDiff(proj, type) {
        assert(getTypeSystem() != nullptr &&
               "a type system is required for this pointer analysis");
    }

    /// Create an singleton instance directly instead of invoking llvm pass
    /// manager
    static AndersenWaveDiffWithType *
    createAndersenWaveDiffWithType(SVFProject *proj) {
        if (diffWaveWithType == nullptr) {
            diffWaveWithType = new AndersenWaveDiffWithType(proj);
            diffWaveWithType->analyze();
            return diffWaveWithType;
        }
        return diffWaveWithType;
    }

    static void releaseAndersenWaveDiffWithType() {
        if (diffWaveWithType != nullptr) {
            delete diffWaveWithType;
        }

        diffWaveWithType = nullptr;
    }

  protected:
    /// SCC detection
    NodeStack &SCCDetect() override;
    /// merge types of nodes in a cycle
    void mergeTypeOfNodes(const NodeBS &nodes);
    /// process "bitcast" CopyCGEdge
    void processCast(const ConstraintEdge *edge) override;
    /// update type of objects when process "bitcast" CopyCGEdge
    void updateObjType(const Type *type, const PointsTo &objs);
    /// process mismatched gep edges
    void processTypeMismatchedGep(NodeID obj, const Type *type);
    /// match types for Gep Edges
    bool matchType(NodeID ptrid, NodeID objid,
                   const NormalGepCGEdge *normalGepEdge) override;
    /// add type for newly created GepObjNode
    void addTypeForGepObjNode(NodeID id,
                              const NormalGepCGEdge *normalGepEdge) override;
};

/*
 * Lazy Cycle Detection Based Andersen Analysis
 */
class AndersenLCD : virtual public Andersen {

  private:
    static AndersenLCD *lcdAndersen;
    EdgeSet metEdges;
    NodeSet lcdCandidates;

  public:
    explicit AndersenLCD(SVFProject *proj, PTATY type = AndersenLCD_WPA)
        : Andersen(proj, type), metEdges({}), lcdCandidates({}) {}

    /// Create an singleton instance directly instead of invoking llvm pass
    /// manager
    static AndersenLCD *createAndersenLCD(SVFProject *proj) {
        if (lcdAndersen == nullptr) {
            lcdAndersen = new AndersenLCD(proj);
            lcdAndersen->analyze();
            return lcdAndersen;
        }
        return lcdAndersen;
    }

    static void releaseAndersenLCD() {
        if (lcdAndersen) {
            delete lcdAndersen;
        }

        lcdAndersen = nullptr;
    }

  protected:
    // 'lcdCandidates' is used to collect nodes need to be visited by SCC
    // detector
    //@{
    inline bool hasLCDCandidate() const { return !lcdCandidates.empty(); };
    inline void cleanLCDCandidate() { lcdCandidates.clear(); };
    inline void addLCDCandidate(NodeID nodeId) {
        lcdCandidates.insert(nodeId);
    };
    //@}

    // 'metEdges' is used to collect edges met by AndersenLCD, to avoid
    // redundant visit
    //@{
    bool isMetEdge(ConstraintEdge *edge) const {
        auto it = metEdges.find(edge->getEdgeID());
        return it != metEdges.end();
    };
    void addMetEdge(ConstraintEdge *edge) {
        metEdges.insert(edge->getEdgeID());
    };
    //@}

    // AndersenLCD worklist processer
    void solveWorklist() override;
    // Solve constraints of each nodes
    void handleCopyGep(ConstraintNode *node) override;
    // Collapse nodes and fields based on 'lcdCandidates'
    virtual void mergeSCC();
    // AndersenLCD specified SCC detector, need to input a nodeStack
    // 'lcdCandidate'
    NodeStack &SCCDetect() override;
    bool mergeSrcToTgt(NodeID nodeId, NodeID newRepId) override;
};

/*!
 * Hybrid Cycle Detection Based Andersen Analysis
 */
class AndersenHCD : virtual public Andersen {

  public:
    using OSCC = SCCDetection<OfflineConsG *>;

  private:
    static AndersenHCD *hcdAndersen;
    NodeSet mergedNodes;
    OfflineConsG *oCG;

  public:
    explicit AndersenHCD(SVFProject *proj, PTATY type = AndersenHCD_WPA)
        : Andersen(proj, type), oCG(nullptr) {}

    /// Create an singleton instance directly instead of invoking llvm pass
    /// manager
    static AndersenHCD *createAndersenHCD(SVFProject *proj) {
        if (hcdAndersen == nullptr) {
            hcdAndersen = new AndersenHCD(proj);
            hcdAndersen->analyze();
            return hcdAndersen;
        }
        return hcdAndersen;
    }

    static void releaseAndersenHCD() {
        if (hcdAndersen != nullptr) {
            delete hcdAndersen;
        }

        hcdAndersen = nullptr;
    }

  protected:
    void initialize() override;

    // Get offline rep node from offline constraint graph
    //@{
    inline bool hasOfflineRep(NodeID nodeId) const {
        return oCG->hasOCGRep(nodeId);
    }
    inline NodeID getOfflineRep(NodeID nodeId) {
        return oCG->getOCGRep(nodeId);
    }
    //@}

    // The set 'mergedNodes' is used to record the merged node, therefore
    // avoiding re-merge nodes
    //@{
    inline bool isaMergedNode(NodeID node) const {
        NodeSet::const_iterator it = mergedNodes.find(node);
        return it != mergedNodes.end();
    };
    inline void setMergedNode(NodeID node) {
        if (!isaMergedNode(node)) {
            mergedNodes.insert(node);
        }
    };
    //@}

    void solveWorklist() override;
    virtual void mergeSCC(NodeID nodeId);
    void mergeNodeAndPts(NodeID node, NodeID tgt);
};

/*!
 * Hybrid Lazy Cycle Detection Based Andersen Analysis
 */
class AndersenHLCD : public AndersenHCD, public AndersenLCD {

  private:
    static AndersenHLCD *hlcdAndersen;

  public:
    explicit AndersenHLCD(SVFProject *proj, PTATY type = AndersenHLCD_WPA)
        : Andersen(proj, type), AndersenHCD(proj, type),
          AndersenLCD(proj, type) {}

    /// Create an singleton instance directly instead of invoking llvm pass
    /// manager
    static AndersenHLCD *createAndersenHLCD(SVFProject *proj) {
        if (hlcdAndersen == nullptr) {
            hlcdAndersen = new AndersenHLCD(proj);
            hlcdAndersen->analyze();
            return hlcdAndersen;
        }
        return hlcdAndersen;
    }

    static void releaseAndersenHLCD() {
        if (hlcdAndersen != nullptr) {
            delete hlcdAndersen;
        }

        hlcdAndersen = nullptr;
    }

  protected:
    void initialize() override { AndersenHCD::initialize(); }
    void solveWorklist() override { AndersenHCD::solveWorklist(); }
    void handleCopyGep(ConstraintNode *node) override {
        AndersenLCD::handleCopyGep(node);
    }
    void mergeSCC(NodeID nodeId) override;
    bool mergeSrcToTgt(NodeID nodeId, NodeID newRepId) override {
        return AndersenLCD::mergeSrcToTgt(nodeId, newRepId);
    }
};

} // End namespace SVF

#endif /* ANDERSENPASS_H_ */
