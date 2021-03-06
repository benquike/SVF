//===- FlowSensitive.cpp -- Sparse flow-sensitive pointer analysis------------//
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
 * FlowSensitive.cpp
 *
 *  Created on: Oct 28, 2013
 *      Author: Yulei Sui
 */

#include "WPA/FlowSensitive.h"
#include "SVF-FE/DCHG.h"
#include "Util/Options.h"
#include "Util/SVFModule.h"
#include "Util/TypeBasedHeapCloning.h"
#include "WPA/Andersen.h"
#include "WPA/WPAStat.h"

using namespace SVF;
using namespace SVFUtil;

/*!
 * Initialize analysis
 */
void FlowSensitive::initialize() {
    PointerAnalysis::initialize();

    ander = AndersenWaveDiff::createAndersenWaveDiff(getSVFProject());

    // When evaluating ctir aliases, we want the whole SVFG.
    if (Options::OPTSVFG)
        svfg = Options::CTirAliasEval ? svfgBuilder.buildFullSVFG(ander)
                                      : svfgBuilder.buildPTROnlySVFG(ander);
    else
        svfg = svfgBuilder.buildPTROnlySVFGWithoutOPT(ander);

    setGraph(svfg);

    // AndersenWaveDiff::releaseAndersenWaveDiff();
    stat = new FlowSensitiveStat(this);
}

FlowSensitive::~FlowSensitive() {
    delete svfg;
    svfg = nullptr;

    delete ander;
    ander = nullptr;

    delete stat;
    stat = nullptr;
}

/*!
 * Start analysis
 */
void FlowSensitive::analyze() {
    /// Initialization for the Solver
    initialize();

    double start = stat->getClk(true);
    /// Start solving constraints
    DBOUT(DGENERAL, outs() << SVFUtil::pasMsg("Start Solving Constraints\n"));

    do {
        numOfIteration++;

        if (0 == numOfIteration % OnTheFlyIterBudgetForStat)
            dumpStat();

        callGraphSCC->find();

        initWorklist();
        solveWorklist();
    } while (updateCallGraph(getIndirectCallsites()));

    DBOUT(DGENERAL, outs() << SVFUtil::pasMsg("Finish Solving Constraints\n"));

    double end = stat->getClk(true);
    solveTime += (end - start) / TIMEINTERVAL;

    if (Options::CTirAliasEval) {
        printCTirAliasStats();
    }

    /// finalize the analysis
    finalize();
}

/*!
 * Finalize analysis
 */
void FlowSensitive::finalize() {
    if (Options::DumpVFG)
        svfg->dump("fs_solved", true);

    NodeStack &nodeStack = WPASolver<SVFG *>::SCCDetect();
    while (nodeStack.empty() == false) {
        NodeID rep = nodeStack.top();
        nodeStack.pop();
        const NodeBS &subNodes = getSCCDetector()->subNodes(rep);
        if (subNodes.count() > maxSCCSize)
            maxSCCSize = subNodes.count();
        if (subNodes.count() > 1) {
            numOfNodesInSCC += subNodes.count();
            numOfSCC++;
        }
    }

    PointerAnalysis::finalize();
}

/*!
 * SCC detection
 */
NodeStack &FlowSensitive::SCCDetect() {
    double start = stat->getClk();
    NodeStack &nodeStack = WPASVFGFSSolver::SCCDetect();
    double end = stat->getClk();
    sccTime += (end - start) / TIMEINTERVAL;
    return nodeStack;
}

/*!
 * Process each SVFG node
 */
void FlowSensitive::processNode(NodeID nodeId) {
    SVFGNode *node = svfg->getGNode(nodeId);
    if (processSVFGNode(node))
        propagate(&node);

    clearAllDFOutVarFlag(node);
}

/*!
 * Process each SVFG node
 */
