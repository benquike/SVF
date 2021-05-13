//===- VFGEdge.h
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
 * VFGEdge.h
 *
 *  Created on: 18 Sep. 2018
 *      Author: Yulei Sui
 */

#ifndef INCLUDE_UTIL_VFGEDGE_H_
#define INCLUDE_UTIL_VFGEDGE_H_

#include "Graphs/GenericGraph.h"
#include "Util/Serialization.h"

namespace SVF {

class VFGNode;

#define MAX_CSID (numeric_limits<CallSiteID>::max())

using GenericVFGEdgeTy = GenericEdge<VFGNode>;
using GenericVFGEdge = GenericVFGEdgeTy;

/*!
 * Interprocedural control-flow and value-flow edge, representing the control-
 * and value-flow dependence between two nodes
 */
class VFGEdge : public GenericVFGEdge {

  public:
    /// seven types of VFG edge
    /// four types of direct value-flow edges:
    /// - CallDirVF
    /// - RetDirVF
    /// - IntraDirectVF
    /// three types of indirect value-flow edges
    /// - CallIndVF
    /// - RetIndVF
    /// - ThreadMHPIndirectVF
    /// FIXME: append 'E' to all the macro names
    /// to indicate 'edge'
    enum VFGEdgeK {
        VF,
        IntraDirectVF,
        IntraIndirectVF,
        DirectVF,
        CallDirVF,
        RetDirVF,
        IndirectVF,
        CallIndVF,
        RetIndVF,
        TheadMHPIndirectVF
    };

    using SVFGEdgeK = VFGEdgeK;

  public:
    /// Constructor
    VFGEdge(VFGNode *s, VFGNode *d, EdgeID id, GEdgeFlag k)
        : GenericVFGEdgeTy(s, d, id, k) {}
    VFGEdge() : VFGEdge(nullptr, nullptr, MAX_EDGEID, VF) {}

    /// Destructor
    virtual ~VFGEdge() {}

    /// Get methods of the components
    //@{
    inline bool isDirectVFGEdge() const {
        return getEdgeKind() == IntraDirectVF || getEdgeKind() == CallDirVF ||
               getEdgeKind() == RetDirVF;
    }
    inline bool isIndirectVFGEdge() const {
        return getEdgeKind() == IntraIndirectVF || getEdgeKind() == CallIndVF ||
               getEdgeKind() == RetIndVF || getEdgeKind() == TheadMHPIndirectVF;
    }
    inline bool isCallVFGEdge() const {
        return getEdgeKind() == CallDirVF || getEdgeKind() == CallIndVF;
    }
    inline bool isRetVFGEdge() const {
        return getEdgeKind() == RetDirVF || getEdgeKind() == RetIndVF;
    }
    inline bool isCallDirectVFGEdge() const {
        return getEdgeKind() == CallDirVF;
    }
    inline bool isRetDirectVFGEdge() const { return getEdgeKind() == RetDirVF; }
    inline bool isCallIndirectVFGEdge() const {
        return getEdgeKind() == CallIndVF;
    }
    inline bool isRetIndirectVFGEdge() const {
        return getEdgeKind() == RetIndVF;
    }
    inline bool isIntraVFGEdge() const {
        return getEdgeKind() == IntraDirectVF ||
               getEdgeKind() == IntraIndirectVF;
    }
    inline bool isThreadMHPIndirectVFGEdge() const {
        return getEdgeKind() == TheadMHPIndirectVF;
    }
    //@}
    using VFGEdgeSetTy = GenericNode<VFGNode, VFGEdge>::GEdgeSetTy;
    using SVFGEdgeSetTy = VFGEdgeSetTy;

    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static bool classof(const GenericVFGEdgeTy *edge);
    //@}

    /// Overloading operator << for dumping ICFG node ID
    //@{
    friend raw_ostream &operator<<(raw_ostream &o, const VFGEdge &edge) {
        o << edge.toString();
        return o;
    }
    //@}

    virtual const std::string toString() const;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<GenericVFGEdgeTy>(*this);
    }
    /// @}
};

/*!
 * SVFG edge representing direct value-flows
 */
class DirectSVFGEdge : public VFGEdge {

  public:
    /// Constructor
    DirectSVFGEdge(VFGNode *s, VFGNode *d, EdgeID id, GEdgeFlag k)
        : VFGEdge(s, d, id, k) {}
    DirectSVFGEdge() : VFGEdge(nullptr, nullptr, MAX_EDGEID, DirectVF) {}

    virtual ~DirectSVFGEdge() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericVFGEdgeTy *edge);
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<VFGEdge>(*this);
    }
    /// @}
};

/*!
 * Intra SVFG edge representing direct intra-procedural value-flows
 */
class IntraDirSVFGEdge : public DirectSVFGEdge {

  public:
    /// Constructor
    IntraDirSVFGEdge(VFGNode *s, VFGNode *d, EdgeID id)
        : DirectSVFGEdge(s, d, id, IntraDirectVF) {}
    IntraDirSVFGEdge()
        : DirectSVFGEdge(nullptr, nullptr, MAX_EDGEID, IntraDirectVF) {}

    virtual ~IntraDirSVFGEdge() {}

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGEdgeTy *edge) {
        return edge->getEdgeKind() == IntraDirectVF;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<DirectSVFGEdge>(*this);
    }
    /// @}
};

/*!
 * SVFG call edge representing direct value-flows from a caller to its callee at
 * a callsite
 */
class CallDirSVFGEdge : public DirectSVFGEdge {

  private:
    CallSiteID csId;

  public:
    /// Constructor
    CallDirSVFGEdge(VFGNode *s, VFGNode *d, EdgeID eid, CallSiteID id)
        : DirectSVFGEdge(s, d, eid, makeEdgeFlagWithAuxInfo(CallDirVF, id)),
          csId(id) {}
    CallDirSVFGEdge()
        : DirectSVFGEdge(nullptr, nullptr, MAX_EDGEID,
                         makeEdgeFlagWithAuxInfo(CallDirVF, MAX_CSID)) {}

    virtual ~CallDirSVFGEdge() {}

    /// Return callsite ID
    inline CallSiteID getCallSiteId() const { return csId; }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGEdgeTy *edge) {
        return edge->getEdgeKind() == CallDirVF;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<DirectSVFGEdge>(*this);
        ar &csId;
    }
    /// @}
};

/*!
 * SVFG return edge connecting direct value-flows from a callee to its caller at
 * a callsite
 */
class RetDirSVFGEdge : public DirectSVFGEdge {

  private:
    CallSiteID csId;

  public:
    /// Constructor
    RetDirSVFGEdge(VFGNode *s, VFGNode *d, EdgeID eid, CallSiteID id)
        : DirectSVFGEdge(s, d, eid, makeEdgeFlagWithAuxInfo(RetDirVF, id)),
          csId(id) {}
    RetDirSVFGEdge()
        : DirectSVFGEdge(nullptr, nullptr, MAX_EDGEID,
                         makeEdgeFlagWithAuxInfo(RetDirVF, MAX_CSID)) {}

    virtual ~RetDirSVFGEdge() {}

    /// Return callsite ID
    inline CallSiteID getCallSiteId() const { return csId; }
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericVFGEdgeTy *edge) {
        return edge->getEdgeKind() == RetDirVF;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<DirectSVFGEdge>(*this);
        ar &csId;
    }
    /// @}
};

} // End namespace SVF

#endif /* INCLUDE_UTIL_VFGEDGE_H_ */
