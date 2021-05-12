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

#ifndef __TESTS_GRAPHS_TESTGRAPH__
#define __TESTS_GRAPHS_TESTGRAPH__

#include "Graphs/GenericGraph.h"
#include <map>
#include <memory>

using namespace std;

namespace SVF {

class TestGraphNode;

/*!
 * Graph node for a test graph
 *
 */
using GeneralTestGraphEdge = GenericEdge<TestGraphNode>;
class TestGraphEdge : public GeneralTestGraphEdge {
  public:
    TestGraphEdge(TestGraphNode *src, TestGraphNode *dst, EdgeID id,
                  GEdgeFlag kind = 0)
        : GeneralTestGraphEdge(src, dst, id, kind) {}
    virtual ~TestGraphEdge() = default;
};

using GeneralTestGraphNode = GenericNode<TestGraphNode, TestGraphEdge>;
class TestGraphNode : public GeneralTestGraphNode {
  public:
    TestGraphNode(NodeID id, GNodeK kind = 0)
        : GeneralTestGraphNode(id, kind) {}
    virtual ~TestGraphNode() = default;
};

using GeneralTestGraph = GenericGraph<TestGraphNode, TestGraphEdge>;
class TestGraph : public GeneralTestGraph {
  public:
    TestGraph() = default;
    virtual ~TestGraph() = default;
};

using TestGraphSPtr = std::shared_ptr<TestGraph>;

using MapGraph = map<NodeID, set<NodeID>>;

TestGraphSPtr buildTestGraph(const MapGraph &_graph);

} /* namespace SVF */

namespace llvm {

template <>
struct GraphTraits<SVF::TestGraphNode *>
    : public GraphTraits<SVF::GeneralTestGraphNode *> {};

template <>
struct GraphTraits<Inverse<SVF::TestGraphNode *>>
    : public GraphTraits<Inverse<SVF::GeneralTestGraphNode *>> {};

template <>
struct GraphTraits<SVF::TestGraph *> : public GraphTraits<GeneralTestGraph *> {
    using NodeRef = SVF::TestGraphNode *;
};

} /* namespace llvm */

#endif /* __TESTS_GRAPHS_TESTGRAPH__ */
