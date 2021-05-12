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
 *     2021-05-11
 *****************************************************************************/

#include "Tests/Graphs/TestGraph.hpp"

#include "Util/SCC.h"
#include "config.h"
#include "gtest/gtest.h"

#include <map>
#include <set>
#include <tuple>
#include <vector>

using namespace std;
using namespace SVF;

using TestGraphSCC = SCCDetection<TestGraph *>;

static void do_scc_test(MapGraph &_graph, set<NodeID> &&roots,
                        int expected_nodes, int expected_edges) {
    TestGraphSPtr g = buildTestGraph(_graph);

    ASSERT_EQ(g->getTotalNodeNum(), expected_nodes);
    ASSERT_EQ(g->getTotalEdgeNum(), expected_edges);

    auto scc = make_unique<TestGraphSCC>(g.get());

    scc->find();

    for (auto it : scc->getRepNodes()) {
        llvm::outs() << it << ", ";
    }

    llvm::outs() << "\n";

    auto &nodeStack = scc->topoNodeStack();

    while (!nodeStack.empty()) {
        auto t = nodeStack.top();

        llvm::outs() << "  === " << t << "\n";

        ASSERT_NE(roots.find(t), roots.end());

        nodeStack.pop();
    }

    auto &repNodes = scc->getRepNodes();

    ASSERT_EQ(repNodes.count(), roots.size());

    for (auto r : roots) {
        ASSERT_TRUE(repNodes.test(r));
    }
}

static void do_scc_test(MapGraph &_graph, set<NodeID> &roots,
                        int expected_nodes, int expected_edges) {
    do_scc_test(_graph, std::move(roots), expected_nodes, expected_edges);
}

TEST(SCCTestSuite, Simple_0) {
    // map<NodeID, NodeID> _graph = {{1, MAX_NODEID}};
    // do_scc_test(_graph, {1}, 1, 0);
    // map<NodeID, NodeID> _graph = {{1, MAX_NODEID}};
    // do_scc_test(_graph, {1}, 1, 0);

    using TestTuple = tuple<MapGraph, set<NodeID>, int, int>;

    vector<TestTuple> tests = {
        {{{1, {}}}, {1}, 1, 0},
        {{{1, {}}, {2, {}}}, {1, 2}, 2, 0},
        {{{1, {2}}, {2, {1}}}, {1}, 2, 2},
        {{{1, {2, 3}}}, {1, 2, 3}, 3, 2},
        {{{1, {2, 3}}, {2, {3}}}, {1, 2, 3}, 3, 3},
        {{{1, {2}}, {2, {3}}, {3, {4, 5}}, {4, {2}}}, {1, 2, 5}, 5, 5},
        {{{1, {2, 3}}, {2, {5, 6}}, {3, {4}}, {4, {5}}, {5, {3}}},
         {1, 2, 5, 6},
         6,
         7}};

    for (auto t : tests) {
        do_scc_test(get<0>(t), get<1>(t), get<2>(t), get<3>(t));
    }
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
