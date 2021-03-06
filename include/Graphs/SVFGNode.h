//===- SVFGNode.h -- Sparse value-flow graph
// node-------------------------------//
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
 * SVFGNode.h
 *
 *  Created on: 13 Sep. 2018
 *      Author: Yulei Sui
 */

#ifndef INCLUDE_MSSA_SVFGNODE_H_
#define INCLUDE_MSSA_SVFGNODE_H_

#include "Graphs/SVFGEdge.h"
#include "Graphs/VFGNode.h"
#include "Util/Serialization.h"

namespace SVF {

/*!
 * Memory region VFGNode (for address-taken objects)
 */
class MRSVFGNode : public VFGNode {
  protected:
    PointsTo cpts;

    // This constructor can only be used by derived classes
    MRSVFGNode(NodeID id, VFGNodeK k) : VFGNode(id, k) {}

  public:
    virtual ~MRSVFGNode() {}
    MRSVFGNode() : VFGNode(MAX_NODEID, MRS) {}

  public:
    /// Return points-to of the MR
    inline const PointsTo &getPointsTo() const { return cpts; }
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericVFGNodeTy *node);
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGNode>(*this);
        ar &cpts;
    }
    /// @}
};

/*
 * SVFG Node stands for entry chi node (address-taken variables)
 */
class FormalINSVFGNode : public MRSVFGNode {
  private:
    const MemSSA::ENTRYCHI *chi = nullptr;

  public:
    /// Constructor
    FormalINSVFGNode(NodeID id, const MemSSA::ENTRYCHI *entry)
        : MRSVFGNode(id, FPIN), chi(entry) {
        cpts = entry->getMR()->getPointsTo();
    }
    FormalINSVFGNode() : MRSVFGNode(MAX_NODEID, FPIN) {}

    virtual ~FormalINSVFGNode() {}

    /// EntryCHI
    inline const MemSSA::ENTRYCHI *getEntryChi() const { return chi; }
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == FPIN;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    // ignore chi
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<MRSVFGNode>(*this);

        // TODO: investigate serialization of chi
    }
    /// @}
};

/*
 * SVFG Node stands for return mu node (address-taken variables)
 */
class FormalOUTSVFGNode : public MRSVFGNode {
  private:
    const MemSSA::RETMU *mu = nullptr;

  public:
    /// Constructor
    FormalOUTSVFGNode(NodeID id, const MemSSA::RETMU *exit);
    FormalOUTSVFGNode() : MRSVFGNode(MAX_NODEID, FPOUT) {}
    virtual ~FormalOUTSVFGNode() {}

    /// RetMU
    inline const MemSSA::RETMU *getRetMU() const { return mu; }
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == FPOUT;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    // ignore mu
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<MRSVFGNode>(*this);
        // TODO: investigate mu
    }
    /// @}
};

/*
 * SVFG Node stands for callsite mu node (address-taken variables)
 */
class ActualINSVFGNode : public MRSVFGNode {
  private:
    const MemSSA::CALLMU *mu = nullptr;
    const CallBlockNode *cs = nullptr;

  public:
    /// Constructor
    ActualINSVFGNode(NodeID id, const MemSSA::CALLMU *m, const CallBlockNode *c)
        : MRSVFGNode(id, APIN), mu(m), cs(c) {
        cpts = m->getMR()->getPointsTo();
    }
    ActualINSVFGNode() : MRSVFGNode(MAX_NODEID, APIN) {}

    virtual ~ActualINSVFGNode() {}

    /// Callsite
    inline const CallBlockNode *getCallSite() const { return cs; }
    /// CallMU
    inline const MemSSA::CALLMU *getCallMU() const { return mu; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == APIN;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()
    // ignore mu
    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<MRSVFGNode>(*this);

        // TODO: investigate mu

        SAVE_ICFGNode(ar, cs);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<MRSVFGNode>(*this);

        // TODO: investigate mu

        LOAD_ICFGNode(ar, cs);
    }
    /// @}
};

/*
 * SVFG Node stands for callsite chi node (address-taken variables)
 */
class ActualOUTSVFGNode : public MRSVFGNode {
  private:
    const MemSSA::CALLCHI *chi = nullptr;
    const CallBlockNode *cs = nullptr;

  public:
    /// Constructor
    ActualOUTSVFGNode(NodeID id, const MemSSA::CALLCHI *c,
                      const CallBlockNode *cal)
        : MRSVFGNode(id, APOUT), chi(c), cs(cal) {
        cpts = c->getMR()->getPointsTo();
    }
    ActualOUTSVFGNode() : MRSVFGNode(MAX_NODEID, APOUT) {}

    virtual ~ActualOUTSVFGNode() {}

