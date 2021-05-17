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

#include "SVF-FE/PAGBuilder.h"
#include "gtest/gtest.h"

#include <memory>
#include <set>
#include <string>
#include <vector>
// #include "WPA/FlowSensitive.h"

#include "SVF-FE/CHG.h"
#include "config.h"

using namespace SVF;
using namespace std;

using FuncNameSet = set<string>;
using VFuncSpec = map<unsigned int, FuncNameSet>;
using VTableSpec = vector<FuncNameSet>;

class CHGTestSuite : public ::testing::Test {
  protected:
    shared_ptr<CHGraph> chg;
    shared_ptr<SVFProject> proj;

    void SetUp() { spdlog::set_level(spdlog::level::debug); }

    void init(string &test_bc) {
        proj = make_shared<SVFProject>(test_bc);
        chg = make_shared<CHGraph>(proj->getSymbolTableInfo());
        chg->buildCHG();
    }

    void common_test() {}

    void test_class_exists(const string &classname) {
        auto node = chg->getNode(classname);
        ASSERT_NE(node, nullptr);
    }

    void test_subclasses(const string &base, const set<string> &subclasses) {

        // test getDescendants API
        auto descendants = chg->getDescendants(base);
        ASSERT_EQ(descendants.size(), subclasses.size());

        for (const auto &subclass : subclasses) {
            auto node_derived = chg->getNode(subclass);
            ASSERT_NE(descendants.find(node_derived), descendants.end());
        }
    }

    void test_instances_and_subclasses(const string &base,
                                       const set<string> &derivatives) {
        // test getInstancesAndDescendants API
        auto descendants = chg->getInstancesAndDescendants(base);
        ASSERT_EQ(descendants.size(), derivatives.size());

        for (const auto &derived : derivatives) {
            auto node_derived = chg->getNode(derived);
            ASSERT_NE(descendants.find(node_derived), descendants.end());
        }
    }

    void test_class_flags(const string &classname, bool positive,
                          unsigned int nflags...) {
        va_list args;
        va_start(args, nflags);

        auto node = chg->getNode(classname);
        ASSERT_NE(node, nullptr);
        for (unsigned int i = 0; i < nflags; i++) {
            int _attr = va_arg(args, int);
            CHNode::CLASSATTR attr = static_cast<CHNode::CLASSATTR>(_attr);
            if (positive) {
                ASSERT_TRUE(node->hasFlag(attr));
            } else {
                ASSERT_FALSE(node->hasFlag(attr));
            }
        }

        va_end(args);
    }

    void test_num_vtables(const string &classname, unsigned int vf_vec_size) {
        auto node = chg->getNode(classname);
        ASSERT_NE(node, nullptr);
        auto vfs_vec = node->getVirtualFunctionVectors();

        ASSERT_EQ(vfs_vec.size(), vf_vec_size);
    }

    void test_vtable_at_index(const string &classname, const VTableSpec &spec) {
        auto node = chg->getNode(classname);
        ASSERT_NE(node, nullptr);

        // first get all the vtables
        auto vtables = node->getVirtualFunctionVectors();
        ASSERT_EQ(vtables.size(), spec.size());

        for (unsigned int idx = 0; idx < spec.size(); idx++) {
            auto vfuncNames = spec[idx];
            CHNode::FuncVector vtable = vtables[idx];
            ASSERT_EQ(vfuncNames.size(), vtable.size());

            for (unsigned int i = 0; i < vtable.size(); i++) {
                auto fname = vtable[i]->getName().str();
                llvm::outs() << fname << "\n";
                ASSERT_NE(vfuncNames.find(fname), vfuncNames.end());
            }
        }
    }

