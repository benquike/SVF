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
 *     2021-04-17
 *****************************************************************************/

#include "Graphs/ICFG.h"
#include "Graphs/PAG.h"
#include "Graphs/SVFG.h"
#include "Graphs/SVFGOPT.h"
#include "Graphs/VFG.h"
#include "Util/Serialization.h"
#include "config.h"
#include "gtest/gtest.h"
#include <sstream>

#include "SVF-FE/SVFProject.h"

using namespace SVF;
using namespace std;

using namespace boost::archive;

#include "Util/boost_classes_export.h"

#if 0

TEST(ICFGSerializationTestSuite, ICFGNodeTest_0) {
    stringstream ss;
    ICFGNode icfgnode;
    ICFGNode *picfgnode = &icfgnode;

    text_oarchive oa{ss};
    oa << picfgnode;
}

TEST(ICFGSerializationTestSuite, PairDefVal_test) {
    IntraCFGEdge::BranchCondition br;

    if (br.first == nullptr) {
        llvm::outs() << "def value of first is nullptr\n";

    } else {
        llvm::outs() << "def value of first is not nullptr\n";
    }

    ASSERT_EQ(br.first, nullptr);
}

TEST(ICFGSerializationTestSuite, ICFGNodeTest_1) {
    stringstream ss;
    ICFGNode icfgnode;

    text_oarchive oa{ss};

    oa << icfgnode;
}

TEST(ICFGSerializationTestSuite, GlobalBlockNodeTest_0) {
    stringstream ss;
    GlobalBlockNode gn;
    ICFGNode *picfgnode = &gn;

    text_oarchive oa{ss};

    try {
        oa << picfgnode;
    } catch (exception &e) {
        llvm::outs() << e.what() << "\n";
    }
}

TEST(ICFGSerializationTestSuite, AllNodes_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    SVFProject proj(test_bc);

    // ICFG ic;
    ICFG *icfg = proj.getICFG();

    stringstream ss;
    // save
    {
        text_oarchive oa{ss};
        // oa.register_type<ICFG>();
        // oa << ic;

        for (const auto &it : *icfg) {
            llvm::outs() << "-------------------------\n";
            llvm::outs() << "id:" << it.first << ": " << it.second->toString()
                         << "\n";
            try {
                oa << it.second;
            } catch (exception &e) {
                llvm::outs() << e.what() << "\n";
            }
            llvm::outs() << "==========================\n";
        }
    }

    // load
    // {
    //     ICFG *loaded_icfg = nullptr;
    //     text_iarchive ia{ss};
    //     ia >> loaded_icfg;
    //     ASSERT_NE(loaded_icfg, nullptr);
    // }
}

TEST(ICFGSerializationTestSuite, Test_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    SVFProject proj(test_bc);

    // ICFG ic;
    ICFG *icfg = proj.getICFG();

    stringstream ss;

    llvm::outs() << "=============  SAVE  ===============\n";
    // save
    {
        text_oarchive oa{ss};
        // oa.register_type<ICFG>();
        // oa << ic;
        try {
            oa << icfg;
        }

        catch (exception &e) {
            llvm::outs() << e.what() << "\n";
        }
    }

    llvm::outs() << "=============  LOAD  ===============\n";

    // load
    {
        ICFG *loaded_icfg = nullptr;

        text_iarchive ia{ss};

        ia >> loaded_icfg;

        ASSERT_NE(loaded_icfg, nullptr);
    }
}
#endif

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
