//===- CenericGraph.h -- Generic graph ---------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2017>  <Yulei Sui>
//

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//===----------------------------------------------------------------------===//

/*
 * GenericGraph.h
 *
 *  Created on: Mar 19, 2014
 *      Author: Yulei Sui
 */

#ifndef GENERICGRAPH_H_
#define GENERICGRAPH_H_

#include "Util/BasicTypes.h"
#include "Util/Serialization.h"
#include <functional>
#include <type_traits>

namespace SVF {

#define MAX_NODEID (numeric_limits<NodeID>::max())
#define MAX_EDGEID (numeric_limits<EdgeID>::max())

/*!
 * Generic edge on the graph as base class
 */
template <typename NodeTy, typename NodePtr = NodeTy *>
class GenericEdge {
  private:
    // Allow serialization to access non-public data members.
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &src;
        ar &dst;
        ar &edgeFlag;
    }

  public:
    /// Node type
    using NodeType = NodeTy;
    /// Edge Flag
    /// Edge format as follows (from lowest bit):
    ///	(1) 0-7 bits encode an edge kind (allow maximum 16 kinds)
    /// (2) 8-63 bits encode a callsite instruction
    using GEdgeFlag = u64_t;
    using GEdgeKind = s32_t;

  private:
    NodePtr src = nullptr; ///< source node
    NodePtr dst = nullptr; ///< destination node
    EdgeID id = MAX_EDGEID;
    GEdgeFlag edgeFlag = 0; ///< edge kind

  protected:
    inline void setId(EdgeID _id) { id = _id; }
    inline void setEdgeFlag(GEdgeFlag flag) { edgeFlag = flag; }

  public:
    /// Constructor
    GenericEdge(NodePtr s, NodePtr d, EdgeID id, GEdgeFlag k)
        : src(s), dst(d), id(id), edgeFlag(k) {}

    // to support serialization
    GenericEdge() = default;

    /// Destructor
    virtual ~GenericEdge() {}

    ///  get methods of the components
    //@{
    inline NodeID getSrcID() const { return src->getId(); }
    inline NodeID getDstID() const { return dst->getId(); }
    inline EdgeID getId() const { return id; }
    inline GEdgeKind getEdgeKind() const { return (EdgeKindMask & edgeFlag); }
    NodePtr getSrcNode() const { return src; }
    NodePtr getDstNode() const { return dst; }
    //@}

    /// Add the hash function for std::set (we also can overload operator< to
    /// implement this)
    //  and duplicated elements in the set are not inserted (binary tree
    //  comparison)
    //@{
    typedef struct {
        template <typename EdgePtrType>
        bool operator()(const EdgePtrType lhs, const EdgePtrType rhs) const {
            // compare flags
            if (lhs->edgeFlag != rhs->edgeFlag) {
                return lhs->edgeFlag < rhs->edgeFlag;
            }

            auto lsrc = lhs->getSrcNode();
            auto lsrcId = lsrc == nullptr ? 0 : lsrc->getId();
            auto ldst = lhs->getDstNode();
            auto ldstId = ldst == nullptr ? 0 : ldst->getId();
            auto rsrc = rhs->getSrcNode();
            auto rsrcId = rsrc == nullptr ? 0 : rsrc->getId();
            auto rdst = rhs->getDstNode();
            auto rdstId = rdst == nullptr ? 0 : rdst->getId();

            if (lsrcId != rsrcId) {
                return lsrcId < rsrcId;
            }

            return ldstId < rdstId;
        }
    } equalGEdge;

    inline bool operator==(const GenericEdge<NodeType> *rhs) const {
        return rhs != nullptr && (rhs->edgeFlag == this->edgeFlag &&
                                  rhs->getSrcID() == this->getSrcID() &&
                                  rhs->getDstID() == this->getDstID());
    }
    //@}

    static constexpr unsigned char EdgeKindMaskBits = 8; ///< We use the
                                                         /// lower 8 bits to
                                                         /// denote edge kind
    static constexpr u64_t EdgeKindMask = (~0ULL) >> (64 - EdgeKindMaskBits);
};

/*!
 * Generic node on the graph as base class
 */
template <typename NodeTy, typename EdgeTy, typename NodePtr = NodeTy *,
          typename EdgePtr = EdgeTy *>
class GenericNode {

  public:
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;
    /// Edge kind
    using GNodeK = s32_t;
    using GEdgeSetTy = OrderedSet<EdgePtr, typename EdgeType::equalGEdge>;
    /// Edge iterator
    ///@{
    using iterator = typename GEdgeSetTy::iterator;
    using const_iterator = typename GEdgeSetTy::const_iterator;
    ///@}

