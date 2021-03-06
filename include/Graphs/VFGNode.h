//===- VFGNode.h
//----------------------------------------------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2018>  <Yulei Sui>
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
 * VFGNode.h
 *
 *  Created on: 18 Sep. 2018
 *      Author: Yulei Sui
 *  Updated by:
 *     Hui Peng <peng124@purdue.edu>
 *     2021-04-15
 */

#ifndef INCLUDE_UTIL_VFGNODE_H_
#define INCLUDE_UTIL_VFGNODE_H_

#include "Graphs/GenericGraph.h"
#include "Graphs/VFGEdge.h"
#include "MemoryModel/PointerAnalysisImpl.h"
#include "Util/Serialization.h"

namespace SVF {

class VFGNode;
class ICFGNode;

using GenericVFGNodeTy = GenericNode<VFGNode, VFGEdge>;
using GenericVFGNode = GenericVFGNodeTy;

/*!
 * Interprocedural control-flow graph node, representing different kinds of
 * program statements including top-level pointers (ValPN) and address-taken
 * objects (ObjPN)
 */
class VFGNode : public GenericVFGNode {

  public:
    /// 24 kinds of ICFG node
    /// Gep represents offset edge for field sensitivity
    enum VFGNodeK {
        Vfg,  // abstract VFGNode
        Stmt, // abstract statement VFG node
        Addr,
        Copy,
        Gep,
        Store,
        Load,
        Cmp,
        BinaryOp,
        UnaryOp,
        TPhi,
        TIntraPhi,
        TInterPhi,
        MPhi,
        MIntraPhi,
        MInterPhi,
        FRet,
        ARet,
        Argument, // abstract argument VFG node
        AParm,
        FParm,
        FunRet,
        MRS, // abstract MRSVFGNode
        APIN,
        APOUT,
        FPIN,
        FPOUT,
        NPtr
    };

    using iterator = VFGEdge::VFGEdgeSetTy::iterator;
    using const_iterator = VFGEdge::VFGEdgeSetTy::const_iterator;
    using CallPESet = Set<const CallPE *>;
    using RetPESet = Set<const RetPE *>;

  public:
    /// Constructor
    VFGNode(NodeID i, VFGNodeK k) : GenericVFGNodeTy(i, k), icfgNode(nullptr) {}
    VFGNode() : GenericVFGNodeTy(MAX_NODEID, Vfg) {}

    virtual ~VFGNode() {}

    /// Return corresponding ICFG node
    virtual const ICFGNode *getICFGNode() const { return icfgNode; }

    /// Set corresponding ICFG node
    virtual void setICFGNode(const ICFGNode *node) { icfgNode = node; }

    /// Get the function of this SVFGNode
    virtual const SVFFunction *getFun() const { return icfgNode->getFun(); }

    /// Overloading operator << for dumping ICFG node ID
    //@{
    friend raw_ostream &operator<<(raw_ostream &o, const VFGNode &node) {
        o << node.toString();
        return o;
    }
    //@}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericVFGNodeTy *node);
    //@}

    virtual const std::string toString() const;

  protected:
    const ICFGNode *icfgNode = nullptr;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<GenericVFGNodeTy>(*this);
        SAVE_ICFGNode(ar, icfgNode);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<GenericVFGNodeTy>(*this);
        LOAD_ICFGNode(ar, icfgNode);
    }
    /// @}
};

/*!
 * ICFG node stands for a program statement
 */
class StmtVFGNode : public VFGNode {

  private:
    const PAGEdge *pagEdge = nullptr;

  public:
    /// Constructor
    StmtVFGNode(NodeID id, const PAGEdge *e, VFGNodeK k)
        : VFGNode(id, k), pagEdge(e) {}
    StmtVFGNode() : VFGNode(MAX_NODEID, Stmt) {}

    virtual ~StmtVFGNode() {}

    /// Whether this node is used for pointer analysis. Both src and dst
    /// PAGNodes are of ptr type.
    inline bool isPTANode() const { return pagEdge->isPTAEdge(); }

    /// PAGNode and PAGEdge
    ///@{
    inline const PAGEdge *getPAGEdge() const { return pagEdge; }