bool FlowSensitive::processSVFGNode(SVFGNode *node) {
    double start = stat->getClk();
    bool changed = false;
    if (auto *addr = llvm::dyn_cast<AddrSVFGNode>(node)) {
        numOfProcessedAddr++;
        if (processAddr(addr))
            changed = true;
    } else if (auto *copy = llvm::dyn_cast<CopySVFGNode>(node)) {
        numOfProcessedCopy++;
        if (processCopy(copy))
            changed = true;
    } else if (auto *gep = llvm::dyn_cast<GepSVFGNode>(node)) {
        numOfProcessedGep++;
        if (processGep(gep))
            changed = true;
    } else if (auto *load = llvm::dyn_cast<LoadSVFGNode>(node)) {
        numOfProcessedLoad++;
        if (processLoad(load))
            changed = true;
    } else if (auto *store = llvm::dyn_cast<StoreSVFGNode>(node)) {
        numOfProcessedStore++;
        if (processStore(store))
            changed = true;
    } else if (auto *phi = llvm::dyn_cast<PHISVFGNode>(node)) {
        numOfProcessedPhi++;
        if (processPhi(phi))
            changed = true;
    } else if (llvm::isa<MSSAPHISVFGNode>(node) ||
               llvm::isa<FormalINSVFGNode>(node) ||
               llvm::isa<FormalOUTSVFGNode>(node) ||
               llvm::isa<ActualINSVFGNode>(node) ||
               llvm::isa<ActualOUTSVFGNode>(node)) {
        numOfProcessedMSSANode++;
        changed = true;
    } else if (llvm::isa<ActualParmSVFGNode>(node) ||
               llvm::isa<FormalParmSVFGNode>(node) ||
               llvm::isa<ActualRetSVFGNode>(node) ||
               llvm::isa<FormalRetSVFGNode>(node) ||
               llvm::isa<NullPtrSVFGNode>(node)) {
        changed = true;
    } else if (llvm::isa<CmpVFGNode>(node) ||
               llvm::isa<BinaryOPVFGNode>(node) ||
               llvm::dyn_cast<UnaryOPVFGNode>(node)) {

    } else
        assert(false && "unexpected kind of SVFG nodes");

    double end = stat->getClk();
    processTime += (end - start) / TIMEINTERVAL;

    return changed;
}

/*!
 * Propagate points-to information from source to destination node
 * Union dfOutput of src to dfInput of dst.
 * Only propagate points-to set of node which exists on the SVFG edge.
 * 1. propagation along direct edge will always return TRUE.
 * 2. propagation along indirect edge will return TRUE if destination node's
 *    IN set has been updated.
 */
bool FlowSensitive::propFromSrcToDst(SVFGEdge *edge) {
    double start = stat->getClk();
    bool changed = false;

    if (auto *dirEdge = llvm::dyn_cast<DirectSVFGEdge>(edge))
        changed = propAlongDirectEdge(dirEdge);
    else if (auto *indEdge = llvm::dyn_cast<IndirectSVFGEdge>(edge))
        changed = propAlongIndirectEdge(indEdge);
    else
        assert(false && "new kind of svfg edge?");

    double end = stat->getClk();
    propagationTime += (end - start) / TIMEINTERVAL;
    return changed;
}

/*!
 * Propagate points-to information along DIRECT SVFG edge.
 */
bool FlowSensitive::propAlongDirectEdge(const DirectSVFGEdge *edge) {
    double start = stat->getClk();
    bool changed = false;

    SVFGNode *src = edge->getSrcNode();
    SVFGNode *dst = edge->getDstNode();
    // If this is an actual-param or formal-ret, top-level pointer's pts must be
    // propagated from src to dst.
    if (auto *ap = llvm::dyn_cast<ActualParmSVFGNode>(src))
        changed = propagateFromAPToFP(ap, dst);
    else if (auto *fp = llvm::dyn_cast<FormalRetSVFGNode>(src))
        changed = propagateFromFRToAR(fp, dst);
    else {
        // Direct SVFG edge links between def and use of a top-level pointer.
        // There's no points-to information propagated along direct edge.
        // Since the top-level pointer's value has been changed at src node,
        // return TRUE to put dst node into the work list.
        changed = true;
    }

    double end = stat->getClk();
    directPropaTime += (end - start) / TIMEINTERVAL;
    return changed;
}