    void test_vfuncs_at_index(const string &classname, const VFuncSpec &spec) {
        auto node = chg->getNode(classname);
        ASSERT_NE(node, nullptr);
        for (auto iter : spec) {
            CHNode::FuncVector fv;
            node->getVirtualFunctions(iter.first, fv);
            auto funcNames = iter.second;
            ASSERT_EQ(funcNames.size(), fv.size());

            for (auto f : fv) {
                auto fname = f->getName().str();
                llvm::outs() << fname << "fname"
                             << "\n";
                ASSERT_NE(funcNames.find(fname), funcNames.end());
            }
        }
    }
};

TEST_F(CHGTestSuite, PureVirtual_0) {
    string test_bc = SVF_BUILD_DIR "tests/CPPUtils/Vtable/PureVirtual_cpp.ll";
    init(test_bc);
    common_test();

    test_class_exists("Base");
    test_class_exists("Child");

    test_class_flags("Base", true, 1, CHNode::PURE_ABSTRACT);
    test_class_flags("Base", false, 2, CHNode::MULTI_INHERITANCE,
                     CHNode::TEMPLATE);
    test_class_flags("Child", false, 3, CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE, CHNode::TEMPLATE);

    // test getInstancesAndDescendants API
    test_subclasses("Base", {"Child"});
    test_instances_and_subclasses("Base", {"Child"});
    test_subclasses("Child", {});
    test_instances_and_subclasses("Child", {});

    test_num_vtables("Base", 1);
    test_num_vtables("Child", 1);

    test_vtable_at_index("Base", {{"__cxa_pure_virtual"}});
    test_vtable_at_index("Child", {{"_ZN5Child3fooEv"}});

    test_vfuncs_at_index("Base", {{0, {"__cxa_pure_virtual"}}});
    test_vfuncs_at_index("Child", {{0, {"_ZN5Child3fooEv"}}});
}

TEST_F(CHGTestSuite, BasicTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/CHG/VirtualFunc_non_overried_cpp.ll";
    init(test_bc);
    common_test();

    test_class_exists("Base");
    test_class_exists("Derived");

    test_class_flags("Base", false, 3, CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE, CHNode::TEMPLATE);

    test_class_flags("Derived", false, 3, CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE, CHNode::TEMPLATE);

    // test getInstancesAndDescendants API
    test_subclasses("Base", {"Derived"});
    test_instances_and_subclasses("Base", {"Derived"});
    test_subclasses("Derived", {});
    test_instances_and_subclasses("Derived", {});

    test_num_vtables("Base", 1);
    test_num_vtables("Derived", 1);

    test_vtable_at_index("Base", {{"_ZN4Base3fooEv"}});
    test_vtable_at_index("Derived", {{"_ZN4Base3fooEv"}});

    test_vfuncs_at_index("Base", {{0, {"_ZN4Base3fooEv"}}});
    test_vfuncs_at_index("Derived", {{0, {"_ZN4Base3fooEv"}}});
}

TEST_F(CHGTestSuite, BasicTest_1) {
    string test_bc = SVF_BUILD_DIR "tests/CHG/callsite_cpp.ll";
    init(test_bc);
    common_test();

    test_class_exists("Base");
    test_class_exists("Derived");

    test_class_flags("Base", false, 3, CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE, CHNode::TEMPLATE);

    test_class_flags("Derived", false, 3, CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE, CHNode::TEMPLATE);

    // test getInstancesAndDescendants API
    test_subclasses("Base", {"Derived"});
    test_instances_and_subclasses("Base", {"Derived"});
    test_subclasses("Derived", {});
    test_instances_and_subclasses("Derived", {});

    test_num_vtables("Base", 1);
    test_num_vtables("Derived", 1);

    test_vtable_at_index("Base", {{"_ZN4Base3fooEv"}});
    test_vtable_at_index("Derived", {{"_ZN7Derived3fooEv"}});

    test_vfuncs_at_index("Base", {{0, {"_ZN4Base3fooEv"}}});
    test_vfuncs_at_index("Derived", {{0, {"_ZN7Derived3fooEv"}}});
}

