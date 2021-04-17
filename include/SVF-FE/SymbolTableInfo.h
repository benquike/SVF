//===- SymbolTableInfo.h -- Symbol information from IR------------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013->  <Yulei Sui>
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
 * SymbolTableInfo.h
 *
 *  Created on: Nov 11, 2013
 *      Author: Yulei Sui
 */

#ifndef INCLUDE_SVF_FE_SYMBOLTABLEINFO_H_
#define INCLUDE_SVF_FE_SYMBOLTABLEINFO_H_

#include "MemoryModel/MemModel.h"
#include "SVF-FE/LLVMModule.h"
#include "Util/NodeIDAllocator.h"

namespace SVF {

/*!
 * Symbol table of the memory model for analysis
 */
class SymbolTableInfo {

  public:
    /// various maps defined
    //{@
    /// llvm value to sym id map
    /// local (%) and global (@) identifiers are pointer types which have a
    /// value node id.
    using ValueToIDMapTy = OrderedMap<const Value *, SymID>;
    /// sym id to memory object map
    using IDToValueMapTy = OrderedMap<SymID, const Value *>;
    using IDToMemMapTy = OrderedMap<SymID, const MemObj *>;
    using MemToIDMapTy = OrderedMap<const MemObj *, SymID>;

    /// function to sym id map
    using FunToIDMapTy = OrderedMap<const Function *, SymID>;
    /// sym id to sym type map
    using IDToSymTyMapTy = OrderedMap<SymID, SYMTYPE>;
    /// struct type to struct info map
    using TypeToFieldInfoMap = OrderedMap<const Type *, StInfo *>;
    using IDToTypeMapTy = OrderedMap<SymID, const Type *>;
    using TypeToIDMapTy = OrderedMap<const Type *, SymID>;
    using CallSiteSet = Set<CallSite>;
    using CallSiteToIDMapTy = OrderedMap<const Instruction *, CallSiteID>;
    using IDToCallSiteMapTy = OrderedMap<CallSiteID, const Instruction *>;

    //@}

  private:
    // node id allocator
    NodeIDAllocator nodeIDAllocator;

    ValueToIDMapTy valSymMap;  ///< map a value to its sym id
    ValueToIDMapTy objSymMap;  ///< map a obj reference to its sym id
    IDToValueMapTy idValueMap; ///< map from its id to the pointer
    IDToMemMapTy Id2MemMap;    ///< map a memory sym id to its obj
    MemToIDMapTy mem2IdMap;
    IDToSymTyMapTy symTyMap;   /// < map a sym id to its type
    FunToIDMapTy returnSymMap; ///< return  map
    FunToIDMapTy varargSymMap; ///< vararg map

    IDToTypeMapTy Id2TypeMap;
    TypeToIDMapTy Type2IdMap;

    CallSiteSet callSiteSet;

    /// Module
    SVFModule *mod;

    /// Max field limit
    static u32_t maxFieldLimit;

    /// Invoke llvm passes to modify module
    void prePassSchedule(SVFModule *svfModule);

    /// Clean up memory
    void destroy();

    /// Whether to model constants
    bool modelConstants;

    /// total number of symbols
    SymID totalSymNum;

  private:
    void collectInst(const Instruction *inst);

    inline void addMemObj(const MemObj *memObj, SymID id) {
        Id2MemMap[id] = memObj;
        mem2IdMap[Id2MemMap[id]] = id;
    }

    inline void addTypeId(const Type *type, SymID id) {
        Id2TypeMap[id] = type;
        Type2IdMap[Id2TypeMap[id]] = id;
    }

    void collectTypeID(const Value *val);
    void collectTypeID(const Type *type);

  public:
    /// Constructor
    explicit SymbolTableInfo(SVFModule *mod)
        : mod(mod), modelConstants(false), totalSymNum(0), maxStruct(nullptr),
          maxStSize(0) {
        // start building the memory model
        // in the co construtor
        buildMemModel();
    }

