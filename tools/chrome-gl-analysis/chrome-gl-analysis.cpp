
/*
 // A tool for analysing webgl interface code in chrome
 //
 // Author: Yulei Sui,
 // Hui Peng <peng124@purdue.edu>
 */
#include <iostream>

#include "Graphs/SVFG.h"
#include "SVF-FE/LLVMUtil.h"
#include "SVF-FE/PAGBuilder.h"
#include "WPA/FlowSensitive.h"

using namespace llvm;
using namespace std;
using namespace SVF;

static llvm::cl::opt<std::string>
    InputFilename(cl::Positional, llvm::cl::desc("<input bitcode>"),
                  llvm::cl::init("-"));

static llvm::cl::opt<std::string> Entry("e",
                                        llvm::cl::desc("entry function name"),
                                        llvm::cl::value_desc("entry"));

/**
 * return the SVFFunction with a name `name`
 * */
const SVFFunction *getFunctionByName(SVFModule *mod, string &name) {

    for (auto it = mod->begin(); it != mod->end(); it++) {
        const SVFFunction *fun = *it;
        Function *llvmFun = fun->getLLVMFun();
        if (llvmFun->getName().str() == name) {
            return fun;
        }
    }
    return nullptr;
}

#if 0
BasicBlock *getEntryBlock(Function *llvmFun) {
  BasicBlock *ret = nullptr;
  if (llvmFun->isDeclaration()) {
    cout << llvmFun->getEntryBlock() <<  " is a declaration" << endl;
    return ret;
  }

  if (llvmFun->begin() == llvmFun->end()) {
    cout << "there is no blocks in the function" << endl;
    return ret;
  }

  ret = &(*llvmFun->begin());

  return ret;
}
#endif

/**
 * Traverse the ICFG of a function
 * using worklist algorithm
 * visitor is responsible for handling the results
 **/

template <class InstVisitorClass>
void traverseFunctionICFG(ICFG *icfg, SVFModule *mod, string &functionName,
                          InstVisitorClass &visitor) {
    const SVFFunction *svfFun = getFunctionByName(mod, functionName);
    if (svfFun == nullptr) {
        cout << "function not found" << endl;
        return;
    }

    Function *llvmFun = svfFun->getLLVMFun();

    cout << "traversing the icfg of " << llvmFun->getName().str() << endl;

    BasicBlock *entryBlock = &llvmFun->getEntryBlock();
    Instruction *inst = &entryBlock->front();

    ICFGNode *iNode = icfg->getBlockICFGNode(inst);
    FIFOWorkList<const ICFGNode *> worklist;
    Set<const ICFGNode *> visited;

    worklist.push(iNode);

    while (!worklist.empty()) {
        const ICFGNode *vNode = worklist.pop();
        visited.insert(vNode);

        if (const auto *ibnode = SVFUtil::dyn_cast<IntraBlockNode>(vNode)) {
            const Instruction *llvmInst = ibnode->getInst();
            visitor.visit(const_cast<Instruction *>(llvmInst));
        } else if (const auto *cbnode =
                       SVFUtil::dyn_cast<CallBlockNode>(vNode)) {
            const Instruction *inst = cbnode->getCallSite();
            visitor.visit(const_cast<Instruction *>(inst));
        } else {
            cout << vNode->getId() << " is not an IntraBlockNode, skipping"
                 << endl;
        }

        for (ICFGNode::const_iterator it = vNode->OutEdgeBegin(),
                                      eit = vNode->OutEdgeEnd();
             it != eit; ++it) {
            ICFGEdge *edge = *it;
            ICFGNode *succNode = edge->getDstNode();
            if (visited.find(succNode) == visited.end()) {
                worklist.push(succNode);
            }
        }
    }
}

/*!
 * An example to query/collect all the values flowing to val
 * in the value-flow graph (VFG)
 *
 *
 * this works by going back along the edges of
 * the VFG from val
 */
