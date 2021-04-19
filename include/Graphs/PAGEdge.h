//===- PAGEdge.h -- PAG edge class-------------------------------------------//
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
 * PAGEdge.h
 *
 *  Created on: Nov 10, 2013
 *      Author: Yulei Sui
 */

#ifndef PAGEDGE_H_
#define PAGEDGE_H_

#include "Graphs/GenericGraph.h"
#include "Graphs/ICFGNode.h"
#include "MemoryModel/LocationSet.h"
#include "Util/Serialization.h"

namespace SVF {

class PAGNode;
class PAG;

/*
 * PAG edge between nodes
 */
using GenericPAGEdgeTy = GenericEdge<PAGNode>;
class PAGEdge : public GenericPAGEdgeTy {

  public:
    /// Thirteen kinds of PAG edges
    /// Gep represents offset edge for field sensitivity
    /// ThreadFork/ThreadJoin is to model parameter passings between thread
    /// spawners and spawnees.
    enum PEDGEK {
        Addr,
        Copy,
        Store,
        Load,
        Call,
        Ret,
        NormalGep,
        VariantGep,
        ThreadFork,
        ThreadJoin,
        Cmp,
        BinaryOp,
        UnaryOp
    };

  private:
    const Value *value = nullptr;           ///< LLVM value
    const BasicBlock *basicBlock = nullptr; ///< LLVM BasicBlock
    ICFGNode *icfgNode = nullptr;           ///< ICFGNode
    EdgeID edgeId;                          ///< Edge ID
  public:

    /// Constructor
    PAGEdge(PAGNode *s, PAGNode *d, PAG *pag, GEdgeFlag k);
    PAGEdge() = default;

    /// Destructor
    virtual ~PAGEdge() {}

    /// ClassOf
    //@{
    static inline bool classof(const PAGEdge *) { return true; }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Addr ||
               edge->getEdgeKind() == PAGEdge::Copy ||
               edge->getEdgeKind() == PAGEdge::Store ||
               edge->getEdgeKind() == PAGEdge::Load ||
               edge->getEdgeKind() == PAGEdge::Call ||
               edge->getEdgeKind() == PAGEdge::Ret ||
               edge->getEdgeKind() == PAGEdge::NormalGep ||
               edge->getEdgeKind() == PAGEdge::VariantGep ||
               edge->getEdgeKind() == PAGEdge::ThreadFork ||
               edge->getEdgeKind() == PAGEdge::ThreadJoin ||
               edge->getEdgeKind() == PAGEdge::Cmp ||
               edge->getEdgeKind() == PAGEdge::BinaryOp ||
               edge->getEdgeKind() == PAGEdge::UnaryOp;
    }
    ///@}

    /// Return Edge ID
    inline EdgeID getEdgeID() const { return edgeId; }
    /// Whether src and dst nodes are both of pointer type
    bool isPTAEdge() const;

    /// Get/set methods for llvm instruction
    //@{
    inline const Instruction *getInst() const {
        return SVFUtil::dyn_cast<Instruction>(value);
    }
    inline void setValue(const Value *val) { value = val; }
    inline const Value *getValue() const { return value; }
    inline void setBB(const BasicBlock *bb) { basicBlock = bb; }
    inline const BasicBlock *getBB() const { return basicBlock; }
    inline void setICFGNode(ICFGNode *node) { icfgNode = node; }
    inline ICFGNode *getICFGNode() const { return icfgNode; }
    //@}

    virtual const std::string toString() const;

    //@}
    /// Overloading operator << for dumping PAGNode value
    //@{
    friend raw_ostream &operator<<(raw_ostream &o, const PAGEdge &edge) {
        o << edge.toString();
        return o;
    }
    //@}

    using PAGEdgeSetTy = GenericNode<PAGNode, PAGEdge>::GEdgeSetTy;
    using PAGEdgeToSetMapTy = Map<EdgeID, PAGEdgeSetTy>;
    using PAGKindToEdgeSetMapTy = PAGEdgeToSetMapTy;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<GenericPAGEdgeTy>(*this);
        SAVE_Value(ar, value);
        SAVE_Value(ar, basicBlock);
        ar &icfgNode;
        ar &edgeId;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<GenericPAGEdgeTy>(*this);
        LOAD_Value(ar, Value, value);
        LOAD_Value(ar, BasicBlock, basicBlock);
        ar &icfgNode;
        ar &edgeId;
    }
    /// @}
};

/*!
 * Copy edge
 */
