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

#ifndef __SVFPROJECT_H_
#define __SVFPROJECT_H_

#include <string>
#include <vector>

#include "Graphs/ICFG.h"
#include "SVF-FE/SymbolTableInfo.h"
#include "Util/SVFModule.h"
#include "Util/ThreadAPI.h"

namespace SVF {

class PAG;

class SVFProject {
  private:
    vector<string> modNameVec;
    SymbolTableInfo *symTableInfo = nullptr;
    SVFModule *svfModule = nullptr;

    PAG *pag = nullptr;
    ICFG *icfg = nullptr;

    ThreadAPI *threadAPI = nullptr;
    // bool _built = false;

  public:
    SVFProject(string &modName) {
        modNameVec.push_back(modName);
        svfModule = new SVFModule(modName);
    }

    SVFProject(vector<string> &modVec) {
        modNameVec.insert(modNameVec.end(), modVec.begin(), modVec.end());
        svfModule = new SVFModule(modVec);
    }

    SVFProject(Module &module) {
        modNameVec.push_back(module.getModuleIdentifier());
        svfModule = new SVFModule(module);
    }

    virtual ~SVFProject();

    SVFModule *getSVFModule() const { return svfModule; }

    SymbolTableInfo *getSymbolTableInfo() {

        if (!symTableInfo) {
            symTableInfo = new SymbolTableInfo(getSVFModule());
        }

        return symTableInfo;
    }

    LLVMModuleSet *getLLVMModSet() const { return svfModule->getLLVMModSet(); }

    PAG *getPAG();

    ICFG *getICFG();

    ThreadAPI *getThreadAPI() {
        if (!threadAPI) {
            threadAPI = new ThreadAPI(getSVFModule());
        }

        return threadAPI;
    }

    /// Return true if this is a thread creation call
    ///@{
    inline bool isThreadForkCall(const CallSite cs) {
        return getThreadAPI()->isTDFork(cs);
    }
    inline bool isThreadForkCall(const Instruction *inst) {
        return getThreadAPI()->isTDFork(inst);
    }
    //@}

    /// Return true if this is a hare_parallel_for call
    ///@{
    inline bool isHareParForCall(const CallSite cs) {
        return getThreadAPI()->isHareParFor(cs);
    }
    inline bool isHareParForCall(const Instruction *inst) {
        return getThreadAPI()->isHareParFor(inst);
    }
    //@}

    /// Return true if this is a thread join call
    ///@{
    inline bool isThreadJoinCall(const CallSite cs) {
        return getThreadAPI()->isTDJoin(cs);
    }
    inline bool isThreadJoinCall(const Instruction *inst) {
        return getThreadAPI()->isTDJoin(inst);
    }
    //@}

    /// Return true if this is a thread exit call
    ///@{
    inline bool isThreadExitCall(const CallSite cs) {
        return getThreadAPI()->isTDExit(cs);
    }
    inline bool isThreadExitCall(const Instruction *inst) {
        return getThreadAPI()->isTDExit(inst);
    }
    //@}

    /// Return true if this is a lock acquire call
    ///@{
    inline bool isLockAquireCall(const CallSite cs) {
        return getThreadAPI()->isTDAcquire(cs);
    }
    inline bool isLockAquireCall(const Instruction *inst) {
        return getThreadAPI()->isTDAcquire(inst);
    }
    //@}

    /// Return true if this is a lock acquire call
    ///@{
    inline bool isLockReleaseCall(const CallSite cs) {
        return getThreadAPI()->isTDRelease(cs);
    }
    inline bool isLockReleaseCall(const Instruction *inst) {
        return getThreadAPI()->isTDRelease(inst);
    }
    //@}

    /// Return true if this is a barrier wait call
    //@{
    inline bool isBarrierWaitCall(const CallSite cs) {
        return getThreadAPI()->isTDBarWait(cs);
    }
    inline bool isBarrierWaitCall(const Instruction *inst) {
        return getThreadAPI()->isTDBarWait(inst);
    }
    //@}

    /// Return thread fork function
    //@{
    inline const Value *getForkedFun(const CallSite cs) {
        return getThreadAPI()->getForkedFun(cs);
    }
    inline const Value *getForkedFun(const Instruction *inst) {
        return getThreadAPI()->getForkedFun(inst);
    }
    //@}

    /// Return sole argument of the thread routine
    //@{
    inline const Value *getActualParmAtForkSite(const CallSite cs) {
        return getThreadAPI()->getActualParmAtForkSite(cs);
    }
    inline const Value *getActualParmAtForkSite(const Instruction *inst) {
        return getThreadAPI()->getActualParmAtForkSite(inst);
    }
    //@}

    /// Return the task function of the parallel_for routine
    //@{
    inline const Value *getTaskFuncAtHareParForSite(const CallSite cs) {
        return getThreadAPI()->getTaskFuncAtHareParForSite(cs);
    }
    inline const Value *getTaskFuncAtHareParForSite(const Instruction *inst) {
        return getThreadAPI()->getTaskFuncAtHareParForSite(inst);
    }
    //@}

    /// Return the task data argument of the parallel_for rountine
    //@{
    inline const Value *getTaskDataAtHareParForSite(const CallSite cs) {
        return getThreadAPI()->getTaskDataAtHareParForSite(cs);
    }
    inline const Value *getTaskDataAtHareParForSite(const Instruction *inst) {
        return getThreadAPI()->getTaskDataAtHareParForSite(inst);
    }
    //@}
};

} // end of namespace SVF

#endif // __SVFPROJECT_H_
