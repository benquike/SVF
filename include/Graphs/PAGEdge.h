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

using GenericPAGEdgeTy = GenericEdge<PAGNode>;
using GenericPAGEdge = GenericPAGEdgeTy;

/*!
 * PAG edge between nodes
 */
class PAGEdge : public GenericPAGEdge {

  public:
    /// Thirteen kinds of PAG edges
    /// Gep represents offset edge for field sensitivity
    /// ThreadFork/ThreadJoin is to model parameter passings between thread
    /// spawners and spawnees.
    enum PEDGEK {
        PagEdge,
        Addr,
        Copy,
        Store,
        Load,
        Call,
        Ret,
        Gep,
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
  public:
    /// Constructor
    PAGEdge(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag, GEdgeFlag k);
    PAGEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(PagEdge);
    }

    /// Destructor
    virtual ~PAGEdge() {}

    /// ClassOf
    //@{
    static bool classof(const GenericPAGEdgeTy *edge);
    ///@}

    /// Whether src and dst nodes are both of pointer type
    bool isPTAEdge() const;

    /// Get/set methods for llvm instruction
    //@{
    inline const Instruction *getInst() const {
        return llvm::dyn_cast<Instruction>(value);
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
};

/*!
 * Addr Edge
 */
class AddrPE : public PAGEdge {
  private:
    AddrPE(const AddrPE &);         ///< place holder
    void operator=(const AddrPE &); ///< place holder
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Addr;
    }
    //@}

    /// constructor
    AddrPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag)
        : PAGEdge(s, d, id, pag, PAGEdge::Addr) {}
    AddrPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(Addr);
    }

    virtual ~AddrPE() {}

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Copy;
    }
    //@}

    /// constructor
    CopyPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag)
        : PAGEdge(s, d, id, pag, PAGEdge::Copy) {}
    CopyPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(Copy);
    }

    virtual ~CopyPE() {}

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Cmp;
    }
    //@}

    /// constructor
    CmpPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag)
        : PAGEdge(s, d, id, pag, PAGEdge::Cmp) {}
    CmpPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(Cmp);
    }

    virtual ~CmpPE() {}

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::BinaryOp;
    }
    //@}

    /// constructor
    BinaryOPPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag)
        : PAGEdge(s, d, id, pag, PAGEdge::BinaryOp) {}
    BinaryOPPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(BinaryOp);
    }

    virtual ~BinaryOPPE() {}

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::UnaryOp;
    }
    //@}

    /// constructor
    UnaryOPPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag)
        : PAGEdge(s, d, id, pag, PAGEdge::UnaryOp) {}
    UnaryOPPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(UnaryOp);
    }

    virtual ~UnaryOPPE() {}

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Store;
    }
    //@}

    /// constructor
    StorePE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag,
            const IntraBlockNode *st);
    StorePE() {
        setId(MAX_EDGEID);
        // only save the Store, ignore other parts
        setEdgeFlag(Store);
    }

    virtual ~StorePE() {}

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::Load;
    }
    //@}

    /// constructor
    LoadPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag)
        : PAGEdge(s, d, id, pag, PAGEdge::Load) {}
    LoadPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(Load);
    }

    virtual ~LoadPE() {}

    virtual const std::string toString() const;
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
    static bool classof(const GenericPAGEdgeTy *edge);
    //@}

    GepPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(Gep);
    }
    virtual ~GepPE() {}

  protected:
    /// constructor
    GepPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag, PEDGEK k)
        : PAGEdge(s, d, id, pag, k) {}
    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::NormalGep;
    }
    //@}

    /// destructor
    virtual ~NormalGepPE() {}

    /// constructor
    NormalGepPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag,
                const LocationSet &l)
        : GepPE(s, d, id, pag, PAGEdge::NormalGep), ls(l) {}
    NormalGepPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(NormalGep);
    }

    /// offset of the gep edge
    inline u32_t getOffset() const { return ls.getOffset(); }
    inline const LocationSet &getLocationSet() const { return ls; }

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::VariantGep;
    }
    //@}

    /// constructor
    VariantGepPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag)
        : GepPE(s, d, id, pag, PAGEdge::VariantGep) {}
    VariantGepPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(VariantGep);
    }

    virtual ~VariantGepPE() {}

    virtual const std::string toString() const;
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
    static bool classof(const GenericPAGEdgeTy *edge);
    //@}

    /// constructor
    CallPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag, const CallBlockNode *i,
           GEdgeKind k = PAGEdge::Call);
    CallPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(Call);
    }

    virtual ~CallPE() {}

    /// Get method for the call instruction
    //@{
    inline const CallBlockNode *getCallInst() const { return inst; }
    inline const CallBlockNode *getCallSite() const { return inst; }
    //@}

    virtual const std::string toString() const;
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
    static bool classof(const GenericPAGEdgeTy *edge);
    //@}

    /// constructor
    RetPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag, const CallBlockNode *i,
          GEdgeKind k = PAGEdge::Ret);
    RetPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(Ret);
    }

    virtual ~RetPE() {}

    /// Get method for call instruction at caller
    //@{
    inline const CallBlockNode *getCallInst() const { return inst; }
    inline const CallBlockNode *getCallSite() const { return inst; }
    //@}

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::ThreadFork;
    }
    //@}

    /// destructor
    virtual ~TDForkPE() {}

    /// constructor
    TDForkPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag,
             const CallBlockNode *i)
        : CallPE(s, d, id, pag, i, PAGEdge::ThreadFork) {}

    TDForkPE() {
        setId(MAX_EDGEID);
        setEdgeFlag(ThreadFork);
    }

    virtual const std::string toString() const;
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
    static inline bool classof(const GenericPAGEdgeTy *edge) {
        return edge->getEdgeKind() == PAGEdge::ThreadJoin;
    }
    //@}

    /// destructor
    virtual ~TDJoinPE() {}
    /// Constructor
    TDJoinPE() = default;
    TDJoinPE(PAGNode *s, PAGNode *d, EdgeID id, PAG *pag,
             const CallBlockNode *i)
        : RetPE(s, d, id, pag, i, PAGEdge::ThreadJoin) {}

    virtual const std::string toString() const;
};

} // End namespace SVF

#endif /* PAGEDGE_H_ */