void traverseOnVFG(const SVFG *vfg, const Value *val) {
    PAG *pag = PAG::getPAG();

    PAGNode *pNode = pag->getPAGNode(pag->getValueNode(val));
    const VFGNode *vNode = vfg->getDefSVFGNode(pNode);
    FIFOWorkList<const VFGNode *> worklist;
    Set<const VFGNode *> visited;
    worklist.push(vNode);

    /// Traverse along VFG
    while (!worklist.empty()) {
        const VFGNode *vNode = worklist.pop();
        visited.insert(vNode);

        for (VFGNode::const_iterator it = vNode->InEdgeBegin(),
                                     eit = vNode->InEdgeEnd();
             it != eit; ++it) {
            VFGEdge *edge = *it;
            VFGNode *succNode = edge->getSrcNode();
            if (visited.find(succNode) == visited.end()) {
                worklist.push(succNode);
            }
        }
    }

    /// Collect all LLVM Values
    for (Set<const VFGNode *>::const_iterator it = visited.begin(),
                                              eit = visited.end();
         it != eit; ++it) {
        const VFGNode *node = *it;
        /// can only query VFGNode involving top-level pointers (starting with %
        /// or
        /// @ in LLVM IR) PAGNode* pNode = vfg->getLHSTopLevPtr(node); Value*
        /// val = pNode->getValue();

        // const PAGNode *pNode = vfg->getLHSTopLevPtr(node);
        // const Value *value = pNode->getValue();

        llvm::outs() << "\t==============================\n";
        llvm::outs() << "\t" << node->toString() << "\n";
        llvm::outs() << "\t==============================\n";
    }
}

class MyBrConditionVisitor : public InstVisitor<MyBrConditionVisitor> {

  private:
    set<const Value *> &res;

  public:
    MyBrConditionVisitor(set<const Value *> &res) : res(res) {}

    void visitBranchInst(BranchInst &brInst) {
        // llvm::outs() << brInst << "\n";
        if (brInst.isConditional()) {
            res.insert(brInst.getCondition());
        }
    }

    void visitSwitchInst(SwitchInst &swInst) {
        res.insert(swInst.getCondition());
    }
};

class MyCallToFuncVisitor : public InstVisitor<MyCallToFuncVisitor> {
  private:
    string &fname;
    set<const Value *> &res;

  public:
    MyCallToFuncVisitor(string &fname, set<const Value *> &res)
        : fname(fname), res(res) {}

    void visitCallInst(CallInst &callInst) {
        // llvm::outs() << callInst << "\n";
        llvm::ImmutableCallSite cs(&callInst);
        if (auto *f = cs.getCalledFunction()) {
            if (f->getName().str() == fname) {
                res.insert(cs.getInstruction());
            }
        }
    }
};

void analyzeArgFlowToCondition(const SVFG *vfg, const Value *val,
                               const Function *function) {
    PAG *pag = PAG::getPAG();

    PAGNode *pNode = pag->getPAGNode(pag->getValueNode(val));
    const VFGNode *vNode = vfg->getDefSVFGNode(pNode);
    FIFOWorkList<const VFGNode *> worklist;
    Set<const VFGNode *> visited;
    worklist.push(vNode);

    /// Traverse along VFG
    while (!worklist.empty()) {
        const VFGNode *vNode = worklist.pop();
        visited.insert(vNode);

        for (auto it = vNode->InEdgeBegin(), eit = vNode->InEdgeEnd();
             it != eit; ++it) {
            VFGEdge *edge = *it;
            VFGNode *prev = edge->getSrcNode();
            if (visited.find(prev) == visited.end()) {
                worklist.push(prev);
            }
        }
    }

    /// Collect all LLVM Values
    for (const auto *node : visited) {

        if (const auto *parmVFGNode =
                SVFUtil::dyn_cast<FormalParmVFGNode>(node)) {
            const PAGNode *param = parmVFGNode->getParam();
            const Function *containingFunc = param->getFunction();

            if (containingFunc != function) {
                continue;
            }

            const Value *llvmValue = param->getValue();

            if (const auto *arg = SVFUtil::dyn_cast<Argument>(llvmValue)) {
                llvm::outs() << "\tArg flowing in#:" << arg->getArgNo() << "\n";
            }
        }
    }
}

