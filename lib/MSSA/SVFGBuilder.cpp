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
    spdlog::debug("Going to build PTROnlySVFG_OPT");
    return build(pta, VFG::PTRONLYSVFG_OPT);
}

SVFG *SVFGBuilder::buildPTROnlySVFGWithoutOPT(BVDataPTAImpl *pta) {
    spdlog::debug("Going to build PTROnlySVFG");
    return build(pta, VFG::PTRONLYSVFG);
}

SVFG *SVFGBuilder::buildFullSVFG(BVDataPTAImpl *pta) {
    spdlog::debug("Going to build FullSVFG_OPT");
    return build(pta, VFG::FULLSVFG_OPT);
}

SVFG *SVFGBuilder::buildFullSVFGWithoutOPT(BVDataPTAImpl *pta) {
    spdlog::debug("Going to build FullSVFG");
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

    /// Note that we use callgraph from andersen analysis here
    spdlog::debug("Starting to create SVFG");
    if (kind == VFG::FULLSVFG_OPT || kind == VFG::PTRONLYSVFG_OPT)
        svfg = new SVFGOPT(mssa, pta->getPAG(), kind);
    else
        svfg = new SVFG(mssa, pta->getPAG(), kind);
    spdlog::debug("Done creating SVFG");

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

    spdlog::debug("Starting to build MemSSA object");

    auto *mssa = new MemSSA(pta, ptrOnlyMSSA);

    DominatorTree dt;
    MemSSADF df;

    SVFModule *svfModule = mssa->getPTA()->getSVFModule();
    for (const auto *fun : *svfModule) {

        spdlog::debug("Starting to build MemSSA on function: {}",
                      fun->getName().str());

        if (isExtCall(fun)) {
            spdlog::debug("{} is an external function, skipping",
                          fun->getName().str());

            continue;
        }

        dt.recalculate(*fun->getLLVMFun());
        df.runOnDT(dt);

        mssa->buildMemSSA(*fun, &df, &dt);
        spdlog::debug("Done building MemSSA on function: {}",
                      fun->getName().str());
    }

    mssa->performStat();
    mssa->dumpMSSA();

    spdlog::debug("Done building MemSSA object");

    return mssa;
}
