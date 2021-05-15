//===- CallGraphBuilder.cpp
//----------------------------------------------------------------//
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
 * CallGraphBuilder.cpp
 *
 *  Created on:
 *      Author: Yulei
 */

#include "SVF-FE/CallGraphBuilder.h"
#include "Graphs/ICFG.h"
#include "SVF-FE/LLVMUtil.h"

using namespace SVF;
using namespace SVFUtil;

PTACallGraph *CallGraphBuilder::buildCallGraph() {
    /// create nodes
    SVFModule *svfModule = proj->getSVFModule();
    LLVMModuleSet *modSet = svfModule->getLLVMModSet();

    /// Step 1. add all functions in the module as nodes to the callgraph
    auto E = svfModule->llvmFunEnd();
    for (auto F = svfModule->llvmFunBegin(); F != E; ++F) {
        const SVFFunction *fun = modSet->getSVFFunction(*F);
        callgraph->addCallGraphNode(fun);
        spdlog::debug("Added Node for : {0}", fun->getName().str());
    }

    /// Step 2. create edges by looking through the direct
    /// call edges
    ThreadAPI *tdAPI = proj->getThreadAPI();
    for (auto F = svfModule->llvmFunBegin(); F != E; ++F) {
        Function *fun = *F;
        auto J = inst_end(*fun);
        for (inst_iterator I = inst_begin(*fun); I != J; ++I) {
            const Instruction *inst = &*I;

            /// how to handle if it is both a thread API
            /// and a normal function
            if (SVFUtil::isNonInstricCallSite(inst)) {
                if (const SVFFunction *callee = getCallee(modSet, inst)) {
                    const CallBlockNode *callBlockNode =
                        icfg->getCallBlockNode(inst);
                    const SVFFunction *caller = modSet->getSVFFunction(fun);
                    callgraph->addDirectCallGraphEdge(callBlockNode, caller,
                                                      callee);
                    spdlog::debug("Added DirectCallEdge: {0} -> {1}",
                                  caller->getName().str(),
                                  callee->getName().str());
                }
            }
            /// merge the ThreadCallGraphBuilder logic here
            if (includeThreadCall) {
                auto *cg = llvm::dyn_cast<ThreadCallGraph>(callgraph);
                if (tdAPI->isTDFork(inst)) {
                    const CallBlockNode *cs = icfg->getCallBlockNode(inst);
                    cg->addForksite(cs);
                    const auto *forkee =
                        llvm::dyn_cast<Function>(tdAPI->getForkedFun(inst));
                    if (forkee) {
                        cg->addDirectForkEdge(cs);
                    }
                    // indirect call to the start routine function
                    else {
                        // here a call to addThreadForkEdgeSetMap
                        // does nothing, if edge is nullptr
                        cg->addThreadForkEdgeSetMap(cs, nullptr);
                    }
                } else if (tdAPI->isHareParFor(inst)) {
                    const CallBlockNode *cs = icfg->getCallBlockNode(inst);
                    cg->addParForSite(cs);
                    const auto *taskFunc = llvm::dyn_cast<Function>(
                        tdAPI->getTaskFuncAtHareParForSite(inst));
                    if (taskFunc) {
                        cg->addDirectParForEdge(cs);
                    }
                    // indirect call to the start routine function
                    else {
                        cg->addHareParForEdgeSetMap(cs, nullptr);
                    }
                } else if (tdAPI->isTDJoin(inst)) {
                    const CallBlockNode *cs = icfg->getCallBlockNode(inst);
                    cg->addJoinsite(cs);
                }
            }
        }
    }

    return callgraph;
}
