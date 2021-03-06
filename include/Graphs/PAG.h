//===- PAG.h -- Program assignment graph--------------------------------------//
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
 * PAG.h
 *
 *  Created on: Nov 1, 2013
 *      Author: Yulei Sui
 */

#ifndef PAG_H_
#define PAG_H_

#include "Graphs/ICFG.h"
#include "PAGEdge.h"
#include "PAGNode.h"
#include "SVF-FE/SVFProject.h"
#include "Util/NodeIDAllocator.h"
#include "Util/SVFUtil.h"

namespace SVF {

using GenericPAGTy = GenericGraph<PAGNode, PAGEdge>;
using GenericPAG = GenericPAGTy;

/*!
 * Program Assignment Graph for pointer analysis
 * SymID and NodeID are equal here (same numbering).
 */
class PAG : public GenericPAG {

  public:
    using CallSiteSet = Set<const CallBlockNode *>;
    using CallSiteToFunPtrMap = OrderedMap<const CallBlockNode *, NodeID>;
    using FunPtrToCallSitesMap = Map<NodeID, CallSiteSet>;
    using MemObjToFieldsMap = Map<NodeID, NodeBS>;
    using PAGEdgeSet = Set<const PAGEdge *>;
    using PAGEdgeList = std::vector<const PAGEdge *>;
    using PAGNodeList = std::vector<const PAGNode *>;
    using CopyPEList = std::vector<const CopyPE *>;
    using BinaryOPList = std::vector<const BinaryOPPE *>;
    using UnaryOPList = std::vector<const UnaryOPPE *>;
    using CmpPEList = std::vector<const CmpPE *>;
    using PHINodeMap = Map<const PAGNode *, CopyPEList>;
    using BinaryNodeMap = Map<const PAGNode *, BinaryOPList>;
    using UnaryNodeMap = Map<const PAGNode *, UnaryOPList>;
    using CmpNodeMap = Map<const PAGNode *, CmpPEList>;
    using FunToArgsListMap = Map<const SVFFunction *, PAGNodeList>;
    using CSToArgsListMap = Map<const CallBlockNode *, PAGNodeList>;
    using CSToRetMap = Map<const RetBlockNode *, const PAGNode *>;
    using FunToRetMap = Map<const SVFFunction *, const PAGNode *>;
    using FunToPAGEdgeSetMap = Map<const SVFFunction *, PAGEdgeSet>;
    using Inst2PAGEdgesMap = Map<const ICFGNode *, PAGEdgeList>;
    using NodeToNodeMap = Map<NodeID, NodeID>;
    using NodeOffset = std::pair<NodeID, Size_t>;
    using NodeLocationSet = std::pair<NodeID, LocationSet>;
    using NodeOffsetMap = Map<NodeOffset, NodeID>;
    using NodeLocationSetMap = Map<NodeLocationSet, NodeID>;
    using GepValPNMap = Map<const Value *, NodeLocationSetMap>;
    using NodePairSetMap = Map<NodePair, NodeID>;

  private:
    /// Maps function names to the entry nodes of the extpag which implements
    /// it. This is to connect arguments and callsites.
    Map<const SVFFunction *, Map<int, PAGNode *>> functionToExternalPAGEntries;
    Map<const SVFFunction *, PAGNode *> functionToExternalPAGReturns;

    /// ValueNodes - This map indicates the Node that a particular Value* is
    /// represented by.  This contains entries for all pointers.
    PAGEdge::PAGKindToEdgeSetMapTy
        PAGEdgeKindToSetMap; // < PAG edge map containing all PAGEdges
    PAGEdge::PAGKindToEdgeSetMapTy
        PTAPAGEdgeKindToSetMap; // < PAG edge map containing only
                                // pointer-related edges, i.e, both RHS and RHS
                                // are of pointer type
    Inst2PAGEdgesMap inst2PAGEdgesMap;    ///< Map a instruction to its PAGEdges
    Inst2PAGEdgesMap inst2PTAPAGEdgesMap; ///< Map a instruction to its
                                          ///< PointerAnalysis related PAGEdges
    GepValPNMap GepValNodeMap; ///< Map a pair<base,off> to a gep value node id
    NodeLocationSetMap
        GepObjNodeMap; ///< Map a pair<base,off> to a gep obj node id
    MemObjToFieldsMap memToFieldsMap; ///< Map a mem object id to all its fields
    PAGEdgeSet
        globPAGEdgesSet;   ///< Global PAGEdges without control flow information
    PHINodeMap phiNodeMap; ///< A set of phi copy edges
    BinaryNodeMap binaryNodeMap;     ///< A set of binary edges
    UnaryNodeMap unaryNodeMap;       ///< A set of unary edges
    CmpNodeMap cmpNodeMap;           ///< A set of comparision edges
    FunToArgsListMap funArgsListMap; ///< Map a function to a list of all its
                                     ///< formal parameters
    CSToArgsListMap callSiteArgsListMap; ///< Map a callsite to a list of all
                                         ///< its actual parameters
    CSToRetMap
        callSiteRetMap; ///< Map a callsite to its callsite returns PAGNodes
    FunToRetMap
        funRetMap; ///< Map a function to its unique function return PAGNodes

