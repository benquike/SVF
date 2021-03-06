/*
 * PointerAnalysisImpl.cpp
 *
 *  Created on: 28Mar.,2020
 *      Author: yulei
 */

#include "MemoryModel/PointerAnalysisImpl.h"
#include "SVF-FE/CPPUtil.h"
#include "SVF-FE/DCHG.h"
#include "Util/Options.h"
#include <fstream>
#include <sstream>

using namespace SVF;
using namespace SVFUtil;
using namespace cppUtil;
using namespace std;

/*!
 * Constructor
 */
BVDataPTAImpl::BVDataPTAImpl(SVFProject *proj, PointerAnalysis::PTATY type,
                             bool alias_check, bool enableVirtualCallAnalysis,
                             bool threadCallGraph)
    : PointerAnalysis(proj, type, alias_check, enableVirtualCallAnalysis,
                      threadCallGraph) {
    if (type == Andersen_BASE || type == Andersen_WPA ||
        type == AndersenWaveDiff_WPA || type == AndersenHCD_WPA ||
        type == AndersenHLCD_WPA || type == AndersenLCD_WPA ||
        type == TypeCPP_WPA || type == FlowS_DDA ||
        type == AndersenWaveDiffWithType_WPA || type == AndersenSCD_WPA ||
        type == AndersenSFR_WPA || type == Steensgaard_WPA) {
        ptD = new MutDiffPTDataTy();
    } else if (type == FSSPARSE_WPA || type == FSTBHC_WPA) {
        if (Options::INCDFPTData) {
            ptD = new IncMutDFPTDataTy(false);
        } else {
            ptD = new MutDFPTDataTy(false);
        }
    } else if (type == VFS_WPA) {
        ptD = new MutVersionedPTDataTy(false);
    } else {
        assert(false && "no points-to data available");
    }

    ptaImplTy = BVDataImpl;
}

/*!
 * Expand all fields of an aggregate in all points-to sets
 */
void BVDataPTAImpl::expandFIObjs(const PointsTo &pts, PointsTo &expandedPts) {
    expandedPts = pts;
    auto pag = getPAG();
    for (auto pit : pts) {
        if (pag->getBaseObjNode(pit) == pit || isFieldInsensitive(pit)) {
            expandedPts |= pag->getAllFieldsObjNode(pit);
        }
    }
}

/*!
 * Store pointer analysis result into a file.
 * It includes the points-to relations, and all PAG nodes including those
 * created when solving Andersen's constraints.
 */
void BVDataPTAImpl::writeToFile(const string &filename) {
    outs() << "Storing pointer analysis results to '" << filename << "'...";

    auto pag = getPAG();

    error_code err;
    ToolOutputFile F(filename.c_str(), err, llvm::sys::fs::F_None);
    if (err) {
        outs() << "  error opening file for writing!\n";
        F.os().clear_error();
        return;
    }

    // Write analysis results to file
    for (auto it = pag->begin(), ie = pag->end(); it != ie; ++it) {
        NodeID var = it->first;
        const PointsTo &pts = getPts(var);

        F.os() << var << " -> { ";
        if (pts.empty()) {
            F.os() << " ";
        } else {
            for (auto it = pts.begin(), ie = pts.end(); it != ie; ++it) {
                F.os() << *it << " ";
            }
        }
        F.os() << "}\n";
    }

    // Write GepPAGNodes to file
    for (auto it = pag->begin(), ie = pag->end(); it != ie; ++it) {
        PAGNode *pagNode = it->second;
        if (auto *gepObjPN = llvm::dyn_cast<GepObjPN>(pagNode)) {
            F.os() << it->first << " ";
            F.os() << pag->getBaseObjNode(it->first) << " ";
            F.os() << gepObjPN->getLocationSet().getOffset() << "\n";
        }
    }

    // Job finish and close file
    F.os().close();
    if (!F.os().has_error()) {
        outs() << "\n";
        F.keep();
        return;
    }
}

/*!
 * Load pointer analysis result form a file.
 * It populates BVDataPTAImpl with the points-to data, and updates PAG with
 * the PAG offset nodes created during Andersen's solving stage.
 */
