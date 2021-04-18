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

#ifndef __TESTS_GRAPHS_ICFG_H__
#define __TESTS_GRAPHS_ICFG_H__

#include "Graphs/ICFG.h"
#include "Tests/Graphs/GenericGraph.hpp"

namespace SVF {

template <>
void node_eq_extra_test<ICFGNode>(const ICFGNode *n1, const ICFGNode *n2) {
    ASSERT_EQ(n1->getFun(), n2->getFun());
    ASSERT_EQ(n1->getBB(), n2->getBB());
}

} /* end of namespace SVF  */

#endif /* __TESTS_GRAPHS_ICFG_H__ */
