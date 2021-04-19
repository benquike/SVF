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

 *  Updated by:
 *     Hui Peng <peng124@purdue.edu>
 *     2021-04-18
 */

#ifndef GENERICGRAPH_H_
#define GENERICGRAPH_H_

#include "Util/BasicTypes.h"
#include "Util/Serialization.h"

namespace SVF {

/*!
 * Generic edge on the graph as base class
 */
template <class NodeTy> class GenericEdge {
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
    NodeTy *src = nullptr; ///< source node
    NodeTy *dst = nullptr; ///< destination node
    EdgeID id = numeric_limits<EdgeID>::max();
    GEdgeFlag edgeFlag = 0; ///< edge kind

  public:
    /// Constructor
    GenericEdge(NodeTy *s, NodeTy *d, EdgeID id, GEdgeFlag k)
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
    NodeType *getSrcNode() const { return src; }
    NodeType *getDstNode() const { return dst; }
    //@}

    /// Add the hash function for std::set (we also can overload operator< to
    /// implement this)
    //  and duplicated elements in the set are not inserted (binary tree
    //  comparison)
    //@{
    typedef struct {
        bool operator()(const GenericEdge<NodeType> *lhs,
                        const GenericEdge<NodeType> *rhs) const {
            if (lhs->edgeFlag != rhs->edgeFlag) {
                return lhs->edgeFlag < rhs->edgeFlag;
            } else {
                NodeType *lsrc = lhs->getSrcNode();
                NodeID lsrcId = lsrc == nullptr ? 0 : lsrc->getId();
                NodeType *ldst = lhs->getDstNode();
                NodeID ldstId = ldst == nullptr ? 0 : ldst->getId();
                NodeType *rsrc = rhs->getSrcNode();
                NodeID rsrcId = rsrc == nullptr ? 0 : rsrc->getId();
                NodeType *rdst = rhs->getDstNode();
                NodeID rdstId = rdst == nullptr ? 0 : rdst->getId();

                if (lsrcId != rsrcId) {
                    return lsrcId < rsrcId;
                } else {
                    return ldstId < rdstId;
                }
            }
        }
    } equalGEdge;

    inline bool operator==(const GenericEdge<NodeType> *rhs) const {
        return rhs != nullptr && (rhs->edgeFlag == this->edgeFlag &&
                                  rhs->getSrcID() == this->getSrcID() &&
                                  rhs->getDstID() == this->getDstID());
    }
    //@}

    static constexpr unsigned char EdgeKindMaskBits =
        8; ///< We use the lower 8 bits to denote edge kind
    static constexpr u64_t EdgeKindMask = (~0ULL) >> (64 - EdgeKindMaskBits);
};

/*!
 * Generic node on the graph as base class
 */
template <class NodeTy, class EdgeTy> class GenericNode {

  public:
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;
    /// Edge kind
    using GNodeK = s32_t;
    using GEdgeSetTy = OrderedSet<EdgeType *, typename EdgeType::equalGEdge>;
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
    NodeID id = numeric_limits<NodeID>::max();       ///< Node ID
    GNodeK nodeKind = numeric_limits<GNodeK>::max(); ///< Node kind

    GEdgeSetTy InEdges;  ///< all incoming edge of this node
    GEdgeSetTy OutEdges; ///< all outgoing edge of this node

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
    inline bool addIncomingEdge(EdgeType *inEdge) {
        return InEdges.insert(inEdge).second;
    }
    inline bool addOutgoingEdge(EdgeType *outEdge) {
        return OutEdges.insert(outEdge).second;
    }
    //@}

    /// Remove incoming and outgoing edges
    ///@{
    inline Size_t removeIncomingEdge(EdgeType *edge) {
        iterator it = InEdges.find(edge);
        assert(it != InEdges.end() && "can not find in edge in SVFG node");
        return InEdges.erase(edge);
    }
    inline Size_t removeOutgoingEdge(EdgeType *edge) {
        iterator it = OutEdges.find(edge);
        assert(it != OutEdges.end() && "can not find out edge in SVFG node");
        return OutEdges.erase(edge);
    }
    ///@}

    /// Find incoming and outgoing edges
    //@{
    inline EdgeType *hasIncomingEdge(EdgeType *edge) const {
        const_iterator it = InEdges.find(edge);
        if (it != InEdges.end()) {
            return *it;
        }

        return nullptr;
    }
    inline EdgeType *hasOutgoingEdge(EdgeType *edge) const {
        const_iterator it = OutEdges.find(edge);
        if (it != OutEdges.end()) {
            return *it;
        }

        return nullptr;
    }
    //@}
};

/*
 * Generic graph for program representation
 * It is base class and needs to be instantiated
 */
template <class NodeTy, class EdgeTy> class GenericGraph {

  public:
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;
    /// NodeID to GenericNode map
    using IDToNodeMapTy = Map<NodeID, NodeType *>;
    using NodeToIDMapTy = Map<NodeType *, NodeID>;
    using IDToEdgeMapTy = Map<EdgeID, EdgeType *>;
    using EdgeToIDMapTy = Map<EdgeType *, EdgeID>;
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
#if 0
        for (iterator I = IDToNodeMap.begin(), E = IDToNodeMap.end(); I != E;
             ++I) {
            // NodeType* node = I->second;
            // for(typename NodeType::iterator it = node->InEdgeBegin(), eit =
            // node->InEdgeEnd(); it!=eit; ++it)
            //         delete *it;
        }
#endif
        for (auto I : IDToNodeMap) {
            // delete a node
            delete I.second;
        }

        for (auto &I : IDToEdgeMap) {
            // delete an edge
            delete (I.second);
        }
    }
    /// Iterators
    //@{
    inline iterator begin() { return IDToNodeMap.begin(); }
    inline iterator end() { return IDToNodeMap.end(); }
    inline const_iterator begin() const { return IDToNodeMap.begin(); }
    inline const_iterator end() const { return IDToNodeMap.end(); }
    //}@

