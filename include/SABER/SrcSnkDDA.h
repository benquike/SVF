//===- SrcSnkDDA.h -- Source-sink analyzer-----------------------------------//
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
 * SrcSnkDDA.h
 *
 *  Created on: Apr 1, 2014
 *      Author: Yulei Sui
 */

#ifndef SRCSNKANALYSIS_H_
#define SRCSNKANALYSIS_H_

#include "Graphs/SVFGOPT.h"
#include "SABER/ProgSlice.h"
#include "SABER/SaberCheckerAPI.h"
#include "SABER/SaberSVFGBuilder.h"
#include "Util/CFLSolver.h"
#include "WPA/Andersen.h"

namespace SVF {

using CFLSrcSnkSolver = CFLSolver<SVFG *, CxtDPItem>;

/*!
 * General source-sink analysis, which serves as a base analysis to be extended
 * for various clients
 */
class SrcSnkDDA : virtual public CFLSrcSnkSolver {

  public:
    using SVFGNodeSet = ProgSlice::SVFGNodeSet;
    using SVFGNodeToSliceMap = Map<const SVFGNode *, ProgSlice *>;
    using SVFGNodeSetIter = SVFGNodeSet::const_iterator;
    using DPIm = CxtDPItem;
    using DPImSet = Set<DPIm>; ///< dpitem set
    using SVFGNodeToDPItemsMap =
        Map<const SVFGNode *,
            DPImSet>; ///< map a SVFGNode to its visited dpitems
    using CallSiteSet = Set<const CallBlockNode *>;
    using SVFGNodeBS = NodeBS;
    using WorkList = ProgSlice::VFWorkList;

  private:
    ProgSlice *_curSlice; /// current program slice
    SVFGNodeSet sources;  /// source nodes
    SVFGNodeSet sinks;    /// source nodes
    PathCondAllocator *pathCondAllocator;
    SVFGNodeToDPItemsMap nodeToDPItemsMap; ///<  record forward visited dpitems
    SVFGNodeSet visitedSet;                ///<  record backward visited nodes

  protected:
    PAG *pag;
    SaberSVFGBuilder svfgBuilder;
    SVFG *svfg;
    PTACallGraph *ptaCallGraph;
    SVFProject *proj;

  public:
    /// Constructor
    SrcSnkDDA(SVFProject *proj)
        : _curSlice(nullptr), pag(proj->getPAG()), svfgBuilder(pag),
          svfg(nullptr), ptaCallGraph(nullptr), proj(proj) {
        pathCondAllocator = new PathCondAllocator(pag->getModule());
    }

    /// Destructor
    virtual ~SrcSnkDDA() {
        if (svfg != nullptr)
            delete svfg;
        svfg = nullptr;

        if (_curSlice != nullptr)
            delete _curSlice;
        _curSlice = nullptr;

        /// the following shared by multiple checkers, thus can not be released.
        // if (ptaCallGraph != nullptr)
        //    delete ptaCallGraph;
        // ptaCallGraph = nullptr;

        // if(pathCondAllocator)
        //    delete pathCondAllocator;
        // pathCondAllocator = nullptr;
        delete pathCondAllocator;
    }

    /// Start analysis here
    virtual void analyze();

    /// Initialize analysis
    virtual void initialize();

    /// Finalize analysis
    virtual void finalize() { dumpSlices(); }

    /// Get PAG
    PAG *getPAG() const { return getSVFG()->getPAG(); }

    /// Get SVFG
    inline const SVFG *getSVFG() const { return graph(); }

    /// Get Callgraph
    inline PTACallGraph *getCallgraph() const { return ptaCallGraph; }

    /// Whether this svfg node may access global variable
    inline bool isGlobalSVFGNode(const SVFGNode *node) const {
        return svfgBuilder.isGlobalSVFGNode(node);
    }
    /// Slice operations
    //@{
    virtual void setCurSlice(const SVFGNode *src);

