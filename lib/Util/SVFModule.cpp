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

#include <string>
#include <vector>

#include "SVF-FE/LLVMModule.h"
#include "Util/SVFModule.h"

using namespace std;

namespace SVF {

/// Constructors
SVFModule::SVFModule(string moduleName) : moduleIdentifier(moduleName) {

    llvmModSet = new LLVMModuleSet(this);
    llvmModSet->buildSVFModule(moduleName);
}

SVFModule::SVFModule(vector<string> &modVec)
    : moduleIdentifier(*modVec.begin()) {

    llvmModSet = new LLVMModuleSet(this);
    llvmModSet->buildSVFModule(modVec);
}

SVFModule::SVFModule(Module &module)
    : moduleIdentifier(module.getModuleIdentifier()) {

    llvmModSet = new LLVMModuleSet(this);
    llvmModSet->buildSVFModule(module);
}

SVFModule::~SVFModule() {
    for (const auto fun : FunctionSet) {
        delete fun;
    }

    delete llvmModSet;
}

} // End namespace SVF
