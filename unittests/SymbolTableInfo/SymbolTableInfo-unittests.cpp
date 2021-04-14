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

#include <llvm/Demangle/Demangle.h>
#include <map>
#include <memory>
#include <string>

#include "SVF-FE/SVFProject.h"
#include "config.h"
#include "gtest/gtest.h"

using namespace std;
using namespace SVF;

class SymbolTableInfoTest : public ::testing::Test {
    protected:
        unique_ptr<SVFProject> p_proj;

        void init_if_needed(string &bc_file) {
            if (p_proj == nullptr) {
                p_proj = make_unique<SVFProject>(bc_file);
            }
        }

        void common_symbol_table_tests(string &bc_file) {
            init_if_needed(bc_file);

            SymbolTableInfo *symInfo = p_proj->getSymbolTableInfo();
            ASSERT_NE(symInfo, nullptr);

            ASSERT_TRUE(symInfo->getModule());

            //  the output of this line is 3,
            //  which is what I expected.
            //  it actually only records 4 symbols
            llvm::outs() << symInfo->getTotalSymNum() << "\n";

            // these 2 pointers should point to the same object
            ASSERT_EQ(symInfo->getModule(), p_proj->getSVFModule());

            // Test the special nodes
            ASSERT_EQ(symInfo->blkPtrSymID(), SYMTYPE::BlkPtr);
            ASSERT_EQ(symInfo->nullPtrSymID(), SYMTYPE::NullPtr);
            ASSERT_EQ(symInfo->constantSymID(), SYMTYPE::ConstantObj);
            ASSERT_EQ(symInfo->blackholeSymID(), SYMTYPE::BlackHole);

            // here we test that the same function always gets the
            // same id
            //
            map<string, SymID> fval_id_map;
            map<string, SymID> fobj_id_map;
            SVFModule *mod = p_proj->getSVFModule();
            for (auto it = mod->llvmFunBegin();
                 it != mod->llvmFunEnd(); it++) {
                Function *fun = *it;
                string fname = fun->getName().str();
                llvm::outs() << "function name:`" << fname << "`\n";
                llvm::outs() << "demangled function name:`"
                             << llvm::demangle(fname) << "`\n";
                ASSERT_TRUE(symInfo->hasValSym(fun));
                llvm::outs() << "Val Sym ID:" << symInfo->getValSym(fun) << "\n";
                llvm::outs() << "Obj Sym ID:" << symInfo->getObjSym(fun) << "\n";

                fval_id_map[fname] = symInfo->getValSym(fun);
                fobj_id_map[fname] = symInfo->getObjSym(fun);
            }

            // here we create another project from the same file
            SVFProject proj2(bc_file);
            SVFModule *mod2 = proj2.getSVFModule();
            ASSERT_NE(mod, mod2);

            SymbolTableInfo *symInfo2 = proj2.getSymbolTableInfo();
            ASSERT_NE(symInfo, symInfo2);
            for (auto it = mod2->llvmFunBegin();
                 it != mod2->llvmFunEnd(); it++) {
                Function *fun = *it;
                string fname = fun->getName().str();
                ASSERT_TRUE(symInfo2->hasValSym(fun));

                ASSERT_EQ(symInfo2->getValSym(fun), fval_id_map[fname]);
                ASSERT_EQ(symInfo2->getObjSym(fun), fobj_id_map[fname]);
            }

            // TODO: more tests on other values,
            // e.g., global variables, internal values etc.

            auto &symId2TypeMap_1 = symInfo->symIDToTypeMap();
            auto &symId2TypeMap_2 = symInfo2->symIDToTypeMap();
            // # of symbols should be the same
            ASSERT_EQ(symId2TypeMap_1.size(), symId2TypeMap_2.size());
            // check the type of each symbol
            for (auto &[id, type]: symId2TypeMap_1) {
                ASSERT_NE(symId2TypeMap_2.find(id), symId2TypeMap_2.end());
                ASSERT_EQ(type, symId2TypeMap_2[id]);
            }

            // check the value symbols
            auto &valSyms_1 = symInfo->valSyms();
            auto &idToVal_1 = symInfo->idToValueMap();
            auto &valSyms_2 = symInfo2->valSyms();
            auto &idToVal_2 = symInfo2->idToValueMap();
            ASSERT_EQ(valSyms_1.size(), valSyms_2.size());
            ASSERT_EQ(valSyms_1.size(), idToVal_1.size());
            ASSERT_EQ(valSyms_2.size(), idToVal_2.size());
            for (auto &[pvalue, id] : valSyms_1) {
                ASSERT_EQ(pvalue,  idToVal_1[id]);
                ASSERT_NE(idToVal_2.find(id), idToVal_2.end());
                // the ptr to value should be different
                ASSERT_NE(pvalue, idToVal_2[id]);
            }

            auto &objSyms_1 = symInfo->objSyms();
            auto &idToObj_1 = symInfo->idToObjMap();
            auto &objSyms_2 = symInfo2->objSyms();
            auto &idToObj_2 = symInfo2->idToObjMap();
            ASSERT_EQ(objSyms_1.size(), objSyms_2.size());
            ASSERT_EQ(idToObj_1.size(), idToObj_2.size());

            for (auto &[pvalue, id] : objSyms_1) {
                // ASSERT_EQ(pvalue,  idToObj_1[id]);
                ASSERT_NE(idToObj_2.find(id), idToObj_2.end());
            }

            auto &retSyms_1 = symInfo->retSyms();
            auto &retSyms_2 = symInfo2->retSyms();
            ASSERT_EQ(retSyms_1.size(), retSyms_2.size());

            auto &varargSyms_1 = symInfo->varargSyms();
            auto &varargSyms_2 = symInfo2->varargSyms();
            ASSERT_EQ(varargSyms_1.size(), varargSyms_2.size());

            // TODO: more tests
        }

        void cmp_symtableid_and_pagid(string &bc_file) {
            init_if_needed(bc_file);
        }
};

TEST_F(SymbolTableInfoTest, Construction_0) {
    string ll_file = SVF_BUILD_DIR "tests/simple/simple_cpp.ll";

    common_symbol_table_tests(ll_file);
    cmp_symtableid_and_pagid(ll_file);
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