/*!
 * Propagate points-to information from actual-param to formal-param.
 *  Not necessary if SVFGOPT is used instead of original SVFG.
 */
bool FlowSensitive::propagateFromAPToFP(const ActualParmSVFGNode *ap,
                                        const SVFGNode *dst) {
    const auto *fp = llvm::dyn_cast<FormalParmSVFGNode>(dst);
    assert(fp && "expecting a formal param node");

    NodeID pagDst = fp->getParam()->getId();
    const PointsTo &srcCPts = getPts(ap->getParam()->getId());
    bool changed = unionPts(pagDst, srcCPts);

    return changed;
}

/*!
 * Propagate points-to information from formal-ret to actual-ret.
 * Not necessary if SVFGOPT is used instead of original SVFG.
 */
bool FlowSensitive::propagateFromFRToAR(const FormalRetSVFGNode *fr,
                                        const SVFGNode *dst) {
    const auto *ar = llvm::dyn_cast<ActualRetSVFGNode>(dst);
    assert(ar && "expecting an actual return node");

    NodeID pagDst = ar->getRev()->getId();
    const PointsTo &srcCPts = getPts(fr->getRet()->getId());
    bool changed = unionPts(pagDst, srcCPts);

    return changed;
}

/*!
 * Propagate points-to information along INDIRECT SVFG edge.
 */
bool FlowSensitive::propAlongIndirectEdge(const IndirectSVFGEdge *edge) {
    double start = stat->getClk();

    SVFGNode *src = edge->getSrcNode();
    SVFGNode *dst = edge->getDstNode();

    bool changed = false;

    // Get points-to targets may be used by next SVFG node.
    // Propagate points-to set for node used in dst.
    const PointsTo &pts = edge->getPointsTo();
    for (const auto &ptd : pts) {
        if (propVarPtsFromSrcToDst(ptd, src, dst))
            changed = true;

        if (isFieldInsensitive(ptd)) {
            /// If this is a field-insensitive obj, propagate all field node's
            /// pts
            const NodeBS &allFields = getAllFieldsObjNode(ptd);
            for (const auto &allField : allFields) {
                if (propVarPtsFromSrcToDst(allField, src, dst))
                    changed = true;
            }
        }
    }

    double end = stat->getClk();
    indirectPropaTime += (end - start) / TIMEINTERVAL;
    return changed;
}

/*!
 * Propagate points-to information of a certain variable from src to dst.
 */
bool FlowSensitive::propVarPtsFromSrcToDst(NodeID var, const SVFGNode *src,
                                           const SVFGNode *dst) {
    bool changed = false;
    if (llvm::isa<StoreSVFGNode>(src)) {
        if (updateInFromOut(src, var, dst, var))
            changed = true;
    } else {
        if (updateInFromIn(src, var, dst, var))
            changed = true;
    }
    return changed;
}

/*!
 * Process address node
 */
bool FlowSensitive::processAddr(const AddrSVFGNode *addr) {
    double start = stat->getClk();
    NodeID srcID = addr->getPAGSrcNodeID();
    /// TODO: If this object has been set as field-insensitive, just
    ///       add the insensitive object node into dst pointer's pts.
    if (isFieldInsensitive(srcID))
        srcID = getFIObjNode(srcID);
    bool changed = addPts(addr->getPAGDstNodeID(), srcID);
    double end = stat->getClk();
    addrTime += (end - start) / TIMEINTERVAL;
    return changed;
}

/*!
 * Process copy node
 */
bool FlowSensitive::processCopy(const CopySVFGNode *copy) {
    double start = stat->getClk();
    bool changed = unionPts(copy->getPAGDstNodeID(), copy->getPAGSrcNodeID());
    double end = stat->getClk();
    copyTime += (end - start) / TIMEINTERVAL;
    return changed;
}

/*!
 * Process mssa phi node
 */
bool FlowSensitive::processPhi(const PHISVFGNode *phi) {
    double start = stat->getClk();
    bool changed = false;
    NodeID pagDst = phi->getRes()->getId();
    for (auto it = phi->opVerBegin(), eit = phi->opVerEnd(); it != eit; ++it) {
        NodeID src = it->second->getId();
        const PointsTo &srcPts = getPts(src);
        if (unionPts(pagDst, srcPts))
            changed = true;
    }

    double end = stat->getClk();
    phiTime += (end - start) / TIMEINTERVAL;
    return changed;
}