    CallSiteToFunPtrMap indCallSiteToFunPtrMap; ///< Map an indirect callsite to
                                                ///< its function pointer
                                                ///< CallBlockNode -> SymId
                                                ///< of the called
                                                ///< value (function ptr)
    FunPtrToCallSitesMap
        funPtrToCallSitesMap; ///< Map SymID of a function ptr (an llvm value)
                              ///< to the callsites where
                              ///< it is used the reverse mapping of
                              ///< indCallSiteToFunPtrMap
                              ///< Note, the same value may be called at
                              ///< multiple locations, thus the value is a set
                              ///< of CallBlockNode

    bool fromFile; ///< Whether the PAG is built according to user specified
                   ///< data from a txt file
    /// Valid pointers for pointer analysis resolution connected by PAG edges
    /// (constraints) this set of candidate pointers can change during pointer
    /// resolution (e.g. adding new object nodes)
    OrderedNodeSet candidatePointers;
    NodeID
        nodeNumAfterPAGBuild; // initial node number after building PAG,
                              // excluding later added nodes, e.g., gepobj nodes
    ICFG *icfg = nullptr;     // ICFG
    SymbolTableInfo *symbolTableInfo = nullptr;
    SVFProject *proj = nullptr;

    CallSiteSet callSiteSet; /// all the callsites of a program

    /// Clean up memory
    void destroy();

    NodeIDAllocator nodeIdAllocator;

    using LabelID = u32_t;
    using Inst2LabelMap = Map<const ICFGNode *, LabelID>;
    Inst2LabelMap inst2LabelMap;     ///< Call site Instruction to label map
    u64_t callEdgeLabelCounter = 0;  ///< Call site Instruction counter
    u64_t storeEdgeLabelCounter = 0; ///< Store Instruction counter

  public:
    friend class ExternalPAG;

    /// Constructor
    explicit PAG(SVFProject *proj, bool buildFromFile = false);
    PAG() = default;

    u32_t totalPTAPAGEdge;

    /// Return ICFG
    inline ICFG *getICFG() {
        if (icfg == nullptr) {
            icfg = proj->getICFG();
        }

        return icfg;
    }

    /// Return valid pointers
    inline OrderedNodeSet &getAllValidPtrs() { return candidatePointers; }
    /// Initialize candidate pointers
    inline void initialiseCandidatePointers() {
        // collect candidate pointers for demand-driven analysis
        for (auto &nIter : *this) {
            NodeID nodeId = nIter.first;
            // do not compute points-to for isolated node
            if (isValidPointer(nodeId) == false) {
                continue;
            }

            candidatePointers.insert(nodeId);
        }
    }

    /// Destructor
    virtual ~PAG() { destroy(); }

    /// Whether this PAG built from a txt file
    inline bool isBuiltFromFile() { return fromFile; }
    /// PAG build configurations
    //@{
    /// Whether to handle blackhole edge
    static void handleBlackHole(bool b);
    //@}
    ///

    inline SymbolTableInfo *getSymbolTableInfo() const {
        return symbolTableInfo;
    }

    inline NodeIDAllocator &getNodeIDAllocator() { return nodeIdAllocator; }

    /// Compute the unique edgeFlag value from edge kind and call site
    /// Instruction.
    inline PAGEdge::GEdgeFlag makeEdgeFlagWithCallInst(PAGEdge::GEdgeKind k,
                                                       const ICFGNode *cs) {
        auto iter = inst2LabelMap.find(cs);
        u64_t label = (iter != inst2LabelMap.end()) ? iter->second
                                                    : callEdgeLabelCounter++;
        assert(label < numeric_limits<LabelID>::max() && "LabelID overflow");
        return PAGEdge::makeEdgeFlagWithAuxInfo(k, label);
    }