    inline NodeID getPAGSrcNodeID() const { return pagEdge->getSrcID(); }

    inline NodeID getPAGDstNodeID() const { return pagEdge->getDstID(); }

    inline PAGNode *getPAGSrcNode() const { return pagEdge->getSrcNode(); }

    inline PAGNode *getPAGDstNode() const { return pagEdge->getDstNode(); }
    //@}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericVFGNodeTy *node);
    //@}

    inline const Instruction *getInst() const {
        /// should return a valid instruction unless it is a global PAGEdge
        return pagEdge->getInst();
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFGNode>(*this);
        SAVE_PAGEdge(ar, pagEdge);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        LOAD_PAGEdge(ar, pagEdge);
    }

    /// @}
};

/*!
 * VFGNode for loads
 */
class LoadVFGNode : public StmtVFGNode {
  private:
    LoadVFGNode(const LoadVFGNode &);    ///< place holder
    void operator=(const LoadVFGNode &); ///< place holder

  public:
    /// Constructor
    LoadVFGNode(NodeID id, const LoadPE *edge) : StmtVFGNode(id, edge, Load) {}
    LoadVFGNode() : StmtVFGNode(MAX_NODEID, nullptr, Load) {}

    virtual ~LoadVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == Load;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<StmtVFGNode>(*this);
    }
    /// @}
};

/*!
 * VFGNode for stores
 */
class StoreVFGNode : public StmtVFGNode {
  private:
    StoreVFGNode(const StoreVFGNode &);   ///< place holder
    void operator=(const StoreVFGNode &); ///< place holder

  public:
    /// Constructor
    StoreVFGNode(NodeID id, const StorePE *edge)
        : StmtVFGNode(id, edge, Store) {}
    StoreVFGNode() : StmtVFGNode(MAX_NODEID, nullptr, Store) {}

    virtual ~StoreVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == Store;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<StmtVFGNode>(*this);
    }
    /// @}
};

/*!
 * VFGNode for copies
 */
class CopyVFGNode : public StmtVFGNode {
  private:
    CopyVFGNode(const CopyVFGNode &);    ///< place holder
    void operator=(const CopyVFGNode &); ///< place holder

  public:
    /// Constructor
    CopyVFGNode(NodeID id, const CopyPE *copy) : StmtVFGNode(id, copy, Copy) {}
    CopyVFGNode() : StmtVFGNode(MAX_NODEID, nullptr, Copy) {}

    virtual ~CopyVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == Copy;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<StmtVFGNode>(*this);
    }
    /// @}
};

/*!
 * VFGNode for compare instruction, e.g., bool b = (a!=c);
 */

class CmpVFGNode : public VFGNode {
  public:
    using OPVers = Map<u32_t, const PAGNode *>;

  protected:
    const PAGNode *res = nullptr;
    OPVers opVers;

  private:
    CmpVFGNode(const CmpVFGNode &);     ///< place holder
    void operator=(const CmpVFGNode &); ///< place holder

  public:
    /// Constructor
    CmpVFGNode(NodeID id, const PAGNode *r) : VFGNode(id, Cmp), res(r) {
        const auto *cmp = llvm::dyn_cast<CmpInst>(r->getValue());
        assert(cmp && "not a binary operator?");
    }
    CmpVFGNode() : VFGNode(MAX_NODEID, Cmp) {}

    virtual ~CmpVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == Cmp;
    }
    //@}
    /// Operands at a BinaryNode
    //@{
    inline const PAGNode *getOpVer(u32_t pos) const {
        auto it = opVers.find(pos);
        assert(it != opVers.end() && "version is nullptr, did not rename?");
        return it->second;
    }
    inline void setOpVer(u32_t pos, const PAGNode *node) { opVers[pos] = node; }
    inline const PAGNode *getRes() const { return res; }
    inline u32_t getOpVerNum() const { return opVers.size(); }
    inline OPVers::const_iterator opVerBegin() const { return opVers.begin(); }
    inline OPVers::const_iterator opVerEnd() const { return opVers.end(); }
    //@}
    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFGNode>(*this);
        SAVE_PAGNode(ar, res);

        /// TODO: confirm this works or not
        /// ar &opVers;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        // ar &res;
        LOAD_PAGNode(ar, res);
        /// TODO: confirm this works or not
        /// ar &opVers;
    }

    /// @}
};

