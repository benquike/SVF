//===- GraphPrinter.h -- Print Generic Graph---------------------//
//
//                     SVF: Static Value-Flow Analysis
//
// Copyright (C) <2013-2017>  <Yulei Sui>
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
 * GraphPrinter.h
 *
 *  Created on: 19.Sep.,2018
 *      Author: Yulei
 */

#ifndef INCLUDE_UTIL_GRAPHPRINTER_H_
#define INCLUDE_UTIL_GRAPHPRINTER_H_

#include <llvm/ADT/GraphTraits.h>
#include <llvm/Support/FileSystem.h> // for file open flag
#include <llvm/Support/ToolOutputFile.h>
#include <system_error>

namespace llvm {

/*
 * Dump and print the graph for debugging
 */
class GraphPrinter {

  public:
    GraphPrinter() {}

    /*!
     *  Write the graph into dot file for debugging purpose
     */
    template <class GraphType>
    static void WriteGraphToFile(llvm::raw_ostream &O,
                                 const std::string &GraphName,
                                 const GraphType &GT, bool simple = false) {
        // Filename of the output dot file
        std::string Filename = GraphName + ".dot";
        O << "Writing '" << Filename << "'...";
        std::error_code ErrInfo;
        llvm::ToolOutputFile F(Filename.c_str(), ErrInfo,
                               llvm::sys::fs::F_None);

        if (!ErrInfo) {
            // dump the ValueFlowGraph here
            WriteGraph(F.os(), GT, simple);
            F.os().close();
            if (!F.os().has_error()) {
                O << "\n";
                F.keep();
                return;
            }
        }
        O << "  error opening file for writing!\n";
        F.os().clear_error();
    }

    /*!
     * Print the graph to command line
     */
    template <class GraphType>
    static void PrintGraph(llvm::raw_ostream &O, const std::string &GraphName,
                           const GraphType &GT) {
        /// Define the GTraits and node iterator for printing
        using GTraits = llvm::GraphTraits<GraphType>;

        using NodeRef = typename GTraits::NodeRef;
        using node_iterator = typename GTraits::nodes_iterator;
        using child_iterator = typename GTraits::ChildIteratorType;

        O << "Printing Graph '" << GraphName << "'...\n";
        // Print each node name and its edges
        node_iterator I = GTraits::nodes_begin(GT);
        node_iterator E = GTraits::nodes_end(GT);
        for (; I != E; ++I) {
            NodeRef *Node = *I;
            O << "node :" << Node << "'\n";
            child_iterator EI = GTraits::child_begin(Node);
            child_iterator EE = GTraits::child_end(Node);
            for (unsigned i = 0; EI != EE && i != 64; ++EI, ++i) {
                O << "child :" << *EI << "'\n";
            }
        }
    }
};

} // End namespace llvm

#endif /* INCLUDE_UTIL_GRAPHPRINTER_H_ */
