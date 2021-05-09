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
 *     2021-04-26
 *****************************************************************************/

#include "gtest/gtest.h"
#include <iostream>
#include <memory>

#include "Graphs/ConsG.h"
#include "Graphs/ICFG.h"
#include "Graphs/OfflineConsG.h"
#include "Graphs/PAG.h"
#include "Graphs/PTACallGraph.h"
#include "Graphs/SVFG.h"
#include "Graphs/SVFGOPT.h"
#include "Graphs/ThreadCallGraph.h"
#include "Graphs/VFG.h"
#include "SVF-FE/CHG.h"
#include "SVF-FE/DCHG.h"
#include "Util/Casting.h"

using namespace SVF;
using namespace std;

#define define_cast_test(FROM_TYPE, TO_TYPE)                                   \
    {                                                                          \
        auto *_p = new FROM_TYPE();                                            \
        ASSERT_TRUE(llvm::isa<TO_TYPE>(_p));                                   \
        auto *_q = llvm::dyn_cast<TO_TYPE>(_p);                                \
        auto *_r = llvm::cast<TO_TYPE>(_p);                                    \
        delete _p;                                                             \
    }                                                                          \
    {                                                                          \
        auto _p = make_unique<FROM_TYPE>();                                    \
        ASSERT_TRUE(llvm::isa<TO_TYPE>(_p));                                   \
        auto _q = llvm::dyn_cast<TO_TYPE>(_p);                                 \
        auto _r = llvm::cast<TO_TYPE>(_p);                                     \
    }                                                                          \
    {                                                                          \
        auto _p = make_shared<FROM_TYPE>();                                    \
        ASSERT_TRUE(llvm::isa<TO_TYPE>(_p));                                   \
        auto _q = llvm::dyn_cast<TO_TYPE>(_p);                                 \
        auto _r = llvm::cast<TO_TYPE>(_p);                                     \
    }

TEST(CastTestSuite, auto_raw_ptr){
#include "casting_test_rawptr.h"
}

TEST(CastTestSuite, auto_uinque_ptr) {
    // #include "casting_test_unique_ptr.h"
}

TEST(CastTestSuite, auto_shared_ptr){
#include "casting_test_shared_ptr.h"
}

TEST(CastTestSuite, raw_ptr_basic_0) {
    ICFGNode *globalNode = new GlobalBlockNode();
    ICFGNode *funEntryNode = new FunEntryBlockNode();
    ICFGNode *funExitNode = new FunExitBlockNode();

    ASSERT_TRUE(llvm::isa<ICFGNode>(globalNode));
    ASSERT_TRUE(llvm::isa<GlobalBlockNode>(globalNode));
    ASSERT_FALSE(llvm::isa<InterBlockNode>(globalNode));
    ASSERT_FALSE(llvm::isa<IntraBlockNode>(globalNode));
    ASSERT_TRUE(llvm::isa<ICFGNode>(funEntryNode));
    ASSERT_TRUE(llvm::isa<InterBlockNode>(funEntryNode));
    ASSERT_TRUE(llvm::isa<FunEntryBlockNode>(funEntryNode));
    ASSERT_TRUE(llvm::isa<ICFGNode>(funExitNode));
    ASSERT_TRUE(llvm::isa<InterBlockNode>(funExitNode));
    ASSERT_TRUE(llvm::isa<FunExitBlockNode>(funExitNode));
    ASSERT_FALSE(llvm::isa<FunEntryBlockNode>(funExitNode));
    ASSERT_FALSE(llvm::isa<CallBlockNode>(funExitNode));
    ASSERT_FALSE(llvm::isa<RetBlockNode>(funExitNode));

    // the following code should work at runtime
    GlobalBlockNode *g_c = llvm::dyn_cast<GlobalBlockNode>(globalNode);
    ASSERT_NE(g_c, nullptr);
    FunEntryBlockNode *fentry = llvm::dyn_cast<FunEntryBlockNode>(funEntryNode);
    ASSERT_NE(fentry, nullptr);
    FunExitBlockNode *fexit = llvm::dyn_cast<FunExitBlockNode>(funExitNode);
    ASSERT_NE(fexit, nullptr);
    FunEntryBlockNode *fg_c = llvm::dyn_cast<FunEntryBlockNode>(globalNode);
    ASSERT_EQ(fg_c, nullptr);

    delete funExitNode;
    delete funEntryNode;
    delete globalNode;
}