/*!
 * VFGNode for binary operator instructions, e.g., a = b + c;
 */
class BinaryOPVFGNode : public VFGNode {
  public:
    using OPVers = Map<u32_t, const PAGNode *>;

  protected:
    const PAGNode *res = nullptr;
    OPVers opVers;

  private:
    BinaryOPVFGNode(const BinaryOPVFGNode &); ///< place holder
    void operator=(const BinaryOPVFGNode &);  ///< place holder

  public:
    /// Constructor
    BinaryOPVFGNode(NodeID id, const PAGNode *r)
        : VFGNode(id, BinaryOp), res(r) {
        const auto *binary = llvm::dyn_cast<BinaryOperator>(r->getValue());
        assert(binary && "not a binary operator?");
    }
    BinaryOPVFGNode() : VFGNode(MAX_NODEID, BinaryOp) {}
    virtual ~BinaryOPVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == BinaryOp;
    }
    //@}
    /// Operands at a BinaryNode
    //@{
    inline const PAGNode *getOpVer(u32_t pos) const {
        auto it = opVers.find(pos);
        assert(it != opVers.end() && "version is nullptr, did not rename?");
        return it->second;
    }
    inline void setOpVer(u32_t pos, const PAGNode *node) { opVers[pos] = node; }
    inline const PAGNode *getRes() const { return res; }
    inline u32_t getOpVerNum() const { return opVers.size(); }
    inline OPVers::const_iterator opVerBegin() const { return opVers.begin(); }
    inline OPVers::const_iterator opVerEnd() const { return opVers.end(); }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        SAVE_PAGNode(ar, res);
        // ar &opVers;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        LOAD_PAGNode(ar, res);
        // ar &opVers;
    }
    /// @}
};

/*!
 * VFGNode for unary operator instructions, e.g., a = -b;
 */
class UnaryOPVFGNode : public VFGNode {
  public:
    using OPVers = Map<u32_t, const PAGNode *>;

  protected:
    const PAGNode *res = nullptr;
    OPVers opVers;

  private:
    UnaryOPVFGNode(const UnaryOPVFGNode &); ///< place holder
    void operator=(const UnaryOPVFGNode &); ///< place holder

  public:
    /// Constructor
    UnaryOPVFGNode(NodeID id, const PAGNode *r) : VFGNode(id, UnaryOp), res(r) {
        const Value *val = r->getValue();
        bool unop = (llvm::isa<UnaryOperator>(val) ||
                     llvm::isa<BranchInst>(val) || llvm::isa<SwitchInst>(val));
        assert(unop && "not a unary operator or a BranchInst or a SwitchInst?");
    }
    UnaryOPVFGNode() : VFGNode(MAX_NODEID, UnaryOp) {}

    virtual ~UnaryOPVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == UnaryOp;
    }
    //@}
    /// Operands at a UnaryNode
    //@{
    inline const PAGNode *getOpVer(u32_t pos) const {
        auto it = opVers.find(pos);
        assert(it != opVers.end() && "version is nullptr, did not rename?");
        return it->second;
    }
    inline void setOpVer(u32_t pos, const PAGNode *node) { opVers[pos] = node; }
    inline const PAGNode *getRes() const { return res; }
    inline u32_t getOpVerNum() const { return opVers.size(); }
    inline OPVers::const_iterator opVerBegin() const { return opVers.begin(); }
    inline OPVers::const_iterator opVerEnd() const { return opVers.end(); }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        SAVE_PAGNode(ar, res);
        // ar &opVers;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        LOAD_PAGNode(ar, res);
        // ar &opVers;
    }
    /// @}
};

/*!
 * VFGNode for Gep
 */
class GepVFGNode : public StmtVFGNode {
  private:
    GepVFGNode(const GepVFGNode &);     ///< place holder
    void operator=(const GepVFGNode &); ///< place holder

  public:
    /// Constructor
    GepVFGNode(NodeID id, const GepPE *edge) : StmtVFGNode(id, edge, Gep) {}
    GepVFGNode() : StmtVFGNode(MAX_NODEID, nullptr, Gep) {}
    virtual ~GepVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == Gep;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<StmtVFGNode>(*this);
    }
    /// @}
};

