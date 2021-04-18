#include <memory>
#include <string>

#include "SVF-FE/SVFProject.h"
#include "WPA/FlowSensitive.h"
#include "config.h"
#include "gtest/gtest.h"

#include "Tests/Graphs/GenericGraph.hpp"
#include "Tests/Graphs/ICFG.hpp"

using namespace std;
using namespace SVF;

class ICFGTestSuite : public ::testing::Test {
  protected:
    unique_ptr<SVFProject> p_proj;

    void init(string &bc_file) { p_proj = make_unique<SVFProject>(bc_file); }
};

TEST_F(ICFGTestSuite, StaticCallTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    init(test_bc);

    ICFG *icfg = p_proj->getICFG();

    ASSERT_TRUE(icfg != nullptr);
    ASSERT_TRUE(icfg->getPAG() != nullptr);

    unique_ptr<ICFG> icfg2 = make_unique<ICFG>(icfg->getPAG());
    graph_eq_test(icfg, icfg2.get());

    // icfg->view();
}

TEST_F(ICFGTestSuite, FPtrTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";
    init(test_bc);

    ICFG *icfg = p_proj->getICFG();

    ASSERT_TRUE(icfg != nullptr);
    ASSERT_TRUE(icfg->getPAG() != nullptr);

    unique_ptr<ICFG> icfg2 = make_unique<ICFG>(icfg->getPAG());
    graph_eq_test(icfg, icfg2.get());

    // icfg->view();
}

TEST_F(ICFGTestSuite, VirtTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";
    init(test_bc);

    ICFG *icfg = p_proj->getICFG();

    ASSERT_TRUE(icfg != nullptr);
    ASSERT_TRUE(icfg->getPAG() != nullptr);

    unique_ptr<ICFG> icfg2 = make_unique<ICFG>(icfg->getPAG());
    graph_eq_test(icfg, icfg2.get());

    FlowSensitive *fs_pta = FlowSensitive::createFSWPA(p_proj.get(), true);

    /// Call Graph
    PTACallGraph *callgraph = fs_pta->getPTACallGraph();

    // callgraph->view();

    SVFGBuilder svfBuilder(true);
    SVFG *svfg = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);

    // icfg->view();
}

TEST_F(ICFGTestSuite, VirtTest_1) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    init(test_bc);

    ICFG *icfg = p_proj->getICFG();

    ASSERT_TRUE(icfg != nullptr);
    ASSERT_TRUE(icfg->getPAG() != nullptr);

    unique_ptr<ICFG> icfg2 = make_unique<ICFG>(icfg->getPAG());
    graph_eq_test(icfg, icfg2.get());

    FlowSensitive *fs_pta = FlowSensitive::createFSWPA(p_proj.get(), true);

    /// Call Graph
    // PTACallGraph *callgraph = fs_pta->getPTACallGraph();

    // callgraph->view();

    // SVFGBuilder svfBuilder(true);
    // SVFG *svfg = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);

    // icfg->view();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
