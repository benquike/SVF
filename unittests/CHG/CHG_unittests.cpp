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
 *     2021-03-19
 *****************************************************************************/

#include "SVF-FE/PAGBuilder.h"
#include "gtest/gtest.h"

#include <memory>
#include <string>
#include <vector>

// #include "WPA/FlowSensitive.h"

#include "SVF-FE/CHG.h"
#include "config.h"

using namespace SVF;
using namespace std;

class CHGTestSuite : public ::testing::Test {
  protected:
    unique_ptr<CHGraph> p_chg;

    void test_input_common(string &bc_fname) {
        SVFProject proj(bc_fname);

        p_chg = make_unique<CHGraph>(proj.getSymbolTableInfo());
        p_chg->buildCHG();
        // p_chg->view();
        for (const auto it : *p_chg) {
            llvm::outs() << it.first << "\n";
        }
    }
};

TEST_F(CHGTestSuite, BasicTest_0) {
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    test_input_common(test_bc);
}

TEST_F(CHGTestSuite, BasicTest_1) {
    string test_bc = SVF_BUILD_DIR "/tests/ICFG/virt_call_test_cpp.ll";
    test_input_common(test_bc);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