    ///@{
    /// add a node with a specified id
    inline bool addGNode(NodeID id, NodeType *node) {

        // llvm::outs() << __PRETTY_FUNCTION__ << ": add node: " << id << "\n";

        if (IDToNodeMap.find(id) == IDToNodeMap.end()) {
            assert(NodeToIDMap.find(node) == NodeToIDMap.end() &&
                   "node exists");
            IDToNodeMap[id] = node;
            NodeToIDMap[node] = id;
            return true;
        }
        return false;
    }

    /// Add a Node
    inline bool addGNode(NodeType *node) {

        if (node != nullptr) {
            auto id = node->getId();
            return addGNode(id, node);
        }

        return false;
    }
    ///}@

    inline bool addGEdge(EdgeType *edge) {
        if (edge != nullptr) {
            auto id = edge->getId();

            llvm::outs() << __PRETTY_FUNCTION__ << ": id: " << id
                         << ", addr: " << edge << "\n";

            if (id == 103) {
                llvm::outs() << "edge 103\n";
            }

            if (IDToEdgeMap.find(id) == IDToEdgeMap.end()) {
                assert(EdgeToIDMap.find(edge) == EdgeToIDMap.end() &&
                       "edge exists");
                IDToEdgeMap[id] = edge;
                EdgeToIDMap[edge] = id;

                bool added1 = edge->getDstNode()->addIncomingEdge(edge);
                bool added2 = edge->getSrcNode()->addOutgoingEdge(edge);
                assert(added1 && added2 && "edge not added??");
                return true;
            }
        }
        return false;
    }

    /// Get a node
    inline NodeType *getGNode(NodeID id) const {
        const_iterator it = IDToNodeMap.find(id);
        assert(it != IDToNodeMap.end() && "Node not found!");
        return it->second;
    }

    /// Has a node
    inline bool hasGNode(NodeID id) const {
        const_iterator it = IDToNodeMap.find(id);
        return it != IDToNodeMap.end();
    }

    inline bool hasGEdge(NodeType *src, NodeType *dst,
                         typename EdgeType::GEdgeKind kind) {
        if (src == nullptr || dst == nullptr) {
            return false;
        }
        for (auto oe : dst->getOutEdges()) {
            if (oe->getDstNode() == dst && oe->getEdgeKind() == kind) {
                return true;
            }
        }
        return false;
    }

    inline EdgeType *getGEdge(NodeType *src, NodeType *dst,
                              typename EdgeType::GEdgeKind kind) {
        if (src == nullptr || dst == nullptr) {
            return nullptr;
        }
        for (auto oe : dst->getOutEdges()) {
            if (oe->getDstNode() == dst && oe->getEdgeKind() == kind) {
                return oe;
            }
        }
        return nullptr;
    }

    /// Delete a node
    inline void removeGNode(NodeType *node) {
        assert(node->hasIncomingEdge() == false &&
               node->hasOutgoingEdge() == false &&
               "node which have edges can't be deleted");
        iterator it = IDToNodeMap.find(node->getId());
        assert(it != IDToNodeMap.end() && "can not find the node");
        IDToNodeMap.erase(it);
    }

    /// Get total number of node/edge
    inline NodeID getTotalNodeNum() const { return IDToNodeMap.size(); }
    inline EdgeID getTotalEdgeNum() const { return IDToEdgeMap.size(); }

    inline NodeID getDummyNodeId() const {
        return numeric_limits<NodeID>::max();
    }

    inline EdgeID getDummyEdgeId() const {
        return numeric_limits<EdgeID>::max();
    }