/*!
 * Process gep node
 */
bool FlowSensitive::processGep(const GepSVFGNode *edge) {
    double start = stat->getClk();
    bool changed = false;
    const PointsTo &srcPts = getPts(edge->getPAGSrcNodeID());

    PointsTo tmpDstPts;
    if (llvm::isa<VariantGepPE>(edge->getPAGEdge())) {
        for (NodeID o : srcPts) {
            if (isBlkObjOrConstantObj(o)) {
                tmpDstPts.set(o);
                continue;
            }

            setObjFieldInsensitive(o);
            tmpDstPts.set(getFIObjNode(o));
        }
    } else if (const auto *normalGep =
                   llvm::dyn_cast<NormalGepPE>(edge->getPAGEdge())) {
        for (NodeID o : srcPts) {
            if (isBlkObjOrConstantObj(o)) {
                tmpDstPts.set(o);
                continue;
            }

            NodeID fieldSrcPtdNode =
                getGepObjNode(o, normalGep->getLocationSet());
            tmpDstPts.set(fieldSrcPtdNode);
        }
    } else {
        assert(false && "FlowSensitive::processGep: New type GEP edge type?");
    }

    if (unionPts(edge->getPAGDstNodeID(), tmpDstPts))
        changed = true;

    double end = stat->getClk();
    gepTime += (end - start) / TIMEINTERVAL;
    return changed;
}

/*!
 * Process load node
 *
 * Foreach node \in src
 * pts(dst) = union pts(node)
 */
bool FlowSensitive::processLoad(const LoadSVFGNode *load) {
    double start = stat->getClk();
    bool changed = false;
    auto pag = getPAG();

    NodeID dstVar = load->getPAGDstNodeID();

    const PointsTo &srcPts = getPts(load->getPAGSrcNodeID());
    for (const auto &ptd : srcPts) {
        if (pag->isConstantObj(ptd) || pag->isNonPointerObj(ptd))
            continue;

        if (unionPtsFromIn(load, ptd, dstVar))
            changed = true;

        if (isFieldInsensitive(ptd)) {
            /// If the ptd is a field-insensitive node, we should also get all
            /// field nodes' points-to sets and pass them to pagDst.
            const NodeBS &allFields = getAllFieldsObjNode(ptd);
            for (const auto &allField : allFields) {
                if (unionPtsFromIn(load, allField, dstVar))
                    changed = true;
            }
        }
    }

    double end = stat->getClk();
    loadTime += (end - start) / TIMEINTERVAL;
    return changed;
}

/*!
 * Process store node
 *
 * foreach node \in dst
 * pts(node) = union pts(src)
 */
bool FlowSensitive::processStore(const StoreSVFGNode *store) {

    const PointsTo &dstPts = getPts(store->getPAGDstNodeID());

    /// STORE statement can only be processed if the pointer on the LHS
    /// points to something. If we handle STORE with an empty points-to
    /// set, the OUT set will be updated from IN set. Then if LHS pointer
    /// points-to one target and it has been identified as a strong
    /// update, we can't remove those points-to information computed
    /// before this strong update from the OUT set.
    if (dstPts.empty())
        return false;

    double start = stat->getClk();
    bool changed = false;
    auto pag = getPAG();
    if (getPts(store->getPAGSrcNodeID()).empty() == false) {
        for (const auto &ptd : dstPts) {
            if (pag->isConstantObj(ptd) || pag->isNonPointerObj(ptd))
                continue;

            if (unionPtsFromTop(store, store->getPAGSrcNodeID(), ptd))
                changed = true;
        }
    }

    double end = stat->getClk();
    storeTime += (end - start) / TIMEINTERVAL;

    double updateStart = stat->getClk();
    // also merge the DFInSet to DFOutSet.
    /// check if this is a strong updates store
    NodeID singleton;
    bool isSU = isStrongUpdate(store, singleton);
    if (isSU) {
        svfgHasSU.set(store->getId());
        if (strongUpdateOutFromIn(store, singleton))
            changed = true;
    } else {
        svfgHasSU.reset(store->getId());
        if (weakUpdateOutFromIn(store))
            changed = true;
    }
    double updateEnd = stat->getClk();
    updateTime += (updateEnd - updateStart) / TIMEINTERVAL;

    return changed;
}

