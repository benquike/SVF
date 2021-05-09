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

#include "Graphs/ICFG.h"
#include "Graphs/PAG.h"
#include "Graphs/VFG.h"
#include "Util/SVFBasicTypes.h"
#include "Util/Serialization.h"
#include <sstream>

#include "config.h"
#include "gtest/gtest.h"

using namespace std;
using namespace SVF;

using namespace boost::archive;

class TestClass {
  public:
    TestClass() = default;
    virtual ~TestClass() = default;

  private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, unsigned int version) {}
};

TEST(MapTestSuite, BasicTest_0) {
    Map<int, int> m;
    stringstream ss;

    {
        text_oarchive oa{ss};
        oa << m;
        Map<int, int> *pm = &m;
        oa << pm;
    }
}

TEST(MapTestSuite, BasicTest_1) {
    Map<NodeID, TestClass *> m;
    stringstream ss;

    {
        text_oarchive oa{ss};
        oa << m;

        Map<NodeID, TestClass *> *pm = &m;
        oa << pm;
    }
}

TEST(MapTestSuite, BasicTest_2) {
    Map<NodeID, ICFGNode *> m, *pm;
    stringstream ss;

    {
        text_oarchive oa{ss};
        pm = &m;
        oa << m << pm;
    }
}

TEST(MapTestSuite, BasicTest_3) {
    GenericICFGTy::IDToNodeMapTy m, *pm = &m;
    stringstream ss;

    {
        text_oarchive oa{ss};
        oa << m << pm;
    }
}

template <typename NodeType>
class TestClass2 {
  public:
    TestClass2() = default;
    virtual ~TestClass2() = default;

  private:
    Map<NodeID, NodeType *> m;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, unsigned int version) {
        ar &m;
    }
};

using NT = TestClass2<ICFGNode>;

class TestClass3 : public NT {
  public:
    TestClass3() = default;
    virtual ~TestClass3() = default;

  private:
    int x;

    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, unsigned int version) {
        ar &boost::serialization::base_object<NT>(*this);
        ar &x;
    }
};

BOOST_CLASS_EXPORT(TestClass3)

TEST(MapTestSuite, BasicTest_4) {
    TestClass3 t;
    NT *pt = &t;
    stringstream ss;

    {
        text_oarchive oa{ss};
        oa << pt;
    }
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
