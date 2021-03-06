//===- ProgSlice.cpp -- Program slicing--------------------------------------//
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
 * ProgSlice.cpp
 *
 *  Created on: Apr 5, 2014
 *      Author: Yulei Sui
 */

#include "SABER/ProgSlice.h"
#include "SABER/SaberAnnotator.h"

using namespace SVF;
using namespace SVFUtil;

/*!
 * Compute path conditions for nodes on the backward slice
 * path condition of each node is calculated starting from root node (source)
 * Given a SVFGNode n, its path condition C is allocated (path_i stands for one
 * of m program paths reaches n)
 *
 * C = \bigvee Guard(path_i),  0 < i < m
 * Guard(path_i) = \bigwedge VFGGuard(x,y),  suppose (x,y) are two SVFGNode
 * nodes on path_i
 */
bool ProgSlice::AllPathReachableSolve() {
    const SVFGNode *source = getSource();
    VFWorkList worklist;
    worklist.push(source);
    /// mark source node conditions to be true
    setVFCond(source, getTrueCond());

    while (!worklist.empty()) {
        const SVFGNode *node = worklist.pop();
        setCurSVFGNode(node);
        Condition *cond = getVFCond(node);
        for (auto it = node->OutEdgeBegin(), eit = node->OutEdgeEnd();
             it != eit; ++it) {
            const SVFGEdge *edge = (*it);
            const SVFGNode *succ = edge->getDstNode();
            if (inBackwardSlice(succ)) {
                Condition *vfCond = nullptr;
                const BasicBlock *nodeBB = getSVFGNodeBB(node);
                const BasicBlock *succBB = getSVFGNodeBB(succ);
                /// clean up the control flow conditions for next round guard
                /// computation
                clearCFCond();

                if (edge->isCallVFGEdge()) {
                    vfCond = ComputeInterCallVFGGuard(
                        nodeBB, succBB, getCallSite(edge)->getParent());
                } else if (edge->isRetVFGEdge()) {
                    vfCond = ComputeInterRetVFGGuard(
                        nodeBB, succBB, getRetSite(edge)->getParent());
                } else
                    vfCond = ComputeIntraVFGGuard(nodeBB, succBB);

                Condition *succPathCond = condAnd(cond, vfCond);
                if (setVFCond(succ, condOr(getVFCond(succ), succPathCond)))
                    worklist.push(succ);
            }

            DBOUT(DSaber, outs() << " node (" << node->getId() << ") --> "
                                 << "succ (" << succ->getId()
                                 << ") condition: " << getVFCond(succ) << "\n");
        }
    }

    return isSatisfiableForAll();
}

/*!
 * Solve by computing disjunction of conditions from all sinks (e.g., memory
 * leak)
 */
bool ProgSlice::isSatisfiableForAll() {

    Condition *guard = getFalseCond();
    for (auto it = sinksBegin(), eit = sinksEnd(); it != eit; ++it) {
        guard = condOr(guard, getVFCond(*it));
    }
    setFinalCond(guard);

    return guard == getTrueCond();
}

/*!
 * Solve by analysing each pair of sinks (e.g., double free)
 */
bool ProgSlice::isSatisfiableForPairs() {

    for (auto it = sinksBegin(), eit = sinksEnd(); it != eit; ++it) {
        for (auto sit = it, esit = sinksEnd(); sit != esit; ++sit) {
            if (*it == *sit)
                continue;
            Condition *guard = condAnd(getVFCond(*sit), getVFCond(*it));
            if (guard != getFalseCond()) {
                setFinalCond(guard);
                return false;
            }
        }
    }

    return true;
}

