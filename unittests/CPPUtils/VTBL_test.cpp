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

#include "SVF-FE/CHG.h"
#include "SVF-FE/DCHG.h"
#include "SVF-FE/LLVMUtil.h"
#include "gtest/gtest.h"
#include <string>
#include <vector>

#include <llvm/Demangle/Demangle.h>

#include "SVF-FE/SVFProject.h"
#include "config.h"

using namespace SVF;
using namespace std;
using namespace llvm;
using namespace cppUtil;

using llvm::demangle;

class VTblParseTesting : public ::testing::Test {
  protected:
    SVFProject *proj;

    void SetUp() override {}

    void TearDown() override {
        delete proj;
    }

    void init(string &ll_file) {
        proj = new SVFProject(ll_file);
    }

    void dump_vtbls_from_globalvariable() {
        Module *mod = proj->getLLVMModSet()->getMainLLVMModule();
        for (auto it = mod->global_begin(); it != mod->global_end(); it++) {
            const GlobalVariable *gv = &(*it);
            if (isValVtbl(gv) && gv->getNumOperands() > 0) {
                ASSERT_TRUE(gv->hasInitializer());
                ASSERT_EQ(gv->getOperand(0), gv->getInitializer());
                llvm::outs() << "vtable:" << *gv << "\n";
                const Constant *init = gv->getInitializer();
                llvm::outs() << "init: " << *init << "\n";
                llvm::outs()
                    << "# of operands: " << gv->getNumOperands() << "\n";
                llvm::outs() << *(init->getType()) << "\n";
            }
        }
    }

    void dump_targets_for_cs(CommonCHGraph *chg, CallSite cs, int indent,
                             string &graphName) {

        auto &vfuncSet = chg->getCSVFsBasedonCHA(cs);

        for (int i = 0; i < indent; i++) {
            llvm::outs() << "\t";
        }
        llvm::outs() << "# target (from" << graphName
                     << "): " << vfuncSet.size() << "\n";

        for (auto fp : vfuncSet) {
            for (int i = 0; i <= indent; i++) {
                llvm::outs() << "\t";
            }
            llvm::outs() << " -" << llvm::demangle(fp->getName()) << "\n";
        }
    }

    void check_chgraph() {
        SymbolTableInfo *symbolTableInfo = proj->getSymbolTableInfo();
        CHGraph *chg = new CHGraph(symbolTableInfo);
        chg->buildCHG();
        // DCHGraph *dchg = new DCHGraph(svfMod);
        // dchg->buildCHG(true);

        auto *svfMod = proj->getSVFModule();

        for (auto fit = svfMod->llvmFunBegin(); fit != svfMod->llvmFunEnd();
             fit++) {

            llvm::outs() << "***** checking function: " << (*fit)->getName()
                         << "\n";

            for (auto &bb : **fit) {
                for (auto &inst : bb) {

                    if (llvm::isa<llvm::CallInst>(inst) ||
                        llvm::isa<llvm::CallBrInst>(inst) ||
                        llvm::isa<llvm::InvokeInst>(inst)) {
                        llvm::CallSite cs(&inst);
                        if (isVirtualCallSite(cs)) {
                            llvm::outs()
                                << "+++++++++ CallInst: " << inst << "\n";
                            static string chg_name("CHG");
                            dump_targets_for_cs(chg, cs, 1, chg_name);
                            // static string dchg_name("DCHG");
                            // dump_targets_for_cs(dchg, cs, 1, dchg_name);
                        }
                    }
                }
            }
        }
    }
};

TEST_F(VTblParseTesting, SimpleInheritance) {
    string ll_file =
        SVF_BUILD_DIR "tests/CPPUtils/Vtable/SimpleInheritance_cpp.ll";
    init(ll_file);
    dump_vtbls_from_globalvariable();
}

TEST_F(VTblParseTesting, MultipleInheritance) {
    string ll_file =
        SVF_BUILD_DIR "tests/CPPUtils/Vtable/MultipleInheritance_cpp.ll";
    init(ll_file);
    dump_vtbls_from_globalvariable();
}

TEST_F(VTblParseTesting, PureVirtual) {
    string ll_file = SVF_BUILD_DIR "tests/CPPUtils/Vtable/PureVirtual_cpp.ll";
    init(ll_file);
    dump_vtbls_from_globalvariable();
}

TEST_F(VTblParseTesting, VirtualInheritance) {
    string ll_file =
        SVF_BUILD_DIR "tests/CPPUtils/Vtable/VirtualInheritance_cpp_dbg.ll";
    init(ll_file);
    // dump_vtbls_from_globalvariable();
    check_chgraph();
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
