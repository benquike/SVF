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
#include "SVF-FE/SymbolTableInfo.h"
#include "Util/SVFModule.h"
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

    void cmp_symtableid_and_pagid(string &bc_file) {
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
        SVFModule *mod = p_proj->getSVFModule();
        // here we create another project from the same file
        SVFProject proj2(bc_file);
        SVFModule *mod2 = proj2.getSVFModule();
        ASSERT_NE(mod, mod2);
        SymbolTableInfo *symInfo2 = proj2.getSymbolTableInfo();
        ASSERT_NE(symInfo, symInfo2);

        // check globals
        for (auto it1 = mod->global_begin(), it2 = mod2->global_begin();
             it1 != mod->global_end() && it2 != mod2->global_end();
             it1++, it2++) {
            ASSERT_NE(*it1, *it2);
            ASSERT_TRUE(symInfo->hasValSym(*it1));
            ASSERT_TRUE(symInfo2->hasValSym(*it2));
            ASSERT_EQ(symInfo->getValSym(*it1), symInfo2->getValSym(*it2));
        }
        // check aliases
        for (auto it1 = mod->alias_begin(), it2 = mod2->alias_begin();
             it1 != mod->alias_end() && it2 != mod2->alias_end();
             it1++, it2++) {
            ASSERT_NE(*it1, *it2);
            ASSERT_TRUE(symInfo->hasValSym(*it1));
            ASSERT_TRUE(symInfo2->hasValSym(*it2));
            ASSERT_EQ(symInfo->getValSym(*it1), symInfo2->getValSym(*it2));
        }

        auto it1 = mod->llvmFunBegin(), it2 = mod2->llvmFunBegin();
        for (; it1 != mod->llvmFunEnd() && it2 != mod2->llvmFunEnd();
             it1++, it2++) {
            Function *fun1 = *it1;
            Function *fun2 = *it2;

            ASSERT_NE(fun1, fun2);
            ASSERT_STREQ(fun1->getName().str().c_str(),
                         fun2->getName().str().c_str());

            ASSERT_TRUE(symInfo->hasValSym(fun1));
            ASSERT_TRUE(symInfo2->hasValSym(fun2));

            ASSERT_EQ(symInfo->getValSym(fun1), symInfo2->getValSym(fun2));

            auto bb1 = fun1->begin(), bb2 = fun2->begin();
            for (; bb1 != fun1->end() && bb2 != fun2->end(); bb1++, bb2++) {

                BasicBlock *pbb1 = &(*bb1), *pbb2 = &(*bb2);

                ASSERT_NE(pbb1, pbb2);

                ASSERT_TRUE(symInfo->hasValSym(pbb1));
                ASSERT_TRUE(symInfo2->hasValSym(pbb2));

                ASSERT_EQ(symInfo->getValSym(pbb1), symInfo2->getValSym(pbb2));

                auto inst1 = pbb1->begin(), inst2 = pbb2->begin();
                for (; inst1 != pbb1->end() && inst2 != pbb2->end();
                     inst1++, inst2++) {
                    Instruction *pinst1 = &(*inst1), *pinst2 = &(*inst2);
                    ASSERT_NE(pinst1, pinst2);
                    ASSERT_TRUE(symInfo->hasValSym(pinst1));
                    ASSERT_TRUE(symInfo2->hasValSym(pinst2));

                    ASSERT_EQ(symInfo->getValSym(pinst1),
                              symInfo2->getValSym(pinst2));
                }

                ASSERT_EQ(inst1, pbb1->end());
                ASSERT_EQ(inst2, pbb2->end());
            }

            ASSERT_EQ(bb1, fun1->end());
            ASSERT_EQ(bb2, fun2->end());
        }

        ASSERT_EQ(it1, mod->llvmFunEnd());
        ASSERT_EQ(it2, mod2->llvmFunEnd());

        // TODO: more tests on other values,
        // e.g., global variables, internal values etc.

        auto &symId2TypeMap_1 = symInfo->symIDToTypeMap();
        auto &symId2TypeMap_2 = symInfo2->symIDToTypeMap();
        // # of symbols should be the same
        ASSERT_EQ(symId2TypeMap_1.size(), symId2TypeMap_2.size());
        // check the type of each symbol
        for (auto &[id, type] : symId2TypeMap_1) {
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
            ASSERT_EQ(pvalue, idToVal_1[id]);
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

        // Test type info
        //
        auto &id2TypeMap1 = symInfo->getIdToTypeMap();
        auto &id2TypeMap2 = symInfo->getIdToTypeMap();

        ASSERT_EQ(id2TypeMap1.size(), id2TypeMap2.size());
        for (auto it : id2TypeMap1) {
            ASSERT_TRUE(id2TypeMap2.find(it.first) != id2TypeMap2.end());
            ASSERT_EQ(id2TypeMap1[it.first], id2TypeMap2[it.first]);
        }

        // TODO: more tests
        //
    }

    void common_symbol_table_tests(string &bc_file) {
        init_if_needed(bc_file);
        ASSERT_NE(p_proj->getSVFModule(), nullptr);
        ASSERT_NE(p_proj->getLLVMModSet(), nullptr);

        SymbolTableInfo *symInfo = p_proj->getSymbolTableInfo();

        ASSERT_NE(symInfo, nullptr);
        ASSERT_NE(symInfo->getModule(), nullptr);
        llvm::outs() << symInfo->getTotalSymNum() << "\n";
    }
};

TEST_F(SymbolTableInfoTest, Construction_0) {
    string ll_file = SVF_BUILD_DIR "tests/simple/simple_cpp.ll";

    common_symbol_table_tests(ll_file);
    cmp_symtableid_and_pagid(ll_file);
}

TEST_F(SymbolTableInfoTest, Construction_1) {
    string ll_file = SVF_BUILD_DIR "tests/CHG/callsite_cpp.ll";
    common_symbol_table_tests(ll_file);
    cmp_symtableid_and_pagid(ll_file);
}

TEST_F(SymbolTableInfoTest, Construction_2) {
    string ll_file = SVF_BUILD_DIR "tests/CPPUtils/Vtable/PureVirtual_cpp.ll";
    common_symbol_table_tests(ll_file);
    cmp_symtableid_and_pagid(ll_file);
}

TEST_F(SymbolTableInfoTest, Construction_WebGL_IR) {
    string ll_file = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";
    common_symbol_table_tests(ll_file);
    cmp_symtableid_and_pagid(ll_file);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