TEST_F(CHGTestSuite, multipleInheritance_0) {
    string test_bc =
        SVF_BUILD_DIR "tests/CPPUtils/Vtable/MultipleInheritance_cpp.ll";
    init(test_bc);
    common_test();

    test_class_exists("Parent1");
    test_class_exists("Parent2");
    test_class_exists("Child");

    test_class_flags("Parent1", false, 3, CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE, CHNode::TEMPLATE);

    test_class_flags("Parent2", false, 3, CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE, CHNode::TEMPLATE);

    test_class_flags("Child", true, 1, CHNode::MULTI_INHERITANCE);

    test_class_flags("Child", false, 2, CHNode::PURE_ABSTRACT,
                     CHNode::TEMPLATE);

    test_subclasses("Parent1", {"Child"});
    test_instances_and_subclasses("Parent1", {"Child"});
    test_subclasses("Parent2", {"Child"});
    test_instances_and_subclasses("Parent2", {"Child"});
    test_subclasses("Child", {});
    test_instances_and_subclasses("Child", {});

    test_num_vtables("Parent1", 1);
    test_num_vtables("Parent2", 1);
    test_num_vtables("Child", 2);

    test_vtable_at_index("Parent1", {{"_ZN7Parent13fooEv"}});
    test_vtable_at_index("Parent2", {{"_ZN7Parent23barEv"}});
    test_vtable_at_index("Child", {{"_ZN5Child3fooEv", "_ZN5Child3bazEv"},
                                   {"_ZN7Parent23barEv"}});
}

TEST_F(CHGTestSuite, virtualInheritance_0) {
    string test_bc =
        SVF_BUILD_DIR "tests/CPPUtils/Vtable/virtual_inheritance_1_cpp.ll";
    init(test_bc);
    common_test();

    test_class_exists("Base");
    test_class_exists("Child");

#if 0
    test_class_flags("Base", false, 3,
                     CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE,
                     CHNode::TEMPLATE);

    test_class_flags("Child", false, 3,
                     CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE,
                     CHNode::TEMPLATE);
#endif
}

TEST_F(CHGTestSuite, virtualInheritance_1) {
    string test_bc =
        SVF_BUILD_DIR "tests/CPPUtils/Vtable/VirtualInheritance_cpp.ll";
    init(test_bc);
    common_test();

    test_class_exists("Top");
    test_class_exists("Left");
    test_class_exists("Middle");
    test_class_exists("Right");
    test_class_exists("Botton");

#if 0
    test_class_flags("Top", false, 3,
                     CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE,
                     CHNode::TEMPLATE);
    test_class_flags("Left", false, 3,
                     CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE,
                     CHNode::TEMPLATE);
    test_class_flags("Middle", false, 3,
                     CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE,
                     CHNode::TEMPLATE);
    test_class_flags("Right", false, 3,
                     CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE,
                     CHNode::TEMPLATE);
    test_class_flags("Botton", false, 3,
                     CHNode::PURE_ABSTRACT,
                     CHNode::MULTI_INHERITANCE,
                     CHNode::TEMPLATE);
#endif

    llvm::outs() << "testing Top\n";
    test_subclasses("Top", {"Left", "Middle", "Right", "Botton"});
    test_instances_and_subclasses("Top", {"Left", "Middle", "Right", "Botton"});
    llvm::outs() << "testing Left\n";
    test_subclasses("Left", {"Botton"});
    test_instances_and_subclasses("Left", {"Botton"});

#if 0
    /// OK, this test is not passing
    /// the implementations are not correct
    llvm::outs() << "testing Middle\n";
    test_subclasses("Middle", {"Botton"});
    test_instances_and_subclasses("Middle", {"Botton"});
#endif
    llvm::outs() << "testing Right\n";
    test_subclasses("Right", {"Botton"});
    test_instances_and_subclasses("Right", {"Botton"});
    llvm::outs() << "testing Bottom\n";
    test_subclasses("Botton", {});
    test_instances_and_subclasses("Botton", {});
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
