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
 *     2021-04-18
 *****************************************************************************/

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

#include "Util/boost_classes_export.h"

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
