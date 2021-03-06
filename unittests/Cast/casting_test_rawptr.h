/// This file is automatically generated by
/// scripts/gen_graph_cast_tests.py graph_type_info.yaml .

/// DONOT edit this file manually

// BEGIN OF object (de)allocation
GlobalBlockNode *globalBlockNode = new GlobalBlockNode();
ICFGNode *iCFGNode = new ICFGNode();
IntraBlockNode *intraBlockNode = new IntraBlockNode();
InterBlockNode *interBlockNode = new InterBlockNode();
FunEntryBlockNode *funEntryBlockNode = new FunEntryBlockNode();
FunExitBlockNode *funExitBlockNode = new FunExitBlockNode();
CallBlockNode *callBlockNode = new CallBlockNode();
RetBlockNode *retBlockNode = new RetBlockNode();
ValPN *valPN = new ValPN();
GepValPN *gepValPN = new GepValPN();
DummyValPN *dummyValPN = new DummyValPN();
ObjPN *objPN = new ObjPN();
GepObjPN *gepObjPN = new GepObjPN();
CloneGepObjPN *cloneGepObjPN = new CloneGepObjPN();
FIObjPN *fIObjPN = new FIObjPN();
CloneFIObjPN *cloneFIObjPN = new CloneFIObjPN();
DummyObjPN *dummyObjPN = new DummyObjPN();
CloneDummyObjPN *cloneDummyObjPN = new CloneDummyObjPN();
RetPN *retPN = new RetPN();
VarArgPN *varArgPN = new VarArgPN();
StmtVFGNode *stmtVFGNode = new StmtVFGNode();
VFGNode *vFGNode = new VFGNode();
LoadVFGNode *loadVFGNode = new LoadVFGNode();
StoreVFGNode *storeVFGNode = new StoreVFGNode();
CopyVFGNode *copyVFGNode = new CopyVFGNode();
GepVFGNode *gepVFGNode = new GepVFGNode();
AddrVFGNode *addrVFGNode = new AddrVFGNode();
CmpVFGNode *cmpVFGNode = new CmpVFGNode();
BinaryOPVFGNode *binaryOPVFGNode = new BinaryOPVFGNode();
UnaryOPVFGNode *unaryOPVFGNode = new UnaryOPVFGNode();
PHIVFGNode *pHIVFGNode = new PHIVFGNode();
IntraPHIVFGNode *intraPHIVFGNode = new IntraPHIVFGNode();
InterPHIVFGNode *interPHIVFGNode = new InterPHIVFGNode();
ArgumentVFGNode *argumentVFGNode = new ArgumentVFGNode();
ActualParmVFGNode *actualParmVFGNode = new ActualParmVFGNode();
FormalParmVFGNode *formalParmVFGNode = new FormalParmVFGNode();
ActualRetVFGNode *actualRetVFGNode = new ActualRetVFGNode();
FormalRetVFGNode *formalRetVFGNode = new FormalRetVFGNode();
NullPtrVFGNode *nullPtrVFGNode = new NullPtrVFGNode();
FormalINSVFGNode *formalINSVFGNode = new FormalINSVFGNode();
MRSVFGNode *mRSVFGNode = new MRSVFGNode();
FormalOUTSVFGNode *formalOUTSVFGNode = new FormalOUTSVFGNode();
ActualINSVFGNode *actualINSVFGNode = new ActualINSVFGNode();
ActualOUTSVFGNode *actualOUTSVFGNode = new ActualOUTSVFGNode();
MSSAPHISVFGNode *mSSAPHISVFGNode = new MSSAPHISVFGNode();
IntraMSSAPHISVFGNode *intraMSSAPHISVFGNode = new IntraMSSAPHISVFGNode();
InterMSSAPHISVFGNode *interMSSAPHISVFGNode = new InterMSSAPHISVFGNode();
AddrCGEdge *addrCGEdge = new AddrCGEdge();
ConstraintEdge *constraintEdge = new ConstraintEdge();
CopyCGEdge *copyCGEdge = new CopyCGEdge();
StoreCGEdge *storeCGEdge = new StoreCGEdge();
LoadCGEdge *loadCGEdge = new LoadCGEdge();
GepCGEdge *gepCGEdge = new GepCGEdge();
NormalGepCGEdge *normalGepCGEdge = new NormalGepCGEdge();
VariantGepCGEdge *variantGepCGEdge = new VariantGepCGEdge();
IntraCFGEdge *intraCFGEdge = new IntraCFGEdge();
ICFGEdge *iCFGEdge = new ICFGEdge();
CallCFGEdge *callCFGEdge = new CallCFGEdge();
RetCFGEdge *retCFGEdge = new RetCFGEdge();
CopyPE *copyPE = new CopyPE();
PAGEdge *pAGEdge = new PAGEdge();
CmpPE *cmpPE = new CmpPE();
BinaryOPPE *binaryOPPE = new BinaryOPPE();
UnaryOPPE *unaryOPPE = new UnaryOPPE();
StorePE *storePE = new StorePE();
LoadPE *loadPE = new LoadPE();
GepPE *gepPE = new GepPE();
NormalGepPE *normalGepPE = new NormalGepPE();
VariantGepPE *variantGepPE = new VariantGepPE();
CallPE *callPE = new CallPE();
TDForkPE *tDForkPE = new TDForkPE();
RetPE *retPE = new RetPE();
TDJoinPE *tDJoinPE = new TDJoinPE();
DirectSVFGEdge *directSVFGEdge = new DirectSVFGEdge();
VFGEdge *vFGEdge = new VFGEdge();
IntraDirSVFGEdge *intraDirSVFGEdge = new IntraDirSVFGEdge();
CallDirSVFGEdge *callDirSVFGEdge = new CallDirSVFGEdge();
RetDirSVFGEdge *retDirSVFGEdge = new RetDirSVFGEdge();
IndirectSVFGEdge *indirectSVFGEdge = new IndirectSVFGEdge();
IntraIndSVFGEdge *intraIndSVFGEdge = new IntraIndSVFGEdge();
CallIndSVFGEdge *callIndSVFGEdge = new CallIndSVFGEdge();
RetIndSVFGEdge *retIndSVFGEdge = new RetIndSVFGEdge();
ThreadMHPIndSVFGEdge *threadMHPIndSVFGEdge = new ThreadMHPIndSVFGEdge();
// END OF object (de)allocation