    /// Compute the unique edgeFlag value from edge kind and store Instruction.
    /// Two store instructions may share the same StorePAGEdge
    inline PAGEdge::GEdgeFlag makeEdgeFlagWithStoreInst(PAGEdge::GEdgeKind k,
                                                        const ICFGNode *store) {
        auto iter = inst2LabelMap.find(store);
        u64_t label = (iter != inst2LabelMap.end()) ? iter->second
                                                    : storeEdgeLabelCounter++;
        assert(label < numeric_limits<LabelID>::max() && "LabelID overflow");
        return PAGEdge::makeEdgeFlagWithAuxInfo(k, label);
    }

    /// Get LLVM Module
    inline SVFModule *getModule() { return symbolTableInfo->getModule(); }
    inline void addCallSite(const CallBlockNode *call) {
        callSiteSet.insert(call);
    }
    inline const CallSiteSet &getCallSiteSet() const { return callSiteSet; }
    /// Get/set methods to get control flow information of a PAGEdge
    //@{
    /// Get edges set according to its kind
    inline PAGEdge::PAGEdgeSetTy &getEdgeSet(PAGEdge::PEDGEK kind) {
        return PAGEdgeKindToSetMap[kind];
    }
    /// Get PTA edges set according to its kind
    inline PAGEdge::PAGEdgeSetTy &getPTAEdgeSet(PAGEdge::PEDGEK kind) {
        return PTAPAGEdgeKindToSetMap[kind];
    }
    /// Whether this instruction has PAG Edge
    inline bool hasPAGEdgeList(const ICFGNode *inst) const {
        return inst2PAGEdgesMap.find(inst) != inst2PAGEdgesMap.end();
    }
    inline bool hasPTAPAGEdgeList(const ICFGNode *inst) const {
        return inst2PTAPAGEdgesMap.find(inst) != inst2PTAPAGEdgesMap.end();
    }
    /// Given an instruction, get all its PAGEdges
    inline PAGEdgeList &getInstPAGEdgeList(const ICFGNode *inst) {
        return inst2PAGEdgesMap[inst];
    }
    /// Given an instruction, get all its PTA PAGEdges
    inline PAGEdgeList &getInstPTAPAGEdgeList(const ICFGNode *inst) {
        return inst2PTAPAGEdgesMap[inst];
    }
    /// Add a PAGEdge into instruction map
    inline void addToInstPAGEdgeList(ICFGNode *inst, PAGEdge *edge) {
        edge->setICFGNode(inst);
        inst2PAGEdgesMap[inst].push_back(edge);
        if (edge->isPTAEdge()) {
            inst2PTAPAGEdgesMap[inst].push_back(edge);
        }
    }
    /// Get global PAGEdges (not in a procedure)
    inline void addGlobalPAGEdge(const PAGEdge *edge) {
        globPAGEdgesSet.insert(edge);
    }
    /// Get global PAGEdges (not in a procedure)
    inline PAGEdgeSet &getGlobalPAGEdgeSet() { return globPAGEdgesSet; }
    /// Add phi node information
    inline void addPhiNode(const PAGNode *res, const CopyPE *edge) {
        phiNodeMap[res].push_back(edge);
    }
    /// Whether this PAGNode is a result operand a of phi node
    inline bool isPhiNode(const PAGNode *node) const {
        return phiNodeMap.find(node) != phiNodeMap.end();
    }
    /// Get all phi copy edges
    inline PHINodeMap &getPhiNodeMap() { return phiNodeMap; }
    /// Add phi node information
    inline void addBinaryNode(const PAGNode *res, const BinaryOPPE *edge) {
        binaryNodeMap[res].push_back(edge);
    }
    /// Whether this PAGNode is a result operand a of phi node
    inline bool isBinaryNode(const PAGNode *node) const {
        return binaryNodeMap.find(node) != binaryNodeMap.end();
    }
    /// Get all phi copy edges
    inline BinaryNodeMap &getBinaryNodeMap() { return binaryNodeMap; }
    /// Add unary node information
    inline void addUnaryNode(const PAGNode *res, const UnaryOPPE *edge) {
        unaryNodeMap[res].push_back(edge);
    }
    /// Whether this PAGNode is an unary node
    inline bool isUnaryNode(const PAGNode *node) const {
        return unaryNodeMap.find(node) != unaryNodeMap.end();
    }
    /// Get all unary edges
    inline UnaryNodeMap &getUnaryNodeMap() { return unaryNodeMap; }
    /// Add phi node information
    inline void addCmpNode(const PAGNode *res, const CmpPE *edge) {
        cmpNodeMap[res].push_back(edge);
    }
    /// Whether this PAGNode is a result operand a of phi node
    inline bool isCmpNode(const PAGNode *node) const {
        return cmpNodeMap.find(node) != cmpNodeMap.end();
    }
    /// Get all phi copy edges
    inline CmpNodeMap &getCmpNodeMap() { return cmpNodeMap; }
    //@}