    inline NodeID getNextNodeId() {
        assert(currentNodeId < numeric_limits<NodeID>::max() &&
               "node id overflow");
        return currentNodeId++;
    }
    inline EdgeID getNextEdgeId() {
        assert(currentEdgeId < numeric_limits<EdgeID>::max() &&
               "edge id overflow");
        return currentEdgeId++;
    }

  protected:
    IDToNodeMapTy IDToNodeMap; ///< id - node map
    NodeToIDMapTy NodeToIDMap; ///< node - id map
    IDToEdgeMapTy IDToEdgeMap; ///< id - edge map
    EdgeToIDMapTy EdgeToIDMap; ///< edge - id map

    void setCurrentNodeId(NodeID id) { currentNodeId = id; }
    void setCurrentEdgeId(EdgeID id) { currentNodeId = id; }

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

template <class EdgeTy, class NodeTy> struct edge_unary_function {
    NodeTy operator()(EdgeTy edge) const { return edge->getDstNode(); }
};

template <class PairTy, class NodeTy> struct pair_unary_function {
    NodeTy operator()(PairTy pair) const { return pair.second; }
};

/*!
 * GraphTraits for nodes
 */
template <class NodeTy, class EdgeTy>
struct GraphTraits<SVF::GenericNode<NodeTy, EdgeTy> *> {
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;

    using DerefEdge = edge_unary_function<EdgeType *, NodeType *>;

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    using ChildIteratorType =
        mapped_iterator<typename SVF::GenericNode<NodeTy, EdgeTy>::iterator,
                        DerefEdge>;

    static NodeType *getEntryNode(NodeType *pagN) { return pagN; }

    static inline ChildIteratorType child_begin(const NodeType *N) {
        return map_iterator(N->OutEdgeBegin(), DerefEdge());
    }
    static inline ChildIteratorType child_end(const NodeType *N) {
        return map_iterator(N->OutEdgeEnd(), DerefEdge());
    }
    static inline ChildIteratorType direct_child_begin(const NodeType *N) {
        return map_iterator(N->directOutEdgeBegin(), DerefEdge());
    }
    static inline ChildIteratorType direct_child_end(const NodeType *N) {
        return map_iterator(N->directOutEdgeEnd(), DerefEdge());
    }
};

/*!
 * Inverse GraphTraits for node which is used for inverse traversal.
 */
template <class NodeTy, class EdgeTy>
struct GraphTraits<Inverse<SVF::GenericNode<NodeTy, EdgeTy> *>> {
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;

    using DerefEdge = edge_unary_function<EdgeType *, NodeType *>;

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    using ChildIteratorType =
        mapped_iterator<typename SVF::GenericNode<NodeTy, EdgeTy>::iterator,
                        DerefEdge>;

    static inline NodeType *getEntryNode(Inverse<NodeType *> G) {
        return G.Graph;
    }

    static inline ChildIteratorType child_begin(const NodeType *N) {
        return map_iterator(N->InEdgeBegin(), DerefEdge());
    }
    static inline ChildIteratorType child_end(const NodeType *N) {
        return map_iterator(N->InEdgeEnd(), DerefEdge());
    }

    static inline unsigned getNodeID(const NodeType *N) { return N->getId(); }
};

/*!
 * GraphTraints
 */
template <class NodeTy, class EdgeTy>
struct GraphTraits<SVF::GenericGraph<NodeTy, EdgeTy> *>
    : public GraphTraits<SVF::GenericNode<NodeTy, EdgeTy> *> {
    using GenericGraphTy = SVF::GenericGraph<NodeTy, EdgeTy>;
    using NodeType = NodeTy;
    using EdgeType = EdgeTy;

    static NodeType *getEntryNode(GenericGraphTy *pag) {
        return nullptr; // return null here, maybe later we could create a dummy
                        // node
    }
    using PairTy = std::pair<SVF::NodeID, NodeType *>;
    using DerefVal = pair_unary_function<PairTy, NodeType *>;

    // nodes_iterator/begin/end - Allow iteration over all nodes in the graph
    using nodes_iterator =
        mapped_iterator<typename GenericGraphTy::iterator, DerefVal>;

    static nodes_iterator nodes_begin(GenericGraphTy *G) {
        return map_iterator(G->begin(), DerefVal());
    }
    static nodes_iterator nodes_end(GenericGraphTy *G) {
        return map_iterator(G->end(), DerefVal());
    }

    static unsigned graphSize(GenericGraphTy *G) {
        return G->getTotalNodeNum();
    }

    static inline unsigned getNodeID(NodeType *N) { return N->getId(); }
    static NodeType *getNode(GenericGraphTy *G, SVF::NodeID id) {
        return G->getGNode(id);
    }
};

} // End namespace llvm

#endif /* GENERICGRAPH_H_ */
