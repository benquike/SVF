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

using namespace SVF;

static TestGraphNode *getOrCreateNode(TestGraphSPtr g, NodeID id) {
    TestGraphNode *ret = nullptr;

    if (id == MAX_NODEID) {
        // this is a special id,
        // to handle case where we only define a node
        // in which case, we set the dst to be the max node id
        return ret;
    }

    if (!g->hasGNode(id)) {
        ret = new TestGraphNode(id);
        g->addGNode(ret);
    } else {
        ret = g->getGNode(id);
    }

    return ret;
}

TestGraphSPtr SVF::buildTestGraph(const MapGraph &_graph) {
    TestGraphSPtr ret = make_shared<TestGraph>();

    for (auto &it : _graph) {
        TestGraphNode *src = getOrCreateNode(ret, it.first);

        for (auto dstId : it.second) {
            TestGraphNode *dst = getOrCreateNode(ret, dstId);

            if (dst != nullptr && src != nullptr) {
                TestGraphEdge *edge =
                    new TestGraphEdge(src, dst, ret->getNextEdgeId());
                ret->addGEdge(edge);
            }
        }
    }

    return ret;
}