    ///
    /// Get/set method for function/callsite arguments and returns
    //@{
    /// Add function argument nodes
    inline void addFunArgs(const SVFFunction *fun, const PAGNode *arg) {
        FunEntryBlockNode *funEntryBlockNode = icfg->getFunEntryBlockNode(fun);
        /// save the PAG Arg node to the FunEntryBlockNode in the ICFG
        funEntryBlockNode->addFormalParms(arg);
        /// save the PAG Arg node in the mapping
        /// fun <--> [pag-arg0, pag-arg1, pag-arg2, ....]
        funArgsListMap[fun].push_back(arg);
    }

    ///
    /// \brief handle function ret node (PAG)
    ///
    /// 1. add the PAG ret node to the ICFG FunExitBlockNode of the function
    /// 2. setup the mapping between the function and PAG ret node
    inline void addFunRet(const SVFFunction *fun, const PAGNode *ret) {
        FunExitBlockNode *funExitBlockNode = icfg->getFunExitBlockNode(fun);
        funExitBlockNode->addFormalRet(ret);
        funRetMap[fun] = ret;
    }

    ///
    /// \brief set up one actual arguement for a callsite
    ///
    /// 1. Save the actual argument to the ICFG CallBlockNode
    /// 2. Save the actual argument to PAG::callSiteArgsListMap.
    ///
    inline void addCallSiteArgs(CallBlockNode *callBlockNode,
                                const PAGNode *arg) {
        /// add the argument value the ICFG CallBlockNode
        /// FIXME: what is the point of this design here
        callBlockNode->addActualParms(arg);

        // add the argument to the callSiteArgsListMap in PAG
        callSiteArgsListMap[callBlockNode].push_back(arg);
    }

    /// Add callsite returns
    inline void addCallSiteRets(RetBlockNode *retBlockNode,
                                const PAGNode *callInstNode) {
        retBlockNode->addActualRet(callInstNode);
        callSiteRetMap[retBlockNode] = callInstNode;
    }

    /// Function has arguments list
    inline bool hasFunArgsList(const SVFFunction *func) const {
        return (funArgsListMap.find(func) != funArgsListMap.end());
    }

    /// Get function arguments list
    inline FunToArgsListMap &getFunArgsMap() { return funArgsListMap; }

    /// Get function arguments list
    inline const PAGNodeList &getFunArgsList(const SVFFunction *func) const {
        auto it = funArgsListMap.find(func);
        assert(it != funArgsListMap.end() &&
               "this function doesn't have arguments");
        return it->second;
    }

    /// Callsite has argument list
    inline bool hasCallSiteArgsMap(const CallBlockNode *cs) const {
        return (callSiteArgsListMap.find(cs) != callSiteArgsListMap.end());
    }

    /// Get callsite argument list
    inline CSToArgsListMap &getCallSiteArgsMap() { return callSiteArgsListMap; }

    /// Get callsite argument list
    inline const PAGNodeList &
    getCallSiteArgsList(const CallBlockNode *cs) const {
        auto it = callSiteArgsListMap.find(cs);
        assert(it != callSiteArgsListMap.end() &&
               "this call site doesn't have arguments");
        return it->second;
    }

    /// Get callsite return
    inline CSToRetMap &getCallSiteRets() { return callSiteRetMap; }

    /// Get callsite return
    inline const PAGNode *getCallSiteRet(const RetBlockNode *cs) const {
        auto it = callSiteRetMap.find(cs);
        assert(it != callSiteRetMap.end() &&
               "this call site doesn't have return");
        return it->second;
    }

    inline bool callsiteHasRet(const RetBlockNode *cs) const {
        return callSiteRetMap.find(cs) != callSiteRetMap.end();
    }

    /// Get function return list
    inline FunToRetMap &getFunRets() { return funRetMap; }

