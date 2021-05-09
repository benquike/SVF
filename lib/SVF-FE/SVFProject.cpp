/******************************************************************************
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * Author:
 *     Hui Peng <peng124@purdue.edu>
 * Date:
 *     2021-03-19
 *****************************************************************************/

#include "SVF-FE/SVFProject.h"
#include "Graphs/PAG.h"

namespace SVF {

SVFProject *SVFProject::currentProject = nullptr;

SVFProject::SVFProject(string &modName) {
    modNameVec.push_back(modName);
    svfModule = new SVFModule(modNameVec);
    currentProject = this;
}

SVFProject::SVFProject(vector<string> &modVec) {
    assert(!modVec.empty() && "no module files are provided");
    modNameVec.insert(modNameVec.end(), modVec.begin(), modVec.end());
    svfModule = new SVFModule(modNameVec);
    currentProject = this;
}

SVFProject::SVFProject(Module &module) {
    modNameVec.push_back(module.getModuleIdentifier());
    svfModule = new SVFModule(module);
    currentProject = this;
}

SymbolTableInfo *SVFProject::getSymbolTableInfo() {

    if (!symTableInfo) {
        symTableInfo = new SymbolTableInfo(getSVFModule());
    }

    return symTableInfo;
}

LLVMModuleSet *SVFProject::getLLVMModSet() const {
    return svfModule->getLLVMModSet();
}

ThreadAPI *SVFProject::getThreadAPI() {
    if (!threadAPI) {
        threadAPI = new ThreadAPI(getSVFModule());
    }

    return threadAPI;
}

PAG *SVFProject::getPAG() {
    if (!pag) {
        // TODO: add support for options
        pag = new PAG(this);
    }

    return pag;
}

ICFG *SVFProject::getICFG() {
    if (!icfg) {
        icfg = getPAG()->getICFG();
    }

    return icfg;
}

SVFProject::~SVFProject() {
    delete threadAPI;
    delete pag;
    delete symTableInfo;
    delete svfModule;
}

/// Return true if this is a thread creation call
///@{
bool SVFProject::isThreadForkCall(const CallSite cs) {
    return getThreadAPI()->isTDFork(cs);
}
bool SVFProject::isThreadForkCall(const Instruction *inst) {
    return getThreadAPI()->isTDFork(inst);
}
//@}

/// Return true if this is a hare_parallel_for call
///@{
bool SVFProject::isHareParForCall(const CallSite cs) {
    return getThreadAPI()->isHareParFor(cs);
}
bool SVFProject::isHareParForCall(const Instruction *inst) {
    return getThreadAPI()->isHareParFor(inst);
}
//@}

/// Return true if this is a thread join call
///@{
bool SVFProject::isThreadJoinCall(const CallSite cs) {
    return getThreadAPI()->isTDJoin(cs);
}
bool SVFProject::isThreadJoinCall(const Instruction *inst) {
    return getThreadAPI()->isTDJoin(inst);
}
//@}

/// Return true if this is a thread exit call
///@{
bool SVFProject::isThreadExitCall(const CallSite cs) {
    return getThreadAPI()->isTDExit(cs);
}
bool SVFProject::isThreadExitCall(const Instruction *inst) {
    return getThreadAPI()->isTDExit(inst);
}
//@}

/// Return true if this is a lock acquire call
///@{
bool SVFProject::isLockAquireCall(const CallSite cs) {
    return getThreadAPI()->isTDAcquire(cs);
}
bool SVFProject::isLockAquireCall(const Instruction *inst) {
    return getThreadAPI()->isTDAcquire(inst);
}
//@}

/// Return true if this is a lock acquire call
///@{
bool SVFProject::isLockReleaseCall(const CallSite cs) {
    return getThreadAPI()->isTDRelease(cs);
}
bool SVFProject::isLockReleaseCall(const Instruction *inst) {
    return getThreadAPI()->isTDRelease(inst);
}
//@}

/// Return true if this is a barrier wait call
//@{
bool SVFProject::isBarrierWaitCall(const CallSite cs) {
    return getThreadAPI()->isTDBarWait(cs);
}
bool SVFProject::isBarrierWaitCall(const Instruction *inst) {
    return getThreadAPI()->isTDBarWait(inst);
}
//@}

/// Return thread fork function
//@{
const Value *SVFProject::getForkedFun(const CallSite cs) {
    return getThreadAPI()->getForkedFun(cs);
}
const Value *SVFProject::getForkedFun(const Instruction *inst) {
    return getThreadAPI()->getForkedFun(inst);
}
//@}

/// Return sole argument of the thread routine
//@{
const Value *SVFProject::getActualParmAtForkSite(const CallSite cs) {
    return getThreadAPI()->getActualParmAtForkSite(cs);
}
const Value *SVFProject::getActualParmAtForkSite(const Instruction *inst) {
    return getThreadAPI()->getActualParmAtForkSite(inst);
}
//@}

/// Return the task function of the parallel_for routine
//@{
const Value *SVFProject::getTaskFuncAtHareParForSite(const CallSite cs) {
    return getThreadAPI()->getTaskFuncAtHareParForSite(cs);
}
const Value *SVFProject::getTaskFuncAtHareParForSite(const Instruction *inst) {
    return getThreadAPI()->getTaskFuncAtHareParForSite(inst);
}
//@}

/// Return the task data argument of the parallel_for rountine
//@{
const Value *SVFProject::getTaskDataAtHareParForSite(const CallSite cs) {
    return getThreadAPI()->getTaskDataAtHareParForSite(cs);
}
const Value *SVFProject::getTaskDataAtHareParForSite(const Instruction *inst) {
    return getThreadAPI()->getTaskDataAtHareParForSite(inst);
}
//@}

const Value *getValueByIdFromCurrentProject(SymID id) {
    SVFProject *currProject = SVFProject::getCurrentProject();
    assert(currProject != nullptr && "current project is null");
    auto *symTable = currProject->getSymbolTableInfo();
    auto &id2Val = symTable->idToValSym();
    assert(id2Val.find(id) != id2Val.end() && "id does not exist");
    return id2Val[id];
}

SymID getIdByValueFromCurrentProject(const Value *value) {
    SVFProject *currProject = SVFProject::getCurrentProject();
    assert(currProject != nullptr && "current project is null");
    auto *symTable = currProject->getSymbolTableInfo();
    assert(symTable->hasValSym(value) && "value does not exist");
    return symTable->getValSymId(value);
}

const MemObj *getMemObjByIdFromCurrentProject(SymID id) {
    SVFProject *currProject = SVFProject::getCurrentProject();
    assert(currProject != nullptr && "current project is null");
    auto *symTable = currProject->getSymbolTableInfo();

    return symTable->getMemObj(id);
}

SymID getIdByMemObjFromCurrentProject(const MemObj *memObj) {
    SVFProject *currProject = SVFProject::getCurrentProject();
    assert(currProject != nullptr && "current project is null");
    auto *symTable = currProject->getSymbolTableInfo();

    return symTable->getMemObjId(memObj);
}

const Type *getTypeByIdFromCurrentProject(SymID id) {
    SVFProject *currProject = SVFProject::getCurrentProject();
    assert(currProject != nullptr && "current project is null");
    auto *symTable = currProject->getSymbolTableInfo();

    return symTable->getType(id);
}

SymID getIdByTypeFromCurrentProject(const Type *type) {
    SVFProject *currProject = SVFProject::getCurrentProject();
    assert(currProject != nullptr && "current project is null");
    auto *symTable = currProject->getSymbolTableInfo();

    return symTable->getTypeId(type);
}

} // end of namespace SVF
