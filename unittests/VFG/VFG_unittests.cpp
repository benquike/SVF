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
#include "WPA/Andersen.h"
#include "WPA/FlowSensitive.h"
#include "config.h"
#include "gtest/gtest.h"

#include "Tests/Graphs/GenericGraph.hpp"
#include "Tests/Graphs/ICFG.hpp"

using namespace std;
using namespace SVF;

class VFGTestSuite : public ::testing::Test {
  protected:
    unique_ptr<SVFProject> p_proj;
    string test_file;

    void init(string &bc_file) {
        test_file = bc_file;
        // p_proj = make_unique<SVFProject>(bc_file);
    }

    void cmp_test() {
        // Andersen *ander =
        // AndersenWaveDiff::createAndersenWaveDiff(p_proj.get());

        SVFProject proj(test_file);
        Andersen *ander = AndersenWaveDiff::createAndersenWaveDiff(&proj);
        Andersen *ander2 = AndersenWaveDiff::createAndersenWaveDiff(&proj);
        PAG *pag = proj.getPAG();
        PTACallGraph *callgraph = ander->getPTACallGraph();

        // #if 0
        {
            unique_ptr<VFG> vfg1 = make_unique<VFG>(callgraph, pag);
            unique_ptr<VFG> vfg2 = make_unique<VFG>(callgraph, pag);

            llvm::outs() << "test VFG\n";
            graph_eq_test(vfg1.get(), vfg2.get());
        }
        // #endif

        {
            SVFGBuilder svfBuilder;
            SVFG *svfg1 = svfBuilder.buildFullSVFGWithoutOPT(ander);
            SVFG *svfg2 = svfBuilder.buildFullSVFGWithoutOPT(ander2);

            llvm::outs() << "test SVFG\n";
            graph_eq_test(svfg1, svfg2);

            delete svfg1;
            delete svfg2;
        }

#if 0

        {
            FlowSensitive *fs_pta = FlowSensitive::createFSWPA(&proj, true);
            PTACallGraph *callgraph = fs_pta->getPTACallGraph();
            SVFGBuilder svfBuilder;
            SVFG *svfg1 = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);
            SVFG *svfg2 = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);
            llvm::outs() << "test SVFG2\n";
            graph_eq_test(svfg1, svfg2);

            delete fs_pta;
        }
#endif
        delete ander;
        delete ander2;
    }
};

TEST_F(VFGTestSuite, StaticCallTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    init(test_bc);
    cmp_test();
}

TEST_F(VFGTestSuite, FPtrTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";
    init(test_bc);
    cmp_test();
}

TEST_F(VFGTestSuite, VirtTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";
    init(test_bc);
    cmp_test();
}

TEST_F(VFGTestSuite, VirtTest_1) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    init(test_bc);
    cmp_test();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