bool BVDataPTAImpl::readFromFile(const string &filename) {
    outs() << "Loading pointer analysis results from '" << filename << "'...";

    auto pag = getPAG();

    ifstream F(filename.c_str());
    if (!F.is_open()) {
        outs() << "  error opening file for reading!\n";
        return false;
    }

    // Read analysis results from file
    PTDataTy *ptD = getPTDataTy();
    string line;

    // Read points-to sets
    string delimiter1 = " -> { ";
    string delimiter2 = " }";
    while (F.good()) {
        // Parse a single line in the form of "var -> { obj1 obj2 obj3 }"
        getline(F, line);
        size_t pos = line.find(delimiter1);
        if (pos == string::npos) {
            break;
        }
        if (line.back() != '}') {
            break;
        }

        // var
        NodeID var = atoi(line.substr(0, pos).c_str());

        // objs
        pos = pos + delimiter1.length();
        size_t len = line.length() - pos - delimiter2.length();
        string objs = line.substr(pos, len);
        if (!objs.empty()) {
            istringstream ss(objs);
            NodeID obj;
            while (ss.good()) {
                ss >> obj;
                ptD->addPts(var, obj);
            }
        }
    }

    // Read PAG offset nodes
    while (F.good()) {
        // Parse a single line in the form of "ID baseNodeID offset"
        istringstream ss(line);
        NodeID id;
        NodeID base;
        size_t offset;
        ss >> id >> base >> offset;

        NodeID n =
            pag->getGepObjNode(pag->getObject(base), LocationSet(offset));
        assert(id == n && "Error adding GepObjNode into PAG!");

        getline(F, line);
    }

    // Update callgraph
    updateCallGraph(pag->getIndirectCallsites());

    F.close();
    outs() << "\n";

    return true;
}

/*!
 * Dump points-to of each pag node
 */
void BVDataPTAImpl::dumpTopLevelPtsTo() {
    for (const auto &nIter : this->getAllValidPtrs()) {
        const PAGNode *node = getPAG()->getGNode(nIter);
        if (getPAG()->isValidTopLevelPtr(node)) {
            const PointsTo &pts = this->getPts(node->getId());
            outs() << "\nNodeID " << node->getId() << " ";

            if (pts.empty()) {
                outs() << "\t\tPointsTo: {empty}\n\n";
            } else {
                outs() << "\t\tPointsTo: { ";
                for (const auto &it : pts) {
                    outs() << it << " ";
                }
                outs() << "}\n\n";
            }
        }
    }

    outs().flush();
}

/*!
 * Dump all points-to including top-level (ValPN) and address-taken (ObjPN)
 * variables
 */
void BVDataPTAImpl::dumpAllPts() {
    OrderedNodeSet pagNodes;
    for (auto &it : *getPAG()) {
        pagNodes.insert(it.first);
    }

    for (NodeID n : pagNodes) {
        outs() << "----------------------------------------------\n";
        dumpPts(n, this->getPts(n));
    }

    outs() << "----------------------------------------------\n";
}

/*!
 * On the fly call graph construction
 * callsites are indirect callsites to be analyzed based on
 * points-to or CPP vtable analysis
 * results newEdges is the new indirect call edges discovered
 *
 * If a callsite is a c++ virtual call, c++ specific analysis
 * is prioritized,
 * which will check whether c++ vtable analysis is enabled, if so
 * that analysis will be used, otherwise, pointer analysis
 * is the fallback solution.
 */
void BVDataPTAImpl::onTheFlyCallGraphSolve(const CallSiteToFunPtrMap &callsites,
                                           CallEdgeMap &newEdges) {
    auto pag = getPAG();
    for (auto callsite : callsites) {
        const CallBlockNode *cs = callsite.first;
        CallSite llvmCS = SVFUtil::getLLVMCallSite(cs->getCallSite());

        if (isVirtualCallSite(llvmCS, getPAG()->getModule()->getLLVMModSet())) {
            const Value *vtbl = getVCallVtblPtr(llvmCS);
            assert(pag->hasValueNode(vtbl));
            NodeID vtblId = pag->getValueNode(vtbl);
            resolveCPPIndCalls(cs, getPts(vtblId), newEdges);
        } else {
            resolveIndCalls(callsite.first, getPts(callsite.second), newEdges);
        }
    }
}

/*!
 * Return alias results based on our points-to/alias analysis
 */
AliasResult BVDataPTAImpl::alias(const MemoryLocation &LocA,
                                 const MemoryLocation &LocB) {
    return alias(LocA.Ptr, LocB.Ptr);
}

/*!
 * Return alias results based on our points-to/alias analysis
 */
AliasResult BVDataPTAImpl::alias(const Value *V1, const Value *V2) {
    auto pag = getPAG();
    return alias(pag->getValueNode(V1), pag->getValueNode(V2));
}

/*!
 * Return alias results based on our points-to/alias analysis
 */
AliasResult BVDataPTAImpl::alias(NodeID node1, NodeID node2) {
    return alias(getPts(node1), getPts(node2));
}

/*!
 * Return alias results based on our points-to/alias analysis
 */
AliasResult BVDataPTAImpl::alias(const PointsTo &p1, const PointsTo &p2) {

    PointsTo pts1;
    expandFIObjs(p1, pts1);
    PointsTo pts2;
    expandFIObjs(p2, pts2);

    if (containBlackHoleNode(pts1) || containBlackHoleNode(pts2) ||
        pts1.intersects(pts2)) {
        return llvm::MayAlias;
    }

    return llvm::NoAlias;
}
