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
 *     2021-03-21
 *****************************************************************************/

#include "SVF-FE/SVFProject.h"
#include <string>

#include "config.h"
#include "gtest/gtest.h"

using namespace std;
using namespace SVF;

TEST(SVFProjectTests, Test0) {
    string ll_file = SVF_BUILD_DIR "tests/simple/simple_cpp.ll";
    SVFProject proj(ll_file);

    ASSERT_NE(proj.getSVFModule(), nullptr);
    ASSERT_NE(proj.getSymbolTableInfo(), nullptr);
    ASSERT_NE(proj.getLLVMModSet(), nullptr);
    ASSERT_NE(proj.getICFG(), nullptr);
    ASSERT_NE(proj.getPAG(), nullptr);
    ASSERT_NE(proj.getThreadAPI(), nullptr);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
