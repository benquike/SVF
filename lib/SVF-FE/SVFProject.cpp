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

using namespace SVF;

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
    delete icfg;
    delete pag;
    delete symTableInfo;
    delete svfModule;
}
