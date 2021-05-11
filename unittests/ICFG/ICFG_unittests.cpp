/******************************************************************************
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Author:
 *     Hui Peng <peng124@purdue.edu>
 * Date:
 *     2021-04-18
 *****************************************************************************/

#include <memory>
#include <string>

#include "SVF-FE/SVFProject.h"
#include "WPA/FlowSensitive.h"
#include "config.h"
#include "gtest/gtest.h"

#include "Tests/Graphs/GraphTest_Routines.hpp"
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

    // FlowSensitive *fs_pta = FlowSensitive::createFSWPA(p_proj.get(), true);

    /// Call Graph
    // PTACallGraph *callgraph = fs_pta->getPTACallGraph();

    // callgraph->view();

    // SVFGBuilder svfBuilder(true);
    // SVFG *svfg = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);

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

    // FlowSensitive *fs_pta = FlowSensitive::createFSWPA(p_proj.get(), true);
    /// Call Graph
    // PTACallGraph *callgraph = fs_pta->getPTACallGraph();

    // callgraph->view();

    // SVFGBuilder svfBuilder(true);
    // SVFG *svfg = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);

    // icfg->view();
}

#if 0
TEST_F(ICFGTestSuite, WebGL_TEST_0) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";
    init(test_bc);

    ICFG *icfg = p_proj->getICFG();

    ASSERT_TRUE(icfg != nullptr);
    ASSERT_TRUE(icfg->getPAG() != nullptr);

    llvm::outs().flush();

    llvm::outs() << "\n\n=====================\n\n";

    unique_ptr<ICFG> icfg2 = make_unique<ICFG>(icfg->getPAG());
    // unique_ptr<ICFG> icfg3 = make_unique<ICFG>(icfg->getPAG());
    // this test won't pass because PAGBuilder
    // will update the ICFG
    // in PAGBuilder::handleExtCall
    // This is just a note
    // #0  SVF::IntraBlockNode::IntraBlockNode (this=0x6100028f5c40, id=229217,
    // i=0x61000003cdb8, svfMod=0x610000000040) at
    // /home/peng124/src/SVF/include/Graphs/ICFGNode.h:209 #1 0x00007ffff7504a66
    // in SVF::ICFG::addIntraBlockICFGNode (this=0x6160003bb280,
    // inst=0x61000003cdb8) at /home/peng124/src/SVF/lib/Graphs/ICFG.cpp:451 #2
    // 0x00007ffff75009b0 in SVF::ICFG::getIntraBlockNode (this=0x6160003bb280,
    // inst=0x61000003cdb8) at /home/peng124/src/SVF/lib/Graphs/ICFG.cpp:272 #3
    // 0x00007ffff79c6b43 in SVF::PAGBuilder::addStoreEdge (this=0x7fffffffc020,
    // src=315045, dst=315043) at
    // /home/peng124/src/SVF/include/SVF-FE/PAGBuilder.h:286 #4
    // 0x00007ffff79bfa32 in SVF::PAGBuilder::addComplexConsForExt
    // (this=0x7fffffffc020, D=0x607000320f08, S=0x60d000892238, sz=2) at
    // /home/peng124/src/SVF/lib/SVF-FE/PAGBuilder.cpp:988 #5 0x00007ffff79c0b8c
    // in SVF::PAGBuilder::handleExtCall (this=0x7fffffffc020, cs=...,
    // callee=0x60400070b890) at
    // /home/peng124/src/SVF/lib/SVF-FE/PAGBuilder.cpp:1089 #6
    // 0x00007ffff79bdee4 in SVF::PAGBuilder::visitCallSite
    // (this=0x7fffffffc020, cs=...) at
    // /home/peng124/src/SVF/lib/SVF-FE/PAGBuilder.cpp:781 #7 0x00007ffff79ee184
    // in SVF::PAGBuilder::visitCallInst (this=0x7fffffffc020, I=...) at
    // /home/peng124/src/SVF/include/SVF-FE/PAGBuilder.h:146 #8
    // 0x00007ffff79edea3 in llvm::InstVisitor<SVF::PAGBuilder,
    // void>::visitIntrinsicInst (this=0x7fffffffc020, I=...) at
    // /usr/lib/llvm-10/include/llvm/IR/InstVisitor.h:219 #9  0x00007ffff79ee283
    // in llvm::InstVisitor<SVF::PAGBuilder, void>::visitMemIntrinsic
    // (this=0x7fffffffc020, I=...) at
    // /usr/lib/llvm-10/include/llvm/IR/InstVisitor.h:215 #10 0x00007ffff79ee253
    // in llvm::InstVisitor<SVF::PAGBuilder, void>::visitMemTransferInst
    // (this=0x7fffffffc020, I=...) at
    // /usr/lib/llvm-10/include/llvm/IR/InstVisitor.h:214 #11 0x00007ffff79edf63
    // in llvm::InstVisitor<SVF::PAGBuilder, void>::visitMemCpyInst
    // (this=0x7fffffffc020, I=...) at
    // /usr/lib/llvm-10/include/llvm/IR/InstVisitor.h:212 #12 0x00007ffff79edd92
    // in llvm::InstVisitor<SVF::PAGBuilder, void>::delegateCallInst
    // (this=0x7fffffffc020, I=...) at
    // /usr/lib/llvm-10/include/llvm/IR/InstVisitor.h:310 #13 0x00007ffff79ed16d
    // in llvm::InstVisitor<SVF::PAGBuilder, void>::visitCall
    // (this=0x7fffffffc020, I=...) at
    // /usr/lib/llvm-10/include/llvm/IR/Instruction.def:209
    // #14 0x00007ffff79c4b6f in llvm::InstVisitor<SVF::PAGBuilder, void>::visit
    // (this=0x7fffffffc020, I=...) at
    // /usr/lib/llvm-10/include/llvm/IR/Instruction.def:209 #15
    // 0x00007ffff79b7168 in SVF::PAGBuilder::build (this=0x7fffffffc020) at
    // /home/peng124/src/SVF/lib/SVF-FE/PAGBuilder.cpp:158

    // graph_eq_test(icfg2.get(), icfg);
}
#endif

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