/*
 * ICFG Node stands for a top level pointer ssa phi node or a formal parameter
 * or a return parameter
 */
class PHIVFGNode : public VFGNode {

  public:
    using OPVers = Map<u32_t, const PAGNode *>;

  protected:
    const PAGNode *res = nullptr;
    OPVers opVers;

  public:
    /// Constructor
    PHIVFGNode(NodeID id, const PAGNode *r, VFGNodeK k = TPhi);
    PHIVFGNode() : VFGNode(MAX_NODEID, TPhi) {}

    virtual ~PHIVFGNode() {}

    /// Whether this phi node is of pointer type (used for pointer analysis).
    inline bool isPTANode() const { return res->isPointer(); }

    /// Operands at a llvm PHINode
    //@{
    inline const PAGNode *getOpVer(u32_t pos) const {
        auto it = opVers.find(pos);
        assert(it != opVers.end() && "version is nullptr, did not rename?");
        return it->second;
    }
    inline void setOpVer(u32_t pos, const PAGNode *node) { opVers[pos] = node; }
    inline const PAGNode *getRes() const { return res; }
    inline u32_t getOpVerNum() const { return opVers.size(); }
    inline OPVers::const_iterator opVerBegin() const { return opVers.begin(); }
    inline OPVers::const_iterator opVerEnd() const { return opVers.end(); }
    //@}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericVFGNodeTy *node);
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        SAVE_PAGNode(ar, res);
        // ar &opVers;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        LOAD_PAGNode(ar, res);
        // ar &opVers;
    }
    /// @}
};

/*
 * Intra LLVM PHI Node
 */
class IntraPHIVFGNode : public PHIVFGNode {

  public:
    using OPIncomingBBs = Map<u32_t, const ICFGNode *>;

  private:
    OPIncomingBBs opIncomingBBs;

  public:
    /// Constructor
    IntraPHIVFGNode(NodeID id, const PAGNode *r)
        : PHIVFGNode(id, r, TIntraPhi) {}
    IntraPHIVFGNode() : PHIVFGNode(MAX_NODEID, nullptr, TIntraPhi) {}
    virtual ~IntraPHIVFGNode() {}

    inline const ICFGNode *getOpIncomingBB(u32_t pos) const {
        auto it = opIncomingBBs.find(pos);
        assert(it != opIncomingBBs.end() &&
               "version is nullptr, did not rename?");
        return it->second;
    }
    inline void setOpVerAndBB(u32_t pos, const PAGNode *node,
                              const ICFGNode *bb) {
        opVers[pos] = node;
        opIncomingBBs[pos] = bb;
    }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == TIntraPhi;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PHIVFGNode>(*this);

        /// TODO: check whether this is needed
        /// ar &opIncomingBBs;
    }
    /// @}
};

class AddrVFGNode : public StmtVFGNode {
  private:
    AddrVFGNode(const AddrVFGNode &);    ///< place holder
    void operator=(const AddrVFGNode &); ///< place holder

  public:
    /// Constructor
    AddrVFGNode(NodeID id, const AddrPE *edge) : StmtVFGNode(id, edge, Addr) {}
    AddrVFGNode() : StmtVFGNode(MAX_NODEID, nullptr, Addr) {}
    virtual ~AddrVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == Addr;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<StmtVFGNode>(*this);
    }
    /// @}
};

class ArgumentVFGNode : public VFGNode {

  protected:
    const PAGNode *param = nullptr;

  public:
    /// Constructor
    ArgumentVFGNode(NodeID id, const PAGNode *p, VFGNodeK k)
        : VFGNode(id, k), param(p) {}
    ArgumentVFGNode() : VFGNode(MAX_NODEID, Argument) {}
    virtual ~ArgumentVFGNode() {}