TEST(CastTestSuite, unique_ptr_basic_0) {
    // define_cast_test(GlobalBlockNode, ICFGNode)
    unique_ptr<ICFGNode> node = make_unique<GlobalBlockNode>();
    ASSERT_TRUE(llvm::isa<ICFGNode>(node));
    ASSERT_TRUE(llvm::isa<GlobalBlockNode>(node));
    // unique_ptr<ICFGNode> icfgNode = llvm::dyn_cast<ICFGNode>(node);
    ASSERT_NE(node.get(), nullptr);
    auto addr0 = node.get();
    unique_ptr<ICFGNode> icfgNode = llvm::unique_dyn_cast<ICFGNode>(node);
    ASSERT_EQ(addr0, icfgNode.get());

    ASSERT_NE(icfgNode.get(), nullptr);
    ASSERT_EQ(node.get(), nullptr);
    unique_ptr<ICFGNode> ep;
    ep = std::move(icfgNode);
    ASSERT_EQ(icfgNode.get(), nullptr);
    ASSERT_NE(ep.get(), nullptr);

    unique_ptr<ICFGNode> n2 = make_unique<ICFGNode>();
    ASSERT_NE(n2.get(), nullptr);
    auto addr1 = n2.get();
    // after the assignment, we check that the address n2
    // is holding is different
    n2 = std::move(ep);
    ASSERT_NE(n2.get(), addr1);

    {
        unique_ptr<ICFGNode> globalNode = make_unique<GlobalBlockNode>();
        unique_ptr<ICFGNode> funEntryNode = make_unique<FunEntryBlockNode>();
        unique_ptr<ICFGNode> funExitNode = make_unique<FunExitBlockNode>();

        unique_ptr<GlobalBlockNode> g_c =
            llvm::dyn_cast<GlobalBlockNode>(globalNode);
        ASSERT_NE(g_c.get(), nullptr);

        unique_ptr<FunEntryBlockNode> fentry =
            llvm::dyn_cast<FunEntryBlockNode>(funEntryNode);
        ASSERT_NE(fentry.get(), nullptr);

        unique_ptr<FunExitBlockNode> fexit =
            llvm::dyn_cast<FunExitBlockNode>(funExitNode);
        ASSERT_NE(fexit.get(), nullptr);

        unique_ptr<FunEntryBlockNode> fc =
            llvm::dyn_cast<FunEntryBlockNode>(globalNode);
        ASSERT_EQ(fc.get(), nullptr);
    }
}

TEST(CastTestSuite, shared_ptr_basic_0) {
    shared_ptr<ICFGNode> globalNode = make_shared<GlobalBlockNode>();
    shared_ptr<ICFGNode> funEntryNode = make_shared<FunEntryBlockNode>();
    shared_ptr<ICFGNode> funExitNode = make_shared<FunExitBlockNode>();

    ASSERT_TRUE(llvm::isa<GlobalBlockNode>(globalNode));
    ASSERT_TRUE(llvm::isa<ICFGNode>(globalNode));
    ASSERT_TRUE(llvm::isa<FunEntryBlockNode>(funEntryNode));
    ASSERT_TRUE(llvm::isa<InterBlockNode>(funEntryNode));
    ASSERT_TRUE(llvm::isa<ICFGNode>(funEntryNode));
    ASSERT_FALSE(llvm::isa<GlobalBlockNode>(funExitNode));
    ASSERT_TRUE(llvm::isa<FunExitBlockNode>(funExitNode));
    ASSERT_TRUE(llvm::isa<InterBlockNode>(funEntryNode));
    ASSERT_TRUE(llvm::isa<ICFGNode>(funEntryNode));

    shared_ptr<GlobalBlockNode> g_c =
        llvm::dyn_cast<GlobalBlockNode>(globalNode);
    ASSERT_NE(g_c.get(), nullptr);

    shared_ptr<FunEntryBlockNode> fentry =
        llvm::dyn_cast<FunEntryBlockNode>(funEntryNode);
    ASSERT_NE(fentry.get(), nullptr);

    shared_ptr<FunExitBlockNode> fexit =
        llvm::dyn_cast<FunExitBlockNode>(funExitNode);
    ASSERT_NE(fexit.get(), nullptr);

    ASSERT_NE(g_c.get(), nullptr);
    shared_ptr<FunEntryBlockNode> fc = llvm::dyn_cast<FunEntryBlockNode>(g_c);
    ASSERT_EQ(fc.get(), nullptr);
}

TEST(CastTestSuite, ThreadMHPIndSVFGEdge_for_delete_redundant) {
    GenericVFGEdgeTy *gVFGEdge = new ThreadMHPIndSVFGEdge();

    ASSERT_NE(gVFGEdge, nullptr);
    // VFGEdge *vfgEdge = llvm::dyn_cast<VFGEdge>(gVFGEdge);
    IndirectSVFGEdge *indSVFGEdge = llvm::dyn_cast<IndirectSVFGEdge>(gVFGEdge);
    ThreadMHPIndSVFGEdge *tmhpEdge =
        llvm::dyn_cast<ThreadMHPIndSVFGEdge>(gVFGEdge);
    // VFGEdge *tmhpEdge2 = llvm::dyn_cast<ThreadMHPIndSVFGEdge>(VFGEdge);
    cout << indSVFGEdge << endl;
    cout << tmhpEdge << endl;
    ASSERT_NE(indSVFGEdge, nullptr);
    ThreadMHPIndSVFGEdge *tmhpEdge2 =
        llvm::dyn_cast<ThreadMHPIndSVFGEdge>(indSVFGEdge);

    delete gVFGEdge;
}

