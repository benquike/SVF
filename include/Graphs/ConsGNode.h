//===- ConsGNode.h -- Constraint graph node-----------------------------------//
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
 * ConsGNode.h
 *
 *  Created on: Mar 19, 2014
 *      Author: Yulei Sui
 */

#ifndef CONSGNODE_H_
#define CONSGNODE_H_

#include "Graphs/ConsGEdge.h"
namespace SVF {

/*!
 * Constraint node
 */

class ConstraintNode;

using GenericConsNodeTy = GenericNode<ConstraintNode, ConstraintEdge>;
class ConstraintNode : public GenericConsNodeTy {

  public:
    using iterator = ConstraintEdge::ConstraintEdgeSetTy::iterator;
    using const_iterator = ConstraintEdge::ConstraintEdgeSetTy::const_iterator;
    bool _isPWCNode;

    enum SCCEdgeFlag { Copy, Direct };

  private:
    ConstraintEdge::ConstraintEdgeSetTy
        loadInEdges; ///< all incoming load edge of this node
    ConstraintEdge::ConstraintEdgeSetTy
        loadOutEdges; ///< all outgoing load edge of this node

    ConstraintEdge::ConstraintEdgeSetTy
        storeInEdges; ///< all incoming store edge of this node
    ConstraintEdge::ConstraintEdgeSetTy
        storeOutEdges; ///< all outgoing store edge of this node

    /// Copy/call/ret/gep incoming edge of this node,
    /// To be noted: this set is only used when SCC detection, and node merges
    ConstraintEdge::ConstraintEdgeSetTy directInEdges;
    ConstraintEdge::ConstraintEdgeSetTy directOutEdges;

    ConstraintEdge::ConstraintEdgeSetTy copyInEdges;
    ConstraintEdge::ConstraintEdgeSetTy copyOutEdges;

    ConstraintEdge::ConstraintEdgeSetTy gepInEdges;
    ConstraintEdge::ConstraintEdgeSetTy gepOutEdges;

    ConstraintEdge::ConstraintEdgeSetTy
        addressInEdges; ///< all incoming address edge of this node
    ConstraintEdge::ConstraintEdgeSetTy
        addressOutEdges; ///< all outgoing address edge of this node

    PAG *pag = nullptr;

  public:
    static SCCEdgeFlag sccEdgeFlag;

    NodeBS strides;
    bool newExpand;
    NodeBS baseIds;

    static void setSCCEdgeFlag(SCCEdgeFlag f) { sccEdgeFlag = f; }

    ConstraintNode(NodeID i, PAG *pag)
        : GenericConsNodeTy(i, 0), _isPWCNode(false), pag(pag),
          newExpand(false) {}

    virtual ~ConstraintNode() {}

    virtual const std::string toString() const;

    /// Overloading operator << for dumping ICFG node ID
    //@{
    friend raw_ostream &operator<<(raw_ostream &o, const ConstraintNode &node) {
        o << node.toString();
        return o;
    }
    //@}

    /// Whether a node involves in PWC, if so, all its points-to elements should
    /// become field-insensitive.
    //@{
    inline bool isPWCNode() const { return _isPWCNode; }
    inline void setPWCNode() { _isPWCNode = true; }
    //@}

    /// Direct and Indirect PAG edges
    inline bool isdirectEdge(ConstraintEdge::ConstraintEdgeK kind) {
        return (kind == ConstraintEdge::Copy ||
                kind == ConstraintEdge::NormalGep ||
                kind == ConstraintEdge::VariantGep);
    }
    inline bool isIndirectEdge(ConstraintEdge::ConstraintEdgeK kind) {
        return (kind == ConstraintEdge::Load || kind == ConstraintEdge::Store);
    }