const CallBlockNode *ProgSlice::getCallSite(const SVFGEdge *edge) const {
    assert(edge->isCallVFGEdge() && "not a call svfg edge?");
    if (const auto *callEdge = llvm::dyn_cast<CallDirSVFGEdge>(edge))
        return getSVFG()->getCallSite(callEdge->getCallSiteId());

    return getSVFG()->getCallSite(
        llvm::cast<CallIndSVFGEdge>(edge)->getCallSiteId());
}
const CallBlockNode *ProgSlice::getRetSite(const SVFGEdge *edge) const {
    assert(edge->isRetVFGEdge() && "not a return svfg edge?");
    if (const auto *callEdge = llvm::dyn_cast<RetDirSVFGEdge>(edge))
        return getSVFG()->getCallSite(callEdge->getCallSiteId());

    return getSVFG()->getCallSite(
        llvm::cast<RetIndSVFGEdge>(edge)->getCallSiteId());
}

/*!
 * Return llvm value for
 * addr/copy/gep/load/phi/actualParam/formalParam/actualRet/formalRet but not
 * for store/mssaphi/actualIn/acutalOut/formalIn/formalOut
 */
const Value *ProgSlice::getLLVMValue(const SVFGNode *node) const {
    if (const auto *stmt = llvm::dyn_cast<StmtSVFGNode>(node)) {
        if (llvm::isa<StoreSVFGNode>(stmt) == false) {
            if (stmt->getPAGDstNode()->hasValue())
                return stmt->getPAGDstNode()->getValue();
        }
    } else if (const auto *phi = llvm::dyn_cast<PHISVFGNode>(node)) {
        return phi->getRes()->getValue();
    } else if (const auto *ap = llvm::dyn_cast<ActualParmSVFGNode>(node)) {
        return ap->getParam()->getValue();
    } else if (const auto *fp = llvm::dyn_cast<FormalParmSVFGNode>(node)) {
        return fp->getParam()->getValue();
    } else if (const auto *ar = llvm::dyn_cast<ActualRetSVFGNode>(node)) {
        return ar->getRev()->getValue();
    } else if (const auto *fr = llvm::dyn_cast<FormalRetSVFGNode>(node)) {
        return fr->getRet()->getValue();
    }

    return nullptr;
}

/*!
 * Evaluate Atoms of a condition
 * TODO: for now we only evaluate one path, evaluate every single path
 *
 * Atom -- a propositional valirable: a, b, c
 * Literal -- an atom or its negation: a, ~a
 * Clause  -- A disjunction of some literals: a \vee b
 * CNF formula -- a conjunction of some clauses:  (a \vee b ) \wedge (c \vee d)
 */
std::string ProgSlice::evalFinalCond() const {
    std::string str;
    raw_string_ostream rawstr(str);
    NodeBS elems = pathAllocator->exactCondElem(finalCond);
    Set<std::string> locations;
    for (const auto &elem : elems) {
        Condition *atom = pathAllocator->getCond(elem);
        const Instruction *tinst = pathAllocator->getCondInst(atom);
        locations.insert(getSourceLoc(tinst));
    }
    /// print leak path after eliminating duplicated element
    for (const auto &location : locations) {
        rawstr << "\t\t  --> (" << location << ") \n";
    }

    return rawstr.str();
}

/*!
 * Annotate program paths according to the final path condition computed
 */
void ProgSlice::annotatePaths() {

    SaberAnnotator annotator(this);
    annotator.annotateSource();
    annotator.annotateSinks();

    NodeBS elems = pathAllocator->exactCondElem(finalCond);
    for (const auto &elem : elems) {
        Condition *atom = pathAllocator->getCond(elem);
        const Instruction *tinst = pathAllocator->getCondInst(atom);
        if (const auto *br = llvm::dyn_cast<BranchInst>(tinst)) {
            annotator.annotateFeasibleBranch(br, 0);
            annotator.annotateFeasibleBranch(br, 1);
        }
    }
}

void ProgSlice::destroy() {
    /// TODO: how to clean bdd memory
    //	for(SVFGNodeToCondMap::const_iterator it = svfgNodeToCondMap.begin(),
    // eit = svfgNodeToCondMap.end(); it!=eit; ++it){
    //		pathAllocator->markForRelease(it->second);
    //	}
    //	for(BBToCondMap::const_iterator it = bbToCondMap.begin(), eit =
    // bbToCondMap.end(); it!=eit; ++it){
    //		pathAllocator->markForRelease(it->second);
    //	}
}
