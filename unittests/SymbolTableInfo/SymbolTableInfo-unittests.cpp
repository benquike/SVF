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

#include "SVF-FE/SVFProject.h"
#include <map>
#include <string>

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

    //  the output of this line is 3,
    //  which is what I expected.
    //  it actually only records 4 symbols
    llvm::outs() << symInfo->getTotalSymNum() << "\n";

    // these 2 pointers should point to the same object
    ASSERT_EQ(symInfo->getModule(), proj.getSVFModule());

    // very basic test
    ASSERT_EQ(symInfo->blkPtrSymID(), SYMTYPE::BlkPtr);
    ASSERT_EQ(symInfo->nullPtrSymID(), SYMTYPE::NullPtr);
    ASSERT_EQ(symInfo->constantSymID(), SYMTYPE::ConstantObj);
    ASSERT_EQ(symInfo->blackholeSymID(), SYMTYPE::BlackHole);

    // here we test that the same function always gets the
    // same id
    //
    map<string, SymID> fval_id_map;
    map<string, SymID> fobj_id_map;
    SVFModule *mod = proj.getSVFModule();
    for (auto it = mod->llvmFunBegin(); it != mod->llvmFunEnd(); it++) {
        Function *fun = *it;
        string fname = fun->getName().str();
        llvm::outs() << fname << "\n";
        ASSERT_TRUE(symInfo->hasValSym(fun));
        llvm::outs() << "Val Sym ID:" << symInfo->getValSym(fun) << "\n";
        llvm::outs() << "Obj Sym ID:" << symInfo->getObjSym(fun) << "\n";

        fval_id_map[fname] = symInfo->getValSym(fun);
        fobj_id_map[fname] = symInfo->getObjSym(fun);
    }

    // here we create another project from the same file
    SVFProject proj2(ll_file);
    SVFModule *mod2 = proj2.getSVFModule();
    SymbolTableInfo *symInfo2 = proj2.getSymbolTableInfo();
    for (auto it = mod2->llvmFunBegin(); it != mod2->llvmFunEnd(); it++) {
        Function *fun = *it;
        string fname = fun->getName().str();
        ASSERT_TRUE(symInfo2->hasValSym(fun));

        ASSERT_EQ(symInfo2->getValSym(fun), fval_id_map[fname]);
        ASSERT_EQ(symInfo2->getObjSym(fun), fobj_id_map[fname]);
    }

    // TODO: more tests on other values,
    // e.g., global variables, internal values etc.
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

TEST_F(SymbolTableInfoTest, Construction_2) {
    string ll_file = SVF_BUILD_DIR "tests/CPPUtils/Vtable/PureVirtual_cpp.ll";
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
