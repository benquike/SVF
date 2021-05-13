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

#include "Graphs/PTACallGraph.h"
#include "SVF-FE/SVFProject.h"
#include "WPA/FlowSensitive.h"

#include "config.h"
#include "gtest/gtest.h"

#include "Tests/Graphs/GraphTest_Routines.hpp"
#include "Tests/Graphs/ICFG.hpp"

using namespace std;
using namespace SVF;

class PTACGTestSuite : public ::testing::Test {
  protected:
    unique_ptr<SVFProject> p_proj;

    void init(string &bc_file) {
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%H:%M:%S %z] [%!] [%^---%L---%$] [thread %t] %v");
        p_proj = make_unique<SVFProject>(bc_file);
    }
};

TEST_F(PTACGTestSuite, StaticCallTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    init(test_bc);
    auto fs_pta1 = unique_ptr<FlowSensitive>(
        FlowSensitive::createFSWPA(p_proj.get(), true));

    PTACallGraph *callgraph1 = fs_pta1->getPTACallGraph();

    SVFProject proj2(test_bc);

    auto fs_pta2 =
        unique_ptr<FlowSensitive>(FlowSensitive::createFSWPA(&proj2, true));

    PTACallGraph *callgraph2 = fs_pta2->getPTACallGraph();

    graph_eq_test(callgraph1, callgraph2);
}

TEST_F(PTACGTestSuite, FPtrTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";

    init(test_bc);
    auto fs_pta1 = unique_ptr<FlowSensitive>(
        FlowSensitive::createFSWPA(p_proj.get(), true));
    PTACallGraph *callgraph1 = fs_pta1->getPTACallGraph();

    SVFProject proj2(test_bc);
    auto fs_pta2 =
        unique_ptr<FlowSensitive>(FlowSensitive::createFSWPA(&proj2, true));
    PTACallGraph *callgraph2 = fs_pta2->getPTACallGraph();

    graph_eq_test(callgraph1, callgraph2);
}

TEST_F(PTACGTestSuite, VirtTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";

    init(test_bc);
    auto fs_pta1 = unique_ptr<FlowSensitive>(
        FlowSensitive::createFSWPA(p_proj.get(), true));
    PTACallGraph *callgraph1 = fs_pta1->getPTACallGraph();

    SVFProject proj2(test_bc);
    auto fs_pta2 =
        unique_ptr<FlowSensitive>(FlowSensitive::createFSWPA(&proj2, true));
    PTACallGraph *callgraph2 = fs_pta2->getPTACallGraph();

    graph_eq_test(callgraph1, callgraph2);
}

TEST_F(PTACGTestSuite, VirtTest_1) {
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    init(test_bc);
    auto fs_pta1 = unique_ptr<FlowSensitive>(
        FlowSensitive::createFSWPA(p_proj.get(), true));

    PTACallGraph *callgraph1 = fs_pta1->getPTACallGraph();

    SVFProject proj2(test_bc);
    auto fs_pta2 =
        unique_ptr<FlowSensitive>(FlowSensitive::createFSWPA(&proj2, true));
    PTACallGraph *callgraph2 = fs_pta2->getPTACallGraph();

    graph_eq_test(callgraph1, callgraph2);
}

TEST_F(PTACGTestSuite, thread_cg_0) {
    string test_bc = SVF_BUILD_DIR "tests/ThreadAPI/SimplePThread_example_c.ll";
    init(test_bc);
    /// this callgraph does not use ThreadCallGraph
    auto fs_pta1 = unique_ptr<FlowSensitive>(
        FlowSensitive::createFSWPA(p_proj.get(), true));

    PTACallGraph *callgraph1 = fs_pta1->getPTACallGraph();

    callgraph1->view();

    // SVFProject proj2(test_bc);
    // auto fs_pta2 =
    //     unique_ptr<FlowSensitive>(FlowSensitive::createFSWPA(&proj2, true));
    // PTACallGraph *callgraph2 = fs_pta2->getPTACallGraph();

    // graph_eq_test(callgraph1, callgraph2);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