  private:
    // Allow serialization to access non-public data members.
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &id;
        ar &nodeKind;
        ar &InEdges;
        ar &OutEdges;
    }

  private:
    NodeID id = MAX_NODEID;                          ///< Node ID
    GNodeK nodeKind = numeric_limits<GNodeK>::max(); ///< Node kind

    GEdgeSetTy InEdges;  ///< all incoming edge of this node
    GEdgeSetTy OutEdges; ///< all outgoing edge of this node

  protected:
    inline void setNodeKind(GNodeK kind) { nodeKind = kind; }
    inline void setId(NodeID _id) { id = _id; }

  public:
    /// Constructor
    GenericNode(NodeID i, GNodeK k) : id(i), nodeKind(k) {}
    // to support serialization
    GenericNode() = default;

    /// Destructor
    virtual ~GenericNode() {}

    /// Get ID
    inline NodeID getId() const { return id; }

    /// Get node kind
    inline GNodeK getNodeKind() const { return nodeKind; }

    /// Get incoming/outgoing edge set
    ///@{
    inline const GEdgeSetTy &getOutEdges() const { return OutEdges; }
    inline const GEdgeSetTy &getInEdges() const { return InEdges; }
    ///@}

    /// Has incoming/outgoing edge set
    //@{
    inline bool hasIncomingEdge() const { return (InEdges.empty() == false); }
    inline bool hasOutgoingEdge() const { return (OutEdges.empty() == false); }
    //@}

    ///  iterators
    //@{
    inline iterator OutEdgeBegin() { return OutEdges.begin(); }
    inline iterator OutEdgeEnd() { return OutEdges.end(); }
    inline iterator InEdgeBegin() { return InEdges.begin(); }
    inline iterator InEdgeEnd() { return InEdges.end(); }
    inline const_iterator OutEdgeBegin() const { return OutEdges.begin(); }
    inline const_iterator OutEdgeEnd() const { return OutEdges.end(); }
    inline const_iterator InEdgeBegin() const { return InEdges.begin(); }
    inline const_iterator InEdgeEnd() const { return InEdges.end(); }
    //@}

    /// Iterators used for SCC detection, overwrite it in child class if
    /// necessory
    //@{
    virtual inline iterator directOutEdgeBegin() { return OutEdges.begin(); }
    virtual inline iterator directOutEdgeEnd() { return OutEdges.end(); }
    virtual inline iterator directInEdgeBegin() { return InEdges.begin(); }
    virtual inline iterator directInEdgeEnd() { return InEdges.end(); }

    virtual inline const_iterator directOutEdgeBegin() const {
        return OutEdges.begin();
    }
    virtual inline const_iterator directOutEdgeEnd() const {
        return OutEdges.end();
    }
    virtual inline const_iterator directInEdgeBegin() const {
        return InEdges.begin();
    }
    virtual inline const_iterator directInEdgeEnd() const {
        return InEdges.end();
    }
    //@}

    /// Add incoming and outgoing edges
    //@{
    inline bool addIncomingEdge(EdgePtr inEdge) {
        return InEdges.insert(inEdge).second;
    }
    inline bool addOutgoingEdge(EdgePtr outEdge) {
        return OutEdges.insert(outEdge).second;
    }
    //@}

    /// Remove incoming and outgoing edges
    ///@{
    inline Size_t removeIncomingEdge(EdgePtr edge) {
        iterator it = InEdges.find(edge);
        assert(it != InEdges.end() && "can not find in edge in SVFG node");
        return InEdges.erase(edge);
    }
    inline Size_t removeOutgoingEdge(EdgePtr edge) {
        iterator it = OutEdges.find(edge);
        assert(it != OutEdges.end() && "can not find out edge in SVFG node");
        return OutEdges.erase(edge);
    }
    ///@}

    /// Find incoming and outgoing edges
    //@{
    inline EdgePtr hasIncomingEdge(EdgePtr edge) const {
        const_iterator it = InEdges.find(edge);
        if (it != InEdges.end()) {
            return *it;
        }

        return nullptr;
    }
    inline EdgePtr hasOutgoingEdge(EdgePtr edge) const {
        const_iterator it = OutEdges.find(edge);
        if (it != OutEdges.end()) {
            return *it;
        }

        return nullptr;
    }
    //@}
};

/// this function handles id map using shared pointers
template <typename ID, typename Value>
void cleanIDMap(
    Map<ID, Value> &m,
    typename enable_if<!is_pointer<Value>::value>::type *d = nullptr) {}

