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
 *     2021-04-14
 *****************************************************************************/

#ifndef SERIALIZATION_H_
#define SERIALIZATION_H_
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/binary_object.hpp>
#include <boost/serialization/bitset.hpp>
#include <boost/serialization/deque.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/nvp.hpp>
// #include <boost/serialization/hash_map.hpp>
// #include <boost/serialization/hash_set.hpp>
#include <boost/serialization/queue.hpp>
#include <boost/serialization/scoped_ptr.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/split_member.hpp>
#include <boost/serialization/stack.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/serialization/traits.hpp>
#include <boost/serialization/type_info_implementation.hpp>
#include <boost/serialization/unique_ptr.hpp>
#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/weak_ptr.hpp>
#include <type_traits>
#include <utility>
#include <vector>

#include "SVF-FE/SVFProject.h"
#include "Util/SVFBasicTypes.h"
#include "Util/SVFModule.h"

using namespace SVF;
using namespace std;

BOOST_SERIALIZATION_SPLIT_FREE(NodeBS)
BOOST_SERIALIZATION_SPLIT_FREE(Set<const SVFFunction *>)

// Save a pointer to SVFFunction
#define SAVE_SVFFunction(ar, fun)                                              \
    do {                                                                       \
        if (fun != nullptr) {                                                  \
            auto *llvm_fun = fun->getLLVMFun();                                \
            ar &getIdByValueFromCurrentProject(llvm_fun);                      \
        } else {                                                               \
            ar &numeric_limits<SymID>::max();                                  \
        }                                                                      \
    } while (0)

// Load a pointer to SVFFunction
#define LOAD_SVFFunction(ar, fun)                                              \
    do {                                                                       \
        SymID id;                                                              \
        ar &id;                                                                \
        if (id < numeric_limits<SymID>::max()) {                               \
            SVFModule *m = SVFProject::getCurrentProject()->getSVFModule();    \
            auto *val = getValueByIdFromCurrentProject(id);                    \
            assert(llvm::isa<Function>(val) && "id not a Function");           \
            fun = m->getSVFFunction(llvm::dyn_cast<Function>(val));            \
        }                                                                      \
    } while (0)

// Save a pointer to MemObj
#define SAVE_MemObj(ar, mem)                                                   \
    do {                                                                       \
        if (mem != nullptr) {                                                  \
            ar &getIdByMemObjFromCurrentProject(mem);                          \
        } else {                                                               \
            ar &numeric_limits<SymID>::max();                                  \
        }                                                                      \
    } while (0)

// Load a pointer to MemObj
#define LOAD_MemObj(ar, mem)                                                   \
    do {                                                                       \
        SymID id;                                                              \
        ar &id;                                                                \
        if (id < numeric_limits<SymID>::max()) {                               \
            mem = getMemObjByIdFromCurrentProject(id);                         \
        }                                                                      \
    } while (0)

// Save any pointer of type llvm::Value (or its derivative types)
#define SAVE_Value(ar, val)                                                    \
    do {                                                                       \
        if (val != nullptr) {                                                  \
            ar &getIdByValueFromCurrentProject(val);                           \
        } else {                                                               \
            ar &numeric_limits<SymID>::max();                                  \
        }                                                                      \
    } while (0)

// load any pointer of type llvm::Value (or its derivative types)
#define LOAD_Value(ar, Type, val)                                              \
    do {                                                                       \
        SymID id;                                                              \
        ar &id;                                                                \
        if (id < numeric_limits<SymID>::max()) {                               \
            auto *lval = getValueByIdFromCurrentProject(id);                   \
            assert(llvm::isa<Type>(lval) && "id not of current type");         \
            val = llvm::dyn_cast<Type>(lval);                                  \
        }                                                                      \
    } while (0)

#define SAVE_Node_Or_Edge(ar, nore)                                            \
    do {                                                                       \
        if (nore) {                                                            \
            auto id = nore->getId();                                           \
            ar &id;                                                            \
        } else {                                                               \
            ar &numeric_limits<NodeID>::max();                                 \
        }                                                                      \
    } while (0)

#define SAVE_ICFGNode(ar, node) SAVE_Node_Or_Edge(ar, node)
#define SAVE_ICFGEdge(ar, edge) SAVE_Node_Or_Edge(ar, edge)

#define LOAD_ICFGNode(ar, node)                                                \
    do {                                                                       \
        NodeID id;                                                             \
        ar &id;                                                                \
        if (id < numeric_limits<NodeID>::max()) {                              \
            auto g = SVFProject::getCurrentProject()->getICFG();               \
            using Type1 = std::remove_pointer<decltype(node)>::type;           \
            using Type = std::remove_const<Type1>::type;                       \
            node = llvm::dyn_cast<Type>(g->getGNode(id));                      \
        } else {                                                               \
            node = nullptr;                                                    \
        }                                                                      \
    } while (0)

#define LOAD_ICFGEdge(ar, edge)                                                \
    do {                                                                       \
        NodeID id;                                                             \
        ar &id;                                                                \
        if (id < numeric_limits<NodeID>::max()) {                              \
            auto g = SVFProject::getCurrentProject()->getICFG();               \
            using Type1 = std::remove_pointer<decltype(edge)>::type;           \
            using Type = std::remove_const<Type1>::type;                       \
            edge = llvm::dyn_cast<Type>(g->getGEdge(id));                      \
        } else {                                                               \
            edge = nullptr;                                                    \
        }                                                                      \
    } while (0)

#define SAVE_PAGEdge(ar, edge) SAVE_Node_Or_Edge(ar, edge)

