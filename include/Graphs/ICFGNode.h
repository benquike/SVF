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

using GenericICFGNodeTy = GenericNode<ICFGNode, ICFGEdge>;
using GenericICFGNode = GenericICFGNodeTy;

/*!
 * Interprocedural control-flow graph node, representing different kinds of
 * program statements including top-level pointers (ValPN) and address-taken
 * objects (ObjPN)
 */
class ICFGNode : public GenericICFGNode {

  public:
    /// 22 kinds of ICFG node
    /// Gep represents offset edge for field sensitivity
    enum ICFGNodeK {
        AbstractNode,
        IntraBlock,
        InterBlock,
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
    ICFGNode(NodeID id, ICFGNodeK k)
        : GenericICFGNode(id, k), fun(nullptr), bb(nullptr) {}

    ICFGNode(NodeID id, ICFGNodeK k, const SVFFunction *fun)
        : GenericICFGNode(id, k), fun(fun) {
        if (fun != nullptr &&
            fun->getLLVMFun()->begin() != fun->getLLVMFun()->end()) {
            bb = &(fun->getLLVMFun()->getEntryBlock());
        }
    }

    ICFGNode(NodeID id, ICFGNodeK k, const Instruction *inst, SVFModule *mod)
        : GenericICFGNode(id, k) {
        if (inst != nullptr) {
            fun = mod->getLLVMModSet()->getSVFFunction(inst->getFunction());
            bb = inst->getParent();
        }
    }

    ICFGNode() : GenericICFGNode(MAX_NODEID, AbstractNode) {}

    virtual ~ICFGNode(){};

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

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericICFGNodeTy *node);
    //@}

  protected:
    const SVFFunction *fun = nullptr;
    const BasicBlock *bb = nullptr;
    VFGNodeList VFGNodes; //< a list of VFGNodes
    PAGEdgeList pagEdges; //< a list of PAGEdges
};

/*!
 * Unique ICFG node stands for all global initializations
 */
class GlobalBlockNode : public ICFGNode {

  public:
    GlobalBlockNode(NodeID id) : ICFGNode(id, GlobalBlock) { bb = nullptr; }
    GlobalBlockNode() : GlobalBlockNode(MAX_NODEID){};
    // GlobalBlockNode() = default;

    virtual ~GlobalBlockNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == GlobalBlock;
    }
    //@}

    virtual const std::string toString() const;
};

/*!
 * ICFG node stands for a program statement
 */
class IntraBlockNode : public ICFGNode {
  private:
    const Instruction *inst = nullptr;

  public:
    IntraBlockNode(NodeID id, const Instruction *i, SVFModule *svfMod)
        : ICFGNode(id, IntraBlock, i, svfMod), inst(i) {}

    IntraBlockNode() : ICFGNode(MAX_NODEID, IntraBlock) {}

    virtual ~IntraBlockNode() {}

    inline const Instruction *getInst() const { return inst; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == IntraBlock;
    }
    //@}

    const std::string toString() const override;
};

/*!
 * Abstract ICFG node standing for instructions handling function
 * call and return
 */
class InterBlockNode : public ICFGNode {

  public:
    /// Constructor
    InterBlockNode(NodeID id, ICFGNodeK k, const SVFFunction *fun)
        : ICFGNode(id, k, fun) {}
    InterBlockNode(NodeID id, ICFGNodeK k, const Instruction *i, SVFModule *mod)
        : ICFGNode(id, k, i, mod) {}
    InterBlockNode() : ICFGNode(MAX_NODEID, InterBlock) {}

    virtual ~InterBlockNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericICFGNodeTy *node);
    //@}
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
    FunEntryBlockNode(NodeID id, const SVFFunction *f)
        : InterBlockNode(id, FunEntryBlock, f) {}
    FunEntryBlockNode() : InterBlockNode(MAX_NODEID, FunEntryBlock, nullptr) {}

    virtual ~FunEntryBlockNode() {}

    /// Return function
    inline const SVFFunction *getFun() const { return fun; }

    /// Return the set of formal parameters
    inline const FormalParmNodeVec &getFormalParms() const { return FPNodes; }

    /// Add formal parameters
    inline void addFormalParms(const PAGNode *fp) { FPNodes.push_back(fp); }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunEntryBlock;
    }
    //@}

    const virtual std::string toString() const;
};

/*!
 * Function exit ICFGNode containing
 * (at most one) FormalRetVFGNode of a function
 */
class FunExitBlockNode : public InterBlockNode {

  private:
    const PAGNode *formalRet = nullptr;

  public:
    FunExitBlockNode(NodeID id, const SVFFunction *f)
        : InterBlockNode(id, FunExitBlock, f) {}
    FunExitBlockNode() : InterBlockNode(MAX_NODEID, FunExitBlock, nullptr) {}

    virtual ~FunExitBlockNode() {}

    /// Return function
    inline const SVFFunction *getFun() const { return fun; }

    /// Return actual return parameter
    inline const PAGNode *getFormalRet() const { return formalRet; }

    /// Add actual return parameter
    inline void addFormalRet(const PAGNode *fr) { formalRet = fr; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunExitBlock;
    }
    //@}

    virtual const std::string toString() const;
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
        : InterBlockNode(id, FunCallBlock, c, svfMod), cs(c), ret(nullptr),
          svfMod(svfMod) {}

    CallBlockNode() : InterBlockNode(MAX_NODEID, FunCallBlock, nullptr) {}
    virtual ~CallBlockNode() {}

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
    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunCallBlock;
    }
    //@}

    virtual const std::string toString() const;
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
        : InterBlockNode(id, FunRetBlock, c, svfMod), cs(c), actualRet(nullptr),
          callBlockNode(cb) {}

    RetBlockNode() : InterBlockNode(MAX_NODEID, FunRetBlock, nullptr) {}

    virtual ~RetBlockNode() {}

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
    static inline bool classof(const GenericICFGNodeTy *node) {
        return node->getNodeKind() == FunRetBlock;
    }
    //@}

    virtual const std::string toString() const;
};

} // End namespace SVF

#endif /* ICFGNode_H_ */
