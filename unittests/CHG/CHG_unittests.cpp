/******************************************************************************
 * Copyright (c) 2021 Hui Peng.
 * All rights reserved. This program and the accompanying materials are made
 * available under the private copyright of Hui Peng. It is only for studying
 * purpose, usage for other purposes is not allowed.
 *
 * Author:
 *     Hui Peng <peng124@purdue.edu>
 * Date:
 *     2021-02-27
 *****************************************************************************/
#include "gtest/gtest.h"
#include "SVF-FE/PAGBuilder.h"

#include <string>
#include <vector>

// #include "WPA/FlowSensitive.h"

#include "SVF-FE/CHG.h"

#include "config.h"

using namespace SVF;
using namespace std;

TEST(CHGTestSuite, BasicTest_0) {
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    vector<string> moduleNameVec{test_bc};
    SVFModule *svfModule =
    LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);
    PAGBuilder builder;
    PAG *pag = builder.build(svfModule);

    ASSERT_TRUE(pag != nullptr);
	CHGraph *chg = new CHGraph(pag->getModule());
    ASSERT_TRUE(chg != nullptr);

    chg->buildCHG();

}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
