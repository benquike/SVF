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
 *     2021-05-10
 *****************************************************************************/

#include <memory>

#include "Graphs/SVFG.h"
#include "Graphs/SVFGOPT.h"
#include "Graphs/VFG.h"
#include "SVF-FE/SVFProject.h"
#include "Tests/Graphs/GraphTest_Routines.hpp"
#include "Util/Serialization.h"

#include "WPA/Andersen.h"
#include "config.h"
#include "gtest/gtest.h"

#include <sstream>

#include "Util/boost_classes_export.h"

using namespace SVF;
using namespace std;

using namespace boost::archive;

class VFGSerializationTestSuite : public ::testing::Test {

  protected:
    void SetUp() {
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%H:%M:%S %z] [%n] [%^---%L---%$] [thread %t] %v");
    }

    void test_nodes_serialization(SVFProject &currProj, shared_ptr<VFG> g,
                                  string &test_bc, bool new_proj = false) {
        for (auto it : *g) {

            llvm::outs() << "======== testing node:" << it.first << "\n";
            stringstream ss;
            auto node = it.second;

            SVFProject::setCurrentProject(&currProj);

            // SAVE to ss
            {
                text_oarchive oa{ss};
                oa << node;
            }
            /// LOAD and test
            {
                VFGNode *n = nullptr;
                shared_ptr<SVFProject> _proj;
                if (new_proj) {
                    _proj = make_shared<SVFProject>(test_bc);
                }

                text_iarchive ia{ss};
                ia >> n;
                ASSERT_NE(n, nullptr);
                node_eq_test(n, node);
                boost::serialization::access::destroy(n);
                // delete n;
            }
        }
    }

    void test_edges_serialization(SVFProject &currProj, shared_ptr<VFG> g,
                                  string &test_bc, bool new_proj = false) {
        for (auto it = g->edge_begin(); it != g->edge_end(); it++) {
            llvm::outs() << "======== testing edge:" << it->first << "\n";
            stringstream ss;
            auto edge = it->second;

            SVFProject::setCurrentProject(&currProj);
            // SAVE to ss
            {
                text_oarchive oa{ss};
                oa << edge;
            }
            /// LOAD and test
            {
                VFGEdge *e = nullptr;

                unique_ptr<SVFProject> _proj;
                if (new_proj) {
                    _proj = make_unique<SVFProject>(test_bc);
                }
                text_iarchive ia{ss};
                ia >> e;

                ASSERT_NE(e, nullptr);
                edge_eq_test(e, edge);
                boost::serialization::access::destroy(e);
                // delete e;
            }
        }
    }

    void test_graph_serialization(SVFProject &currProj, shared_ptr<VFG> g,
                                  string &test_bc, bool new_proj = false) {
        stringstream ss;

        SVFProject::setCurrentProject(&currProj);
        /// SAVE the graph to ss
        {
            text_oarchive oa{ss};
            oa << g.get();
        }

        {
            VFG *vfg = nullptr;
            shared_ptr<SVFProject> _proj;
            if (new_proj) {
                _proj = make_shared<SVFProject>(test_bc);
            }

            text_iarchive ia{ss};
            ia >> vfg;
            ASSERT_NE(vfg, nullptr);
            node_and_edge_id_test(g.get());
            node_and_edge_id_test(vfg);
            graph_eq_test(vfg, g.get());
            boost::serialization::access::destroy(vfg);
        }
    }

    void vfg_node_and_edge_serialization_test(string &test_bc) {
        SVFProject proj(test_bc);
        auto ander = unique_ptr<AndersenWaveDiff>(
            AndersenWaveDiff::createAndersenWaveDiff(&proj));
        PAG *pag = proj.getPAG();
        PTACallGraph *callgraph = ander->getPTACallGraph();
        auto vfg = make_shared<VFG>(callgraph, pag);

        test_nodes_serialization(proj, vfg, test_bc);
        test_edges_serialization(proj, vfg, test_bc);
        test_graph_serialization(proj, vfg, test_bc);
        test_nodes_serialization(proj, vfg, test_bc, true);
        test_edges_serialization(proj, vfg, test_bc, true);
        test_graph_serialization(proj, vfg, test_bc, true);
    }

    void svfg_node_and_edge_serialization_test(string &test_bc,
                                               bool graph_only = false) {
        SVFProject proj(test_bc);
        auto fs_pta =
            unique_ptr<FlowSensitive>(FlowSensitive::createFSWPA(&proj, true));
        PAG *pag = proj.getPAG();

        PTACallGraph *callgraph = fs_pta->getPTACallGraph();
        SVFGBuilder svfBuilder;
        auto svfg =
            shared_ptr<SVFG>(svfBuilder.buildFullSVFGWithoutOPT(fs_pta.get()));

        if (!graph_only) {
            test_nodes_serialization(proj, svfg, test_bc);
            test_edges_serialization(proj, svfg, test_bc);
        }
        test_graph_serialization(proj, svfg, test_bc);

        if (!graph_only) {
            test_nodes_serialization(proj, svfg, test_bc, true);
            test_edges_serialization(proj, svfg, test_bc, true);
        }

        test_graph_serialization(proj, svfg, test_bc, true);
    }
};

TEST_F(VFGSerializationTestSuite, StaticCallTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    vfg_node_and_edge_serialization_test(test_bc);
    svfg_node_and_edge_serialization_test(test_bc);
}

TEST_F(VFGSerializationTestSuite, FPtrTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";
    vfg_node_and_edge_serialization_test(test_bc);
    svfg_node_and_edge_serialization_test(test_bc);
}

TEST_F(VFGSerializationTestSuite, VirtTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";
    vfg_node_and_edge_serialization_test(test_bc);
    svfg_node_and_edge_serialization_test(test_bc);
}

TEST_F(VFGSerializationTestSuite, VirtTest_1) {
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    vfg_node_and_edge_serialization_test(test_bc);
    svfg_node_and_edge_serialization_test(test_bc);
}

#if 0
// The following tests are still failing
// comment then out for the mement

// 1. this test runs stack overflow.
TEST_F(VFGSerializationTestSuite, WebGL_VFG_0) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";
    vfg_node_and_edge_serialization_test(test_bc);
}


// this is due to a bug in SVFG construction
TEST_F(VFGSerializationTestSuite, WebGL_SVFG_all) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";
    svfg_node_and_edge_serialization_test(test_bc);
}

TEST_F(VFGSerializationTestSuite, WebGL_SVFG_graph_only) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";
    svfg_node_and_edge_serialization_test(test_bc, true);
}

#endif

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
