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

using namespace std;
using namespace SVF;

class ConsGraphTestSuite : public ::testing::Test {
  protected:
    unique_ptr<SVFProject> p_proj;
    string test_file;

    void init(string &bc_file) {
        test_file = bc_file;
        p_proj = make_unique<SVFProject>(bc_file);
    }

    void cmp_test() {
        {
            ASSERT_EQ(p_proj->getPAG(), p_proj->getPAG());

            unique_ptr<ConstraintGraph> cg1 =
                make_unique<ConstraintGraph>(p_proj->getPAG());
            unique_ptr<ConstraintGraph> cg2 =
                make_unique<ConstraintGraph>(p_proj->getPAG());

            graph_eq_test(cg1.get(), cg2.get());
        }
    }
};

TEST_F(ConsGraphTestSuite, StaticCallTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    init(test_bc);
    cmp_test();
}

TEST_F(ConsGraphTestSuite, FPtrTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";
    init(test_bc);
    cmp_test();
}

TEST_F(ConsGraphTestSuite, VirtTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";
    init(test_bc);
    cmp_test();
}

TEST_F(ConsGraphTestSuite, VirtTest_1) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    init(test_bc);
    cmp_test();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