/*!
 * Return TRUE if this is a strong update STORE statement.
 */
bool FlowSensitive::isStrongUpdate(const SVFGNode *node, NodeID &singleton) {
    bool isSU = false;
    auto pag = getPAG();
    if (const auto *store = llvm::dyn_cast<StoreSVFGNode>(node)) {
        const PointsTo &dstCPSet = getPts(store->getPAGDstNodeID());
        if (dstCPSet.count() == 1) {
            /// Find the unique element in cpts
            PointsTo::iterator it = dstCPSet.begin();
            singleton = *it;

            // Strong update can be made if this points-to target is not heap,
            // array or field-insensitive.
            if (!isHeapMemObj(singleton) && !isArrayMemObj(singleton) &&
                pag->getBaseObj(singleton)->isFieldInsensitive() == false &&
                !isLocalVarInRecursiveFun(singleton)) {
                isSU = true;
            }
        }
    }
    return isSU;
}

/*!
 * Update call graph and SVFG by connecting the
 * actual param and ret nodes with the formal
 * param and ret nodes
 */
bool FlowSensitive::updateCallGraph(const CallSiteToFunPtrMap &callsites) {
    double start = stat->getClk();
    CallEdgeMap callGraphNewEdges;

    ///
    /// find out the new targets at
    /// indirect callsites in the callgraph
    /// results are saved in callGraphNewEdges
    ///
    onTheFlyCallGraphSolve(callsites, callGraphNewEdges);

    SVFGEdgeSetTy svfgNewEdges;

    ///
    /// connect caller and collee in the SVFG
    /// using the results collected in callGraphNewEdges
    /// including actual and formal paramenter nodes
    /// and ret nodes
    ///
    connectCallerAndCallee(callGraphNewEdges, svfgNewEdges);

    ///
    /// Update the worklist for the next step,
    /// if needed
    ///
    updateConnectedNodes(svfgNewEdges);

    double end = stat->getClk();
    updateCallGraphTime += (end - start) / TIMEINTERVAL;
    return (!callGraphNewEdges.empty());
}

/*!
 *  Handle parameter passing in SVFG
 */
void FlowSensitive::connectCallerAndCallee(const CallEdgeMap &callGraphNewEdges,
                                           SVFGEdgeSetTy &svfgNewEdges) {
    auto iter = callGraphNewEdges.begin();
    auto eiter = callGraphNewEdges.end();
    for (; iter != eiter; iter++) {
        const CallBlockNode *cs = iter->first;
        const FunctionSet &functions = iter->second;
        for (const auto *func : functions) {
            svfg->connectCallerAndCallee(cs, func, svfgNewEdges);
        }
    }
}

/*!
 * Push nodes connected during update call graph into worklist so they will be
 * solved during next iteration.
 */
void FlowSensitive::updateConnectedNodes(const SVFGEdgeSetTy &edges) {
    for (auto *edge : edges) {
        SVFGNode *dstNode = edge->getDstNode();
        if (llvm::isa<PHISVFGNode>(dstNode)) {
            /// If this is a formal-param or actual-ret node, we need to solve
            /// this phi node in next iteration
            pushIntoWorklist(dstNode->getId());
        } else if (llvm::isa<FormalINSVFGNode>(dstNode) ||
                   llvm::isa<ActualOUTSVFGNode>(dstNode)) {
            /// If this is a formal-in or actual-out node, we need to propagate
            /// points-to information from its predecessor node.
            bool changed = false;

            SVFGNode *srcNode = edge->getSrcNode();

            const PointsTo &pts =
                llvm::cast<IndirectSVFGEdge>(edge)->getPointsTo();
            for (const auto &ptd : pts) {
                if (propVarPtsAfterCGUpdated(ptd, srcNode, dstNode))
                    changed = true;

                if (isFieldInsensitive(ptd)) {
                    /// If this is a field-insensitive obj, propagate all field
                    /// node's pts
                    const NodeBS &allFields = getAllFieldsObjNode(ptd);
                    for (const auto &allField : allFields) {
                        if (propVarPtsAfterCGUpdated(allField, srcNode,
                                                     dstNode))
                            changed = true;
                    }
                }
            }

            if (changed)
                pushIntoWorklist(dstNode->getId());
        }
    }
}