    inline ProgSlice *getCurSlice() const { return _curSlice; }
    inline void addSinkToCurSlice(const SVFGNode *node) {
        _curSlice->addToSinks(node);
        addToCurForwardSlice(node);
    }
    inline bool isInCurForwardSlice(const SVFGNode *node) {
        return _curSlice->inForwardSlice(node);
    }
    inline bool isInCurBackwardSlice(const SVFGNode *node) {
        return _curSlice->inBackwardSlice(node);
    }
    inline void addToCurForwardSlice(const SVFGNode *node) {
        _curSlice->addToForwardSlice(node);
    }
    inline void addToCurBackwardSlice(const SVFGNode *node) {
        _curSlice->addToBackwardSlice(node);
    }
    //@}

    /// Initialize sources and sinks
    ///@{
    virtual void initSrcs() = 0;
    virtual void initSnks() = 0;
    virtual bool isSourceLikeFun(const SVFFunction *fun) = 0;
    virtual bool isSinkLikeFun(const SVFFunction *fun) = 0;
    virtual bool isSource(const SVFGNode *node) = 0;
    virtual bool isSink(const SVFGNode *node) = 0;
    ///@}

    /// Identify allocation wrappers
    bool isInAWrapper(const SVFGNode *src, CallSiteSet &csIdSet);

    /// report bug on the current analyzed slice
    virtual void reportBug(ProgSlice *slice) = 0;

    /// Get sources/sinks
    //@{
    inline const SVFGNodeSet &getSources() const { return sources; }
    inline SVFGNodeSetIter sourcesBegin() const { return sources.begin(); }
    inline SVFGNodeSetIter sourcesEnd() const { return sources.end(); }
    inline void addToSources(const SVFGNode *node) { sources.insert(node); }
    inline const SVFGNodeSet &getSinks() const { return sinks; }
    inline SVFGNodeSetIter sinksBegin() const { return sinks.begin(); }
    inline SVFGNodeSetIter sinksEnd() const { return sinks.end(); }
    inline void addToSinks(const SVFGNode *node) { sinks.insert(node); }
    //@}

    /// Get path condition allocator
    PathCondAllocator *getPathAllocator() const { return pathCondAllocator; }

  protected:
    /// Forward traverse
    inline void FWProcessCurNode(const DPIm &item) override {
        const SVFGNode *node = getNode(item.getCurNodeID());
        if (isSink(node)) {
            addSinkToCurSlice(node);
            _curSlice->setPartialReachable();
        } else
            addToCurForwardSlice(node);
    }
    /// Backward traverse
    inline void BWProcessCurNode(const DPIm &item) override {
        const SVFGNode *node = getNode(item.getCurNodeID());
        if (isInCurForwardSlice(node)) {
            addToCurBackwardSlice(node);
        }
    }
    /// Propagate information forward by matching context
    void FWProcessOutgoingEdge(const DPIm &item, SVFGEdge *edge) override;
    /// Propagate information backward without matching context, as forward
    /// analysis already did it
    void BWProcessIncomingEdge(const DPIm &item, SVFGEdge *edge) override;
    /// Whether has been visited or not, in order to avoid recursion on SVFG
    //@{
    inline bool forwardVisited(const SVFGNode *node, const DPIm &item) {
        auto it = nodeToDPItemsMap.find(node);
        if (it != nodeToDPItemsMap.end())
            return it->second.find(item) != it->second.end();
        else
            return false;
    }
    inline void addForwardVisited(const SVFGNode *node, const DPIm &item) {
        nodeToDPItemsMap[node].insert(item);
    }
    inline bool backwardVisited(const SVFGNode *node) {
        return visitedSet.find(node) != visitedSet.end();
    }
    inline void addBackwardVisited(const SVFGNode *node) {
        visitedSet.insert(node);
    }
    inline void clearVisitedMap() {
        nodeToDPItemsMap.clear();
        visitedSet.clear();
    }
    //@}

    /// Whether it is all path reachable from a source
    virtual bool isAllPathReachable() { return _curSlice->isAllReachable(); }
    /// Whether it is some path reachable from a source
    virtual bool isSomePathReachable() {
        return _curSlice->isPartialReachable();
    }
    /// Dump SVFG with annotated slice informaiton
    //@{
    void dumpSlices();
    void annotateSlice(ProgSlice *slice);
    void printBDDStat();
    //@}
};

} // End namespace SVF

#endif /* SRCSNKDDA_H_ */
