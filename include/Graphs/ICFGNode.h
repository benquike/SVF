//===- ICFGNode.h -- ICFG node------------------------------------------------//
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
 * ICFGNode.h
 *
 *  Created on: Sep 11, 2018
 *      Author: Yulei
 */

#ifndef ICFGNODE_H_
#define ICFGNODE_H_

#include "Graphs/GenericGraph.h"
#include "Graphs/ICFGEdge.h"
#include "Util/SVFUtil.h"
#include "Util/Serialization.h"

namespace SVF {

class ICFGNode;
class RetBlockNode;
class CallPE;
class RetPE;
class PAGEdge;
class PAGNode;
class VFGNode;

/*!
 * Interprocedural control-flow graph node, representing different kinds of
 * program statements including top-level pointers (ValPN) and address-taken
 * objects (ObjPN)
 */
using GenericICFGNodeTy = GenericNode<ICFGNode, ICFGEdge>;

class ICFGNode : public GenericICFGNodeTy {

  public:
    /// 22 kinds of ICFG node
    /// Gep represents offset edge for field sensitivity
    enum ICFGNodeK {
        IntraBlock,
        FunEntryBlock,
        FunExitBlock,
        FunCallBlock,
        FunRetBlock,
        GlobalBlock
    };

    using iterator = ICFGEdge::ICFGEdgeSetTy::iterator;
    using const_iterator = ICFGEdge::ICFGEdgeSetTy::const_iterator;
    using CallPESet = Set<const CallPE *>;
    using RetPESet = Set<const RetPE *>;
    using VFGNodeList = std::list<const VFGNode *>;
    using PAGEdgeList = std::list<const PAGEdge *>;

  public:
    /// Constructor
    ICFGNode(NodeID i, ICFGNodeK k)
        : GenericICFGNodeTy(i, k), fun(nullptr), bb(nullptr) {}

    ICFGNode() = default;

    /// Return the function of this ICFGNode
    virtual const SVFFunction *getFun() const { return fun; }

    /// Return the function of this ICFGNode
    virtual const BasicBlock *getBB() const { return bb; }

    /// Overloading operator << for dumping ICFG node ID
    //@{
    friend raw_ostream &operator<<(raw_ostream &o, const ICFGNode &node) {
        o << node.toString();
        return o;
    }
    //@}

    /// Set/Get methods of VFGNodes
    ///@{
    inline void addVFGNode(const VFGNode *vfgNode) {
        VFGNodes.push_back(vfgNode);
    }

    inline const VFGNodeList &getVFGNodes() const { return VFGNodes; }
    ///@}

    /// Set/Get methods of VFGNodes
    ///@{
    inline void addPAGEdge(const PAGEdge *edge) { pagEdges.push_back(edge); }

    inline const PAGEdgeList &getPAGEdges() const { return pagEdges; }
    ///@}

    virtual const std::string toString() const;

    void dump() const;

  protected:
    const SVFFunction *fun = nullptr;
    const BasicBlock *bb = nullptr;
    VFGNodeList VFGNodes; //< a list of VFGNodes
    PAGEdgeList pagEdges; //< a list of PAGEdges

  private:
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        // save parent object
        ar &boost::serialization::base_object<GenericICFGNodeTy>(*this);

        if (fun != nullptr) {
            auto *llvm_fun = fun->getLLVMFun();
            auto fun_id = getIdByValueFromCurrentProject(llvm_fun);
            ar &fun_id;
        } else {
            ar &numeric_limits<SymID>::max();
        }

        if (bb != nullptr) {
            auto bb_id = getIdByValueFromCurrentProject(bb);
            ar &bb_id;
        } else {
            ar &numeric_limits<SymID>::max();
        }

        ar &VFGNodes;
        ar &pagEdges;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        // save parent object
        ar &boost::serialization::base_object<GenericICFGNodeTy>(*this);

        SVFProject *currProject = SVFProject::getCurrentProject();
        assert(currProject != nullptr && "current project is null");
        SymID fun_id;
        SymID bb_id;

        ar &fun_id;
        ar &bb_id;

        if (fun_id < numeric_limits<SymID>::max()) {
            SVFModule *mod = currProject->getSVFModule();
            auto *fun_val = getValueByIdFromCurrentProject(fun_id);
            assert(llvm::isa<Function>(fun_val) && "fun_id not a Function");
            fun = mod->getSVFFunction(llvm::dyn_cast<Function>(fun_val));
        }

        if (bb_id < numeric_limits<SymID>::max()) {
            auto *bb_val = getValueByIdFromCurrentProject(bb_id);
            assert(llvm::isa<BasicBlock>(bb_val) && "bb_id not a BasicBlock");
            bb = llvm::dyn_cast<BasicBlock>(bb_val);
        }

        ar &VFGNodes;
        ar &pagEdges;
    }
};

/*!
 * Unique ICFG node stands for all global initializations
 */
class GlobalBlockNode : public ICFGNode {