    /// Whether this argument node is of pointer type (used for pointer
    /// analysis).
    inline bool isPTANode() const { return param->isPointer(); }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericVFGNodeTy *node);
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        SAVE_PAGNode(ar, param);
        // ar &opVers;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        LOAD_PAGNode(ar, param);
        // ar &opVers;
    }

    // template <typename Archive>
    // void serialize(Archive &ar, const unsigned int version) {
    //     ar &boost::serialization::base_object<VFGNode>(*this);
    //     ar &param;
    // }
    /// @}
};

/*
 * ICFG Node stands for acutal parameter node (top level pointers)
 */
class ActualParmVFGNode : public ArgumentVFGNode {
  private:
    const CallBlockNode *cs = nullptr;

  public:
    /// Constructor
    ActualParmVFGNode(NodeID id, const PAGNode *n, const CallBlockNode *c)
        : ArgumentVFGNode(id, n, AParm), cs(c) {}
    ActualParmVFGNode() : ArgumentVFGNode(MAX_NODEID, nullptr, AParm) {}
    virtual ~ActualParmVFGNode() {}

    /// Return callsite
    inline const CallBlockNode *getCallSite() const { return cs; }

    /// Return parameter
    inline const PAGNode *getParam() const { return param; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == AParm;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        SAVE_ICFGNode(ar, cs);
        // ar &opVers;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        /// ar &res;
        LOAD_ICFGNode(ar, cs);
        // ar &opVers;
    }

    // template <typename Archive>
    // void serialize(Archive &ar, const unsigned int version) {
    //     ar &boost::serialization::base_object<ArgumentVFGNode>(*this);
    //     ar &cs;
    // }
    /// @}
};

/*
 * ICFG Node stands for formal parameter node (top level pointers)
 */
class FormalParmVFGNode : public ArgumentVFGNode {
  private:
    const SVFFunction *fun = nullptr;
    CallPESet callPEs;

  public:
    /// Constructor
    FormalParmVFGNode(NodeID id, const PAGNode *n, const SVFFunction *f)
        : ArgumentVFGNode(id, n, FParm), fun(f) {}
    FormalParmVFGNode() : ArgumentVFGNode(MAX_NODEID, nullptr, FParm) {}

    virtual ~FormalParmVFGNode() {}

    /// Return parameter
    inline const PAGNode *getParam() const { return param; }

    /// Return function
    inline const SVFFunction *getFun() const override { return fun; }
    /// Return call edge
    inline void addCallPE(const CallPE *call) { callPEs.insert(call); }
    /// Call edge iterator
    ///@{
    inline CallPESet::const_iterator callPEBegin() const {
        return callPEs.begin();
    }
    inline CallPESet::const_iterator callPEEnd() const { return callPEs.end(); }
    //@}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == FParm;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<ArgumentVFGNode>(*this);
        SAVE_SVFFunction(ar, fun);
        /// TODO: check whether this is needed
        /// ar &callPEs;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<ArgumentVFGNode>(*this);
        LOAD_SVFFunction(ar, fun);
        /// ar &callPEs;
    }
    /// @}
};

/*!
 * Callsite receive paramter
 */
class ActualRetVFGNode : public ArgumentVFGNode {
  private:
    const CallBlockNode *cs = nullptr;

    ActualRetVFGNode(const ActualRetVFGNode &); ///< place holder
    void operator=(const ActualRetVFGNode &);   ///< place holder

  public:
    /// Constructor
    ActualRetVFGNode(NodeID id, const PAGNode *n, const CallBlockNode *c)
        : ArgumentVFGNode(id, n, ARet), cs(c) {}
    ActualRetVFGNode() : ArgumentVFGNode(MAX_NODEID, nullptr, ARet) {}

    virtual ~ActualRetVFGNode() {}

    /// Return callsite
    inline const CallBlockNode *getCallSite() const { return cs; }
    /// Receive parameter at callsite
    inline const SVFFunction *getCaller() const { return cs->getCaller(); }
    /// Receive parameter at callsite
    inline const PAGNode *getRev() const { return param; }
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == ARet;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<ArgumentVFGNode>(*this);
        // ar &cs;
        SAVE_ICFGNode(ar, cs);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<ArgumentVFGNode>(*this);
        LOAD_ICFGNode(ar, cs);
    }
    /// @}
};

/*!
 * Callee return ICFG node
 */