/// this function handles id map using raw pointers
template <typename ID, typename Value>
void cleanIDMap(
    Map<ID, Value> &m,
    typename enable_if<is_pointer<Value>::value>::type *d = nullptr) {
    for (auto &it : m) {
        delete it.second;
    }
}

/*
 * Generic graph for program representation
 * It is base class and needs to be instantiated
 */
template <typename NodeTy, typename EdgeTy, typename NodePtr = NodeTy *,
          typename EdgePtr = EdgeTy *>
class GenericGraph {

  public:
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;
    /// NodeID to GenericNode map
    using IDToNodeMapTy = Map<NodeID, NodePtr>;
    using NodeToIDMapTy = Map<NodePtr, NodeID>;
    using IDToEdgeMapTy = Map<EdgeID, EdgePtr>;
    using EdgeToIDMapTy = Map<EdgePtr, EdgeID>;
    /// Node Iterators
    //@{
    using iterator = typename IDToNodeMapTy::iterator;
    using const_iterator = typename IDToNodeMapTy::const_iterator;
    //@}

    /// Constructor
    GenericGraph() : currentNodeId(0), currentEdgeId(0) {}

    /// Destructor
    virtual ~GenericGraph() { destroy(); }

    virtual void view() {}

    /// Release memory
    void destroy() {
        cleanIDMap(IDToNodeMap);
        cleanIDMap(IDToEdgeMap);
    }
    /// Iterators
    //@{
    inline iterator begin() { return IDToNodeMap.begin(); }
    inline iterator end() { return IDToNodeMap.end(); }
    inline const_iterator begin() const { return IDToNodeMap.begin(); }
    inline const_iterator end() const { return IDToNodeMap.end(); }
    //}@

    /// APIs for handling nodes @{
    /// add a node with a specified id
    inline bool addGNode(NodeID id, NodePtr node) {
        assert(IDToNodeMap.find(id) == IDToNodeMap.end() && "node exists");
        assert(NodeToIDMap.find(node) == NodeToIDMap.end() && "node exists");

        IDToNodeMap[id] = node;
        NodeToIDMap[node] = id;

        return true;
    }

    /// Add a Node
    inline bool addGNode(NodePtr node) {

        if (node != nullptr) {
            auto id = node->getId();
            return addGNode(id, node);
        }

        return false;
    }

    /// Get a node
    inline NodePtr getGNode(NodeID id) const {
        const_iterator it = IDToNodeMap.find(id);
        assert(it != IDToNodeMap.end() && "Node not found!");
        return it->second;
    }

    /// node query by id
    inline bool hasGNode(NodeID id) const {
        return IDToNodeMap.find(id) != IDToNodeMap.end();
    }

    inline bool hasGNode(NodePtr node) const {
        return NodeToIDMap.find(node) != NodeToIDMap.end();
    }

    /// Delete a node
    inline void removeGNode(NodePtr node) {
        assert(node->hasIncomingEdge() == false &&
               node->hasOutgoingEdge() == false &&
               "node which have edges can't be deleted");
        auto it = IDToNodeMap.find(node->getId());
        assert(it != IDToNodeMap.end() && "can not find the node Id");
        IDToNodeMap.erase(it);

        auto it2 = NodeToIDMap.find(node);
        assert(it2 != NodeToIDMap.end() && "can not find the node");
        NodeToIDMap.erase(it2);
    }
    ///}@

    /// APIs for handling nodes @{
    /// add edge
    inline bool addGEdge(EdgePtr edge) {

        assert(edge != nullptr && "edge is null");

        auto srcNode = edge->getSrcNode();
        assert(srcNode != nullptr && "source node is null");
        auto dstNode = edge->getDstNode();
        assert(dstNode != nullptr && "dest node is null");
        assert(hasGNode(srcNode) && "source node not exists");
        assert(hasGNode(dstNode) && "dest node not exists");

        auto id = edge->getId();
        assert(EdgeToIDMap.find(edge) == EdgeToIDMap.end() && "edge exists");
        assert(IDToEdgeMap.find(id) == IDToEdgeMap.end() && "edge id exists?");

        IDToEdgeMap[id] = edge;
        EdgeToIDMap[edge] = id;

        bool added1 = dstNode->addIncomingEdge(edge);
        bool added2 = srcNode->addOutgoingEdge(edge);
        assert(added1 && added2 && "edge not added on both nodes");

        return true;
    }

    inline bool hasGEdge(EdgePtr edge) {
        return EdgeToIDMap.find(edge) != EdgeToIDMap.end();
    }

    inline bool hasGEdge(EdgeID id) {
        return IDToEdgeMap.find(id) != IDToEdgeMap.end();
    }

