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

#include "Tests/Graphs/GraphTest_Routines.hpp"
#include "Tests/Graphs/ICFG.hpp"

using namespace std;
using namespace SVF;

class VFGTestSuite : public ::testing::Test {
  protected:
    unique_ptr<SVFProject> p_proj;
    string test_file;

    void init(string &bc_file) {
        test_file = bc_file;
        p_proj = make_unique<SVFProject>(bc_file);
    }

    void dump_indirect_callsites(PAG *pag, PTACallGraph *callgraph) {
        llvm::outs() << "================================\n";
        for (auto [cbn, fptr_id] : pag->getIndirectCallsites()) {
            llvm::outs() << "  ++++++++++++++++++++++++++++\n";
            auto callInst = cbn->getCallSite();
            llvm::outs() << "  Instr:" << *(callInst) << "\n";

            /// dump the source location
            if (auto *loc = SVFUtil::getDILocation(callInst)) {
                llvm::outs()
                    << "  src: " << loc->getFile()->getFilename() << "\n";
                llvm::outs() << "  line: " << loc->getLine() << "\n";
                llvm::outs()
                    << "  code: " << SVFUtil::getSrcCodeFromIR(callInst)
                    << "\n";
            }

            llvm::outs() << "  fptr value:"
                         << *(pag->getGNode(fptr_id)->getValue()) << "\n";

            if (callgraph) {
                llvm::outs() << "  callees:\n";
                PTACallGraph::FunctionSet fs;
                callgraph->getCallees(cbn, fs);
                for (auto callee : fs) {
                    llvm::outs() << "    -" << callee->getName() << "\n";
                }
            }

            llvm::outs() << "  --------------------------\n";
        }

        llvm::outs() << "**********************************\n";
    }

    void cmp_test(bool vcall_analysis = false, bool thread_cg = false) {

        llvm::outs() << "test_bc:" << test_file << "\n";

        llvm::outs() << "vcall_analysis:" << vcall_analysis
                     << ", thread_cg:" << thread_cg << "\n";

        Andersen *ander = AndersenWaveDiff::createAndersenWaveDiff(
            p_proj.get(), vcall_analysis, thread_cg);

        llvm::outs() << "ander built\n";
        Andersen *ander2 = AndersenWaveDiff::createAndersenWaveDiff(
            p_proj.get(), vcall_analysis, thread_cg);
        llvm::outs() << "ander2 built\n";
        PAG *pag = p_proj->getPAG();
        dump_indirect_callsites(pag, ander->getPTACallGraph());
        dump_indirect_callsites(pag, ander2->getPTACallGraph());

        llvm::outs() << "doing graph euquality test\n";
        PTACallGraph *callgraph = ander->getPTACallGraph();

        {
            unique_ptr<VFG> vfg1 = make_unique<VFG>(callgraph, pag);
            unique_ptr<VFG> vfg2 = make_unique<VFG>(callgraph, pag);

            llvm::outs() << "test VFG\n";
            graph_eq_test(vfg1.get(), vfg2.get());
        }

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
        //// FIXME: memory leak bugs to be fixed.
        {
            FlowSensitive *fs_pta
                = FlowSensitive::createFSWPA(p_proj.get(), true);
            PTACallGraph *callgraph = fs_pta->getPTACallGraph();
            SVFGBuilder svfBuilder;
            SVFG *svfg1 = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);
            SVFG *svfg2 = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);
            /// llvm::outs() << "test SVFG2\n";
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
    cmp_test(true, true);
    cmp_test(false, true);
    cmp_test(true, false);
    cmp_test(false, false);
}

#if 0
TEST_F(VFGTestSuite, StaticCallTest_1) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    init(test_bc);

    auto andersen = AndersenWaveDiff::createAndersenWaveDiff(p_proj.get());
    PTACallGraph *callgraph = andersen->getPTACallGraph();

    llvm::outs() << "start dumping indirect callsites\n";
    dump_indirect_callsites(p_proj->getPAG(), nullptr);
    llvm::outs() << "end dumping indirect callsites\n";

    auto vfg = make_unique<VFG>(callgraph, p_proj->getPAG());

    delete andersen;
}
#endif

TEST_F(VFGTestSuite, FPtrTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp_dbg.ll";
    init(test_bc);
    cmp_test(true, true);
    cmp_test(false, true);
    cmp_test(true, false);
    cmp_test(false, false);
}

#if 0
TEST_F(VFGTestSuite, FPtrTest_1) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";
    init(test_bc);

    llvm::outs() << "start dumping indirect callsites\n";
    dump_indirect_callsites();
    llvm::outs() << "end dumping indirect callsites\n";
}
#endif

TEST_F(VFGTestSuite, VirtTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";
    init(test_bc);
    cmp_test();
    cmp_test(true, true);
    cmp_test(false, true);
    cmp_test(true, false);
    cmp_test(false, false);
}

TEST_F(VFGTestSuite, VirtTest_1) {
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    init(test_bc);
    cmp_test(true, true);
    cmp_test(false, true);
    cmp_test(true, false);
    cmp_test(false, false);
}

TEST_F(VFGTestSuite, MultiInheritance_0) {
    string test_bc =
        SVF_BUILD_DIR "tests/CPPUtils/Vtable/MultipleInheritance_cpp.ll";

    init(test_bc);

    cmp_test(true, true);
    cmp_test(false, true);
    cmp_test(true, false);
    cmp_test(false, false);
}

TEST_F(VFGTestSuite, MultiInheritance_1) {
    string test_bc =
        SVF_BUILD_DIR "tests/CPPUtils/Vtable/"
                      "MultipleInheritance_varying_signature_cpp_dbg.ll";
    init(test_bc);

    cmp_test(true, true);
    cmp_test(false, true);
    cmp_test(true, false);
    cmp_test(false, false);
}

TEST_F(VFGTestSuite, WebGL_IR_0) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";
    init(test_bc);

    cmp_test(true, true);
    // cmp_test(false, true);
    // cmp_test(true, false);
    // cmp_test(false, false);
}

/// here we construct 2 project and compare the VFG
TEST_F(VFGTestSuite, WebGL_IR_1) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";

    SVFProject proj1(test_bc);
    auto ander1 = unique_ptr<AndersenWaveDiff>(
        AndersenWaveDiff::createAndersenWaveDiff(&proj1));
    unique_ptr<VFG> vfg1 =
        make_unique<VFG>(ander1->getPTACallGraph(), proj1.getPAG());

    llvm::outs() << "vfg1 done\n";
    SVFProject proj2(test_bc);
    auto ander2 = unique_ptr<AndersenWaveDiff>(
        AndersenWaveDiff::createAndersenWaveDiff(&proj2));
    unique_ptr<VFG> vfg2 =
        make_unique<VFG>(ander1->getPTACallGraph(), proj2.getPAG());
    llvm::outs() << "vfg1 done\n";
    graph_eq_test(vfg1.get(), vfg2.get());
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
