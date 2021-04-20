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

#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>

#include "Graphs/ICFG.h"
#include "Graphs/PAG.h"
#include "Graphs/SVFG.h"
#include "Graphs/SVFGOPT.h"
#include "Graphs/VFG.h"
#include "Util/Serialization.h"
#include "config.h"
#include "gtest/gtest.h"
#include <memory>
#include <sstream>

#include "SVF-FE/SVFProject.h"

using namespace SVF;
using namespace std;

using namespace boost::archive;
// using namespace boost::filesystem;

#include "Util/boost_classes_export.h"

class ICFGSerializationTestSuite : public ::testing::Test {
  protected:
    unique_ptr<SVFProject> p_proj;
    string test_file;
    string bin_archive_file;

    void init(string &bc_file) {
        test_file = bc_file;
        bin_archive_file = test_file + ".bin.archive";
        p_proj = make_unique<SVFProject>(bc_file);
    }

    void test_save() {
        string bin_archive_file = test_file + ".bin.archive";
        ofstream ofs(bin_archive_file);

        {
            ICFG *icfg = p_proj->getICFG();
            binary_oarchive oa{ofs};
            oa << icfg;
        }
    }

    void test_load() {
        if (!boost::filesystem::exists(bin_archive_file)) {
            llvm::outs() << "bin archive: " << bin_archive_file
                         << " dose not exist\n";
            return;
        }

        ifstream ifs(bin_archive_file);
        {
            ICFG *icfg = nullptr;
            binary_iarchive ia{ifs};
            ia >> icfg;
        }
    }
};

TEST_F(ICFGSerializationTestSuite, ICFGNodeTest_0) {
    stringstream ss;
    ICFGNode icfgnode;
    ICFGNode *picfgnode = &icfgnode;

    text_oarchive oa{ss};
    oa << picfgnode;
}

TEST_F(ICFGSerializationTestSuite, PairDefVal_test) {
    IntraCFGEdge::BranchCondition br;

    if (br.first == nullptr) {
        llvm::outs() << "def value of first is nullptr\n";

    } else {
        llvm::outs() << "def value of first is not nullptr\n";
    }

    ASSERT_EQ(br.first, nullptr);
}

TEST_F(ICFGSerializationTestSuite, ICFGNodeTest_1) {
    stringstream ss;
    ICFGNode icfgnode;

    text_oarchive oa{ss};

    oa << icfgnode;
}

TEST_F(ICFGSerializationTestSuite, GlobalBlockNodeTest_0) {
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

TEST_F(ICFGSerializationTestSuite, AllNodes_0) {
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
#if 0
TEST_F(ICFGSerializationTestSuite, StaticCallTest_0_save) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    init(test_bc);

    // SVFProject proj(test_bc);
    // ICFG ic;
    // ICFG *icfg = proj.getICFG();
    // stringstream ss;

    llvm::outs() << "=============  SAVE  ===============\n";
    test_save();
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

#define DEF_TESTS(NAME, test_path)                                             \
    TEST_F(ICFGSerializationTestSuite, NAME##_save) {                          \
        string test_bc = SVF_BUILD_DIR test_path;                              \
        init(test_bc);                                                         \
        llvm::outs() << "=============  SAVE  ===============\n";              \
        test_save();                                                           \
    }                                                                          \
    TEST_F(ICFGSerializationTestSuite, NAME##_load) {                          \
        string test_bc = SVF_BUILD_DIR test_path;                              \
        init(test_bc);                                                         \
        llvm::outs() << "=============  LOAD  ===============\n";              \
        test_load();                                                           \
    }

DEF_TESTS(StaticCallTest_0, "tests/ICFG/static_call_test_cpp.ll")
DEF_TESTS(FPtrTest_0, "tests/ICFG/fptr_test_cpp.ll")
DEF_TESTS(VirtTest_0, "tests/ICFG/virt_call_test_cpp.ll")
DEF_TESTS(VirtTest_1, "tests/CHG/callsite_cpp.ll")

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
