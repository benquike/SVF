//===- MemSSA.cpp -- Base class of pointer analyses------------------//
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
 * MemSSA.cpp
 *
 *  Created on: Dec 14, 2013
 *      Author: Yulei Sui
 */

#include "MSSA/MemSSA.h"
#include "Graphs/SVFGStat.h"
#include "MSSA/MemPartition.h"
#include "SVF-FE/LLVMUtil.h"
#include "Util/Options.h"

using namespace SVF;
using namespace SVFUtil;

static std::string kDistinctMemPar = "distinct";
static std::string kIntraDisjointMemPar = "intra-disjoint";
static std::string kInterDisjointMemPar = "inter-disjoint";

double MemSSA::timeOfGeneratingMemRegions = 0; ///< Time for allocating regions
double MemSSA::timeOfCreateMUCHI =
    0; ///< Time for generating mu/chi for load/store/calls
double MemSSA::timeOfInsertingPHI = 0; ///< Time for inserting phis
double MemSSA::timeOfSSARenaming = 0;  ///< Time for SSA rename

/*!
 * Constructor
 */
MemSSA::MemSSA(BVDataPTAImpl *p, bool ptrOnlyMSSA)
    : pta(p), df(nullptr), dt(nullptr) {

    assert((pta->getAnalysisTy() != PointerAnalysis::Default_PTA) &&
           "please specify a pointer analysis");
    if (!Options::MemPar.getValue().empty()) {
        std::string strategy = Options::MemPar.getValue();
        if (strategy == kDistinctMemPar)
            mrGen = new DistinctMRG(pta, ptrOnlyMSSA);
        else if (strategy == kIntraDisjointMemPar)
            mrGen = new IntraDisjointMRG(pta, ptrOnlyMSSA);
        else if (strategy == kInterDisjointMemPar)
            mrGen = new InterDisjointMRG(pta, ptrOnlyMSSA);
        else
            assert(false && "unrecognised memory partition strategy");
    } else {
        mrGen = new IntraDisjointMRG(pta, ptrOnlyMSSA);
    }

    stat = new MemSSAStat(this);

    /// Generate whole program memory regions
    double mrStart = stat->getClk(true);
    mrGen->generateMRs();
    double mrEnd = stat->getClk(true);
    timeOfGeneratingMemRegions += (mrEnd - mrStart) / TIMEINTERVAL;
}

/*!
 * Set DF/DT
 */
void MemSSA::setCurrentDFDT(DominanceFrontier *f, DominatorTree *t) {
    df = f;
    dt = t;
    usedRegs.clear();
    reg2BBMap.clear();
}

/*!
 * Start building memory SSA
 */
void MemSSA::buildMemSSA(const SVFFunction &fun, DominanceFrontier *f,
                         DominatorTree *t) {

    assert(!isExtCall(&fun) &&
           "we do not build memory ssa for external functions");

    DBOUT(DMSSA, outs() << "Building Memory SSA for function " << fun.getName()
                        << " \n");

    setCurrentDFDT(f, t);

    /// Create mus/chis for loads/stores/calls for memory regions
    double muchiStart = stat->getClk(true);
    createMUCHI(fun);
    double muchiEnd = stat->getClk(true);
    timeOfCreateMUCHI += (muchiEnd - muchiStart) / TIMEINTERVAL;

    /// Insert PHI for memory regions
    double phiStart = stat->getClk(true);
    insertPHI(fun);
    double phiEnd = stat->getClk(true);
    timeOfInsertingPHI += (phiEnd - phiStart) / TIMEINTERVAL;

    /// SSA rename for memory regions
    double renameStart = stat->getClk(true);
    SSARename(fun);
    double renameEnd = stat->getClk(true);
    timeOfSSARenaming += (renameEnd - renameStart) / TIMEINTERVAL;
}

/*!
 * Create mu/chi according to memory regions
 * collect used mrs in usedRegs and construction map from region to BB for prune
 * SSA phi insertion
 */
