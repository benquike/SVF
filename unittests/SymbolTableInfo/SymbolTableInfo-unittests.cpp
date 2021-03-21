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

#include <string>

#include "SVF-FE/SVFProject.h"

#include "config.h"
#include "gtest/gtest.h"

using namespace std;
using namespace SVF;

class SymbolTableInfoTest : public ::testing::Test {
  protected:
};

TEST_F(SymbolTableInfoTest, Construction_0) {
    string ll_file = SVF_BUILD_DIR "tests/simple/simple_cpp.ll";
    SVFProject proj(ll_file);

    ASSERT_NE(proj.getSVFModule(), nullptr);

    ASSERT_NE(proj.getLLVMModSet(), nullptr);
    SymbolTableInfo *symInfo = proj.getSymbolTableInfo();

    ASSERT_NE(symInfo, nullptr);

    ASSERT_TRUE(symInfo->getModule());

    llvm::outs() << symInfo->getTotalSymNum() << "\n";
}

TEST_F(SymbolTableInfoTest, Construction_1) {
    string ll_file = SVF_BUILD_DIR "tests/CHG/callsite_cpp.ll";
    SVFProject proj(ll_file);
    ASSERT_NE(proj.getSVFModule(), nullptr);
    ASSERT_NE(proj.getLLVMModSet(), nullptr);

    SymbolTableInfo *symInfo = proj.getSymbolTableInfo();

    ASSERT_NE(symInfo, nullptr);

    ASSERT_TRUE(symInfo->getModule());

    llvm::outs() << symInfo->getTotalSymNum() << "\n";
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
