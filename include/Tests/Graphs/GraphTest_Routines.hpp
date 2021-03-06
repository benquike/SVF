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

#ifndef __TESTS_GRAPHS_GRAPHTEST_ROUTINES_H__
#define __TESTS_GRAPHS_GRAPHTEST_ROUTINES_H__

#include "Graphs/GenericGraph.h"
#include "gtest/gtest.h"

namespace SVF {

template <typename GEdge>
void edge_eq_extra_test(const GEdge *e1, const GEdge *e2) {}

template <typename GEdge>
void edge_eq_test(const GEdge *e1, const GEdge *e2) {
    ASSERT_NE(e1, e2);
    ASSERT_EQ(e1->getEdgeKind(), e2->getEdgeKind());
    ASSERT_EQ(e1->getSrcID(), e2->getSrcID());
    ASSERT_EQ(e1->getDstID(), e2->getDstID());
    edge_eq_extra_test(e1, e2);
}

template <typename GNode>
void node_eq_extra_test(const GNode *n1, const GNode *n2) {}

template <typename GNode>
void node_eq_test(const GNode *n1, const GNode *n2) {

    ASSERT_NE(n1, n2);
    ASSERT_EQ(n1->getId(), n2->getId());
    ASSERT_EQ(n1->getNodeKind(), n2->getNodeKind());
    ASSERT_EQ(n1->getInEdges().size(), n2->getInEdges().size());
    ASSERT_EQ(n1->getOutEdges().size(), n2->getOutEdges().size());

    {

        for (auto e_it1 = n1->InEdgeBegin(); e_it1 != n1->InEdgeEnd();
             e_it1++) {
            auto *e1 = *e_it1;
            auto pos = n2->getInEdges().find(e1);
            ASSERT_NE(pos, n2->getInEdges().end());
            auto *e2 = *pos;
            edge_eq_test(e1, e2);
        }
    }

    {

        for (auto e_it1 = n1->OutEdgeBegin(); e_it1 != n1->OutEdgeEnd();
             e_it1++) {
            auto *e1 = *e_it1;
            auto pos = n2->getOutEdges().find(e1);
            ASSERT_NE(pos, n2->getOutEdges().end());
            auto *e2 = *pos;
            edge_eq_test(e1, e2);
        }
    }

    node_eq_extra_test(n1, n2);
}

// this is intended to be implemented by
// each graph type
template <typename Graph>
void graph_eq_extra_test(const Graph *g1, const Graph *g2) {}

// This template function tests whether
// the ids in the id field of nodes(edges)
// are the same as the id in the graph
template <typename Graph>
void node_and_edge_id_test(Graph *g) {
    for (auto it : *g) {
        auto id1 = it.first;
        auto node = it.second;
        ASSERT_EQ(id1, node->getId());
    }

    for (auto it = g->edge_begin(); it != g->edge_end(); it++) {
        auto id1 = it->first;
        auto edge = it->second;
        ASSERT_EQ(id1, edge->getId());
    }
}

template <typename Graph>
void graph_eq_test(const Graph *g1, const Graph *g2) {
    ASSERT_NE(g1, g2);
    ASSERT_EQ(g1->getTotalEdgeNum(), g2->getTotalEdgeNum());
    ASSERT_EQ(g1->getTotalNodeNum(), g2->getTotalNodeNum());

    ASSERT_GT(g1->getTotalEdgeNum(), 0);
    ASSERT_GT(g1->getTotalNodeNum(), 0);

    node_and_edge_id_test(g1);
    node_and_edge_id_test(g2);

    auto it1 = g1->begin();
    auto it2 = g2->begin();
    for (; it1 != g1->end() && it2 != g2->end(); it1++, it2++) {
        // check the node id
        ASSERT_EQ(it1->first, it2->first);

        // check the node content
        auto *n1 = it1->second;
        auto *n2 = it2->second;
        node_eq_test(n1, n2);
    }

    ASSERT_EQ(it1, g1->end());
    ASSERT_EQ(it2, g2->end());

    graph_eq_extra_test(g1, g2);
}

} /* end of namespace SVF */

#endif /* __TESTS_GRAPHS_GRAPHTEST_ROUTINES_H__ */