    /// Get function return list
    inline const PAGNode *getFunRet(const SVFFunction *func) const {
        auto it = funRetMap.find(func);
        assert(it != funRetMap.end() && "this function doesn't have return");
        return it->second;
    }
    inline bool funHasRet(const SVFFunction *func) const {
        return funRetMap.find(func) != funRetMap.end();
    }
    //@}

    /// Node and edge statistics
    //@{
    inline Size_t getPAGNodeNum() const { return getTotalNodeNum(); }
    inline Size_t getPAGEdgeNum() const { return getTotalEdgeNum(); }
    inline Size_t getValueNodeNum() const {
        return symbolTableInfo->valSymToId().size();
    }
    inline Size_t getObjectNodeNum() const {
        return symbolTableInfo->idToMemObj().size();
    }
    inline Size_t getFieldValNodeNum() const { return GepValNodeMap.size(); }
    inline Size_t getFieldObjNodeNum() const { return GepObjNodeMap.size(); }
    //@}

    /// Due to constaint expression, curInst is used to distinguish different
    /// instructions (e.g., memorycpy) when creating GepValPN.
    inline NodeID getGepValNode(const Value *curInst, NodeID base,
                                const LocationSet &ls) const {
        auto iter = GepValNodeMap.find(curInst);
        if (iter == GepValNodeMap.end()) {
            return UINT_MAX;
        }

        auto lit = iter->second.find(std::make_pair(base, ls));

        if (lit == iter->second.end()) {
            return UINT_MAX;
        }

        return lit->second;
    }

    /// Add/get indirect callsites
    //@{
    inline const CallSiteToFunPtrMap &getIndirectCallsites() const {
        return indCallSiteToFunPtrMap;
    }

    inline void addIndirectCallsites(const CallBlockNode *cs, NodeID funPtr) {
        bool added =
            indCallSiteToFunPtrMap.insert(std::make_pair(cs, funPtr)).second;
        funPtrToCallSitesMap[funPtr].insert(cs);
        assert(added && "adding the same indirect callsite twice?");
    }

    inline NodeID getFunPtr(const CallBlockNode *cs) const {
        auto it = indCallSiteToFunPtrMap.find(cs);
        assert(it != indCallSiteToFunPtrMap.end() &&
               "indirect callsite not have a function pointer?");
        return it->second;
    }

    inline const CallSiteSet &getIndCallSites(NodeID funPtr) const {
        auto it = funPtrToCallSitesMap.find(funPtr);
        assert(it != funPtrToCallSitesMap.end() &&
               "function pointer not used at any indirect callsite?");
        return it->second;
    }

    inline bool isIndirectCallSites(const CallBlockNode *cs) const {
        return (indCallSiteToFunPtrMap.find(cs) !=
                indCallSiteToFunPtrMap.end());
    }

    inline bool isFunPtr(NodeID id) const {
        return (funPtrToCallSitesMap.find(id) != funPtrToCallSitesMap.end());
    }
    //@}

    /// Get an edge according to src, dst and kind
    //@{
    inline PAGEdge *getIntraPAGEdge(NodeID src, NodeID dst,
                                    PAGEdge::PEDGEK kind) {
        return getIntraPAGEdge(getGNode(src), getGNode(dst), kind);
    }

    inline PAGEdge *getIntraPAGEdge(PAGNode *src, PAGNode *dst,
                                    PAGEdge::PEDGEK kind) {
        PAGEdge edge(src, dst, getDummyEdgeId(), this, kind);
        const PAGEdge::PAGEdgeSetTy &edgeSet = getEdgeSet(kind);
        auto it = edgeSet.find(&edge);
        assert(it != edgeSet.end() && "can not find pag edge");
        return (*it);
    }
    //@}

    /// Get PAG Node ID according to LLVM value
    //@{
    /// getNode - Return the node ID corresponding
    /// to the specified pointer.
    /// FIXME: rename the API
    inline NodeID getValueNode(const Value *V) {
        return symbolTableInfo->getValSym(V);
    }

    inline bool hasValueNode(const Value *V) {
        return symbolTableInfo->hasValSym(V);
    }

    /// getObject - Return the obj node id refer to the memory object for the
    /// specified global, heap or alloca instruction according to llvm value.
    inline NodeID getObjectNode(const Value *V) {
        return symbolTableInfo->getObjSym(V);
    }

    /// getObject - return mem object id
    inline NodeID getObjectNode(const MemObj *mem) { return mem->getSymId(); }