    /// Return constraint edges
    //@{
    inline const ConstraintEdge::ConstraintEdgeSetTy &getDirectInEdges() const {
        return directInEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &
    getDirectOutEdges() const {
        return directOutEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getCopyInEdges() const {
        return copyInEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getCopyOutEdges() const {
        return copyOutEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getGepInEdges() const {
        return gepInEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getGepOutEdges() const {
        return gepOutEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getLoadInEdges() const {
        return loadInEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getLoadOutEdges() const {
        return loadOutEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getStoreInEdges() const {
        return storeInEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getStoreOutEdges() const {
        return storeOutEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getAddrInEdges() const {
        return addressInEdges;
    }

    inline const ConstraintEdge::ConstraintEdgeSetTy &getAddrOutEdges() const {
        return addressOutEdges;
    }
    //@}

    PAG *getPAG() const { return pag; }

    ///  Iterators
    //@{
    inline iterator directOutEdgeBegin() override {
        if (sccEdgeFlag == Copy) {
            return copyOutEdges.begin();
        }

        return directOutEdges.begin();
    }

    inline iterator directOutEdgeEnd() override {
        if (sccEdgeFlag == Copy) {
            return copyOutEdges.end();
        }

        return directOutEdges.end();
    }

    inline iterator directInEdgeBegin() override {
        if (sccEdgeFlag == Copy) {
            return copyInEdges.begin();
        }

        return directInEdges.begin();
    }

    inline iterator directInEdgeEnd() override {
        if (sccEdgeFlag == Copy) {
            return copyInEdges.end();
        }

        return directInEdges.end();
    }

    inline const_iterator directOutEdgeBegin() const override {
        if (sccEdgeFlag == Copy) {
            return copyOutEdges.begin();
        }

        return directOutEdges.begin();
    }

    inline const_iterator directOutEdgeEnd() const override {
        if (sccEdgeFlag == Copy) {
            return copyOutEdges.end();
        }

        return directOutEdges.end();
    }

    inline const_iterator directInEdgeBegin() const override {
        if (sccEdgeFlag == Copy) {
            return copyInEdges.begin();
        }

        return directInEdges.begin();
    }

    inline const_iterator directInEdgeEnd() const override {
        if (sccEdgeFlag == Copy) {
            return copyInEdges.end();
        }

        return directInEdges.end();
    }

    ConstraintEdge::ConstraintEdgeSetTy &incomingAddrEdges() {
        return addressInEdges;
    }
    ConstraintEdge::ConstraintEdgeSetTy &outgoingAddrEdges() {
        return addressOutEdges;
    }

    inline const_iterator outgoingAddrsBegin() const {
        return addressOutEdges.begin();
    }
    inline const_iterator outgoingAddrsEnd() const {
        return addressOutEdges.end();
    }
    inline const_iterator incomingAddrsBegin() const {
        return addressInEdges.begin();
    }
    inline const_iterator incomingAddrsEnd() const {
        return addressInEdges.end();
    }

    inline const_iterator outgoingLoadsBegin() const {
        return loadOutEdges.begin();
    }
    inline const_iterator outgoingLoadsEnd() const {
        return loadOutEdges.end();
    }
    inline const_iterator incomingLoadsBegin() const {
        return loadInEdges.begin();
    }
    inline const_iterator incomingLoadsEnd() const { return loadInEdges.end(); }

    inline const_iterator outgoingStoresBegin() const {
        return storeOutEdges.begin();
    }
    inline const_iterator outgoingStoresEnd() const {
        return storeOutEdges.end();
    }
    inline const_iterator incomingStoresBegin() const {
        return storeInEdges.begin();
    }
    inline const_iterator incomingStoresEnd() const {
        return storeInEdges.end();
    }
    //@}

    ///  Add constraint graph edges
    //@{
    inline void addIncomingCopyEdge(CopyCGEdge *inEdge) {
        assert(inEdge->getDstID() == this->getId());
        addIncomingDirectEdge(inEdge);
        bool added = copyInEdges.insert(inEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addIncomingGepEdge(GepCGEdge *inEdge) {
        assert(inEdge->getDstID() == this->getId());
        addIncomingDirectEdge(inEdge);
        bool added = gepInEdges.insert(inEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addOutgoingCopyEdge(CopyCGEdge *outEdge) {
        assert(outEdge->getSrcID() == this->getId());
        addOutgoingDirectEdge(outEdge);
        bool added = copyOutEdges.insert(outEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addOutgoingGepEdge(GepCGEdge *outEdge) {
        assert(outEdge->getSrcID() == this->getId());
        addOutgoingDirectEdge(outEdge);
        bool added = gepOutEdges.insert(outEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addIncomingAddrEdge(AddrCGEdge *inEdge) {
        assert(inEdge->getDstID() == this->getId());
        bool added = addressInEdges.insert(inEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addIncomingLoadEdge(LoadCGEdge *inEdge) {
        assert(inEdge->getDstID() == this->getId());
        bool added = loadInEdges.insert(inEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addIncomingStoreEdge(StoreCGEdge *inEdge) {
        assert(inEdge->getDstID() == this->getId());
        bool added = storeInEdges.insert(inEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addIncomingDirectEdge(ConstraintEdge *inEdge) {
        assert(inEdge->getDstID() == this->getId());
        bool added = directInEdges.insert(inEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }

    inline void addOutgoingAddrEdge(AddrCGEdge *outEdge) {
        assert(outEdge->getSrcID() == this->getId());
        bool added = addressOutEdges.insert(outEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addOutgoingLoadEdge(LoadCGEdge *outEdge) {
        assert(outEdge->getSrcID() == this->getId());
        bool added = loadOutEdges.insert(outEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addOutgoingStoreEdge(StoreCGEdge *outEdge) {
        assert(outEdge->getSrcID() == this->getId());
        bool added = storeOutEdges.insert(outEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    inline void addOutgoingDirectEdge(ConstraintEdge *outEdge) {
        assert(outEdge->getSrcID() == this->getId());
        bool added = directOutEdges.insert(outEdge).second;
        assert(added && "edge not added, duplicated adding!!");
    }
    //@}

    /// Remove constraint graph edges
    //{@
    inline void removeOutgoingAddrEdge(AddrCGEdge *outEdge) {
        Size_t num = addressOutEdges.erase(outEdge);
        assert((num) && "edge not in the set, can not remove!!!");
    }

    inline void removeIncomingAddrEdge(AddrCGEdge *inEdge) {
        Size_t num = addressInEdges.erase(inEdge);
        assert((num) && "edge not in the set, can not remove!!!");
    }

    inline void removeOutgoingDirectEdge(ConstraintEdge *outEdge) {
        if (llvm::isa<GepCGEdge>(outEdge)) {
            gepOutEdges.erase(outEdge);
        } else {
            copyOutEdges.erase(outEdge);
        }
        Size_t num = directOutEdges.erase(outEdge);
        assert((num) && "edge not in the set, can not remove!!!");
    }

    inline void removeIncomingDirectEdge(ConstraintEdge *inEdge) {
        if (llvm::isa<GepCGEdge>(inEdge)) {
            gepInEdges.erase(inEdge);
        } else {
            copyInEdges.erase(inEdge);
        }
        Size_t num = directInEdges.erase(inEdge);
        assert((num) && "edge not in the set, can not remove!!!");
    }

    inline void removeOutgoingLoadEdge(LoadCGEdge *outEdge) {
        Size_t num = loadOutEdges.erase(outEdge);
        assert((num) && "edge not in the set, can not remove!!!");
    }

    inline void removeIncomingLoadEdge(LoadCGEdge *inEdge) {
        Size_t num = loadInEdges.erase(inEdge);
        assert((num) && "edge not in the set, can not remove!!!");
    }

    inline void removeOutgoingStoreEdge(StoreCGEdge *outEdge) {
        Size_t num = storeOutEdges.erase(outEdge);
        assert((num) && "edge not in the set, can not remove!!!");
    }

    inline void removeIncomingStoreEdge(StoreCGEdge *inEdge) {
        Size_t num = storeInEdges.erase(inEdge);
        assert((num) && "edge not in the set, can not remove!!!");
    }
    //@}
};

} // End namespace SVF

#endif /* CONSGNODE_H_ */
