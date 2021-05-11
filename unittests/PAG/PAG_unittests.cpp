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
 *     2021-04-17
 *****************************************************************************/

#include <memory>
#include <string>

#include "Graphs/PAG.h"
#include "SVF-FE/SVFProject.h"

#include "config.h"
#include "gtest/gtest.h"

#include "Tests/Graphs/GraphTest_Routines.hpp"
#include "Tests/Graphs/ICFG.hpp"

using namespace std;
using namespace SVF;

class PAGTestSuite : public ::testing::Test {
  protected:
    unique_ptr<SVFProject> p_proj;

    void init(string &bc_file) { p_proj = make_unique<SVFProject>(bc_file); }
};

TEST_F(PAGTestSuite, StaticCallTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    init(test_bc);

    PAG *pag = p_proj->getPAG();
    ASSERT_TRUE(pag != nullptr);

    unique_ptr<PAG> pag2 = make_unique<PAG>(p_proj.get());
    ASSERT_EQ(pag->getSymbolTableInfo(), pag2->getSymbolTableInfo());
    graph_eq_test(pag, pag2.get());
}

TEST_F(PAGTestSuite, FPtrTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";
    init(test_bc);

    PAG *pag = p_proj->getPAG();
    ASSERT_TRUE(pag != nullptr);

    unique_ptr<PAG> pag2 = make_unique<PAG>(p_proj.get());
    ASSERT_EQ(pag->getSymbolTableInfo(), pag2->getSymbolTableInfo());
    graph_eq_test(pag, pag2.get());
}

TEST_F(PAGTestSuite, VirtTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";
    init(test_bc);

    PAG *pag = p_proj->getPAG();
    ASSERT_TRUE(pag != nullptr);

    unique_ptr<PAG> pag2 = make_unique<PAG>(p_proj.get());
    ASSERT_EQ(pag->getSymbolTableInfo(), pag2->getSymbolTableInfo());
    graph_eq_test(pag, pag2.get());
}

TEST_F(PAGTestSuite, VirtTest_1) {
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    init(test_bc);

    PAG *pag = p_proj->getPAG();
    ASSERT_TRUE(pag != nullptr);

    unique_ptr<PAG> pag2 = make_unique<PAG>(p_proj.get());
    ASSERT_EQ(pag->getSymbolTableInfo(), pag2->getSymbolTableInfo());
    graph_eq_test(pag, pag2.get());
}

TEST_F(PAGTestSuite, WebGL_PAG_Test_1) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";
    init(test_bc);

    PAG *pag = p_proj->getPAG();
    ASSERT_TRUE(pag != nullptr);

    unique_ptr<PAG> pag2 = make_unique<PAG>(p_proj.get());
    ASSERT_EQ(pag->getSymbolTableInfo(), pag2->getSymbolTableInfo());
    graph_eq_test(pag, pag2.get());
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    llvm::cl::ParseCommandLineOptions(argc, argv, "PAG unittests\n");
    return RUN_ALL_TESTS();
}
