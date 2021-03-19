/******************************************************************************
 * Copyright (c) 2021 Hui Peng.
 * All rights reserved. This program and the accompanying materials are made
 * available under the private copyright of Hui Peng. It is only for studying
 * purpose, usage for other purposes is not allowed.
 *
 * Author:
 *     Hui Peng <peng124@purdue.edu>
 * Date:
 *     2021-03-13
 *****************************************************************************/
#include <string>
#include <vector>

#include <llvm/Demangle/Demangle.h>

#include "SVF-FE/PAGBuilder.h"
#include "SVF-FE/CHG.h"

#include "gtest/gtest.h"
#include "config.h"

using namespace SVF;
using namespace std;

using llvm::demangle;

TEST(GLAnalysis, DumpVTargets) {
    string test_bc = SVF_SRC_DIR
        "/unittests/glanalysis/webgl-ir/webgl_all_rendering_code.bc";
    vector<string> moduleNameVec{test_bc};

    SVFModule *svfModule =
        LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);
    PAG _pag(svfModule);
    PAG *pag = &_pag;

    ASSERT_TRUE(pag != nullptr);
    CHGraph *chg = new CHGraph(pag->getSymbolTableInfo());
    ASSERT_TRUE(chg != nullptr);

    chg->buildCHG();


    // Get the indirect callsites from the PAG
    // CallBlockNode * -> FnPtrId
    auto callSitesToFunPtrIDMap = pag->getIndirectCallsites();

    for (auto [cbn, fnPtrId]: callSitesToFunPtrIDMap) {
        // auto *callInst = llvm::dyn_cast<CallInst>(cbn->getCallSite());
        CallSite cs = SVFUtil::getLLVMCallSite(cbn->getCallSite());

        if (!cppUtil::isVirtualCallSite(cs)) {
            continue;
        }

        const auto *fun = cbn->getFun();


        if (!chg->csHasVFnsBasedonCHA(cs)) {
            continue;
        }

        const auto dname = llvm::demangle(fun->getLLVMFun()->getName().str());
        llvm::outs() << dname << " --> ";
        llvm::outs() << *cbn->getCallSite() << "\n";

        auto vTargets = chg->getCSVFsBasedonCHA(cs);
        for (const auto *tgt: vTargets) {
            llvm::outs() << "\t -" <<
                llvm::demangle(tgt->getLLVMFun()->getName().str()) << "\n";
        }
    }

}


int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
