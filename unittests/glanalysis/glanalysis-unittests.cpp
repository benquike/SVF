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
#include <vector>

#include <llvm/Demangle/Demangle.h>

#include "SVF-FE/CHG.h"
#include "SVF-FE/PAGBuilder.h"

#include "config.h"
#include "gtest/gtest.h"

using namespace SVF;
using namespace std;

using llvm::demangle;

TEST(GLAnalysis, DumpVTargets) {
    string test_bc = SVF_SRC_DIR
        "tools/chrome-gl-analysis/chrome_webgl_ir/webgl_all_rendering_code.bc";
    SVFProject proj(test_bc);
    PAG *pag = proj.getPAG();

    ASSERT_TRUE(pag != nullptr);
    auto chg = make_unique<CHGraph>(pag->getSymbolTableInfo());
    ASSERT_TRUE(chg != nullptr);

    chg->buildCHG();

    // Get the indirect callsites from the PAG
    // CallBlockNode * -> FnPtrId
    auto callSitesToFunPtrIDMap = pag->getIndirectCallsites();

    for (auto [cbn, fnPtrId] : callSitesToFunPtrIDMap) {
        // auto *callInst = llvm::dyn_cast<CallInst>(cbn->getCallSite());
        CallSite cs = SVFUtil::getLLVMCallSite(cbn->getCallSite());

        if (!cppUtil::isVirtualCallSite(cs)) {
            continue;
        }

        const auto *fun = cbn->getFun();

        if (!chg->csHasVFnsBasedonCHA(cs)) {
            continue;
        }

        const auto dname = llvm::demangle(fun->getLLVMFun()->getName().str());
        llvm::outs() << dname << " --> ";
        llvm::outs() << *cbn->getCallSite() << "\n";

        auto vTargets = chg->getCSVFsBasedonCHA(cs);
        for (const auto *tgt : vTargets) {
            llvm::outs() << "\t -"
                         << llvm::demangle(tgt->getLLVMFun()->getName().str())
                         << "\n";
        }
    }
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