    inline bool hasGEdge(NodePtr src, NodePtr dst,
                         typename EdgeType::GEdgeKind kind) {
        if (src == nullptr || dst == nullptr) {
            return false;
        }
        for (auto oe : src->getOutEdges()) {
            if (oe->getDstNode() == dst && oe->getEdgeKind() == kind) {
                return true;
            }
        }
        return false;
    }

    inline EdgePtr getGEdge(NodePtr src, NodePtr dst,
                            typename EdgeType::GEdgeKind kind) {
        if (src == nullptr || dst == nullptr) {
            return nullptr;
        }
        Size_t n = 0;
        EdgePtr ret = nullptr;
        for (auto oe : src->getOutEdges()) {
            if (oe->getDstNode() == dst && oe->getEdgeKind() == kind) {
                n++;
                ret = oe;
            }
        }

        assert(n <= 1 && "there are more than 1 edges with the same "
                         "types between two nodes");

        return ret;
    }

    inline EdgePtr getGEdge(EdgeID id) {
        assert(IDToEdgeMap.find(id) != IDToEdgeMap.end() &&
               "edge id not exists");
        return IDToEdgeMap[id];
    }

    inline void removeGEdge(EdgeID id) { return removeGEdge(getGEdge(id)); }

    inline void removeGEdge(EdgePtr edge) {
        assert(edge != nullptr && "edge is null");

        auto srcNode = edge->getSrcNode();
        assert(srcNode != nullptr && "source node is null");
        auto dstNode = edge->getDstNode();
        assert(dstNode != nullptr && "dest node is null");
        assert(hasGNode(srcNode) && "source node not exists");
        assert(hasGNode(dstNode) && "dest node not exists");

        srcNode->removeOutgoingEdge(edge);
        dstNode->removeIncomingEdge(edge);

        auto id = edge->getId();
        auto it = EdgeToIDMap.find(edge);
        assert(it != EdgeToIDMap.end() && "edge not exists");
        EdgeToIDMap.erase(it);

        auto it2 = IDToEdgeMap.find(id);
        assert(it2 != IDToEdgeMap.end() && "edge id not exits");
        IDToEdgeMap.erase(it2);
    }
    ///}@

    /// Get total number of node/edge
    inline NodeID getTotalNodeNum() const { return IDToNodeMap.size(); }
    inline EdgeID getTotalEdgeNum() const { return IDToEdgeMap.size(); }

    /// generate an invalid id for query
    /// some APIs in SVF generates node or
    /// edge oject  and use it to query
    /// the exsitence of some edges with c
    /// ertain source and dest in the graph
    /// FIXME: we can drop those
    /// implmentations later on
    inline NodeID getDummyNodeId() const { return MAX_NODEID; }
    inline EdgeID getDummyEdgeId() const { return MAX_EDGEID; }

    inline NodeID getNextNodeId() {
        assert(currentNodeId < MAX_NODEID && "node id overflow");
        return currentNodeId++;
    }
    inline EdgeID getNextEdgeId() {
        assert(currentEdgeId < MAX_EDGEID && "edge id overflow");
        return currentEdgeId++;
    }

  protected:
    IDToNodeMapTy IDToNodeMap; ///< id - node map
    NodeToIDMapTy NodeToIDMap; ///< node - id map
    IDToEdgeMapTy IDToEdgeMap; ///< id - edge map
    EdgeToIDMapTy EdgeToIDMap; ///< edge - id map

    void setCurrentNodeId(NodeID id) { currentNodeId = id; }
    void setCurrentEdgeId(EdgeID id) { currentEdgeId = id; }

  private:
    NodeID currentNodeId = 0;
    EdgeID currentEdgeId = 0;

  private:
    // Allow serialization to access non-public data members.
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &IDToNodeMap;
        ar &NodeToIDMap;
        ar &IDToEdgeMap;
        ar &EdgeToIDMap;
        ar &currentEdgeId;
        ar &currentNodeId;
    }
};

} // End namespace SVF

/* !
 * GraphTraits specializations for generic graph algorithms.
 * Provide graph traits for tranversing from a node using standard graph
 * traversals.
 */
