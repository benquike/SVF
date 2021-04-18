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

#include "Util/Serialization.h"
#include "config.h"
#include "gtest/gtest.h"
#include <sstream>

using namespace boost::archive;

using namespace std;

class TestClass {
  private:
    int x;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &x;
    }

  public:
    TestClass(int _x) { x = _x; }
    TestClass() = default;

    int getX() const { return x; }
};

TEST(SerializationTestSuite, rawptr_test) {
    stringstream ss;

    // save the object
    {
        text_oarchive oa{ss};
        TestClass t(0xdeadbeef);
        TestClass *pt = &t;

        oa << pt;
    }

    // load
    {
        text_iarchive ia{ss};
        TestClass *pt;
        ia >> pt;
        cout << std::hex << pt->getX() << endl;
        delete pt;
    }
}

TEST(SerializationTestSuite, null_test) {
    stringstream ss;
    {
        text_oarchive oa{ss};
        TestClass *pt = nullptr;
        oa << pt;
    }

    {
        text_iarchive ia(ss);
        TestClass *pt;
        ia >> pt;

        ASSERT_EQ(pt, nullptr);
    }
}

class PtrToSelf {
  private:
    int x;
    PtrToSelf *p;

  public:
    PtrToSelf() = default;
    PtrToSelf(int x) : x(x), p(nullptr) {}

    void setPtr(PtrToSelf *p) { this->p = p; }

    PtrToSelf *getPtr() const { return p; }

    int getX() const { return x; }

    virtual ~PtrToSelf() = default;

  private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &x;
        ar &p;
    }
};

TEST(SerializationTestSuite, PtrToSelfTest) {
    stringstream ss;
    // save
    {
        PtrToSelf p(1234);
        p.setPtr(&p);

        text_oarchive oa{ss};
        oa << p;
    }

    {
        text_iarchive ia{ss};
        PtrToSelf p;
        ia >> p;

        ASSERT_EQ(p.getX(), 1234);
        ASSERT_EQ(p.getPtr(), &p);
    }
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
