#!/usr/bin/env python

# /******************************************************************************
# * This program is free software: you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation, either version 3 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# *  but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
# *
# *
# * Author:
# *     Hui Peng <peng124@purdue.edu>
# * Date:
# *     2021-04-17
# *****************************************************************************/


import datetime

Serialization_ClassTypes = {
    'ICFG Nodes': [
#        'GenericICFGNodeTy',
        'ICFGNode',
        'GlobalBlockNode',
        'IntraBlockNode',
        'FunEntryBlockNode',
        'FunExitBlockNode',
        'CallBlockNode',
        'RetBlockNode'
    ],
    'ICFG Edges': [
#        'GenericICFGEdgeTy',
        'ICFGEdge',
        'IntraCFGEdge',
        'CallCFGEdge',
        'RetCFGEdge'
    ],
    'ICFG Graphs': [
#       'GenericICFGTy',
        'ICFG'
    ],
    'PAG Nodes': [
#        'GenericPAGNodeTy',
        'PAGNode',
        'ValPN',
        'ObjPN',
        'GepValPN',
        'FIObjPN',
        'RetPN',
        'VarArgPN',
        'DummyValPN',
        'DummyObjPN',
        'CloneDummyObjPN',
        'CloneGepObjPN',
        'CloneFIObjPN'
    ],
    'PAG Edges': [
#        'GenericPAGEdgeTy',
        'PAGEdge',
        'AddrPE',
        'CopyPE',
        'CmpPE',
        'BinaryOPPE',
        'UnaryOPPE',
        'StorePE',
        'LoadPE',
        'GepPE',
        'NormalGepPE',
        'VariantGepPE',
        'CallPE',
        'RetPE',
        'TDForkPE',
        'TDJoinPE'
    ],
    'PAG Graphs': [
#        'GenericPAGTy',
        'PAG'
    ],
    'PTA Callgraph Nodes': [
#        'GenericCallGraphNodeTy',
        'PTACallGraphNode',
    ],
    'PTA Callgraph Edges': [
#        'GenericCallGraphEdgeTy',
        'PTACallGraphEdge',
    ],
    'PTA Callgraphs': [
#        'GenericCallGraphTy',
        'PTACallGraph',
    ],
    'VFG Nodes': [
#        'GenericVFGNodeTy',
        'VFGNode',
        'StmtVFGNode',
        'LoadVFGNode',
        'StoreVFGNode',
        'CopyVFGNode',
        'CmpVFGNode',
        'BinaryOPVFGNode',
        'UnaryOPVFGNode',
        'GepVFGNode',
        'PHIVFGNode',
        'IntraPHIVFGNode',
        'AddrVFGNode',
        'ArgumentVFGNode',
        'ActualParmVFGNode',
        'FormalParmVFGNode',
        'ActualRetVFGNode',
        'FormalRetVFGNode',
        'InterPHIVFGNode',
        'NullPtrVFGNode',
    ],
    'VFG Edges': [
#        'GenericVFGEdgeTy',
        'VFGEdge',
        'DirectSVFGEdge',
        'IntraDirSVFGEdge',
        'CallDirSVFGEdge',
        'RetDirSVFGEdge',
    ],
    'VFGS': [
#        'GenericVFGTy',
        'VFG',
    ],
    'SVFG Nodes': [
        'MRSVFGNode',
        'FormalINSVFGNode',
        'FormalOUTSVFGNode',
        'ActualINSVFGNode',
        'ActualOUTSVFGNode',
        'MSSAPHISVFGNode',
        'IntraMSSAPHISVFGNode',
        'InterMSSAPHISVFGNode',
    ],
    'SVFG Edges': [
        'IndirectSVFGEdge',
        'IntraIndSVFGEdge',
        'CallIndSVFGEdge',
        'RetIndSVFGEdge',
        'ThreadMHPIndSVFGEdge',
    ],
    'SVFG': [
        'SVFG'
    ],
    'SVFGOPT': [
        'SVFGOPT'
    ]
}



def _convert_name_to_file_guard_macro(filename):
    pfx = '../include/'
    if filename.startswith(pfx):
        filename = filename.replace(pfx, '')

    filename = filename.replace('/', '_')
    filename = filename.replace('.', '_')
    filename ='__' + filename + '__'
    return filename.upper()

def gen_export_header(header_filename, export_macro, namespace):
    with open(header_filename, 'w') as of:
        of.write('/// Automatically genenerated by ' + __file__ + '\n')
        of.write('/// Date & time: ' +
                 datetime.datetime.now().strftime('%m/%d/%Y %H:%M:%S') + '\n')
        of.write('/// Note: do not include this file directly\n\n')

        fguard_macro = _convert_name_to_file_guard_macro(header_filename)
        of.write('#ifndef ' + fguard_macro + '\n')
        of.write('#define ' + fguard_macro + '\n')

        of.write('namespace ' + namespace +  ' {\n')
        for k in sorted(Serialization_ClassTypes.keys()):

            of.write('// ' + k + '\n')
            for cls in Serialization_ClassTypes[k]:
                of.write('class {};\n'.format(cls))

        of.write('} // end of namespace ' + namespace + '\n')

        for k in sorted(Serialization_ClassTypes.keys()):
            of.write('// ' + k + '\n')
            for cls in Serialization_ClassTypes[k]:
                of.write('{}({}::{})\n'.format(export_macro, namespace, cls))


        of.write('#endif\n')

def main():

    gen_export_header('../include/Util/boost_classes_export_key.h',
                      'BOOST_CLASS_EXPORT_KEY', 'SVF')
    gen_export_header('../include/Util/boost_classes_export_impl.h',
                      'BOOST_CLASS_EXPORT_IMPLEMENT', 'SVF')


if __name__ == '__main__':
    main()