void MemSSA::createMUCHI(const SVFFunction &fun) {

    PAG *pag = pta->getPAG();

    DBOUT(DMSSA, outs() << "\t creating mu chi for function " << fun.getName()
                        << "\n");
    // 1. create mu/chi
    //	insert a set of mus for memory regions at each load
    //  inset a set of chis for memory regions at each store

    // 2. find global names (region name before renaming) of each memory region,
    // collect used mrs in usedRegs, and collect its def basic block in
    // reg2BBMap in the form of mu(r) and r = chi (r) a) mu(r): 		if(r
    // \not\in varKills) global = global \cup r b) r = chi(r): 		if(r \not\in
    // varKills) global = global \cup r
    //		varKills = varKills \cup r
    //		block(r) = block(r) \cup bb_{chi}

    /// get all reachable basic blocks from function entry
    /// ignore dead basic blocks
    BBList reachableBBs;
    getFunReachableBBs(fun.getLLVMFun(), getDT(fun), reachableBBs);

    for (const auto *bb : reachableBBs) {
        varKills.clear();
        for (const auto &it : *bb) {
            const Instruction *inst = &it;
            if (mrGen->hasPAGEdgeList(inst)) {
                PAGEdgeList &pagEdgeList = mrGen->getPAGEdgesFromInst(inst);
                for (const auto *inst : pagEdgeList) {
                    if (const auto *load = llvm::dyn_cast<LoadPE>(inst))
                        AddLoadMU(bb, load, mrGen->getLoadMRSet(load));
                    else if (const auto *store = llvm::dyn_cast<StorePE>(inst))
                        AddStoreCHI(bb, store, mrGen->getStoreMRSet(store));
                }
            }
            if (isNonInstricCallSite(inst)) {
                const CallBlockNode *cs =
                    pag->getICFG()->getCallBlockNode(inst);
                if (mrGen->hasRefMRSet(cs))
                    AddCallSiteMU(cs, mrGen->getCallSiteRefMRSet(cs));

                if (mrGen->hasModMRSet(cs))
                    AddCallSiteCHI(cs, mrGen->getCallSiteModMRSet(cs));
            }
        }
    }

    // create entry chi for this function including all memory regions
    // initialize them with version 0 and 1 r_1 = chi (r_0)
    for (const auto *mr : usedRegs) {
        // initialize mem region version and stack for renaming phase
        mr2CounterMap[mr] = 0;
        mr2VerStackMap[mr].clear();
        auto *chi = new ENTRYCHI(&fun, mr);
        chi->setOpVer(newSSAName(mr, chi));
        chi->setResVer(newSSAName(mr, chi));
        funToEntryChiSetMap[&fun].insert(chi);

        /// if the function does not have a reachable return instruction from
        /// function entry then we won't create return mu for it
        if (functionDoesNotRet(fun.getLLVMFun()) == false) {
            auto *mu = new RETMU(&fun, mr);
            funToReturnMuSetMap[&fun].insert(mu);
        }
    }
}

/*
 * Insert phi node
 */
void MemSSA::insertPHI(const SVFFunction &fun) {

    DBOUT(DMSSA,
          outs() << "\t insert phi for function " << fun.getName() << "\n");

    const DominanceFrontier *df = getDF(fun);
    // record whether a phi of mr has already been inserted into the bb.
    BBToMRSetMap bb2MRSetMap;

    // start inserting phi node
    for (const auto *mr : usedRegs) {
        BBList bbs = reg2BBMap[mr];
        while (!bbs.empty()) {
            const BasicBlock *bb = bbs.back();
            bbs.pop_back();
            auto it = df->find(const_cast<BasicBlock *>(bb));
            if (it == df->end()) {
                writeWrnMsg("bb not in the dominance frontier map??");
                continue;
            }
            const DominanceFrontierBase::DomSetType &domSet = it->second;
            for (auto *pbb : domSet) {
                // if we never insert this phi node before
                if (0 == bb2MRSetMap[pbb].count(mr)) {
                    bb2MRSetMap[pbb].insert(mr);
                    // insert phi node
                    AddMSSAPHI(pbb, mr);
                    // continue to insert phi in its iterative dominate
                    // frontiers
                    bbs.push_back(pbb);
                }
            }
        }
    }
}

/*!
 * SSA construction algorithm
 */
void MemSSA::SSARename(const SVFFunction &fun) {

    DBOUT(DMSSA,
          outs() << "\t ssa rename for function " << fun.getName() << "\n");

    SSARenameBB(fun.getLLVMFun()->getEntryBlock());
}

/*!
 * Renaming for each memory regions
 * See the renaming algorithm in book Engineering A Compiler (Figure 9.12)
 */
