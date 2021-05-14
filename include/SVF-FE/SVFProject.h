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

#include "Util/BasicTypes.h"

using namespace std;

namespace SVF {

class PAG;
class ICFG;
class SymbolTableInfo;
class SVFModule;
class LLVMModuleSet;
class ThreadAPI;
class MemObj;

class SVFProject {
  private:
    vector<string> modNameVec;
    SymbolTableInfo *symTableInfo = nullptr;
    SVFModule *svfModule = nullptr;

    PAG *pag = nullptr;
    ICFG *icfg = nullptr;

    ThreadAPI *threadAPI = nullptr;
    // bool _built = false;
    static SVFProject *currentProject;

  public:
    explicit SVFProject(string &modName);
    explicit SVFProject(vector<string> &modVec);
    explicit SVFProject(Module &module);

    static SVFProject *getCurrentProject() { return currentProject; }
    static void setCurrentProject(SVFProject *proj) { currentProject = proj; }

    virtual ~SVFProject();

    SVFModule *getSVFModule() const { return svfModule; }

    SymbolTableInfo *getSymbolTableInfo();

    LLVMModuleSet *getLLVMModSet() const;

    PAG *getPAG();

    ICFG *getICFG();

    ThreadAPI *getThreadAPI();

    /// Return true if this is a thread creation call
    ///@{
    bool isThreadForkCall(const CallSite cs);
    bool isThreadForkCall(const Instruction *inst);
    //@}

    /// Return true if this is a hare_parallel_for call
    ///@{
    bool isHareParForCall(const CallSite cs);
    bool isHareParForCall(const Instruction *inst);
    //@}

    /// Return true if this is a thread join call
    ///@{
    bool isThreadJoinCall(const CallSite cs);
    bool isThreadJoinCall(const Instruction *inst);
    //@}

    /// Return true if this is a thread exit call
    ///@{
    bool isThreadExitCall(const CallSite cs);
    bool isThreadExitCall(const Instruction *inst);
    //@}

    /// Return true if this is a lock acquire call
    ///@{
    bool isLockAquireCall(const CallSite cs);
    bool isLockAquireCall(const Instruction *inst);
    //@}

    /// Return true if this is a lock acquire call
    ///@{
    bool isLockReleaseCall(const CallSite cs);
    bool isLockReleaseCall(const Instruction *inst);
    //@}

    /// Return true if this is a barrier wait call
    //@{
    bool isBarrierWaitCall(const CallSite cs);
    bool isBarrierWaitCall(const Instruction *inst);
    //@}

    /// Return thread fork function
    //@{
    const Value *getForkedFun(const CallSite cs);
    const Value *getForkedFun(const Instruction *inst);
    //@}

    /// Return sole argument of the thread routine
    //@{
    const Value *getActualParmAtForkSite(const CallSite cs);
    const Value *getActualParmAtForkSite(const Instruction *inst);
    //@}

    /// Return the task function of the parallel_for routine
    //@{
    const Value *getTaskFuncAtHareParForSite(const CallSite cs);
    const Value *getTaskFuncAtHareParForSite(const Instruction *inst);
    //@}

    /// Return the task data argument of the parallel_for rountine
    //@{
    const Value *getTaskDataAtHareParForSite(const CallSite cs);
    const Value *getTaskDataAtHareParForSite(const Instruction *inst);
    //@}
};

const Value *getValueByIdFromCurrentProject(SymID id);
SymID getIdByValueFromCurrentProject(const Value *value);
const MemObj *getMemObjByIdFromCurrentProject(SymID id);
SymID getIdByMemObjFromCurrentProject(const MemObj *memObj);
} // end of namespace SVF

#endif // __SVFPROJECT_H_