  public:
    GlobalBlockNode(NodeID id) : ICFGNode(id, GlobalBlock) { bb = nullptr; }
    GlobalBlockNode() = default;
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GlobalBlockNode *) { return true; }

    static inline bool classof(const ICFGNode *node) {
        return node->getNodeKind() == GlobalBlock;
    }

    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == GlobalBlock;
    }
    //@}

    virtual const std::string toString() const;

  private:
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<ICFGNode>(*this);
    }
};

/*!
 * ICFG node stands for a program statement
 */
class IntraBlockNode : public ICFGNode {
  private:
    const Instruction *inst = nullptr;

  public:
    IntraBlockNode(NodeID id, const Instruction *i, SVFModule *svfMod)
        : ICFGNode(id, IntraBlock), inst(i) {
        fun = svfMod->getLLVMModSet()->getSVFFunction(inst->getFunction());
        bb = inst->getParent();
    }

    IntraBlockNode() = default;

    inline const Instruction *getInst() const { return inst; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const IntraBlockNode *) { return true; }

    static inline bool classof(const ICFGNode *node) {
        return node->getNodeKind() == IntraBlock;
    }

    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == IntraBlock;
    }
    //@}

    const std::string toString() const;

  private:
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<ICFGNode>(*this);
        auto inst_id = getIdByValueFromCurrentProject(inst);
        ar &inst_id;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<ICFGNode>(*this);

        SymID inst_id;
        ar &inst_id;

        auto *inst_val = getValueByIdFromCurrentProject(inst_id);
        assert(llvm::isa<Instruction>(inst_val) &&
               "inst_id not an Instruction");
        inst = llvm::dyn_cast<Instruction>(inst_val);
    }
};

class InterBlockNode : public ICFGNode {

  public:
    /// Constructor
    InterBlockNode(NodeID id, ICFGNodeK k) : ICFGNode(id, k) {}
    InterBlockNode() = default;

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const InterBlockNode *) { return true; }

    static inline bool classof(const ICFGNode *node) {
        return node->getNodeKind() == FunEntryBlock ||
               node->getNodeKind() == FunExitBlock ||
               node->getNodeKind() == FunCallBlock ||
               node->getNodeKind() == FunRetBlock;
    }

    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunEntryBlock ||
               node->getNodeKind() == FunExitBlock ||
               node->getNodeKind() == FunCallBlock ||
               node->getNodeKind() == FunRetBlock;
    }
    //@}

  private:
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<ICFGNode>(*this);
    }
};

/*!
 * Function entry ICFGNode containing a set of
 * FormalParmVFGNodes of a function
 */
class FunEntryBlockNode : public InterBlockNode {

  public:
    using FormalParmNodeVec = std::vector<const PAGNode *>;

  private:
    FormalParmNodeVec FPNodes;

  public:
    FunEntryBlockNode(NodeID id, const SVFFunction *f);
    FunEntryBlockNode() = default;

    /// Return function
    inline const SVFFunction *getFun() const { return fun; }

    /// Return the set of formal parameters
    inline const FormalParmNodeVec &getFormalParms() const { return FPNodes; }

    /// Add formal parameters
    inline void addFormalParms(const PAGNode *fp) { FPNodes.push_back(fp); }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const FunEntryBlockNode *) { return true; }

    static inline bool classof(const InterBlockNode *node) {
        return node->getNodeKind() == FunEntryBlock;
    }

    static inline bool classof(const ICFGNode *node) {
        return node->getNodeKind() == FunEntryBlock;
    }

    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunEntryBlock;
    }
    //@}

    const virtual std::string toString() const;

  private:
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<InterBlockNode>(*this);
        ar &FPNodes;
    }
};

/*!
 * Function exit ICFGNode containing
 * (at most one) FormalRetVFGNode of a function
 */
class FunExitBlockNode : public InterBlockNode {

  private:
    const SVFFunction *fun = nullptr;
    const PAGNode *formalRet = nullptr;

  public:
    FunExitBlockNode(NodeID id, const SVFFunction *f);
    FunExitBlockNode() = default;

    /// Return function
    inline const SVFFunction *getFun() const { return fun; }

    /// Return actual return parameter
    inline const PAGNode *getFormalRet() const { return formalRet; }

    /// Add actual return parameter
    inline void addFormalRet(const PAGNode *fr) { formalRet = fr; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const FunEntryBlockNode *) { return true; }

    static inline bool classof(const ICFGNode *node) {
        return node->getNodeKind() == FunExitBlock;
    }

    static inline bool classof(const InterBlockNode *node) {
        return node->getNodeKind() == FunExitBlock;
    }

    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunExitBlock;
    }
    //@}

    virtual const std::string toString() const;

  private:
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<InterBlockNode>(*this);
        auto *llvm_fun = fun->getLLVMFun();
        auto fun_id = getIdByValueFromCurrentProject(llvm_fun);
        ar &fun_id;
        ar &formalRet;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<InterBlockNode>(*this);

        SymID fun_id;
        ar &fun_id;
        const Value *fun_val = getValueByIdFromCurrentProject(fun_id);
        assert(llvm::isa<Function>(fun_val) && "not a function");

        SVFProject *currProject = SVFProject::getCurrentProject();
        SVFModule *mod = currProject->getSVFModule();
        fun = mod->getSVFFunction(llvm::dyn_cast<Function>(fun_val));

        ar &formalRet;
    }
};

