//===- SVFGEdge.h -- Sparse value-flow graph
// edge-------------------------------//
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
 * SVFGEdge.h
 *
 *  Created on: 13 Sep. 2018
 *      Author: Yulei Sui
 */

#ifndef INCLUDE_MSSA_SVFGEDGE_H_
#define INCLUDE_MSSA_SVFGEDGE_H_

#include "Graphs/VFGEdge.h"
#include "MSSA/MemSSA.h"
#include "Util/Serialization.h"

namespace SVF {

/*!
 * SVFG edge representing indirect value-flows from a caller to its callee at a
 * callsite
 */
class IndirectSVFGEdge : public VFGEdge {

  public:
    using MRVerSet = Set<const MRVer *>;

  private:
    MRVerSet mrs;
    PointsTo cpts;

  public:
    /// Constructor
    IndirectSVFGEdge(VFGNode *s, VFGNode *d, EdgeID id, GEdgeFlag k)
        : VFGEdge(s, d, id, k) {}
    IndirectSVFGEdge() : VFGEdge(nullptr, nullptr, MAX_EDGEID, IndirectVF) {}

    virtual ~IndirectSVFGEdge() {}

    /// Handle memory region
    //@{
    inline bool addPointsTo(const PointsTo &c) { return (cpts |= c); }
    inline const PointsTo &getPointsTo() const { return cpts; }

    inline MRVerSet &getMRVer() { return mrs; }
    inline bool addMrVer(const MRVer *mr) {
        // collect memory regions' pts to edge;
        cpts |= mr->getMR()->getPointsTo();
        return mrs.insert(mr).second;
    }
    //@}

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
        /// ar &mrs;
        ar &cpts;
    }
    /// @}
};

/*!
 * Intra SVFG edge representing indirect intra-procedural value-flows
 */
class IntraIndSVFGEdge : public IndirectSVFGEdge {

  public:
    IntraIndSVFGEdge(VFGNode *s, VFGNode *d, EdgeID id)
        : IndirectSVFGEdge(s, d, id, IntraIndirectVF) {}
    IntraIndSVFGEdge()
        : IndirectSVFGEdge(nullptr, nullptr, MAX_EDGEID, IntraIndirectVF) {}

    virtual ~IntraIndSVFGEdge() {}

    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static inline bool classof(const GenericVFGEdgeTy *edge) {
        return edge->getEdgeKind() == IntraIndirectVF;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<IndirectSVFGEdge>(*this);
    }
    /// @}
};

/*!
 * SVFG call edge representing indirect value-flows from a caller to its callee
 * at a callsite
 */
class CallIndSVFGEdge : public IndirectSVFGEdge {

  private:
    CallSiteID csId;

  public:
    CallIndSVFGEdge(VFGNode *s, VFGNode *d, EdgeID eid, CallSiteID id)
        : IndirectSVFGEdge(s, d, eid, makeEdgeFlagWithAuxInfo(CallIndVF, id)),
          csId(id) {}
    CallIndSVFGEdge()
        : IndirectSVFGEdge(nullptr, nullptr, MAX_EDGEID,
                           makeEdgeFlagWithAuxInfo(CallIndVF, MAX_CSID)) {}

    virtual ~CallIndSVFGEdge() {}

    inline CallSiteID getCallSiteId() const { return csId; }
    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static inline bool classof(const GenericVFGEdgeTy *edge) {
        return edge->getEdgeKind() == CallIndVF;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<IndirectSVFGEdge>(*this);
        ar &csId;
    }
    /// @}
};

/*!
 * SVFG return edge representing direct value-flows from a callee to its caller
 * at a callsite
 */
class RetIndSVFGEdge : public IndirectSVFGEdge {

  private:
    CallSiteID csId;

  public:
    RetIndSVFGEdge(VFGNode *s, VFGNode *d, EdgeID eid, CallSiteID id)
        : IndirectSVFGEdge(s, d, eid, makeEdgeFlagWithAuxInfo(RetIndVF, id)),
          csId(id) {}
    RetIndSVFGEdge()
        : IndirectSVFGEdge(nullptr, nullptr, MAX_EDGEID,
                           makeEdgeFlagWithAuxInfo(RetIndVF, MAX_CSID)){};

    virtual ~RetIndSVFGEdge() {}

    inline CallSiteID getCallSiteId() const { return csId; }
    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static inline bool classof(const GenericVFGEdgeTy *edge) {
        return edge->getEdgeKind() == RetIndVF;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<IndirectSVFGEdge>(*this);
        ar &csId;
    }
    /// @}
};

/*!
 * MHP SVFG edge representing indirect value-flows between
 * two memory access may-happen-in-parallel in multithreaded program
 */
class ThreadMHPIndSVFGEdge : public IndirectSVFGEdge {

  public:
    ThreadMHPIndSVFGEdge(VFGNode *s, VFGNode *d, EdgeID id)
        : IndirectSVFGEdge(s, d, id, TheadMHPIndirectVF) {}
    ThreadMHPIndSVFGEdge()
        : IndirectSVFGEdge(nullptr, nullptr, MAX_EDGEID, TheadMHPIndirectVF) {}

    virtual ~ThreadMHPIndSVFGEdge() {}

    //@{ Methods for support type inquiry through isa, cast, and dyn_cast:
    static inline bool classof(const GenericVFGEdgeTy *edge) {
        return edge->getEdgeKind() == TheadMHPIndirectVF;
    }
    //@}

    const std::string toString() const override;

  private:
    /// support for serialization
    /// @{
    friend class boost::serialization::access;

    template <typename Archive>
    void serialize(Archive &ar, const unsigned int version) {
        ar &boost::serialization::base_object<IndirectSVFGEdge>(*this);
    }
    /// @}
};

} // End namespace SVF

#endif /* INCLUDE_MSSA_SVFGEDGE_H_ */