class AddrPE : public PAGEdge {
  private:
    AddrPE(const AddrPE &);         ///< place holder
    void operator=(const AddrPE &); ///< place holder
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const AddrPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::Addr;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Addr;
    }
    //@}

    /// constructor
    AddrPE(PAGNode *s, PAGNode *d, PAG *pag)
        : PAGEdge(s, d, pag, PAGEdge::Addr) {}
    AddrPE() = default;

    virtual ~AddrPE() {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
    }
    /// @}
};

/*!
 * Copy edge
 */
class CopyPE : public PAGEdge {
  private:
    CopyPE(const CopyPE &);         ///< place holder
    void operator=(const CopyPE &); ///< place holder
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const CopyPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::Copy;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Copy;
    }
    //@}

    /// constructor
    CopyPE(PAGNode *s, PAGNode *d, PAG *pag)
        : PAGEdge(s, d, pag, PAGEdge::Copy) {}
    CopyPE() = default;

    virtual ~CopyPE() {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
    }
    /// @}
};

/*!
 * Compare instruction edge
 */
class CmpPE : public PAGEdge {
  private:
    CmpPE(const CmpPE &);          ///< place holder
    void operator=(const CmpPE &); ///< place holder
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const CmpPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::Cmp;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Cmp;
    }
    //@}

    /// constructor
    CmpPE(PAGNode *s, PAGNode *d, PAG *pag)
        : PAGEdge(s, d, pag, PAGEdge::Cmp) {}
    CmpPE() = default;

    virtual ~CmpPE() {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
    }
    /// @}
};

/*!
 * Binary instruction edge
 */
class BinaryOPPE : public PAGEdge {
  private:
    BinaryOPPE(const BinaryOPPE &);     ///< place holder
    void operator=(const BinaryOPPE &); ///< place holder
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const BinaryOPPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::BinaryOp;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::BinaryOp;
    }
    //@}

    /// constructor
    BinaryOPPE(PAGNode *s, PAGNode *d, PAG *pag)
        : PAGEdge(s, d, pag, PAGEdge::BinaryOp) {}
    BinaryOPPE() = default;

    virtual ~BinaryOPPE() {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
    }
    /// @}
};

/*!
 * Unary instruction edge
 */
class UnaryOPPE : public PAGEdge {
  private:
    UnaryOPPE(const UnaryOPPE &);      ///< place holder
    void operator=(const UnaryOPPE &); ///< place holder
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const UnaryOPPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::UnaryOp;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::UnaryOp;
    }
    //@}

    /// constructor
    UnaryOPPE(PAGNode *s, PAGNode *d, PAG *pag)
        : PAGEdge(s, d, pag, PAGEdge::UnaryOp) {}
    UnaryOPPE() = default;

    virtual ~UnaryOPPE() {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
    }
    /// @}
};

/*!
 * Store edge
 */
class StorePE : public PAGEdge {
  private:
    StorePE(const StorePE &);        ///< place holder
    void operator=(const StorePE &); ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const StorePE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::Store;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Store;
    }
    //@}

    /// constructor
    StorePE(PAGNode *s, PAGNode *d, PAG *pag, const IntraBlockNode *st);
    StorePE() = default;

    virtual ~StorePE() {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
    }
    /// @}
};

/*!
 * Load edge
 */
class LoadPE : public PAGEdge {
  private:
    LoadPE(const LoadPE &);         ///< place holder
    void operator=(const LoadPE &); ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const LoadPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::Load;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Load;
    }
    //@}

    /// constructor
    LoadPE(PAGNode *s, PAGNode *d, PAG *pag)
        : PAGEdge(s, d, pag, PAGEdge::Load) {}
    LoadPE() = default;

    virtual ~LoadPE() {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
    }
    /// @}
};

/*!
 * Gep edge
 */
class GepPE : public PAGEdge {
  private:
    GepPE(const GepPE &);          ///< place holder
    void operator=(const GepPE &); ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GepPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::NormalGep ||
               edge->getEdgeKind() == PAGEdge::VariantGep;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::NormalGep ||
               edge->getEdgeKind() == PAGEdge::VariantGep;
    }
    //@}

  protected:
    /// constructor
    GepPE(PAGNode *s, PAGNode *d, PAG *pag, PEDGEK k) : PAGEdge(s, d, pag, k) {}
    GepPE() = default;
    virtual ~GepPE() {}
    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
    }
    /// @}
};

/*!
 * Gep edge with a fixed offset
 */
class NormalGepPE : public GepPE {
  private:
    NormalGepPE(const NormalGepPE &);    ///< place holder
    void operator=(const NormalGepPE &); ///< place holder