    /// Get memory object - Return memory object according to pag node id
    /// return whole allocated memory object if this node is a gep obj node
    /// return nullptr is this node is not a ObjPN type
    //@{
    inline const MemObj *getObject(NodeID id) const {
        const PAGNode *node = getGNode(id);
        if (const ObjPN *objPN = llvm::dyn_cast<ObjPN>(node)) {
            return getObject(objPN);
        }

        return nullptr;
    }

    inline const MemObj *getObject(const ObjPN *node) const {
        return node->getMemObj();
    }
    //@}

    /// GetReturnNode - Return the unique node representing the return value of
    /// a function
    inline NodeID getReturnNode(const SVFFunction *func) const {
        return symbolTableInfo->getRetSym(func->getLLVMFun());
    }

    /// getVarargNode - Return the unique node representing the variadic
    /// argument of a variadic function.
    inline NodeID getVarargNode(const SVFFunction *func) const {
        return symbolTableInfo->getVarargSym(func->getLLVMFun());
    }

    /// Get a field PAG Object node according to base mem obj and offset
    NodeID getGepObjNode(const MemObj *obj, const LocationSet &ls);

    /// Get a field obj PAG node according to a mem obj and a given offset
    NodeID getGepObjNode(NodeID id, const LocationSet &ls);

    /// Get a field-insensitive obj PAG node according to a mem obj
    //@{
    inline NodeID getFIObjNode(const MemObj *obj) const {
        return obj->getSymId();
    }

    inline NodeID getFIObjNode(NodeID id) const {
        PAGNode *node = getGNode(id);
        assert(llvm::isa<ObjPN>(node) && "need an object node");
        auto *obj = llvm::cast<ObjPN>(node);
        return getFIObjNode(obj->getMemObj());
    }
    //@}

    /// Get black hole and constant id
    //@{
    inline NodeID getBlackHoleNodeID() const {
        return symbolTableInfo->blackholeSymID();
    }

    inline NodeID getConstantNodeID() const {
        return symbolTableInfo->constantSymID();
    }

    inline NodeID getBlkPtr() const { return symbolTableInfo->blkPtrSymID(); }

    inline NodeID getNullPtr() const { return symbolTableInfo->nullPtrSymID(); }

    inline bool isBlkPtr(NodeID id) const {
        return (SymbolTableInfo::isBlkPtr(id));
    }

    inline bool isNullPtr(NodeID id) const {
        return (SymbolTableInfo::isNullPtr(id));
    }

    inline bool isBlkObjOrConstantObj(NodeID id) const {
        return (isBlkObj(id) || isConstantObj(id));
    }

    inline bool isBlkObj(NodeID id) const {
        return SymbolTableInfo::isBlkObj(id);
    }

    inline bool isConstantObj(NodeID id) const {
        const MemObj *obj = getObject(id);
        assert(obj && "not an object node?");
        return SymbolTableInfo::isConstantObj(id) || obj->isConstant();
    }

    inline bool isNonPointerObj(NodeID id) const {
        PAGNode *node = getGNode(id);
        if (FIObjPN *fiNode = llvm::dyn_cast<FIObjPN>(node)) {
            return (fiNode->getMemObj()->hasPtrObj() == false);
        } else if (GepObjPN *gepNode = llvm::dyn_cast<GepObjPN>(node)) {
            return (gepNode->getMemObj()->isNonPtrFieldObj(
                gepNode->getLocationSet()));
        } else if (llvm::isa<DummyObjPN>(node)) {
            return false;
        } else {
            assert(false && "expecting a object node");
            return false;
        }
    }

    inline const MemObj *getBlackHoleObj() const {
        return symbolTableInfo->getBlkObj();
    }

    inline const MemObj *getConstantObj() const {
        return symbolTableInfo->getConstantObj();
    }
    //@}

    inline u32_t getNodeNumAfterPAGBuild() const {
        return nodeNumAfterPAGBuild;
    }

    inline void setNodeNumAfterPAGBuild(u32_t num) {
        nodeNumAfterPAGBuild = num;
    }

    /// Base and Offset methods for Value and Object node
    //@{
    /// Get a base pointer node given a field pointer
    NodeID getBaseValNode(NodeID nodeId);
    LocationSet getLocationSetFromBaseNode(NodeID nodeId);

    inline NodeID getBaseObjNode(NodeID id) const {
        return getBaseObj(id)->getSymId();
    }

    inline const MemObj *getBaseObj(NodeID id) const {
        const PAGNode *node = getGNode(id);
        assert(llvm::isa<ObjPN>(node) && "need an object node");
        const auto *obj = llvm::cast<ObjPN>(node);
        return obj->getMemObj();
    }
    //@}