    /// Get a SymbolTableInfo instance,
    //  depending on the program option,
    //  an instance of either LocSymTableInfo
    //  or SymbolTableInfo will be returned
    //@{
    static SymbolTableInfo *SymbolInfo(SVFModule *mod);

    virtual ~SymbolTableInfo() { destroy(); }
    //@}

    /// Set / Get modelConstants
    //@{
    void setModelConstants(bool _modelConstants) {
        modelConstants = _modelConstants;
    }
    bool getModelConstants() const { return modelConstants; }
    //@}

    /// Get callsite set
    //@{
    inline const CallSiteSet &getCallSiteSet() const { return callSiteSet; }
    //@}

    /// Module
    inline SVFModule *getModule() { return mod; }

    /// Helper method to get the size of the type from target data layout
    //@{
    u32_t getTypeSizeInBytes(const Type *type);
    u32_t getTypeSizeInBytes(const StructType *sty, u32_t field_index);
    //@}

    /// Start building memory model
    void buildMemModel();

    /// collect the syms
    //@{
    void collectSym(const Value *val);

    void collectVal(const Value *val);

    void collectObj(const Value *val);

    void collectRet(const Function *val);

    void collectVararg(const Function *val);
    //@}

    /// special value
    // @{
    static bool isNullPtrSym(const Value *val);

    static bool isBlackholeSym(const Value *val);

    bool isConstantObjSym(const Value *val);

    static inline bool isBlkPtr(NodeID id) { return (id == BlkPtr); }

    static inline bool isNullPtr(NodeID id) { return (id == NullPtr); }

    static inline bool isBlkObj(NodeID id) { return (id == BlackHole); }

    static inline bool isConstantObj(NodeID id) { return (id == ConstantObj); }

    static inline bool isBlkObjOrConstantObj(NodeID id) {
        return (isBlkObj(id) || isConstantObj(id));
    }

    inline void createBlkOrConstantObj(SymID symId) {
        assert(isBlkObjOrConstantObj(symId));
        assert(Id2MemMap.find(symId) == Id2MemMap.end());
        auto *memObj = new MemObj(symId, this);
        addMemObj(memObj, symId);
    }

    inline const MemObj *getBlkObj() const { return getObj(blackholeSymID()); }
    inline const MemObj *getConstantObj() const {
        return getObj(constantSymID());
    }

    inline SymID blkPtrSymID() const { return BlkPtr; }

    inline SymID nullPtrSymID() const { return NullPtr; }

    inline SymID constantSymID() const { return ConstantObj; }

    inline SymID blackholeSymID() const { return BlackHole; }

    /// Can only be invoked by PAG::addDummyNode() when creaing PAG from file.
    inline const MemObj *createDummyObj(SymID symId, const Type *type) {
        assert(Id2MemMap.find(symId) == Id2MemMap.end() &&
               "this dummy obj has been created before");
        auto *memObj = new MemObj(symId, this, type);
        addMemObj(memObj, symId);
        return memObj;
    }
    // @}

    /// Handle constant expression
    // @{
    void handleGlobalCE(const GlobalVariable *G);
    void handleGlobalInitializerCE(const Constant *C, u32_t offset);
    void handleCE(const Value *val);
    // @}

    /// Get different kinds of syms
    //@{
    /// FIXME: rename this API to getValSymID
    SymID getValSym(const Value *val) {

        if (isNullPtrSym(val)) {
            return nullPtrSymID();
        }

        if (isBlackholeSym(val)) {
            return blkPtrSymID();
        }

        auto iter = valSymMap.find(val);
        assert(iter != valSymMap.end() && "value sym not found");
        return iter->second;
    }

    /// switch to this api.
    SymID getValSymId(const Value *val) { return getValSym(val); }