    /// Callsite
    inline const CallBlockNode *getCallSite() const { return cs; }
    /// CallCHI
    inline const MemSSA::CALLCHI *getCallCHI() const { return chi; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == APOUT;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    BOOST_SERIALIZATION_SPLIT_MEMBER()
    // ignore mu
    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<MRSVFGNode>(*this);

        // TODO: investigate mu

        SAVE_ICFGNode(ar, cs);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<MRSVFGNode>(*this);

        // TODO: investigate mu

        LOAD_ICFGNode(ar, cs);
    }
    /// @}
};

/*
 * SVFG Node stands for a memory ssa phi node or formalIn or ActualOut
 */
class MSSAPHISVFGNode : public MRSVFGNode {
  public:
    using OPVers = Map<u32_t, MRVerSPtr>;

  protected:
    const MemSSA::MDEF *res = nullptr;
    OPVers opVers;

  public:
    /// Constructor
    MSSAPHISVFGNode(NodeID id, const MemSSA::MDEF *def, VFGNodeK k = MPhi)
        : MRSVFGNode(id, k), res(def) {
        if (nullptr != def) {
            cpts = def->getMR()->getPointsTo();
        }
    }
    MSSAPHISVFGNode() : MRSVFGNode(MAX_NODEID, MPhi) {}

    virtual ~MSSAPHISVFGNode() {}

    /// MSSA phi operands
    //@{
    inline const MRVerSPtr getOpVer(u32_t pos) const {
        auto it = opVers.find(pos);
        assert(it != opVers.end() && "version is nullptr, did not rename?");
        return it->second;
    }
    inline void setOpVer(u32_t pos, const MRVerSPtr node) {
        opVers[pos] = node;
    }
    inline const MemSSA::MDEF *getRes() const { return res; }
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

    // ignore chi
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<MRSVFGNode>(*this);
        /// FIXME: look into the following fields
        /// ar &res;
        /// ar &opVers;
    }
    /// @}
};

/*
 * Intra MSSA PHI
 */
class IntraMSSAPHISVFGNode : public MSSAPHISVFGNode {

  public:
    /// Constructor
    IntraMSSAPHISVFGNode(NodeID id, const MemSSA::PHI *phi)
        : MSSAPHISVFGNode(id, phi, MIntraPhi) {}
    IntraMSSAPHISVFGNode() : MSSAPHISVFGNode(MAX_NODEID, nullptr, MIntraPhi) {}

    virtual ~IntraMSSAPHISVFGNode() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == MIntraPhi;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    // ignore chi
    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<MSSAPHISVFGNode>(*this);
    }
    /// @}
};

/*
 * Inter MSSA PHI (formalIN/ActualOUT)
 */
class InterMSSAPHISVFGNode : public MSSAPHISVFGNode {

  public:
    /// Constructor interPHI for formal parameter
    InterMSSAPHISVFGNode(NodeID id, const FormalINSVFGNode *fi)
        : MSSAPHISVFGNode(id, fi->getEntryChi(), MInterPhi), fun(fi->getFun()),
          callInst(nullptr) {}
    /// Constructor interPHI for actual return
    InterMSSAPHISVFGNode(NodeID id, const ActualOUTSVFGNode *ao)
        : MSSAPHISVFGNode(id, ao->getCallCHI(), MInterPhi), fun(nullptr),
          callInst(ao->getCallSite()) {}

    InterMSSAPHISVFGNode() : MSSAPHISVFGNode(MAX_NODEID, nullptr, MInterPhi) {}

    virtual ~InterMSSAPHISVFGNode() {}

    inline bool isFormalINPHI() const {
        return (fun != nullptr) && (callInst == nullptr);
    }

    inline bool isActualOUTPHI() const {
        return (fun == nullptr) && (callInst != nullptr);
    }

    inline const SVFFunction *getFun() const override {
        assert(isFormalINPHI() && "expect a formal parameter phi");
        return fun;
    }

    inline const CallBlockNode *getCallSite() const {
        assert(isActualOUTPHI() && "expect a actual return phi");
        return callInst;
    }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGNodeTy *node) {
        return node->getNodeKind() == MInterPhi;
    }
    //@}

    const std::string toString() const override;

  private:
    const SVFFunction *fun = nullptr;
    const CallBlockNode *callInst = nullptr;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;
    BOOST_SERIALIZATION_SPLIT_MEMBER()

    template <typename Archive>
    void save(Archive &ar, const unsigned int version) const {
        ar &boost::serialization::base_object<MSSAPHISVFGNode>(*this);
        SAVE_SVFFunction(ar, fun);
        SAVE_ICFGNode(ar, callInst);
    }

    template <typename Archive>
    void load(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<MSSAPHISVFGNode>(*this);
        LOAD_SVFFunction(ar, fun);
        LOAD_ICFGNode(ar, callInst);
    }
    /// @}
};

} // End namespace SVF

#endif /* INCLUDE_MSSA_SVFGNODE_H_ */
