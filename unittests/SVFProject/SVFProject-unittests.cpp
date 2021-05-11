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
#include "Util/SVFModule.h"
#include <memory>
#include <string>

#include "Graphs/ICFG.h"
#include "Graphs/PAG.h"

#include "Tests/Graphs/GraphTest_Routines.hpp"
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

TEST(SVFProjectTests, ResetTest) {
    string ll_file = SVF_BUILD_DIR "tests/simple/simple_cpp.ll";
    SVFProject proj1(ll_file);
    ASSERT_EQ(SVFProject::getCurrentProject(), &proj1);

    {
        SVFProject proj(ll_file);
        ASSERT_EQ(SVFProject::getCurrentProject(), &proj);
    }

    ASSERT_EQ(SVFProject::getCurrentProject(), nullptr);
    SVFProject::setCurrentProject(&proj1);
    ASSERT_EQ(SVFProject::getCurrentProject(), &proj1);
}

void pag_and_icfg_eq_test(SVFProject *proj1, SVFProject *proj2) {
    ASSERT_NE(proj1, proj2);

    auto pag1 = proj1->getPAG();
    auto pag2 = proj2->getPAG();

    graph_eq_test(pag1, pag2);
    graph_eq_test(proj1->getICFG(), proj2->getICFG());
}

void symbol_table_eq_test(SymbolTableInfo *tbl1, SymbolTableInfo *tbl2) {
    ASSERT_NE(tbl1, tbl2);

    auto idToVal1 = tbl1->idToValSym();
    auto idToVal2 = tbl2->idToValSym();

    ASSERT_EQ(idToVal1.size(), idToVal2.size());
    auto it1 = idToVal1.begin();
    auto it2 = idToVal2.begin();

    for (; it1 != idToVal1.end() && it2 != idToVal2.end(); it1++, it2++) {
        ASSERT_EQ(it1->first, it2->first);
    }
}

void SVFMod_eq_test(SVFModule *mod1, SVFModule *mod2) {
    ASSERT_NE(mod1, mod2);

    ASSERT_STREQ(mod1->getModuleIdentifier().c_str(),
                 mod2->getMduleIdentifier().c_str());

    ASSERT_EQ(mod1->getAliasSet().size(), mod2->getAliasSet().size());

    ASSERT_EQ(mod1->getGlobalSet().size(), mod2->getGlobalSet().size());
    ASSERT_EQ(mod1->getLLVMFuncToSVFFuncMap().size(),
              mod2->getLLVMFuncToSVFFuncMap().size());
    ASSERT_EQ(mod1->getFunctionSet().size(), mod2->getFunctionSet().size());
    ASSERT_EQ(mod1->getLLVMFunctionSet().size(),
              mod2->getLLVMFunctionSet().size());
}

TEST(SVFProjectTests, StaticCallTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";

    auto proj1 = make_unique<SVFProject>(test_bc);
    auto proj2 = make_unique<SVFProject>(test_bc);

    pag_and_icfg_eq_test(proj1.get(), proj2.get());
    symbol_table_eq_test(proj1->getSymbolTableInfo(),
                         proj2->getSymbolTableInfo());
    SVFMod_eq_test(proj1->getSVFModule(), proj2->getSVFModule());
}

TEST(SVFProjectTests, FPtrTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/fptr_test_cpp.ll";

    auto proj1 = make_unique<SVFProject>(test_bc);
    auto proj2 = make_unique<SVFProject>(test_bc);

    pag_and_icfg_eq_test(proj1.get(), proj2.get());
    symbol_table_eq_test(proj1->getSymbolTableInfo(),
                         proj2->getSymbolTableInfo());
    SVFMod_eq_test(proj1->getSVFModule(), proj2->getSVFModule());
}

TEST(SVFProjectTests, VirtTest_0) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "tests/ICFG/virt_call_test_cpp.ll";

    auto proj1 = make_unique<SVFProject>(test_bc);
    auto proj2 = make_unique<SVFProject>(test_bc);

    pag_and_icfg_eq_test(proj1.get(), proj2.get());
    symbol_table_eq_test(proj1->getSymbolTableInfo(),
                         proj2->getSymbolTableInfo());
    SVFMod_eq_test(proj1->getSVFModule(), proj2->getSVFModule());
}

TEST(SVFProjectTests, VirtTest_1) {
    // string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";
    string test_bc = SVF_BUILD_DIR "/tests/CHG/callsite_cpp.ll";

    auto proj1 = make_unique<SVFProject>(test_bc);
    auto proj2 = make_unique<SVFProject>(test_bc);

    pag_and_icfg_eq_test(proj1.get(), proj2.get());
    symbol_table_eq_test(proj1->getSymbolTableInfo(),
                         proj2->getSymbolTableInfo());
    SVFMod_eq_test(proj1->getSVFModule(), proj2->getSVFModule());
}

TEST(SVFProjectTests, WebGL_0) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";

    auto proj1 = make_unique<SVFProject>(test_bc);
    auto proj2 = make_unique<SVFProject>(test_bc);

    pag_and_icfg_eq_test(proj1.get(), proj2.get());
    symbol_table_eq_test(proj1->getSymbolTableInfo(),
                         proj2->getSymbolTableInfo());
    SVFMod_eq_test(proj1->getSVFModule(), proj2->getSVFModule());
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