class FormalRetVFGNode : public ArgumentVFGNode {
  private:
    const SVFFunction *fun = nullptr;
    RetPESet retPEs;

    FormalRetVFGNode(const FormalRetVFGNode &); ///< place holder
    void operator=(const FormalRetVFGNode &);   ///< place holder

  public:
    /// Constructor
    FormalRetVFGNode(NodeID id, const PAGNode *n, const SVFFunction *f);
    FormalRetVFGNode() : ArgumentVFGNode(MAX_NODEID, nullptr, FRet) {}
    virtual ~FormalRetVFGNode() {}

    /// Return value at callee
    inline const PAGNode *getRet() const { return param; }
    /// Function
    inline const SVFFunction *getFun() const override { return fun; }
    /// RetPE
    inline void addRetPE(const RetPE *retPE) { retPEs.insert(retPE); }
    /// RetPE iterators
    inline RetPESet::const_iterator retPEBegin() const {
        return retPEs.begin();
    }
    inline RetPESet::const_iterator retPEEnd() const { return retPEs.end(); }
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == FRet;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<ArgumentVFGNode>(*this);
        SAVE_SVFFunction(ar, fun);
        // ar &retPEs;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<ArgumentVFGNode>(*this);
        LOAD_SVFFunction(ar, fun);
        // ar &retPEs;
    }
    /// @}
};

/*
 * Inter LLVM PHI node (formal parameter)
 */
class InterPHIVFGNode : public PHIVFGNode {

  public:
    /// Constructor interPHI for formal parameter
    InterPHIVFGNode(NodeID id, const FormalParmVFGNode *fp)
        : PHIVFGNode(id, fp->getParam(), TInterPhi), fun(fp->getFun()),
          callInst(nullptr) {}
    /// Constructor interPHI for actual return
    InterPHIVFGNode(NodeID id, const ActualRetVFGNode *ar)
        : PHIVFGNode(id, ar->getRev(), TInterPhi), fun(ar->getCaller()),
          callInst(ar->getCallSite()) {}

    InterPHIVFGNode() : PHIVFGNode(MAX_NODEID, nullptr, TInterPhi) {}

    virtual ~InterPHIVFGNode() {}

    inline bool isFormalParmPHI() const {
        return (fun != nullptr) && (callInst == nullptr);
    }

    inline bool isActualRetPHI() const {
        return (fun != nullptr) && (callInst != nullptr);
    }

    inline const SVFFunction *getFun() const override {
        assert((isFormalParmPHI() || isActualRetPHI()) &&
               "expect a formal parameter phi");
        return fun;
    }

    inline const CallBlockNode *getCallSite() const {
        assert(isActualRetPHI() && "expect a actual return phi");
        return callInst;
    }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == TInterPhi;
    }
    //@}

    const std::string toString() const override;

  private:
    const SVFFunction *fun = nullptr;
    const CallBlockNode *callInst = nullptr;

    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<PHIVFGNode>(*this);
        SAVE_SVFFunction(ar, fun);
        SAVE_ICFGNode(ar, callInst);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<PHIVFGNode>(*this);
        LOAD_SVFFunction(ar, fun);
        // ar &callInst;
        LOAD_ICFGNode(ar, callInst);
    }
    /// @}
};

/*!
 * Dummy Definition for undef and null pointers
 */
class NullPtrVFGNode : public VFGNode {
  private:
    const PAGNode *node = nullptr;

  public:
    /// Constructor
    NullPtrVFGNode(NodeID id, const PAGNode *n) : VFGNode(id, NPtr), node(n) {}
    NullPtrVFGNode() : VFGNode(MAX_NODEID, NPtr) {}

    virtual ~NullPtrVFGNode() {}

    /// Whether this node is of pointer type (used for pointer analysis).
    inline bool isPTANode() const { return node->isPointer(); }
    /// Return corresponding PAGNode
    const PAGNode *getPAGNode() const { return node; }
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == NPtr;
    }
    //@}

    const std::string toString() const override;

    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<VFGNode>(*this);
        SAVE_PAGNode(ar, node);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        LOAD_PAGNode(ar, node);
    }

    /// @}
};

} // End namespace SVF

#endif /* INCLUDE_UTIL_VFGNODE_H_ */