    /// Get all fields of an object
    //@{
    NodeBS &getAllFieldsObjNode(const MemObj *obj);
    NodeBS &getAllFieldsObjNode(NodeID id);
    NodeBS getFieldsAfterCollapse(NodeID id);
    //@}

    /// add node into PAG
    //@{
    /// Add a PAG node into Node map
    inline NodeID addNode(PAGNode *node, NodeID i) {
        addGNode(i, node);
        return i;
    }

    /// Add a value (pointer) node
    inline NodeID addValNode(const Value *val, NodeID i) {
        PAGNode *node = new ValPN(val, i);
        return addValNode(val, node, i);
    }

    /// Add a memory obj node
    inline NodeID addObjNode(const Value *val, NodeID i) {
        const MemObj *mem =
            symbolTableInfo->getObj(symbolTableInfo->getObjSym(val));
        assert(((mem->getSymId() == i) ||
                (symbolTableInfo->getGlobalRep(val) != val)) &&
               "not same object id?");
        return addFIObjNode(mem);
    }

    /// Add a unique return node for a procedure
    inline NodeID addRetNode(const SVFFunction *val, NodeID i) {
        PAGNode *node = new RetPN(val, i);
        return addRetNode(val, node, i);
    }

    /// Add a unique vararg node for a procedure
    inline NodeID addVarargNode(const SVFFunction *val, NodeID i) {
        PAGNode *node = new VarArgPN(val, i);
        return addNode(node, i);
    }

    /// Add a temp field value node, this method can only invoked by
    /// getGepValNode
    NodeID addGepValNode(const Value *curInst, const Value *val,
                         const LocationSet &ls, NodeID i, const Type *type,
                         u32_t fieldidx);

    /// Add a field obj node, this method can only invoked by getGepObjNode
    NodeID addGepObjNode(const MemObj *obj, const LocationSet &ls);

    /// Add a field-insensitive node, this method can only invoked by
    /// getFIGepObjNode
    NodeID addFIObjNode(const MemObj *obj);
    //@}

    ///  Add a dummy value/object node according to node ID (llvm value is null)
    //@{
    inline NodeID addDummyValNode() {
        return addDummyValNode(nodeIdAllocator.allocateValueId());
    }

    inline NodeID addDummyValNode(NodeID i) {
        return addValNode(nullptr, new DummyValPN(i), i);
    }

    inline NodeID addDummyObjNode(const Type *type = nullptr) {
        return addDummyObjNode(nodeIdAllocator.allocateObjectId(), type);
    }

    inline NodeID addDummyObjNode(NodeID i, const Type *type) {
        const MemObj *mem = addDummyMemObj(i, type);
        return addObjNode(nullptr, new DummyObjPN(i, mem), i);
    }

    inline const MemObj *addDummyMemObj(NodeID i, const Type *type) {
        return symbolTableInfo->createDummyObj(i, type);
    }

    inline NodeID addBlackholeObjNode() {
        auto id = getBlackHoleNodeID();
        return addObjNode(nullptr, new DummyObjPN(id, getBlackHoleObj()), id);
    }

    inline NodeID addConstantObjNode() {
        auto id = getConstantNodeID();
        return addObjNode(nullptr, new DummyObjPN(id, getConstantObj()), id);
    }

    inline NodeID addBlackholePtrNode() { return addDummyValNode(getBlkPtr()); }
    //@}

    /// Add a value (pointer) node
    inline NodeID addValNode(const Value *, PAGNode *node, NodeID i) {
        assert(i < UINT_MAX && "exceeding the maximum node limits");
        return addNode(node, i);
    }

    /// Add a memory obj node
    inline NodeID addObjNode(const Value *, PAGNode *node, NodeID i) {
        assert(i < UINT_MAX && "exceeding the maximum node limits");
        return addNode(node, i);
    }

    /// Add a unique return node for a procedure
    inline NodeID addRetNode(const SVFFunction *, PAGNode *node, NodeID i) {
        return addNode(node, i);
    }

    /// Add a unique vararg node for a procedure
    inline NodeID addVarargNode(const SVFFunction *, PAGNode *node, NodeID i) {
        return addNode(node, i);
    }

    /// Add an edge into PAG
    //@{
    /// Add a PAG edge
    bool addEdge(PAGNode *src, PAGNode *dst, PAGEdge *edge);

