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

#ifndef __TESTS_GRAPHS_GENERICGRAPH_H__
#define __TESTS_GRAPHS_GENERICGRAPH_H__

#include "Graphs/GenericGraph.h"
#include "gtest/gtest.h"

namespace SVF {

template <typename GEdge>
void edge_eq_extra_test(const GEdge *e1, const GEdge *e2) {}

template <typename GEdge> void edge_eq_test(const GEdge *e1, const GEdge *e2) {
    ASSERT_NE(e1, e2);
    ASSERT_EQ(e1->getEdgeKind(), e2->getEdgeKind());
    ASSERT_EQ(e1->getSrcID(), e2->getSrcID());
    ASSERT_EQ(e1->getDstID(), e2->getDstID());
    edge_eq_extra_test(e1, e2);
}

template <typename GNode>
void node_eq_extra_test(const GNode *n1, const GNode *n2) {}

template <typename GNode> void node_eq_test(const GNode *n1, const GNode *n2) {

    ASSERT_NE(n1, n2);
    ASSERT_EQ(n1->getId(), n2->getId());
    ASSERT_EQ(n1->getNodeKind(), n2->getNodeKind());
    ASSERT_EQ(n1->getInEdges().size(), n2->getInEdges().size());
    ASSERT_EQ(n1->getOutEdges().size(), n2->getOutEdges().size());

    {
        auto e_it1 = n1->InEdgeBegin();
        auto e_it2 = n2->InEdgeBegin();
        for (; e_it1 != n1->InEdgeEnd() && e_it2 != n2->InEdgeEnd();
             e_it1++, e_it2++) {
            auto *e1 = *e_it1;
            auto *e2 = *e_it2;
            edge_eq_test(e1, e2);
        }

        ASSERT_EQ(e_it1, n1->InEdgeEnd());
        ASSERT_EQ(e_it2, n2->InEdgeEnd());
    }

    {
        auto e_it1 = n1->OutEdgeBegin();
        auto e_it2 = n2->OutEdgeBegin();
        for (; e_it1 != n1->OutEdgeEnd() && e_it2 != n2->OutEdgeEnd();
             e_it1++, e_it2++) {
            auto *e1 = *e_it1;
            auto *e2 = *e_it2;
            edge_eq_test(e1, e2);
        }
        ASSERT_EQ(e_it1, n1->OutEdgeEnd());
        ASSERT_EQ(e_it2, n2->OutEdgeEnd());
    }

    node_eq_extra_test(n1, n2);
}

// this is intended to be implemented by
// each graph type
template <typename Graph>
void graph_eq_extra_test(const Graph *g1, const Graph *g2) {}

template <typename Graph> void graph_eq_test(const Graph *g1, const Graph *g2) {
    ASSERT_NE(g1, g2);
    ASSERT_EQ(g1->edgeNum, g2->edgeNum);
    ASSERT_EQ(g1->nodeNum, g2->nodeNum);

    auto it1 = g1->begin();
    auto it2 = g2->begin();
    for (; it1 != g1->end() && it2 != g2->end(); it1++, it2++) {
        ASSERT_EQ(it1->first, it2->first);

        auto *n1 = it1->second;
        auto *n2 = it2->second;
        node_eq_test(n1, n2);
    }

    ASSERT_EQ(it1, g1->end());
    ASSERT_EQ(it2, g2->end());

    graph_eq_extra_test(g1, g2);
}

} /* end of namespace SVF */

#endif /* __TESTS_GRAPHS_GENERICGRAPH_H__ */
