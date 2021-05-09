//===- ConsGEdge.h -- Constraint graph edge-----------------------------------//
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
 * ConsGEdge.h
 *
 *  Created on: Mar 19, 2014
 *      Author: Yulei Sui
 */

#ifndef CONSGEDGE_H_
#define CONSGEDGE_H_

#include "Graphs/PAG.h"
#include "Util/WorkList.h"

#include <map>
#include <set>

namespace SVF {

class ConstraintNode;
/*!
 * Self-defined edge for constraint resolution
 * including add/remove/re-target, but all the operations do not affect original
 * PAG Edges
 */
using GenericConsEdgeTy = GenericEdge<ConstraintNode>;
class ConstraintEdge : public GenericConsEdgeTy {

  public:
    /// five kinds of constraint graph edges
    /// Gep edge is used for field sensitivity
    enum ConstraintEdgeK {
        AbstractEdge,
        Addr,
        Copy,
        Store,
        Load,
        Gep,
        NormalGep,
        VariantGep
    };

  private:
    EdgeID edgeId;

  public:
    /// Constructor
    ConstraintEdge(ConstraintNode *s, ConstraintNode *d, ConstraintEdgeK k,
                   EdgeID id = 0)
        : GenericConsEdgeTy(s, d, id, k), edgeId(id) {}

    ConstraintEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(AbstractEdge);
    }

    /// Destructor
    virtual ~ConstraintEdge() {}
    /// Return edge ID
    inline EdgeID getEdgeID() const { return edgeId; }

    /// Overloading operator << for dumping ICFG node ID
    //@{
    friend raw_ostream &operator<<(raw_ostream &o, const ConstraintEdge &edge) {
        o << edge.toString();
        return o;
    }
    //@}

    virtual const std::string toString() const;

    /// ClassOf
    static inline bool classof(const GenericConsEdgeTy *edge);
    /// Constraint edge type
    using ConstraintEdgeSetTy =
        GenericNode<ConstraintNode, ConstraintEdge>::GEdgeSetTy;
};

/*!
 * Copy edge
 */
class AddrCGEdge : public ConstraintEdge {
  private:
    AddrCGEdge(const AddrCGEdge &);     ///< place holder
    void operator=(const AddrCGEdge &); ///< place holder
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericConsEdgeTy *edge) {
        return edge->getEdgeKind() == Addr;
    }
    //@}

    /// constructor
    AddrCGEdge(ConstraintNode *s, ConstraintNode *d, EdgeID id, PAG *pag);
    AddrCGEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(Addr);
    }

    virtual ~AddrCGEdge() {}
};

/*!
 * Copy edge
 */
class CopyCGEdge : public ConstraintEdge {
  private:
    CopyCGEdge(const CopyCGEdge &);     ///< place holder
    void operator=(const CopyCGEdge &); ///< place holder
  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericConsEdgeTy *edge) {
        return edge->getEdgeKind() == Copy;
    }
    //@}

    /// constructor
    CopyCGEdge(ConstraintNode *s, ConstraintNode *d, EdgeID id)
        : ConstraintEdge(s, d, Copy, id) {}
    CopyCGEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(Copy);
    }
    virtual ~CopyCGEdge() {}
};

/*!
 * Store edge
 */
class StoreCGEdge : public ConstraintEdge {
  private:
    StoreCGEdge(const StoreCGEdge &);    ///< place holder
    void operator=(const StoreCGEdge &); ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericConsEdgeTy *edge) {
        return edge->getEdgeKind() == Store;
    }
    //@}

    /// constructor
    StoreCGEdge(ConstraintNode *s, ConstraintNode *d, EdgeID id)
        : ConstraintEdge(s, d, Store, id) {}
    StoreCGEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(Store);
    }
    virtual ~StoreCGEdge() {}
};

/*!
 * Load edge
 */
class LoadCGEdge : public ConstraintEdge {
  private:
    LoadCGEdge(const LoadCGEdge &);     ///< place holder
    void operator=(const LoadCGEdge &); ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericConsEdgeTy *edge) {
        return edge->getEdgeKind() == Load;
    }
    //@}

    /// Constructor
    LoadCGEdge(ConstraintNode *s, ConstraintNode *d, EdgeID id)
        : ConstraintEdge(s, d, Load, id) {}
    LoadCGEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(Load);
    }
    virtual ~LoadCGEdge() {}
};

/*!
 * Gep edge
 */
class GepCGEdge : public ConstraintEdge {
  private:
    GepCGEdge(const GepCGEdge &);      ///< place holder
    void operator=(const GepCGEdge &); ///< place holder

  protected:
    /// Constructor
    GepCGEdge(ConstraintNode *s, ConstraintNode *d, ConstraintEdgeK k,
              EdgeID id)
        : ConstraintEdge(s, d, k, id) {}

  public:
    GepCGEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(Gep);
    }

    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static bool classof(const GenericConsEdgeTy *edge);
    //@}

    virtual ~GepCGEdge() {}
};

/*!
 * Gep edge with fixed offset size
 */
class NormalGepCGEdge : public GepCGEdge {
  private:
    NormalGepCGEdge(const NormalGepCGEdge &); ///< place holder
    void operator=(const NormalGepCGEdge &);  ///< place holder

    LocationSet ls; ///< location set of the gep edge

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericConsEdgeTy *edge) {
        return edge->getEdgeKind() == NormalGep;
    }
    //@}

    /// Constructor
    NormalGepCGEdge(ConstraintNode *s, ConstraintNode *d, const LocationSet &l,
                    EdgeID id)
        : GepCGEdge(s, d, NormalGep, id), ls(l) {}
    NormalGepCGEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(NormalGep);
    }

    virtual ~NormalGepCGEdge() {}

    /// Get location set of the gep edge
    inline const LocationSet &getLocationSet() const { return ls; }

    /// Get location set of the gep edge
    inline Size_t getOffset() const { return ls.getOffset(); }
};

/*!
 * Gep edge with variant offset size
 */
class VariantGepCGEdge : public GepCGEdge {
  private:
    VariantGepCGEdge(const VariantGepCGEdge &); ///< place holder
    void operator=(const VariantGepCGEdge &);   ///< place holder

  public:
    /// Methods for support type inquiry through isa, cast, and dyn_cast:
    //@{
    static inline bool classof(const GenericConsEdgeTy *edge) {
        return edge->getEdgeKind() == VariantGep;
    }
    //@}

    /// Constructor
    VariantGepCGEdge(ConstraintNode *s, ConstraintNode *d, EdgeID id)
        : GepCGEdge(s, d, VariantGep, id) {}
    VariantGepCGEdge() {
        setId(MAX_EDGEID);
        setEdgeFlag(VariantGep);
    }
    virtual ~VariantGepCGEdge() {}
};

} // End namespace SVF

#endif /* CONSGEDGE_H_ */