TEST(CastTestSuite, PAGNodeToObjPN) {

    {
        PAGNode *pPAGNode = new ObjPN();
        ObjPN *objPN = llvm::dyn_cast<ObjPN>(pPAGNode);
        delete pPAGNode;
    }

    {
        shared_ptr<PAGNode> pPAGNode = make_shared<ObjPN>();
        shared_ptr<ObjPN> objPN = llvm::dyn_cast<ObjPN>(pPAGNode);
    }

    {
        shared_ptr<GenericPAGNodeTy> pPAGNode = make_shared<ObjPN>();
        shared_ptr<ObjPN> objPN = llvm::dyn_cast<ObjPN>(pPAGNode);
    }
}

// #if 0
TEST(CastTestSuite, shared_ptr_const) {
    {
        const shared_ptr<PAGNode> pPAGNode = make_shared<ObjPN>();
        const shared_ptr<ObjPN> objPN = llvm::dyn_cast<ObjPN>(pPAGNode);
    }
}
// #endif

TEST(CastTestSuite, simple_type_test) {

#if 0
    /// This test show that llvm::is_simple_type treats
    // all types except const X * const as simple types
    // cout << std::boolalpha;
    // FIXME: this test triggers a bug in googletest
    // because of it, the following code can not be built
    // with the following error:
    // undefined reference to `llvm::is_simple_type<std::shared_ptr<SVF::ObjPN>
    // const&>::value'

#if 0
    ASSERT_TRUE(llvm::is_simple_type<int>::value);
    ASSERT_TRUE(llvm::is_simple_type<const volatile int>::value);
    ASSERT_TRUE(llvm::is_simple_type<const int>::value);
    ASSERT_TRUE(llvm::is_simple_type<int *>::value);
    ASSERT_TRUE(llvm::is_simple_type<const int *>::value);
    ASSERT_TRUE(llvm::is_simple_type<int &>::value);
    ASSERT_TRUE(llvm::is_simple_type<const int &>::value);
    ASSERT_TRUE(llvm::is_simple_type<shared_ptr<int>>::value);
    ASSERT_TRUE(llvm::is_simple_type<unique_ptr<int>>::value);
    ASSERT_TRUE(llvm::is_simple_type<shared_ptr<int> &>::value);
    ASSERT_TRUE(llvm::is_simple_type<unique_ptr<int> &>::value);
    ASSERT_TRUE(llvm::is_simple_type<shared_ptr<int> &&>::value);
    ASSERT_TRUE(llvm::is_simple_type<unique_ptr<int> &&>::value);
    ASSERT_TRUE(llvm::is_simple_type<const shared_ptr<int>>::value);
    ASSERT_TRUE(llvm::is_simple_type<const unique_ptr<int>>::value);
    ASSERT_TRUE(llvm::is_simple_type<const shared_ptr<int> &>::value);
    ASSERT_TRUE(llvm::is_simple_type<const unique_ptr<int> &>::value);
    ASSERT_TRUE(llvm::is_simple_type<const shared_ptr<int> &&>::value);
    ASSERT_TRUE(llvm::is_simple_type<const unique_ptr<int> &&>::value);
    ASSERT_TRUE(llvm::is_simple_type<ObjPN>::value);
    ASSERT_TRUE(llvm::is_simple_type<const ObjPN>::value);
    ASSERT_TRUE(llvm::is_simple_type<ObjPN *>::value);
    ASSERT_TRUE(llvm::is_simple_type<const ObjPN *>::value);
    ASSERT_TRUE(llvm::is_simple_type<shared_ptr<ObjPN>>::value);
    ASSERT_TRUE(llvm::is_simple_type<unique_ptr<ObjPN>>::value);
    ASSERT_TRUE(llvm::is_simple_type<shared_ptr<ObjPN> &>::value);
    ASSERT_TRUE(llvm::is_simple_type<unique_ptr<ObjPN> &>::value);
    ASSERT_TRUE(llvm::is_simple_type<shared_ptr<ObjPN> &&>::value);
    ASSERT_TRUE(llvm::is_simple_type<unique_ptr<ObjPN> &&>::value);
    ASSERT_TRUE(llvm::is_simple_type<const shared_ptr<ObjPN>>::value);
    ASSERT_TRUE(llvm::is_simple_type<const unique_ptr<ObjPN>>::value);
    ASSERT_TRUE(llvm::is_simple_type<const shared_ptr<ObjPN> &>::value);
    ASSERT_TRUE(llvm::is_simple_type<const unique_ptr<ObjPN> &>::value);
    ASSERT_TRUE(llvm::is_simple_type<const shared_ptr<ObjPN> &&>::value);
    ASSERT_TRUE(llvm::is_simple_type<const unique_ptr<ObjPN> &&>::value);
#endif

    //// this is not a simple type
    ASSERT_FALSE(llvm::is_simple_type<const ObjPN *const>::value);
    ASSERT_FALSE(llvm::is_simple_type<const int *const>::value);
    ASSERT_FALSE(llvm::is_simple_type<const int *const>::value);
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