#define SAVE_PAGNode(ar, node) SAVE_Node_Or_Edge(ar, node)

#define LOAD_PAGNode(ar, node)                                                 \
    do {                                                                       \
        NodeID id;                                                             \
        ar &id;                                                                \
        if (id < numeric_limits<NodeID>::max()) {                              \
            auto g = SVFProject::getCurrentProject()->getPAG();                \
            using Type1 = std::remove_pointer<decltype(node)>::type;           \
            using Type = std::remove_const<Type1>::type;                       \
            node = llvm::dyn_cast<Type>(g->getGNode(id));                      \
        } else {                                                               \
            node = nullptr;                                                    \
        }                                                                      \
    } while (0)

#define LOAD_PAGEdge(ar, edge)                                                 \
    do {                                                                       \
        NodeID id;                                                             \
        ar &id;                                                                \
        if (id < numeric_limits<NodeID>::max()) {                              \
            auto g = SVFProject::getCurrentProject()->getPAG();                \
            using Type1 = std::remove_pointer<decltype(edge)>::type;           \
            using Type = std::remove_const<Type1>::type;                       \
            edge = llvm::dyn_cast<Type>(g->getGEdge(id));                      \
        } else {                                                               \
            edge = nullptr;                                                    \
        }                                                                      \
    } while (0)

namespace boost {
namespace serialization {

template <typename Archive>
void save(Archive &ar, const NodeBS &bs, unsigned int version) {
    vector<unsigned> set_bits;

    for (auto pos : bs) {
        set_bits.push_back(pos);
    }

    ar &set_bits;
}

template <typename Archive>
void load(Archive &ar, NodeBS &bs, unsigned int version) {
    vector<unsigned> set_bits;
    ar &set_bits;

    for (auto pos : set_bits) {
        bs.set(pos);
    }
}

template <typename Archive>
void save(Archive &ar, const Set<const SVFFunction *> &fs,
          unsigned int version) {
    Set<SymID> sid;

    for (const auto *f : fs) {
        SymID fid = getIdByValueFromCurrentProject(f->getLLVMFun());
        sid.insert(fid);
    }

    ar &sid;
}

template <typename Archive>
void load(Archive &ar, Set<const SVFFunction *> &fs, unsigned int version) {
    Set<SymID> sid;
    ar &sid;
    SVFModule *mod = SVFProject::getCurrentProject()->getSVFModule();

    for (auto fid : sid) {
        const Value *v = getValueByIdFromCurrentProject(fid);
        const SVFFunction *sf =
            mod->getSVFFunction(llvm::dyn_cast<Function>(v));
        fs.insert(sf);
    }
}

template <typename KeyType, typename ValueType, typename Archive>
void save_map(Archive &ar, const Map<KeyType, ValueType> &map) {
    Map<SymID, ValueType> Id2ValMap;
    for (auto &[k, v] : map) {
        auto id = getIdByValueFromCurrentProject(k);
        Id2ValMap[id] = v;
    }
    ar &Id2ValMap;
}

template <typename ValueType, typename Archive>
void save_map(Archive &ar, const Map<const SVFFunction *, ValueType> &map) {
    Map<SymID, ValueType> Id2ValMap;
    for (auto &[k, v] : map) {
        auto *llvm_fun = k->getLLVMFun();
        auto id = getIdByValueFromCurrentProject(llvm_fun);
        Id2ValMap[id] = v;
    }
    ar &Id2ValMap;
}

template <typename KeyType, typename ValueType, typename Archive>
void load_map(Archive &ar, Map<KeyType, ValueType> &map) {

    typedef typename std::remove_const<KeyType>::type KeyType_WO_const;
    typedef typename std::remove_pointer<KeyType_WO_const>::type KeyType_Clean;

    Map<SymID, ValueType> Id2ValMap;
    ar &Id2ValMap;

    for (auto [id, v] : Id2ValMap) {
        auto *llvm_val = getValueByIdFromCurrentProject(id);
        auto *k = llvm::dyn_cast<KeyType_Clean>(llvm_val);
        map[k] = v;
    }
}

template <typename ValueType, typename Archive>
void load_map(Archive &ar, Map<const SVFFunction *, ValueType> &map) {
    Map<SymID, ValueType> Id2ValMap;
    SVFModule *mod = SVFProject::getCurrentProject()->getSVFModule();
    for (auto [id, v] : Id2ValMap) {
        const Value *llvm_val = getValueByIdFromCurrentProject(id);
        const Function *fun = llvm::dyn_cast<Function>(llvm_val);
        const SVFFunction *k = mod->getSVFFunction(fun);
        map[k] = v;
    }
}

template <typename FirstType, typename SecondType, typename Archive>
void save_pair(Archive &ar, const std::pair<FirstType, SecondType> &pair) {
    if (pair.first != nullptr) {
        SymID id = getIdByValueFromCurrentProject(pair.first);
        std::pair p(id, pair.second);
        ar &p;
    } else {
        std::pair p(numeric_limits<SymID>::max(), pair.second);
        ar &p;
    }
}

template <typename FirstType, typename SecondType, typename Archive>
void load_pair(Archive &ar, std::pair<FirstType, SecondType> &pair) {
    std::pair<SymID, SecondType> p;
    ar &p;

    if (p.first < numeric_limits<SymID>::max()) {
        pair.first = getValueByIdFromCurrentProject(p.first);
    } else {
        pair.first = nullptr;
    }

    pair.second = p.second;
}

} // end of namespace serialization
} // end of namespace boost

#endif // SERIALIZATION_H_