int main(int argc, char **argv) {

    int arg_num = 0;
    char **arg_value = new char *[argc];
    std::vector<std::string> moduleNameVec;

    // adding ir files to noduleNameVec
    SVFUtil::processArguments(argc, argv, arg_num, arg_value, moduleNameVec);

    // parse and set other options
    cl::ParseCommandLineOptions(
        arg_num, arg_value,
        "A tool for analyzing webgl interface code in chrome\n");

    SVFModule *svfModule =
        LLVMModuleSet::getLLVMModuleSet()->buildSVFModule(moduleNameVec);

    /// Build Program Assignment Graph (PAG)
    PAGBuilder builder;
    PAG *pag = builder.build(svfModule);

    // Andersen *ander = AndersenWaveDiff::createAndersenWaveDiff(pag);
    FlowSensitive *fs_pta = FlowSensitive::createFSWPA(pag);

    /// Call Graph
    PTACallGraph *callgraph = fs_pta->getPTACallGraph();

    /// ICFG
    ICFG *icfg = pag->getICFG();

    SVFGBuilder svfBuilder;
    SVFG *svfg = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);

    set<const llvm::Value *> conditions;

    string fname = "main";

    if (!Entry.empty()) {
        fname = Entry;
    }

    MyBrConditionVisitor brVisitor(conditions);
    traverseFunctionICFG(icfg, svfModule, fname, brVisitor);
    const SVFFunction *svfFunction = getFunctionByName(svfModule, fname);
    Function *llvmFunction = svfFunction->getLLVMFun();

    for (const auto *c : conditions) {
        llvm::outs() << "condition: " << *c << "\n";
        analyzeArgFlowToCondition(svfg, c, llvmFunction);
    }

    const auto &indCsSet = pag->getIndirectCallsites();

    llvm::outs() << "=============================\n";
    for (const auto &[csBlock, valId] : indCsSet) {
        const auto *dn = pag->getPAGNode(valId);
        llvm::outs() << csBlock->toString() << ":" << dn->toString() << "\n";
    }

#if 0
  set<const llvm::Value *> calls;
  string ffname = "emit_log";
  MyCallToFuncVisitor callVisitor(ffname, calls);
  traverseFunctionICFG(icfg, svfModule, fname, callVisitor);

  for (const auto *c : calls) {
    llvm::outs() << *c << "\n";
    const Instruction *callInst = llvm::dyn_cast<Instruction>(c);
    llvm::ImmutableCallSite cs(callInst);

    const auto *arg = cs.getArgOperand(0);
    llvm::outs() << *arg << "\n";

    if (const auto *gep = llvm::dyn_cast<GetElementPtrInst>(arg)) {
      const Value *ptrOp = gep->getPointerOperand();
      llvm::outs() << "Pointer Operand:" << *ptrOp << "\n";

      if (const auto *gv = llvm::dyn_cast<GlobalVariable>(ptrOp)) {
        if (gv->hasInitializer()) {
          const Constant *c = gv->getInitializer();

          if (const auto *cda = llvm::dyn_cast<ConstantDataArray>(c)) {
            llvm::outs() << cda->getAsCString() << "\n";
          }
        }
      }
    }
  }
#endif

    /// Value-Flow Graph (VFG)
    // VFG *vfg = new VFG(callgraph);
    /// Sparse value-flow graph (SVFG)
    // SVFGBuilder svfBuilder;
    // SVFG *svfg = svfBuilder.buildFullSVFGWithoutOPT(fs_pta);

    /// Collect uses of an LLVM Value
    /// traverseOnVFG(svfg, value);
    /// Collect all successor nodes on ICFG

    // llvm::outs() << "conditions size: " << conditions.size() << "\n";
    // for (const auto *v:conditions) {
    //   llvm::outs() << "traversing svfg from: " << *v << "\n";
    //   traverseOnVFG(svfg, v);
    // }

    /// traverseOnICFG(icfg, value);

    return 0;
}