    SymID getMemObjId(const MemObj *memObj) {
        assert(mem2IdMap.find(memObj) != mem2IdMap.end() &&
               "MemObj not exists");

        return mem2IdMap[memObj];
    }

    const MemObj *getMemObj(SymID id) {
        assert(Id2MemMap.find(id) != Id2MemMap.end() && "MemObj ID not exists");

        return Id2MemMap[id];
    }

    SymID getTypeId(const Type *type) {
        assert(Type2IdMap.find(type) != Type2IdMap.end() && "Type not exist");
        return Type2IdMap[type];
    }

    const Type *getType(SymID id) {
        assert(Id2TypeMap.find(id) != Id2TypeMap.end() && "Type id not exist");
        return Id2TypeMap[id];
    }

    inline bool hasValSym(const Value *val) {
        if (isNullPtrSym(val) || isBlackholeSym(val)) {
            return true;
        }

        return (valSymMap.find(val) != valSymMap.end());
    }

    /// find the unique defined global across multiple modules
    inline const Value *getGlobalRep(const Value *val) const {
        if (const auto *gvar = SVFUtil::dyn_cast<GlobalVariable>(val)) {
            LLVMModuleSet *modSet = mod->getLLVMModSet();
            if (modSet->hasGlobalRep(gvar)) {
                val = modSet->getGlobalRep(gvar);
            }
        }
        return val;
    }

    inline SymID getObjSym(const Value *val) const {
        auto iter = objSymMap.find(getGlobalRep(val));
        assert(iter != objSymMap.end() && "obj sym not found");
        return iter->second;
    }

    inline const MemObj *getObj(SymID id) const {
        auto iter = Id2MemMap.find(id);
        assert(iter != Id2MemMap.end() && "obj not found");
        return iter->second;
    }

    inline SymID getRetSym(const Function *val) const {
        auto iter = returnSymMap.find(val);
        assert(iter != returnSymMap.end() && "ret sym not found");
        return iter->second;
    }

    inline SymID getVarargSym(const Function *val) const {
        auto iter = varargSymMap.find(val);
        assert(iter != varargSymMap.end() && "vararg sym not found");
        return iter->second;
    }
    //@}

    /// Statistics
    //@{
    inline Size_t getTotalSymNum() const { return totalSymNum; }
    inline u32_t getMaxStructSize() const { return maxStSize; }
    //@}

    /// Get different kinds of syms maps
    //@{
    inline ValueToIDMapTy &valSyms() { return valSymMap; }

    inline ValueToIDMapTy &objSyms() { return objSymMap; }

    inline IDToMemMapTy &idToObjMap() { return Id2MemMap; }
    inline IDToValueMapTy &idToValueMap() { return idValueMap; }

    inline FunToIDMapTy &retSyms() { return returnSymMap; }

    inline FunToIDMapTy &varargSyms() { return varargSymMap; }

    inline IDToSymTyMapTy &symIDToTypeMap() { return symTyMap; }

    //@}

    /// Get struct info
    //@{
    /// Get an iterator for StructInfo, designed as internal methods
    TypeToFieldInfoMap::iterator getStructInfoIter(const Type *T) {
        assert(T);
        auto it = typeToFieldInfo.find(T);
        if (it != typeToFieldInfo.end()) {
            return it;
        } else {
            collectTypeInfo(T);
            return typeToFieldInfo.find(T);
        }
    }

    /// Get a reference to StructInfo.
    inline StInfo *getStructInfo(const Type *T) {
        return getStructInfoIter(T)->second;
    }