void MemSSA::SSARenameBB(const BasicBlock &bb) {

    PAG *pag = pta->getPAG();
    LLVMModuleSet *modSet = pag->getModule()->getLLVMModSet();
    // record which mem region needs to pop stack
    MRVector memRegs;

    // rename phi result op
    // for each r = phi (...)
    // 		rewrite r as new name
    if (hasPHISet(&bb))
        RenamePhiRes(getPHISet(&bb), memRegs);

    // process mu and chi
    // for each mu(r)
    // 		rewrite r with top mrver of stack(r)
    // for each r = chi(r')
    // 		rewrite r' with top mrver of stack(r)
    // 		rewrite r with new name

    for (BasicBlock::const_iterator it = bb.begin(), eit = bb.end(); it != eit;
         ++it) {
        const Instruction *inst = &*it;
        if (mrGen->hasPAGEdgeList(inst)) {
            PAGEdgeList &pagEdgeList = mrGen->getPAGEdgesFromInst(inst);
            for (const auto *inst : pagEdgeList) {
                if (const LoadPE *load = llvm::dyn_cast<LoadPE>(inst))
                    RenameMuSet(getMUSet(load));
                else if (const auto *store = llvm::dyn_cast<StorePE>(inst))
                    RenameChiSet(getCHISet(store), memRegs);
            }
        }
        if (isNonInstricCallSite(inst)) {
            const CallBlockNode *cs = pag->getICFG()->getCallBlockNode(inst);
            if (mrGen->hasRefMRSet(cs))
                RenameMuSet(getMUSet(cs));

            if (mrGen->hasModMRSet(cs))
                RenameChiSet(getCHISet(cs), memRegs);
        } else if (isReturn(inst)) {
            const SVFFunction *fun = modSet->getSVFFunction(bb.getParent());
            RenameMuSet(getReturnMuSet(fun));
        }
    }

    // fill phi operands of succ basic blocks
    for (succ_const_iterator sit = succ_begin(&bb), esit = succ_end(&bb);
         sit != esit; ++sit) {
        const BasicBlock *succ = *sit;
        u32_t pos = getBBPredecessorPos(&bb, succ);
        if (hasPHISet(succ))
            RenamePhiOps(getPHISet(succ), pos, memRegs);
    }

    // for succ basic block in dominator tree
    const SVFFunction *fun = modSet->getSVFFunction(bb.getParent());
    DominatorTree *dt = getDT(*fun);
    if (DomTreeNode *dtNode = dt->getNode(const_cast<BasicBlock *>(&bb))) {
        for (auto &DI : *dtNode) {
            SSARenameBB(*(DI->getBlock()));
        }
    }
    // for each r = chi(..), and r = phi(..)
    // 		pop ver stack(r)
    while (!memRegs.empty()) {
        const MemRegion *mr = memRegs.back();
        memRegs.pop_back();
        mr2VerStackMap[mr].pop_back();
    }
}

MRVerSPtr MemSSA::newSSAName(const MemRegion *mr, MSSADEF *def) {
    assert(0 != mr2CounterMap.count(mr) &&
           "did not find initial version in map? ");
    assert(0 != mr2VerStackMap.count(mr) &&
           "did not find initial stack in map? ");

    MRVERSION version = mr2CounterMap[mr];
    mr2CounterMap[mr] = version + 1;
    auto mrVer = make_shared<MRVer>(mr, version, def);
    mr2VerStackMap[mr].push_back(mrVer);
    return mrVer;
}

/*!
 * Clean up memory
 */
void MemSSA::destroy() {

    for (auto &iter : load2MuSetMap) {
        for (auto *it : iter.second) {
            delete it;
        }
    }

    for (auto &iter : store2ChiSetMap) {
        for (auto *it : iter.second) {
            delete it;
        }
    }

    for (auto &iter : callsiteToMuSetMap) {
        for (auto *it : iter.second) {
            delete it;
        }
    }

    for (auto &iter : callsiteToChiSetMap) {
        for (auto *it : iter.second) {
            delete it;
        }
    }

    for (auto &iter : funToEntryChiSetMap) {
        for (auto *it : iter.second) {
            delete it;
        }
    }

    for (auto &iter : funToReturnMuSetMap) {
        for (auto *it : iter.second) {
            delete it;
        }
    }

    for (auto &iter : bb2PhiSetMap) {
        for (auto *it : iter.second) {
            delete it;
        }
    }

    delete mrGen;
    mrGen = nullptr;
    delete stat;
    stat = nullptr;
    pta = nullptr;
}

/*!
 * Perform statistics
 */
void MemSSA::performStat() {
    if (pta->printStat())
        stat->performStat();
}

/*!
 * Get loadMU numbers
 */
u32_t MemSSA::getLoadMuNum() const {
    u32_t num = 0;
    auto it = load2MuSetMap.begin();
    auto eit = load2MuSetMap.end();
    for (; it != eit; it++) {
        const MUSet &muSet = it->second;
        num += muSet.size();
    }
    return num;
}

/*!
 * Get StoreCHI numbers
 */
u32_t MemSSA::getStoreChiNum() const {
    u32_t num = 0;
    auto it = store2ChiSetMap.begin();
    auto eit = store2ChiSetMap.end();
    for (; it != eit; it++) {
        const CHISet &chiSet = it->second;
        num += chiSet.size();
    }
    return num;
}

/*!
 * Get EntryCHI numbers
 */
u32_t MemSSA::getFunEntryChiNum() const {
    u32_t num = 0;
    auto it = funToEntryChiSetMap.begin();
    auto eit = funToEntryChiSetMap.end();
    for (; it != eit; it++) {
        const CHISet &chiSet = it->second;
        num += chiSet.size();
    }
    return num;
}

