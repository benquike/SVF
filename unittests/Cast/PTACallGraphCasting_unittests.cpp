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
 *     2021-05-13
 *****************************************************************************/

#include "Graphs/ThreadCallGraph.h"
#include "config.h"
#include "gtest/gtest.h"

using namespace std;
using namespace SVF;

TEST(PATCallGraphCastingTestSuites, HareParForEdgeCasting) {
    GenericCallGraphEdgeTy *hareParForEdge =
        new HareParForEdge(nullptr, nullptr, 0, 0);
    ASSERT_TRUE(llvm::isa<PTACallGraphEdge>(hareParForEdge));
    delete hareParForEdge;
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