    //// Return true if this edge exits
    PAGEdge *hasNonlabeledEdge(PAGNode *src, PAGNode *dst,
                               PAGEdge::PEDGEK kind);

    /// Return true if this labeled edge exits, including store, call and load
    /// two store edge can have same dst and src but located in different basic
    /// blocks, thus flags are needed to distinguish them
    PAGEdge *hasLabeledEdge(PAGNode *src, PAGNode *dst, PAGEdge::PEDGEK kind,
                            const ICFGNode *cs);

    /// Add Address edge
    AddrPE *addAddrPE(NodeID src, NodeID dst);

    /// Add Copy edge
    CopyPE *addCopyPE(NodeID src, NodeID dst);

    /// Add Copy edge
    CmpPE *addCmpPE(NodeID src, NodeID dst);

    /// Add Copy edge
    BinaryOPPE *addBinaryOPPE(NodeID src, NodeID dst);

    /// Add Unary edge
    UnaryOPPE *addUnaryOPPE(NodeID src, NodeID dst);

    /// Add Load edge
    LoadPE *addLoadPE(NodeID src, NodeID dst);

    /// Add Store edge
    StorePE *addStorePE(NodeID src, NodeID dst, const IntraBlockNode *val);

    /// Add Call edge
    CallPE *addCallPE(NodeID src, NodeID dst, const CallBlockNode *cs);
    /// Add Return edge
    RetPE *addRetPE(NodeID src, NodeID dst, const CallBlockNode *cs);
    /// Add Gep edge
    GepPE *addGepPE(NodeID src, NodeID dst, const LocationSet &ls,
                    bool constGep);
    /// Add Offset(Gep) edge
    NormalGepPE *addNormalGepPE(NodeID src, NodeID dst, const LocationSet &ls);
    /// Add Variant(Gep) edge
    VariantGepPE *addVariantGepPE(NodeID src, NodeID dst);
    /// Add Thread fork edge for parameter passing
    TDForkPE *addThreadForkPE(NodeID src, NodeID dst, const CallBlockNode *cs);
    /// Add Thread join edge for parameter passing
    TDJoinPE *addThreadJoinPE(NodeID src, NodeID dst, const CallBlockNode *cs);
    //@}

    /// Set a pointer points-to black hole (e.g. int2ptr)
    PAGEdge *addBlackHoleAddrPE(NodeID node);

    /// Whether a node is a valid pointer
    //@{
    bool isValidPointer(NodeID nodeId) const;

    bool isValidTopLevelPtr(const PAGNode *node);
    //@}

    void initializeExternalPAGs();

    /// Whether an external PAG implementing function exists.
    bool hasExternalPAG(const SVFFunction *function) {
        bool ret = functionToExternalPAGEntries.find(function) !=
                   functionToExternalPAGEntries.end();
        return ret;
    }

    std::vector<std::pair<std::string, std::string>>
    parseExternalPAGs(llvm::cl::list<std::string> &extpagsArgs);

    /// Connects callsite if a external PAG implementing the relevant function
    /// has been added.
    /// Returns true on success, false otherwise.
    bool connectCallsiteToExternalPAG(CallSite *cs);

    /// Dump individual PAGs of specified functions. Currently to outs().
    void dumpFunctions(std::vector<std::string> &functions);

    /// Return graph name
    inline std::string getGraphName() const { return "PAG"; }

    /// Print PAG
    void print();

    /// Dump PAG
    void dump(std::string name);

    /// View graph from the debugger
    void view() override;
};

} // End namespace SVF

namespace llvm {

/* !
 * GraphTraits specializations of PAG to be used for the generic graph
 * algorithms. Provide graph traits for tranversing from a PAG node using
 * standard graph traversals.
 */
template <>
struct GraphTraits<SVF::PAGNode *>
    : public GraphTraits<SVF::GenericNode<SVF::PAGNode, SVF::PAGEdge> *> {};

/// Inverse GraphTraits specializations for PAG node, it is used for inverse
/// traversal.
template <>
struct GraphTraits<Inverse<SVF::PAGNode *>>
    : public GraphTraits<
          Inverse<SVF::GenericNode<SVF::PAGNode, SVF::PAGEdge> *>> {};

template <>
struct GraphTraits<SVF::PAG *>
    : public GraphTraits<SVF::GenericGraph<SVF::PAGNode, SVF::PAGEdge> *> {
    using NodeRef = SVF::PAGNode *;
};

} // End namespace llvm

#endif /* PAG_H_ */
