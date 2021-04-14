#include <string>

#include "gtest/gtest.h"
#include "config.h"

#include "SVF-FE/SVFProject.h"
#include "WPA/FlowSensitive.h"

using namespace std;
using namespace SVF;

TEST(ICFGTestSuite, FPtrTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";
    SVFProject proj(test_bc);

    ICFG *icfg = proj.getICFG();

    ASSERT_TRUE(icfg != nullptr);
    ASSERT_TRUE(icfg->getPAG() != nullptr);

    icfg->view();
}


TEST(ICFGTestSuite, VirtTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";
    SVFProject proj(test_bc);

    ICFG *icfg = proj.getICFG();

    ASSERT_TRUE(icfg != nullptr);
    ASSERT_TRUE(icfg->getPAG() != nullptr);

    FlowSensitive *fs_pta = FlowSensitive::createFSWPA(&proj, true);

    /// Call Graph
    PTACallGraph *callgraph = fs_pta->getPTACallGraph();

    callgraph->view();

    SVFGBuilder svfBuilder(true);
    SVFG *svfg = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);

    // icfg->view();
}

TEST(ICFGTestSuite, VirtTest_1) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    SVFProject proj(test_bc);

    ICFG *icfg = proj.getICFG();

    ASSERT_TRUE(icfg != nullptr);
    ASSERT_TRUE(icfg->getPAG() != nullptr);

    FlowSensitive *fs_pta = FlowSensitive::createFSWPA(&proj, true);

    /// Call Graph
    PTACallGraph *callgraph = fs_pta->getPTACallGraph();

    callgraph->view();

    // SVFGBuilder svfBuilder(true);
    // SVFG *svfg = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);

    // icfg->view();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();;
}
