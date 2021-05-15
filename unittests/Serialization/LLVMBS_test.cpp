#include <iostream>
#include <sstream>
#include <vector>

#include "Util/SVFBasicTypes.h"
#include "Util/Serialization.h"
#include "config.h"
#include "gtest/gtest.h"

using namespace std;
using namespace SVF;

using namespace boost::archive;

TEST(SerializationTestSuite, BS_Test) {

#if 1
    vector<unsigned> sets{1, 3, 4, 7, 100};
    stringstream ss;

    // save the object
    {
        text_oarchive oa{ss};
        NodeBS bs;
        for (auto pos : sets) {
            bs.set(pos);
        }
        oa << bs;
    }

    {
        text_iarchive ia{ss};
        NodeBS bs;
        ia >> bs;

        for (auto pos : sets) {
            ASSERT_TRUE(bs.test(pos));
        }

        vector<unsigned> unsets{2, 5, 6, 8, 9, 10};

        for (auto us : unsets) {
            ASSERT_FALSE(bs.test(us));
        }
    }

    {
        NodeBS bs;

        bs.set(1);

        for (auto it : bs) {
            llvm::outs() << it << "\n";
        }
        llvm::outs() << "========== \n";

        bs.reset(1);
        for (auto it : bs) {
            llvm::outs() << it << "\n";
        }

        llvm::outs() << "========== \n";
    }

#endif

#if 0
    NodeBS bs;
    vector<unsigned> sets{1,3,4,7,100};

    for (auto p : sets) {
        bs.set(p);
    }

    for (auto p : bs) {
        llvm::outs() << p << "\n";
    }

    stringstream ss;
    // save
    {
        text_oarchive oa{ss};
        oa << sets;
    }

    // load
    {
        text_iarchive ia{ss};
        vector<unsigned> loaded_sets;
        ia >> loaded_sets;

        for (auto p : loaded_sets) {
            llvm::outs() << " == " << p << "\n";
        }
    }
#endif
}

int main(int argc, char *argv[]) {
    ::testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