/*!
 * Get RetMU numbers
 */
u32_t MemSSA::getFunRetMuNum() const {
    u32_t num = 0;
    auto it = funToReturnMuSetMap.begin();
    auto eit = funToReturnMuSetMap.end();
    for (; it != eit; it++) {
        const MUSet &muSet = it->second;
        num += muSet.size();
    }
    return num;
}

/*!
 * Get CallMU numbers
 */
u32_t MemSSA::getCallSiteMuNum() const {
    u32_t num = 0;
    auto it = callsiteToMuSetMap.begin();
    auto eit = callsiteToMuSetMap.end();
    for (; it != eit; it++) {
        const MUSet &muSet = it->second;
        num += muSet.size();
    }
    return num;
}

/*!
 * Get CallCHI numbers
 */
u32_t MemSSA::getCallSiteChiNum() const {
    u32_t num = 0;
    auto it = callsiteToChiSetMap.begin();
    auto eit = callsiteToChiSetMap.end();
    for (; it != eit; it++) {
        const CHISet &chiSet = it->second;
        num += chiSet.size();
    }
    return num;
}

/*!
 * Get PHI numbers
 */
u32_t MemSSA::getBBPhiNum() const {
    u32_t num = 0;
    auto it = bb2PhiSetMap.begin();
    auto eit = bb2PhiSetMap.end();
    for (; it != eit; it++) {
        const PHISet &phiSet = it->second;
        num += phiSet.size();
    }
    return num;
}

/*!
 * Print SSA
 */
void MemSSA::dumpMSSA(raw_ostream &Out) {
    if (!Options::DumpMSSA)
        return;

    PAG *pag = pta->getPAG();

    SVFModule *svfMod = pag->getModule();
    for (const auto *fun : *pta->getSVFModule()) {
        if (Options::MSSAFun != "" && Options::MSSAFun != fun->getName())
            continue;

        Out << "==========FUNCTION: " << fun->getName() << "==========\n";
        // dump function entry chi nodes
        if (hasFuncEntryChi(fun)) {
            CHISet &entry_chis = getFuncEntryChiSet(fun);
            for (auto *entry_chi : entry_chis) {
                entry_chi->dump();
            }
        }

        for (auto &bb : *fun->getLLVMFun()) {
            if (bb.hasName())
                Out << bb.getName() << "\n";
            PHISet &phiSet = getPHISet(&bb);
            for (auto *pi : phiSet) {
                pi->dump();
            }

            bool last_is_chi = false;
            for (auto &inst : bb) {
                bool isAppCall =
                    isNonInstricCallSite(&inst) && !isExtCall(&inst, svfMod);
                if (isAppCall || isHeapAllocExtCall(&inst, svfMod)) {
                    const CallBlockNode *cs =
                        pag->getICFG()->getCallBlockNode(&inst);
                    if (hasMU(cs)) {
                        if (!last_is_chi) {
                            Out << "\n";
                        }
                        for (auto *mit : getMUSet(cs)) {
                            mit->dump();
                        }
                    }

                    Out << inst << "\n";

                    if (hasCHI(cs)) {
                        for (auto *cit : getCHISet(cs)) {
                            cit->dump();
                        }
                        Out << "\n";
                        last_is_chi = true;
                    } else
                        last_is_chi = false;
                } else {
                    bool dump_preamble = false;
                    PAGEdgeList &pagEdgeList =
                        mrGen->getPAGEdgesFromInst(&inst);
                    for (const auto *edge : pagEdgeList) {
                        if (const auto *load = llvm::dyn_cast<LoadPE>(edge)) {
                            MUSet &muSet = getMUSet(load);
                            for (auto *it : muSet) {
                                if (!dump_preamble && !last_is_chi) {
                                    Out << "\n";
                                    dump_preamble = true;
                                }
                                it->dump();
                            }
                        }
                    }

                    Out << inst << "\n";

                    bool has_chi = false;
                    for (const auto *edge : pagEdgeList) {
                        if (const StorePE *store =
                                llvm::dyn_cast<StorePE>(edge)) {
                            CHISet &chiSet = getCHISet(store);
                            for (auto *it : chiSet) {
                                has_chi = true;
                                it->dump();
                            }
                        }
                    }
                    if (has_chi) {
                        Out << "\n";
                        last_is_chi = true;
                    } else
                        last_is_chi = false;
                }
            }
        }

        // dump return mu nodes
        if (hasReturnMu(fun)) {
            MUSet &return_mus = getReturnMuSet(fun);
            for (auto *return_mu : return_mus) {
                return_mu->dump();
            }
        }
    }
}