    LocationSet ls; ///< location set of the gep edge

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const NormalGepPE *) { return true; }
    static inline bool classof(const GepPE *edge) {
        return edge->getEdgeKind() == PAGEdge::NormalGep;
    }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::NormalGep;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::NormalGep;
    }
    //@}

    /// destructor
    virtual ~NormalGepPE() {}

    /// constructor
    NormalGepPE() = default;
    NormalGepPE(PAGNode *s, PAGNode *d, PAG *pag, const LocationSet &l)
        : GepPE(s, d, pag, PAGEdge::NormalGep), ls(l) {}

    /// offset of the gep edge
    inline u32_t getOffset() const { return ls.getOffset(); }
    inline const LocationSet &getLocationSet() const { return ls; }

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<GepPE>(*this);
        ar &ls;
    }
    /// @}
};

/*!
 * Gep edge with a variant offset
 */
class VariantGepPE : public GepPE {
  private:
    VariantGepPE(const VariantGepPE &);   ///< place holder
    void operator=(const VariantGepPE &); ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const VariantGepPE *) { return true; }
    static inline bool classof(const GepPE *edge) {
        return edge->getEdgeKind() == PAGEdge::VariantGep;
    }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::VariantGep;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::VariantGep;
    }
    //@}

    /// constructor
    VariantGepPE(PAGNode *s, PAGNode *d, PAG *pag)
        : GepPE(s, d, pag, PAGEdge::VariantGep) {}
    VariantGepPE() = default;
    virtual ~VariantGepPE() {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<GepPE>(*this);
    }
    /// @}
};

/*!
 * Call edge
 */
class CallPE : public PAGEdge {
  private:
    CallPE(const CallPE &);         ///< place holder
    void operator=(const CallPE &); ///< place holder

    const CallBlockNode *inst = nullptr; ///< llvm instruction for this call
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const CallPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::Call ||
               edge->getEdgeKind() == PAGEdge::ThreadFork;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Call ||
               edge->getEdgeKind() == PAGEdge::ThreadFork;
    }
    //@}

    /// constructor
    CallPE() = default;
    CallPE(PAGNode *s, PAGNode *d, PAG *pag, const CallBlockNode *i,
           GEdgeKind k = PAGEdge::Call);

    virtual ~CallPE() {}

    /// Get method for the call instruction
    //@{
    inline const CallBlockNode *getCallInst() const { return inst; }
    inline const CallBlockNode *getCallSite() const { return inst; }
    //@}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
        ar &inst;
    }
    /// @}
};

/*!
 * Return edge
 */
class RetPE : public PAGEdge {
  private:
    RetPE(const RetPE &);          ///< place holder
    void operator=(const RetPE &); ///< place holder

    const CallBlockNode *inst = nullptr; /// the callsite instruction return to
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const RetPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::Ret ||
               edge->getEdgeKind() == PAGEdge::ThreadJoin;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Ret ||
               edge->getEdgeKind() == PAGEdge::ThreadJoin;
    }
    //@}

    /// constructor
    RetPE() = default;
    RetPE(PAGNode *s, PAGNode *d, PAG *pag, const CallBlockNode *i,
          GEdgeKind k = PAGEdge::Ret);

    virtual ~RetPE() {}

    /// Get method for call instruction at caller
    //@{
    inline const CallBlockNode *getCallInst() const { return inst; }
    inline const CallBlockNode *getCallSite() const { return inst; }
    //@}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PAGEdge>(*this);
        ar &inst;
    }
    /// @}
};

/*!
 * Thread Fork Edge
 */
class TDForkPE : public CallPE {
  private:
    TDForkPE(const TDForkPE &);       ///< place holder
    void operator=(const TDForkPE &); ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const TDForkPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::ThreadFork;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::ThreadFork;
    }
    //@}

    /// destructor
    virtual ~TDForkPE() {}

    /// constructor
    TDForkPE() = default;
    TDForkPE(PAGNode *s, PAGNode *d, PAG *pag, const CallBlockNode *i)
        : CallPE(s, d, pag, i, PAGEdge::ThreadFork) {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<CallPE>(*this);
    }
    /// @}
};

/*!
 * Thread Join Edge
 */
class TDJoinPE : public RetPE {
  private:
    TDJoinPE(const TDJoinPE &);       ///< place holder
    void operator=(const TDJoinPE &); ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const TDJoinPE *) { return true; }
    static inline bool classof(const PAGEdge *edge) {
        return edge->getEdgeKind() == PAGEdge::ThreadJoin;
    }
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::ThreadJoin;
    }
    //@}

    /// destructor
    virtual ~TDJoinPE() {}
    /// Constructor
    TDJoinPE() = default;
    TDJoinPE(PAGNode *s, PAGNode *d, PAG *pag, const CallBlockNode *i)
        : RetPE(s, d, pag, i, PAGEdge::ThreadJoin) {}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<RetPE>(*this);
    }
    /// @}
};

} // End namespace SVF

#endif /* PAGEDGE_H_ */
