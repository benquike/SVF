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
 *     2021-05-12
 *****************************************************************************/

#include "SVF-FE/SVFProject.h"
#include "WPA/Andersen.h"

#include "config.h"
#include "gtest/gtest.h"

using namespace std;
using namespace SVF;

class AndersenTestSuite : public ::testing::Test {
  public:
    void SetUp() {
        spdlog::set_level(spdlog::level::debug);
        spdlog::set_pattern("[%H:%M:%S %z] [%!] [%^---%L---%$] [thread %t] %v");
    }
};

TEST_F(AndersenTestSuite, StaticCallTest_0) {
    string test_bc = SVF_BUILD_DIR "tests/ICFG/static_call_test_cpp.ll";
    unique_ptr<SVFProject> proj = make_unique<SVFProject>(test_bc);
    AndersenWaveDiff *anderWD =
        AndersenWaveDiff::createAndersenWaveDiff(proj.get());

    delete anderWD;
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