/*!
 * Propagate points-to information of a certain variable from src to dst.
 */
bool FlowSensitive::propVarPtsAfterCGUpdated(NodeID var, const SVFGNode *src,
                                             const SVFGNode *dst) {
    if (llvm::isa<StoreSVFGNode>(src)) {
        if (propDFOutToIn(src, var, dst, var))
            return true;
    } else {
        if (propDFInToIn(src, var, dst, var))
            return true;
    }
    return false;
}

void FlowSensitive::printCTirAliasStats(void) {
    auto *dchg = llvm::dyn_cast<DCHGraph>(chgraph);
    assert(dchg && "eval-ctir-aliases needs DCHG.");

    // < SVFG node ID (loc), PAG node of interest (top-level pointer) >.
    Set<std::pair<NodeID, NodeID>> cmpLocs;
    for (auto &npair : *svfg) {
        NodeID loc = npair.first;
        SVFGNode *node = npair.second;

        // Only care about loads, stores, and GEPs.
        if (auto *stmt = llvm::dyn_cast<StmtSVFGNode>(node)) {
            if (!llvm::isa<LoadSVFGNode>(stmt) &&
                !llvm::isa<StoreSVFGNode>(stmt) &&
                !llvm::isa<GepSVFGNode>(stmt)) {
                continue;
            }

            if (!TypeBasedHeapCloning::getRawCTirMetadata(
                    stmt->getInst() ? stmt->getInst()
                                    : stmt->getPAGEdge()->getValue())) {
                continue;
            }

            NodeID p = 0;
            if (llvm::isa<LoadSVFGNode>(stmt)) {
                p = stmt->getPAGSrcNodeID();
            } else if (llvm::isa<StoreSVFGNode>(stmt)) {
                p = stmt->getPAGDstNodeID();
            } else if (llvm::isa<GepSVFGNode>(stmt)) {
                p = stmt->getPAGSrcNodeID();
            } else {
                // Not interested.
                continue;
            }

            cmpLocs.insert(std::make_pair(loc, p));
        }
    }

    unsigned mayAliases = 0;
    unsigned noAliases = 0;
    countAliases(cmpLocs, &mayAliases, &noAliases);

    unsigned total = mayAliases + noAliases;
    llvm::outs() << "eval-ctir-aliases " << total << " " << mayAliases << " "
                 << noAliases << " "
                 << "\n";
    llvm::outs() << "  "
                 << "TOTAL : " << total << "\n"
                 << "  "
                 << "MAY   : " << mayAliases << "\n"
                 << "  "
                 << "MAY % : " << 100 * ((double)mayAliases / (double)(total))
                 << "\n"
                 << "  "
                 << "NO    : " << noAliases << "\n"
                 << "  "
                 << "NO  % : " << 100 * ((double)noAliases / (double)(total))
                 << "\n";
}

void FlowSensitive::countAliases(Set<std::pair<NodeID, NodeID>> cmp,
                                 unsigned *mayAliases, unsigned *noAliases) {
    for (std::pair<NodeID, NodeID> locPA : cmp) {
        // loc doesn't make a difference for FSPTA.
        NodeID p = locPA.second;
        for (std::pair<NodeID, NodeID> locPB : cmp) {
            if (locPB == locPA)
                continue;

            NodeID q = locPB.second;

            switch (alias(p, q)) {
            case llvm::NoAlias:
                ++(*noAliases);
                break;
            case llvm::MayAlias:
                ++(*mayAliases);
                break;
            default:
                assert("Not May/NoAlias?");
            }
        }
    }
}