    /// Get a reference to the components of struct_info.
    const inline std::vector<u32_t> &getFattenFieldIdxVec(const Type *T) {
        return getStructInfoIter(T)->second->getFieldIdxVec();
    }
    const inline std::vector<u32_t> &getFattenFieldOffsetVec(const Type *T) {
        return getStructInfoIter(T)->second->getFieldOffsetVec();
    }
    const inline std::vector<FieldInfo> &getFlattenFieldInfoVec(const Type *T) {
        return getStructInfoIter(T)->second->getFlattenFieldInfoVec();
    }
    const inline Type *getOrigSubTypeWithFldInx(const Type *baseType,
                                                u32_t field_idx) {
        return getStructInfoIter(baseType)->second->getFieldTypeWithFldIdx(
            field_idx);
    }
    const inline Type *getOrigSubTypeWithByteOffset(const Type *baseType,
                                                    u32_t byteOffset) {
        return getStructInfoIter(baseType)->second->getFieldTypeWithByteOffset(
            byteOffset);
    }
    //@}

    /// Compute gep offset
    virtual bool computeGepOffset(const User *V, LocationSet &ls);
    /// Get the base type and max offset
    const Type *getBaseTypeAndFlattenedFields(const Value *V,
                                              std::vector<LocationSet> &fields);
    /// Replace fields with flatten fields of T if the number of its fields is
    /// larger than msz.
    u32_t getFields(std::vector<LocationSet> &fields, const Type *T, u32_t msz);
    /// Collect type info
    void collectTypeInfo(const Type *T);
    /// Given an offset from a Gep Instruction, return it modulus offset by
    /// considering memory layout
    virtual LocationSet getModulusOffset(const MemObj *obj,
                                         const LocationSet &ls);

    /// Debug method
    void printFlattenFields(const Type *type);

    static std::string toString(SYMTYPE symtype);

    /// Another debug method
    virtual void dump();

    NodeIDAllocator &getNodeIDAllocator() { return nodeIDAllocator; }

  protected:
    /// Collect the struct info
    virtual void collectStructInfo(const StructType *T);
    /// Collect the array info
    virtual void collectArrayInfo(const ArrayType *T);
    /// Collect simple type (non-aggregate) info
    virtual void collectSimpleTypeInfo(const Type *T);

    /// Every type T is mapped to StInfo
    /// which contains size (fsize) , offset(foffset)
    /// fsize[i] is the number of fields in the largest such struct, else
    /// fsize[i] = 1. fsize[0] is always the size of the expanded struct.
    TypeToFieldInfoMap typeToFieldInfo;

    /// The struct type with the most fields
    const Type *maxStruct{};

    /// The number of fields in max_struct
    u32_t maxStSize{};
};

/*!
 * Bytes/bits-level modeling of memory locations to handle weakly type
 * languages. (declared with one type but accessed as another) Abstract memory
 * objects are created according to the static allocated size.
 */
class LocSymTableInfo : public SymbolTableInfo {

  public:
    /// Constructor
    explicit LocSymTableInfo(SVFModule *mod) : SymbolTableInfo(mod) {}
    /// Destructor
    ~LocSymTableInfo() override {}
    /// Compute gep offset
    bool computeGepOffset(const User *V, LocationSet &ls) override;
    /// Given an offset from a Gep Instruction, return it modulus offset by
    /// considering memory layout
    LocationSet getModulusOffset(const MemObj *obj,
                                 const LocationSet &ls) override;

    /// Verify struct size construction
    void verifyStructSize(StInfo *stInfo, u32_t structSize);

  protected:
    /// Collect the struct info
    void collectStructInfo(const StructType *T) override;
    /// Collect the array info
    void collectArrayInfo(const ArrayType *T) override;
};

class LocObjTypeInfo : public ObjTypeInfo {

  public:
    /// Constructor
    LocObjTypeInfo(SymbolTableInfo *symInfo, const Value *val, Type *t,
                   Size_t max)
        : ObjTypeInfo(symInfo, val, t, max) {}
    /// Destructor
    ~LocObjTypeInfo() override {}
    /// Get the size of this object
    u32_t getObjSize(const Value *val) override;
};

} // End namespace SVF

#endif /* INCLUDE_SVF_FE_SYMBOLTABLEINFO_H_ */