/*!
 * Call ICFGNode containing a set of ActualParmVFGNodes at a callsite
 */
class CallBlockNode : public InterBlockNode {

  public:
    using ActualParmVFGNodeVec = std::vector<const PAGNode *>;

  private:
    const Instruction *cs = nullptr;
    const RetBlockNode *ret = nullptr;
    const SVFModule *svfMod = nullptr;
    ActualParmVFGNodeVec APNodes;

  public:
    CallBlockNode(NodeID id, const Instruction *c, SVFModule *svfMod)
        : InterBlockNode(id, FunCallBlock), cs(c), ret(nullptr),
          svfMod(svfMod) {
        fun = svfMod->getLLVMModSet()->getSVFFunction(cs->getFunction());
        bb = cs->getParent();
    }

    CallBlockNode() = default;
    /// Return callsite
    inline const Instruction *getCallSite() const { return cs; }

    /// Return callsite
    inline const RetBlockNode *getRetBlockNode() const {
        assert(ret && "RetBlockNode not set?");
        return ret;
    }

    /// Return callsite
    inline void setRetBlockNode(const RetBlockNode *r) { ret = r; }

    /// Return callsite
    inline const SVFFunction *getCaller() const {
        return svfMod->getLLVMModSet()->getSVFFunction(cs->getFunction());
    }

    /// Return Basic Block
    inline const BasicBlock *getParent() const { return cs->getParent(); }

    /// Return true if this is an indirect call
    inline bool isIndirectCall() const {
        return nullptr == SVFUtil::getCallee(svfMod->getLLVMModSet(), cs);
    }

    /// Return the set of actual parameters
    inline const ActualParmVFGNodeVec &getActualParms() const {
        return APNodes;
    }

    /// Add actual parameters
    inline void addActualParms(const PAGNode *ap) { APNodes.push_back(ap); }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const CallBlockNode *) { return true; }

    static inline bool classof(const ICFGNode *node) {
        return node->getNodeKind() == FunCallBlock;
    }

    static inline bool classof(const InterBlockNode *node) {
        return node->getNodeKind() == FunCallBlock;
    }

    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunCallBlock;
    }
    //@}

    virtual const std::string toString() const;

  private:
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<InterBlockNode>(*this);

        auto csId = getIdByValueFromCurrentProject(cs);
        ar &csId;
        ar &ret;
        ar &APNodes;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<InterBlockNode>(*this);

        SymID csId;
        ar &csId;
        const auto *cs_val = getValueByIdFromCurrentProject(csId);
        assert(llvm::isa<Instruction>(cs_val) && "Not an Instruction");
        cs = llvm::dyn_cast<Instruction>(cs_val);

        ar &ret;
        ar &APNodes;

        svfMod = SVFProject::getCurrentProject()->getSVFModule();
    }
};

/*!
 * Return ICFGNode containing (at most one) ActualRetVFGNode at a callsite
 */
class RetBlockNode : public InterBlockNode {

  private:
    const Instruction *cs = nullptr;
    const PAGNode *actualRet = nullptr;
    const CallBlockNode *callBlockNode = nullptr;

  public:
    RetBlockNode(NodeID id, const Instruction *c, CallBlockNode *cb,
                 SVFModule *svfMod)
        : InterBlockNode(id, FunRetBlock), cs(c), actualRet(nullptr),
          callBlockNode(cb) {
        fun = svfMod->getLLVMModSet()->getSVFFunction(cs->getFunction());
        bb = cs->getParent();
    }

    RetBlockNode() = default;
    /// Return callsite
    inline const Instruction *getCallSite() const { return cs; }

    inline const CallBlockNode *getCallBlockNode() const {
        return callBlockNode;
    }
    /// Return actual return parameter
    inline const PAGNode *getActualRet() const { return actualRet; }

    /// Add actual return parameter
    inline void addActualRet(const PAGNode *ar) { actualRet = ar; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const RetBlockNode *) { return true; }

    static inline bool classof(const InterBlockNode *node) {
        return node->getNodeKind() == FunRetBlock;
    }

    static inline bool classof(const ICFGNode *node) {
        return node->getNodeKind() == FunRetBlock;
    }

    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunRetBlock;
    }
    //@}

    virtual const std::string toString() const;

  private:
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<InterBlockNode>(*this);

        auto csId = getIdByValueFromCurrentProject(cs);
        ar &csId;

        ar &actualRet;
        ar &callBlockNode;
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<InterBlockNode>(*this);

        SymID csId;
        ar &csId;

        const auto *cs_val = getValueByIdFromCurrentProject(csId);
        assert(llvm::isa<Instruction>(cs_val) && "Not an Instruction");
        cs = llvm::dyn_cast<Instruction>(cs_val);

        ar &actualRet;
        ar &callBlockNode;
    }
};

} // End namespace SVF

#endif /* ICFGNode_H_ */
