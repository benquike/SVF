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

#include <string>
#include <vector>

// #include "WPA/FlowSensitive.h"

#include "SVF-FE/CHG.h"
#include "config.h"

using namespace SVF;
using namespace std;

TEST(CHGTestSuite, BasicTest_0) {
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    SVFProject proj(test_bc);

    CHGraph *chg = new CHGraph(proj.getSymbolTableInfo());
    ASSERT_TRUE(chg != nullptr);

    chg->buildCHG();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
