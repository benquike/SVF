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
 *     2021-04-14
 *****************************************************************************/

#include "Util/Serialization.h"
#include <vector>

// using namespace std;

#if 0
namespace boost {
namespace serialization {

template<typename Archive>
void save(Archive &ar, const NodeBS &bs, unsigned int version) {
    vector<unsigned> set_bits;

    for (auto pos : bs) {
        set_bits.push_back(pos);
    }

    ar & set_bits;
}

template<typename Archive>
void load(Archive &ar, NodeBS &bs, unsigned int version) {
    vector<unsigned> set_bits;
    ar & set_bits;

    for (auto pos : set_bits) {
        bs.set(pos);
    }
}

} // namespace serialization
} // namespace boost

#endif

#include "Graphs/ICFG.h"
#include "Graphs/PAG.h"
#include "Graphs/PTACallGraph.h"
#include "Graphs/SVFG.h"
#include "Graphs/SVFGOPT.h"
#include "Graphs/VFG.h"

// ICFG
BOOST_CLASS_EXPORT(SVF::ICFGNode)
BOOST_CLASS_EXPORT(SVF::GlobalBlockNode)
BOOST_CLASS_EXPORT(SVF::IntraBlockNode)
BOOST_CLASS_EXPORT(SVF::FunEntryBlockNode)
BOOST_CLASS_EXPORT(SVF::FunExitBlockNode)
BOOST_CLASS_EXPORT(SVF::CallBlockNode)
BOOST_CLASS_EXPORT(SVF::RetBlockNode)

BOOST_CLASS_EXPORT(SVF::ICFGEdge)
BOOST_CLASS_EXPORT(SVF::IntraCFGEdge)
BOOST_CLASS_EXPORT(SVF::CallCFGEdge)
BOOST_CLASS_EXPORT(SVF::RetCFGEdge)

BOOST_CLASS_EXPORT(SVF::ICFG)

// PAG
BOOST_CLASS_EXPORT(SVF::PAGNode)
BOOST_CLASS_EXPORT(SVF::ValPN)
BOOST_CLASS_EXPORT(SVF::ObjPN)
BOOST_CLASS_EXPORT(SVF::GepValPN)
BOOST_CLASS_EXPORT(SVF::FIObjPN)
BOOST_CLASS_EXPORT(SVF::RetPN)
BOOST_CLASS_EXPORT(SVF::VarArgPN)
BOOST_CLASS_EXPORT(SVF::DummyValPN)
BOOST_CLASS_EXPORT(SVF::DummyObjPN)
BOOST_CLASS_EXPORT(SVF::CloneDummyObjPN)
BOOST_CLASS_EXPORT(SVF::CloneGepObjPN)
BOOST_CLASS_EXPORT(SVF::CloneFIObjPN)

BOOST_CLASS_EXPORT(SVF::PAGEdge)
BOOST_CLASS_EXPORT(SVF::AddrPE)
BOOST_CLASS_EXPORT(SVF::CopyPE)
BOOST_CLASS_EXPORT(SVF::CmpPE)
BOOST_CLASS_EXPORT(SVF::BinaryOPPE)
BOOST_CLASS_EXPORT(SVF::UnaryOPPE)
BOOST_CLASS_EXPORT(SVF::StorePE)
BOOST_CLASS_EXPORT(SVF::LoadPE)
BOOST_CLASS_EXPORT(SVF::GepPE)
BOOST_CLASS_EXPORT(SVF::NormalGepPE)
BOOST_CLASS_EXPORT(SVF::VariantGepPE)
BOOST_CLASS_EXPORT(SVF::CallPE)
BOOST_CLASS_EXPORT(SVF::RetPE)
BOOST_CLASS_EXPORT(SVF::TDForkPE)
BOOST_CLASS_EXPORT(SVF::TDJoinPE)

BOOST_CLASS_EXPORT(PAG);

// PTA callgraph
BOOST_CLASS_EXPORT(SVF::PTACallGraphEdge)
BOOST_CLASS_EXPORT(SVF::PTACallGraphNode)
BOOST_CLASS_EXPORT(SVF::PTACallGraph)

// VFG
BOOST_CLASS_EXPORT(SVF::VFGNode)
BOOST_CLASS_EXPORT(SVF::StmtVFGNode)
BOOST_CLASS_EXPORT(SVF::LoadVFGNode)
BOOST_CLASS_EXPORT(SVF::StoreVFGNode)
BOOST_CLASS_EXPORT(SVF::CopyVFGNode)
BOOST_CLASS_EXPORT(SVF::CmpVFGNode)
BOOST_CLASS_EXPORT(SVF::BinaryOPVFGNode)
BOOST_CLASS_EXPORT(SVF::UnaryOPVFGNode)
BOOST_CLASS_EXPORT(SVF::GepVFGNode)
BOOST_CLASS_EXPORT(SVF::PHIVFGNode)
BOOST_CLASS_EXPORT(SVF::IntraPHIVFGNode)
BOOST_CLASS_EXPORT(SVF::AddrVFGNode)
BOOST_CLASS_EXPORT(SVF::ArgumentVFGNode)
BOOST_CLASS_EXPORT(SVF::ActualParmVFGNode)
BOOST_CLASS_EXPORT(SVF::FormalParmVFGNode)
BOOST_CLASS_EXPORT(SVF::ActualRetVFGNode)
BOOST_CLASS_EXPORT(SVF::FormalRetVFGNode)
BOOST_CLASS_EXPORT(SVF::InterPHIVFGNode)
BOOST_CLASS_EXPORT(SVF::NullPtrVFGNode)

BOOST_CLASS_EXPORT(SVF::VFGEdge)
BOOST_CLASS_EXPORT(SVF::DirectSVFGEdge)
BOOST_CLASS_EXPORT(SVF::IntraDirSVFGEdge)
BOOST_CLASS_EXPORT(SVF::CallDirSVFGEdge)
BOOST_CLASS_EXPORT(SVF::RetDirSVFGEdge)

BOOST_CLASS_EXPORT(VFG)

// SVFG
BOOST_CLASS_EXPORT(SVF::MRSVFGNode)
BOOST_CLASS_EXPORT(SVF::FormalINSVFGNode)
BOOST_CLASS_EXPORT(SVF::FormalOUTSVFGNode)
BOOST_CLASS_EXPORT(SVF::ActualINSVFGNode)
BOOST_CLASS_EXPORT(SVF::ActualOUTSVFGNode)
BOOST_CLASS_EXPORT(SVF::MSSAPHISVFGNode)
BOOST_CLASS_EXPORT(SVF::IntraMSSAPHISVFGNode)
BOOST_CLASS_EXPORT(SVF::InterMSSAPHISVFGNode)

BOOST_CLASS_EXPORT(SVF::IndirectSVFGEdge)
BOOST_CLASS_EXPORT(SVF::IntraIndSVFGEdge)
BOOST_CLASS_EXPORT(SVF::CallIndSVFGEdge)
BOOST_CLASS_EXPORT(SVF::RetIndSVFGEdge)
BOOST_CLASS_EXPORT(SVF::ThreadMHPIndSVFGEdge)

BOOST_CLASS_EXPORT(SVF::SVFG)

// SVFGOPT
BOOST_CLASS_EXPORT(SVF::SVFGOPT)
