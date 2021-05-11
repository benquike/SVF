//===- SVFGBuilder.cpp -- SVFG builder----------------------------------------//
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
 * SVFGBuilder.cpp
 *
 *  Created on: Apr 15, 2014
 *      Author: Yulei Sui
 */

#include "MSSA/SVFGBuilder.h"
#include "Graphs/SVFG.h"
#include "MSSA/MemSSA.h"
#include "SVF-FE/LLVMUtil.h"
#include "Util/Options.h"
#include "Util/SVFModule.h"
#include "WPA/Andersen.h"
#include <sstream>

using namespace SVF;
using namespace SVFUtil;

SVFG *SVFGBuilder::buildPTROnlySVFG(BVDataPTAImpl *pta) {
    return build(pta, VFG::PTRONLYSVFG_OPT);
}

SVFG *SVFGBuilder::buildPTROnlySVFGWithoutOPT(BVDataPTAImpl *pta) {
    return build(pta, VFG::PTRONLYSVFG);
}

SVFG *SVFGBuilder::buildFullSVFG(BVDataPTAImpl *pta) {
    return build(pta, VFG::FULLSVFG_OPT);
}

SVFG *SVFGBuilder::buildFullSVFGWithoutOPT(BVDataPTAImpl *pta) {
    return build(pta, VFG::FULLSVFG);
}

/*!
 * Create SVFG
 */
void SVFGBuilder::buildSVFG() {
    MemSSA *mssa = svfg->getMSSA();
    svfg->buildSVFG();
    if (mssa->getPTA()->printStat())
        svfg->performStat();
}

/// Create DDA SVFG
SVFG *SVFGBuilder::build(BVDataPTAImpl *pta, VFG::VFGK kind) {

    MemSSA *mssa = buildMSSA(
        pta, (VFG::PTRONLYSVFG == kind || VFG::PTRONLYSVFG_OPT == kind));

    DBOUT(DGENERAL, outs() << pasMsg("Build Sparse Value-Flow Graph \n"));
    /// Note that we use callgraph from andersen analysis here
    if (kind == VFG::FULLSVFG_OPT || kind == VFG::PTRONLYSVFG_OPT)
        svfg = new SVFGOPT(mssa, pta->getPAG(), kind);
    else
        svfg = new SVFG(mssa, pta->getPAG(), kind);
    buildSVFG();

    /// Update call graph using pre-analysis results
    if (Options::SVFGWithIndirectCall || SVFGWithIndCall)
        svfg->updateCallGraph(pta);

    if (Options::DumpVFG)
        svfg->dump("svfg_final");

    return svfg;
}

/*!
 * Release memory
 */
void SVFGBuilder::releaseMemory() { svfg->clearMSSA(); }

MemSSA *SVFGBuilder::buildMSSA(BVDataPTAImpl *pta, bool ptrOnlyMSSA) {

    DBOUT(DGENERAL, outs() << pasMsg("Build Memory SSA \n"));

    auto *mssa = new MemSSA(pta, ptrOnlyMSSA);

    DominatorTree dt;
    MemSSADF df;

    SVFModule *svfModule = mssa->getPTA()->getSVFModule();
    for (const auto *fun : *svfModule) {

        if (isExtCall(fun))
            continue;

        dt.recalculate(*fun->getLLVMFun());
        df.runOnDT(dt);

        mssa->buildMemSSA(*fun, &df, &dt);
    }

    mssa->performStat();
    mssa->dumpMSSA();

    return mssa;
}