// BEGIN OF cast testing
ASSERT_TRUE(llvm::isa<ICFGNode>(globalBlockNode));
ASSERT_NE(llvm::dyn_cast<ICFGNode>(globalBlockNode), nullptr);
ASSERT_FALSE(llvm::isa<GlobalBlockNode>(iCFGNode));
ASSERT_EQ(llvm::dyn_cast<GlobalBlockNode>(iCFGNode), nullptr);
ASSERT_TRUE(llvm::isa<ICFGNode>(intraBlockNode));
ASSERT_NE(llvm::dyn_cast<ICFGNode>(intraBlockNode), nullptr);
ASSERT_FALSE(llvm::isa<IntraBlockNode>(iCFGNode));
ASSERT_EQ(llvm::dyn_cast<IntraBlockNode>(iCFGNode), nullptr);
ASSERT_TRUE(llvm::isa<ICFGNode>(interBlockNode));
ASSERT_NE(llvm::dyn_cast<ICFGNode>(interBlockNode), nullptr);
ASSERT_FALSE(llvm::isa<InterBlockNode>(iCFGNode));
ASSERT_EQ(llvm::dyn_cast<InterBlockNode>(iCFGNode), nullptr);
ASSERT_TRUE(llvm::isa<InterBlockNode>(funEntryBlockNode));
ASSERT_NE(llvm::dyn_cast<InterBlockNode>(funEntryBlockNode), nullptr);
ASSERT_FALSE(llvm::isa<FunEntryBlockNode>(interBlockNode));
ASSERT_EQ(llvm::dyn_cast<FunEntryBlockNode>(interBlockNode), nullptr);
ASSERT_TRUE(llvm::isa<InterBlockNode>(funExitBlockNode));
ASSERT_NE(llvm::dyn_cast<InterBlockNode>(funExitBlockNode), nullptr);
ASSERT_FALSE(llvm::isa<FunExitBlockNode>(interBlockNode));
ASSERT_EQ(llvm::dyn_cast<FunExitBlockNode>(interBlockNode), nullptr);
ASSERT_TRUE(llvm::isa<InterBlockNode>(callBlockNode));
ASSERT_NE(llvm::dyn_cast<InterBlockNode>(callBlockNode), nullptr);
ASSERT_FALSE(llvm::isa<CallBlockNode>(interBlockNode));
ASSERT_EQ(llvm::dyn_cast<CallBlockNode>(interBlockNode), nullptr);
ASSERT_TRUE(llvm::isa<InterBlockNode>(retBlockNode));
ASSERT_NE(llvm::dyn_cast<InterBlockNode>(retBlockNode), nullptr);
ASSERT_FALSE(llvm::isa<RetBlockNode>(interBlockNode));
ASSERT_EQ(llvm::dyn_cast<RetBlockNode>(interBlockNode), nullptr);
ASSERT_TRUE(llvm::isa<PAGNode>(valPN));
ASSERT_NE(llvm::dyn_cast<PAGNode>(valPN), nullptr);
ASSERT_TRUE(llvm::isa<ValPN>(gepValPN));
ASSERT_NE(llvm::dyn_cast<ValPN>(gepValPN), nullptr);
ASSERT_FALSE(llvm::isa<GepValPN>(valPN));
ASSERT_EQ(llvm::dyn_cast<GepValPN>(valPN), nullptr);
ASSERT_TRUE(llvm::isa<ValPN>(dummyValPN));
ASSERT_NE(llvm::dyn_cast<ValPN>(dummyValPN), nullptr);
ASSERT_FALSE(llvm::isa<DummyValPN>(valPN));
ASSERT_EQ(llvm::dyn_cast<DummyValPN>(valPN), nullptr);
ASSERT_TRUE(llvm::isa<PAGNode>(objPN));
ASSERT_NE(llvm::dyn_cast<PAGNode>(objPN), nullptr);
ASSERT_TRUE(llvm::isa<ObjPN>(gepObjPN));
ASSERT_NE(llvm::dyn_cast<ObjPN>(gepObjPN), nullptr);
ASSERT_FALSE(llvm::isa<GepObjPN>(objPN));
ASSERT_EQ(llvm::dyn_cast<GepObjPN>(objPN), nullptr);
ASSERT_TRUE(llvm::isa<GepObjPN>(cloneGepObjPN));
ASSERT_NE(llvm::dyn_cast<GepObjPN>(cloneGepObjPN), nullptr);
ASSERT_FALSE(llvm::isa<CloneGepObjPN>(gepObjPN));
ASSERT_EQ(llvm::dyn_cast<CloneGepObjPN>(gepObjPN), nullptr);
ASSERT_TRUE(llvm::isa<ObjPN>(fIObjPN));
ASSERT_NE(llvm::dyn_cast<ObjPN>(fIObjPN), nullptr);
ASSERT_FALSE(llvm::isa<FIObjPN>(objPN));
ASSERT_EQ(llvm::dyn_cast<FIObjPN>(objPN), nullptr);
ASSERT_TRUE(llvm::isa<FIObjPN>(cloneFIObjPN));
ASSERT_NE(llvm::dyn_cast<FIObjPN>(cloneFIObjPN), nullptr);
ASSERT_FALSE(llvm::isa<CloneFIObjPN>(fIObjPN));
ASSERT_EQ(llvm::dyn_cast<CloneFIObjPN>(fIObjPN), nullptr);
ASSERT_TRUE(llvm::isa<ObjPN>(dummyObjPN));
ASSERT_NE(llvm::dyn_cast<ObjPN>(dummyObjPN), nullptr);
ASSERT_FALSE(llvm::isa<DummyObjPN>(objPN));
ASSERT_EQ(llvm::dyn_cast<DummyObjPN>(objPN), nullptr);
ASSERT_TRUE(llvm::isa<DummyObjPN>(cloneDummyObjPN));
ASSERT_NE(llvm::dyn_cast<DummyObjPN>(cloneDummyObjPN), nullptr);
ASSERT_FALSE(llvm::isa<CloneDummyObjPN>(dummyObjPN));
ASSERT_EQ(llvm::dyn_cast<CloneDummyObjPN>(dummyObjPN), nullptr);
ASSERT_TRUE(llvm::isa<PAGNode>(retPN));
ASSERT_NE(llvm::dyn_cast<PAGNode>(retPN), nullptr);
ASSERT_TRUE(llvm::isa<PAGNode>(varArgPN));
ASSERT_NE(llvm::dyn_cast<PAGNode>(varArgPN), nullptr);
ASSERT_TRUE(llvm::isa<VFGNode>(stmtVFGNode));
ASSERT_NE(llvm::dyn_cast<VFGNode>(stmtVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<StmtVFGNode>(vFGNode));
ASSERT_EQ(llvm::dyn_cast<StmtVFGNode>(vFGNode), nullptr);
ASSERT_TRUE(llvm::isa<StmtVFGNode>(loadVFGNode));
ASSERT_NE(llvm::dyn_cast<StmtVFGNode>(loadVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<LoadVFGNode>(stmtVFGNode));
ASSERT_EQ(llvm::dyn_cast<LoadVFGNode>(stmtVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<StmtVFGNode>(storeVFGNode));
ASSERT_NE(llvm::dyn_cast<StmtVFGNode>(storeVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<StoreVFGNode>(stmtVFGNode));
ASSERT_EQ(llvm::dyn_cast<StoreVFGNode>(stmtVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<StmtVFGNode>(copyVFGNode));
ASSERT_NE(llvm::dyn_cast<StmtVFGNode>(copyVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<CopyVFGNode>(stmtVFGNode));
ASSERT_EQ(llvm::dyn_cast<CopyVFGNode>(stmtVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<StmtVFGNode>(gepVFGNode));
ASSERT_NE(llvm::dyn_cast<StmtVFGNode>(gepVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<GepVFGNode>(stmtVFGNode));
ASSERT_EQ(llvm::dyn_cast<GepVFGNode>(stmtVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<StmtVFGNode>(addrVFGNode));
ASSERT_NE(llvm::dyn_cast<StmtVFGNode>(addrVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<AddrVFGNode>(stmtVFGNode));
ASSERT_EQ(llvm::dyn_cast<AddrVFGNode>(stmtVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<VFGNode>(cmpVFGNode));
ASSERT_NE(llvm::dyn_cast<VFGNode>(cmpVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<CmpVFGNode>(vFGNode));
ASSERT_EQ(llvm::dyn_cast<CmpVFGNode>(vFGNode), nullptr);
ASSERT_TRUE(llvm::isa<VFGNode>(binaryOPVFGNode));
ASSERT_NE(llvm::dyn_cast<VFGNode>(binaryOPVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<BinaryOPVFGNode>(vFGNode));
ASSERT_EQ(llvm::dyn_cast<BinaryOPVFGNode>(vFGNode), nullptr);
ASSERT_TRUE(llvm::isa<VFGNode>(unaryOPVFGNode));
ASSERT_NE(llvm::dyn_cast<VFGNode>(unaryOPVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<UnaryOPVFGNode>(vFGNode));
ASSERT_EQ(llvm::dyn_cast<UnaryOPVFGNode>(vFGNode), nullptr);
ASSERT_TRUE(llvm::isa<VFGNode>(pHIVFGNode));
ASSERT_NE(llvm::dyn_cast<VFGNode>(pHIVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<PHIVFGNode>(vFGNode));
ASSERT_EQ(llvm::dyn_cast<PHIVFGNode>(vFGNode), nullptr);
ASSERT_TRUE(llvm::isa<PHIVFGNode>(intraPHIVFGNode));
ASSERT_NE(llvm::dyn_cast<PHIVFGNode>(intraPHIVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<IntraPHIVFGNode>(pHIVFGNode));
ASSERT_EQ(llvm::dyn_cast<IntraPHIVFGNode>(pHIVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<PHIVFGNode>(interPHIVFGNode));
ASSERT_NE(llvm::dyn_cast<PHIVFGNode>(interPHIVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<InterPHIVFGNode>(pHIVFGNode));
ASSERT_EQ(llvm::dyn_cast<InterPHIVFGNode>(pHIVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<VFGNode>(argumentVFGNode));
ASSERT_NE(llvm::dyn_cast<VFGNode>(argumentVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<ArgumentVFGNode>(vFGNode));
ASSERT_EQ(llvm::dyn_cast<ArgumentVFGNode>(vFGNode), nullptr);
ASSERT_TRUE(llvm::isa<ArgumentVFGNode>(actualParmVFGNode));
ASSERT_NE(llvm::dyn_cast<ArgumentVFGNode>(actualParmVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<ActualParmVFGNode>(argumentVFGNode));
ASSERT_EQ(llvm::dyn_cast<ActualParmVFGNode>(argumentVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<ArgumentVFGNode>(formalParmVFGNode));
ASSERT_NE(llvm::dyn_cast<ArgumentVFGNode>(formalParmVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<FormalParmVFGNode>(argumentVFGNode));
ASSERT_EQ(llvm::dyn_cast<FormalParmVFGNode>(argumentVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<ArgumentVFGNode>(actualRetVFGNode));
ASSERT_NE(llvm::dyn_cast<ArgumentVFGNode>(actualRetVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<ActualRetVFGNode>(argumentVFGNode));
ASSERT_EQ(llvm::dyn_cast<ActualRetVFGNode>(argumentVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<ArgumentVFGNode>(formalRetVFGNode));
ASSERT_NE(llvm::dyn_cast<ArgumentVFGNode>(formalRetVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<FormalRetVFGNode>(argumentVFGNode));
ASSERT_EQ(llvm::dyn_cast<FormalRetVFGNode>(argumentVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<VFGNode>(nullPtrVFGNode));
ASSERT_NE(llvm::dyn_cast<VFGNode>(nullPtrVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<NullPtrVFGNode>(vFGNode));
ASSERT_EQ(llvm::dyn_cast<NullPtrVFGNode>(vFGNode), nullptr);
ASSERT_TRUE(llvm::isa<MRSVFGNode>(formalINSVFGNode));
ASSERT_NE(llvm::dyn_cast<MRSVFGNode>(formalINSVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<FormalINSVFGNode>(mRSVFGNode));
ASSERT_EQ(llvm::dyn_cast<FormalINSVFGNode>(mRSVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<MRSVFGNode>(formalOUTSVFGNode));
ASSERT_NE(llvm::dyn_cast<MRSVFGNode>(formalOUTSVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<FormalOUTSVFGNode>(mRSVFGNode));
ASSERT_EQ(llvm::dyn_cast<FormalOUTSVFGNode>(mRSVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<MRSVFGNode>(actualINSVFGNode));
ASSERT_NE(llvm::dyn_cast<MRSVFGNode>(actualINSVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<ActualINSVFGNode>(mRSVFGNode));
ASSERT_EQ(llvm::dyn_cast<ActualINSVFGNode>(mRSVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<MRSVFGNode>(actualOUTSVFGNode));
ASSERT_NE(llvm::dyn_cast<MRSVFGNode>(actualOUTSVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<ActualOUTSVFGNode>(mRSVFGNode));
ASSERT_EQ(llvm::dyn_cast<ActualOUTSVFGNode>(mRSVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<MRSVFGNode>(mSSAPHISVFGNode));
ASSERT_NE(llvm::dyn_cast<MRSVFGNode>(mSSAPHISVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<MSSAPHISVFGNode>(mRSVFGNode));
ASSERT_EQ(llvm::dyn_cast<MSSAPHISVFGNode>(mRSVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<MSSAPHISVFGNode>(intraMSSAPHISVFGNode));
ASSERT_NE(llvm::dyn_cast<MSSAPHISVFGNode>(intraMSSAPHISVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<IntraMSSAPHISVFGNode>(mSSAPHISVFGNode));
ASSERT_EQ(llvm::dyn_cast<IntraMSSAPHISVFGNode>(mSSAPHISVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<MSSAPHISVFGNode>(interMSSAPHISVFGNode));
ASSERT_NE(llvm::dyn_cast<MSSAPHISVFGNode>(interMSSAPHISVFGNode), nullptr);
ASSERT_FALSE(llvm::isa<InterMSSAPHISVFGNode>(mSSAPHISVFGNode));
ASSERT_EQ(llvm::dyn_cast<InterMSSAPHISVFGNode>(mSSAPHISVFGNode), nullptr);
ASSERT_TRUE(llvm::isa<ConstraintEdge>(addrCGEdge));
ASSERT_NE(llvm::dyn_cast<ConstraintEdge>(addrCGEdge), nullptr);
ASSERT_FALSE(llvm::isa<AddrCGEdge>(constraintEdge));
ASSERT_EQ(llvm::dyn_cast<AddrCGEdge>(constraintEdge), nullptr);
ASSERT_TRUE(llvm::isa<ConstraintEdge>(copyCGEdge));
ASSERT_NE(llvm::dyn_cast<ConstraintEdge>(copyCGEdge), nullptr);
ASSERT_FALSE(llvm::isa<CopyCGEdge>(constraintEdge));
ASSERT_EQ(llvm::dyn_cast<CopyCGEdge>(constraintEdge), nullptr);
ASSERT_TRUE(llvm::isa<ConstraintEdge>(storeCGEdge));
ASSERT_NE(llvm::dyn_cast<ConstraintEdge>(storeCGEdge), nullptr);
ASSERT_FALSE(llvm::isa<StoreCGEdge>(constraintEdge));
ASSERT_EQ(llvm::dyn_cast<StoreCGEdge>(constraintEdge), nullptr);
ASSERT_TRUE(llvm::isa<ConstraintEdge>(loadCGEdge));
ASSERT_NE(llvm::dyn_cast<ConstraintEdge>(loadCGEdge), nullptr);
ASSERT_FALSE(llvm::isa<LoadCGEdge>(constraintEdge));
ASSERT_EQ(llvm::dyn_cast<LoadCGEdge>(constraintEdge), nullptr);
ASSERT_TRUE(llvm::isa<ConstraintEdge>(gepCGEdge));
ASSERT_NE(llvm::dyn_cast<ConstraintEdge>(gepCGEdge), nullptr);
ASSERT_FALSE(llvm::isa<GepCGEdge>(constraintEdge));
ASSERT_EQ(llvm::dyn_cast<GepCGEdge>(constraintEdge), nullptr);
ASSERT_TRUE(llvm::isa<GepCGEdge>(normalGepCGEdge));
ASSERT_NE(llvm::dyn_cast<GepCGEdge>(normalGepCGEdge), nullptr);
ASSERT_FALSE(llvm::isa<NormalGepCGEdge>(gepCGEdge));
ASSERT_EQ(llvm::dyn_cast<NormalGepCGEdge>(gepCGEdge), nullptr);
ASSERT_TRUE(llvm::isa<GepCGEdge>(variantGepCGEdge));
ASSERT_NE(llvm::dyn_cast<GepCGEdge>(variantGepCGEdge), nullptr);
ASSERT_FALSE(llvm::isa<VariantGepCGEdge>(gepCGEdge));
ASSERT_EQ(llvm::dyn_cast<VariantGepCGEdge>(gepCGEdge), nullptr);
ASSERT_TRUE(llvm::isa<ICFGEdge>(intraCFGEdge));
ASSERT_NE(llvm::dyn_cast<ICFGEdge>(intraCFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<IntraCFGEdge>(iCFGEdge));
ASSERT_EQ(llvm::dyn_cast<IntraCFGEdge>(iCFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<ICFGEdge>(callCFGEdge));
ASSERT_NE(llvm::dyn_cast<ICFGEdge>(callCFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<CallCFGEdge>(iCFGEdge));
ASSERT_EQ(llvm::dyn_cast<CallCFGEdge>(iCFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<ICFGEdge>(retCFGEdge));
ASSERT_NE(llvm::dyn_cast<ICFGEdge>(retCFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<RetCFGEdge>(iCFGEdge));
ASSERT_EQ(llvm::dyn_cast<RetCFGEdge>(iCFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(copyPE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(copyPE), nullptr);
ASSERT_FALSE(llvm::isa<CopyPE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<CopyPE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(cmpPE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(cmpPE), nullptr);
ASSERT_FALSE(llvm::isa<CmpPE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<CmpPE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(binaryOPPE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(binaryOPPE), nullptr);
ASSERT_FALSE(llvm::isa<BinaryOPPE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<BinaryOPPE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(unaryOPPE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(unaryOPPE), nullptr);
ASSERT_FALSE(llvm::isa<UnaryOPPE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<UnaryOPPE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(storePE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(storePE), nullptr);
ASSERT_FALSE(llvm::isa<StorePE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<StorePE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(loadPE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(loadPE), nullptr);
ASSERT_FALSE(llvm::isa<LoadPE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<LoadPE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(gepPE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(gepPE), nullptr);
ASSERT_FALSE(llvm::isa<GepPE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<GepPE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<GepPE>(normalGepPE));
ASSERT_NE(llvm::dyn_cast<GepPE>(normalGepPE), nullptr);
ASSERT_FALSE(llvm::isa<NormalGepPE>(gepPE));
ASSERT_EQ(llvm::dyn_cast<NormalGepPE>(gepPE), nullptr);
ASSERT_TRUE(llvm::isa<GepPE>(variantGepPE));
ASSERT_NE(llvm::dyn_cast<GepPE>(variantGepPE), nullptr);
ASSERT_FALSE(llvm::isa<VariantGepPE>(gepPE));
ASSERT_EQ(llvm::dyn_cast<VariantGepPE>(gepPE), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(callPE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(callPE), nullptr);
ASSERT_FALSE(llvm::isa<CallPE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<CallPE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<CallPE>(tDForkPE));
ASSERT_NE(llvm::dyn_cast<CallPE>(tDForkPE), nullptr);
ASSERT_FALSE(llvm::isa<TDForkPE>(callPE));
ASSERT_EQ(llvm::dyn_cast<TDForkPE>(callPE), nullptr);
ASSERT_TRUE(llvm::isa<PAGEdge>(retPE));
ASSERT_NE(llvm::dyn_cast<PAGEdge>(retPE), nullptr);
ASSERT_FALSE(llvm::isa<RetPE>(pAGEdge));
ASSERT_EQ(llvm::dyn_cast<RetPE>(pAGEdge), nullptr);
ASSERT_TRUE(llvm::isa<RetPE>(tDJoinPE));
ASSERT_NE(llvm::dyn_cast<RetPE>(tDJoinPE), nullptr);
ASSERT_FALSE(llvm::isa<TDJoinPE>(retPE));
ASSERT_EQ(llvm::dyn_cast<TDJoinPE>(retPE), nullptr);
ASSERT_TRUE(llvm::isa<VFGEdge>(directSVFGEdge));
ASSERT_NE(llvm::dyn_cast<VFGEdge>(directSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<DirectSVFGEdge>(vFGEdge));
ASSERT_EQ(llvm::dyn_cast<DirectSVFGEdge>(vFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<DirectSVFGEdge>(intraDirSVFGEdge));
ASSERT_NE(llvm::dyn_cast<DirectSVFGEdge>(intraDirSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<IntraDirSVFGEdge>(directSVFGEdge));
ASSERT_EQ(llvm::dyn_cast<IntraDirSVFGEdge>(directSVFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<DirectSVFGEdge>(callDirSVFGEdge));
ASSERT_NE(llvm::dyn_cast<DirectSVFGEdge>(callDirSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<CallDirSVFGEdge>(directSVFGEdge));
ASSERT_EQ(llvm::dyn_cast<CallDirSVFGEdge>(directSVFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<DirectSVFGEdge>(retDirSVFGEdge));
ASSERT_NE(llvm::dyn_cast<DirectSVFGEdge>(retDirSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<RetDirSVFGEdge>(directSVFGEdge));
ASSERT_EQ(llvm::dyn_cast<RetDirSVFGEdge>(directSVFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<VFGEdge>(indirectSVFGEdge));
ASSERT_NE(llvm::dyn_cast<VFGEdge>(indirectSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<IndirectSVFGEdge>(vFGEdge));
ASSERT_EQ(llvm::dyn_cast<IndirectSVFGEdge>(vFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<IndirectSVFGEdge>(intraIndSVFGEdge));
ASSERT_NE(llvm::dyn_cast<IndirectSVFGEdge>(intraIndSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<IntraIndSVFGEdge>(indirectSVFGEdge));
ASSERT_EQ(llvm::dyn_cast<IntraIndSVFGEdge>(indirectSVFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<IndirectSVFGEdge>(callIndSVFGEdge));
ASSERT_NE(llvm::dyn_cast<IndirectSVFGEdge>(callIndSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<CallIndSVFGEdge>(indirectSVFGEdge));
ASSERT_EQ(llvm::dyn_cast<CallIndSVFGEdge>(indirectSVFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<IndirectSVFGEdge>(retIndSVFGEdge));
ASSERT_NE(llvm::dyn_cast<IndirectSVFGEdge>(retIndSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<RetIndSVFGEdge>(indirectSVFGEdge));
ASSERT_EQ(llvm::dyn_cast<RetIndSVFGEdge>(indirectSVFGEdge), nullptr);
ASSERT_TRUE(llvm::isa<IndirectSVFGEdge>(threadMHPIndSVFGEdge));
ASSERT_NE(llvm::dyn_cast<IndirectSVFGEdge>(threadMHPIndSVFGEdge), nullptr);
ASSERT_FALSE(llvm::isa<ThreadMHPIndSVFGEdge>(indirectSVFGEdge));
ASSERT_EQ(llvm::dyn_cast<ThreadMHPIndSVFGEdge>(indirectSVFGEdge), nullptr);
// END OF cast testing

// BEGIN OF object (de)allocation
delete globalBlockNode;
delete iCFGNode;
delete intraBlockNode;
delete interBlockNode;
delete funEntryBlockNode;
delete funExitBlockNode;
delete callBlockNode;
delete retBlockNode;
delete valPN;
delete gepValPN;
delete dummyValPN;
delete objPN;
delete gepObjPN;
delete cloneGepObjPN;
delete fIObjPN;
delete cloneFIObjPN;
delete dummyObjPN;
delete cloneDummyObjPN;
delete retPN;
delete varArgPN;
delete stmtVFGNode;
delete vFGNode;
delete loadVFGNode;
delete storeVFGNode;
delete copyVFGNode;
delete gepVFGNode;
delete addrVFGNode;
delete cmpVFGNode;
delete binaryOPVFGNode;
delete unaryOPVFGNode;
delete pHIVFGNode;
delete intraPHIVFGNode;
delete interPHIVFGNode;
delete argumentVFGNode;
delete actualParmVFGNode;
delete formalParmVFGNode;
delete actualRetVFGNode;
delete formalRetVFGNode;
delete nullPtrVFGNode;
delete formalINSVFGNode;
delete mRSVFGNode;
delete formalOUTSVFGNode;
delete actualINSVFGNode;
delete actualOUTSVFGNode;
delete mSSAPHISVFGNode;
delete intraMSSAPHISVFGNode;
delete interMSSAPHISVFGNode;
delete addrCGEdge;
delete constraintEdge;
delete copyCGEdge;
delete storeCGEdge;
delete loadCGEdge;
delete gepCGEdge;
delete normalGepCGEdge;
delete variantGepCGEdge;
delete intraCFGEdge;
delete iCFGEdge;
delete callCFGEdge;
delete retCFGEdge;
delete copyPE;
delete pAGEdge;
delete cmpPE;
delete binaryOPPE;
delete unaryOPPE;
delete storePE;
delete loadPE;
delete gepPE;
delete normalGepPE;
delete variantGepPE;
delete callPE;
delete tDForkPE;
delete retPE;
delete tDJoinPE;
delete directSVFGEdge;
delete vFGEdge;
delete intraDirSVFGEdge;
delete callDirSVFGEdge;
delete retDirSVFGEdge;
delete indirectSVFGEdge;
delete intraIndSVFGEdge;
delete callIndSVFGEdge;
delete retIndSVFGEdge;
delete threadMHPIndSVFGEdge;
// END OF object (de)allocation