namespace llvm {

// /// this functor returns a
template <typename EdgePtr, typename NodePtr>
struct GetEdgeDest {
    NodePtr operator()(EdgePtr edge) const { return edge->getDstNode(); }
};

template <typename ID, typename NodePtr, typename RetNodeType = NodePtr>
struct GetNodeFromIDPair {
    RetNodeType operator()(
        std::pair<ID, NodePtr> pair,
        typename std::enable_if<is_same<RetNodeType, NodePtr>::value>::type *d =
            nullptr) const {
        return pair.second;
    }
    //// TODO: handle other situations
};

// template <class PairTy, class NodeTy>
// struct pair_unary_function {
//     NodeTy operator()(PairTy pair) const { return pair.second; }
// };

// using GetEdgeDest = std::function<typename _Signature>
/// template <typename EdgePtr, typename NodePtr>
// using GetEdgeDst = std::function<NodePtr(EdgePtr)>;

/// template <typename EdgePtr, typename NodePtr>
/// using GetEdgeDst = std::function<NodePtr(EdgePtr)>;

/*!
 * GraphTraits for nodes
 */
template <typename NodeTy, typename EdgeTy, typename NodePtr, typename EdgePtr>
struct GraphTraits<SVF::GenericNode<NodeTy, EdgeTy, NodePtr, EdgePtr> *> {
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;
    using MapEdgeDest = GetEdgeDest<EdgePtr, NodePtr>;

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    using GNodeType =
        typename SVF::GenericNode<NodeTy, EdgeTy, NodePtr, EdgePtr>;
    using ChildIteratorType =
        mapped_iterator<typename GNodeType::iterator, MapEdgeDest>;

    static NodePtr getEntryNode(NodePtr pagN) { return pagN; }

    static inline ChildIteratorType child_begin(const NodePtr N) {
        return map_iterator(N->OutEdgeBegin(), MapEdgeDest());
    }
    static inline ChildIteratorType child_end(const NodePtr N) {
        return map_iterator(N->OutEdgeEnd(), MapEdgeDest());
    }
    static inline ChildIteratorType direct_child_begin(const NodePtr N) {
        return map_iterator(N->directOutEdgeBegin(), MapEdgeDest());
    }
    static inline ChildIteratorType direct_child_end(const NodePtr N) {
        return map_iterator(N->directOutEdgeEnd(), MapEdgeDest());
    }
};

/*!
 * Inverse GraphTraits for node which is used for inverse traversal.
 */
template <typename NodeTy, typename EdgeTy, typename NodePtr, typename EdgePtr>
struct GraphTraits<
    Inverse<SVF::GenericNode<NodeTy, EdgeTy, NodePtr, EdgePtr> *>> {
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;

    using MapEdgeDest = GetEdgeDest<EdgePtr, NodePtr>;

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    using GNodeType =
        typename SVF::GenericNode<NodeTy, EdgeTy, NodePtr, EdgePtr>;
    using ChildIteratorType =
        mapped_iterator<typename GNodeType::iterator, MapEdgeDest>;

    static inline NodePtr getEntryNode(Inverse<NodePtr> G) { return G.Graph; }

    static inline ChildIteratorType child_begin(const NodePtr N) {
        return map_iterator(N->InEdgeBegin(), MapEdgeDest());
    }
    static inline ChildIteratorType child_end(const NodePtr N) {
        return map_iterator(N->InEdgeEnd(), MapEdgeDest());
    }

    static inline unsigned getNodeID(const NodePtr N) { return N->getId(); }
};

/*!
 * GraphTraints
 */

template <typename NodeTy, typename EdgeTy, typename NodePtr, typename EdgePtr>
struct GraphTraits<SVF::GenericGraph<NodeTy, EdgeTy, NodePtr, EdgePtr> *>
    : public GraphTraits<SVF::GenericNode<NodeTy, EdgeTy, NodePtr, EdgePtr> *> {
    using GGraph = SVF::GenericGraph<NodeTy, EdgeTy, NodePtr, EdgePtr>;
    using GGraphPtr = SVF::GenericGraph<NodeTy, EdgeTy, NodePtr, EdgePtr> *;
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;

    static NodePtr getEntryNode(GGraphPtr pag) {
        return nullptr; // return null here, maybe later we could create a dummy
                        // node
    }
    using MapIDNodePair = GetNodeFromIDPair<SVF::NodeID, NodePtr>;

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    using nodes_iterator =
        mapped_iterator<typename GGraph::iterator, MapIDNodePair>;

    static nodes_iterator nodes_begin(GGraphPtr G) {
        return map_iterator(G->begin(), MapIDNodePair());
    }
    static nodes_iterator nodes_end(GGraphPtr G) {
        return map_iterator(G->end(), MapIDNodePair());
    }

    static unsigned graphSize(GGraphPtr G) { return G->getTotalNodeNum(); }

    static inline unsigned getNodeID(NodePtr N) { return N->getId(); }
    static NodePtr getNode(GGraphPtr G, SVF::NodeID id) {
        return G->getGNode(id);
    }
};

} // End namespace llvm

#endif /* GENERICGRAPH_H_ */
