//===- ExternalPAG.cpp -- Program assignment graph ---------------------------//

/*
 * ExternalPAG.cpp
 *
 *  Created on: Sep 22, 2018
 *      Author: Mohamad Barbar
 */

#include <fstream>
#include <iostream>
#include <sstream>

#include "Graphs/ExternalPAG.h"
#include "Graphs/ICFG.h"
#include "Graphs/PAG.h"
#include "Util/BasicTypes.h"

using namespace SVF;
using namespace SVFUtil;



/*!
 * Dump PAGs for the functions
 */

bool ExternalPAG::addExternalPAG(const SVFFunction *function) {
    // The function does not exist in the module - bad arg?
    // TODO: maybe some warning?
    if (function == nullptr) {
        return false;
    }

    if (pag->hasExternalPAG(function)) {
        return false;
    }

    outs() << "Adding extpag " << this->getFunctionName() << "\n";
    // Temporarily trick SVF Module into thinking we are reading from
    // file to avoid setting BBs/Values (as these nodes don't have any).
    std::string oldSVFModuleFileName = SVFModule::pagFileName();
    SVFModule::setPagFromTXT("tmp");

    // We need: the new nodes.
    //        : the new edges.
    //        : to map function names to the entry nodes.
    //        : to map function names to the return node.

    // To create the new edges.
    Map<NodeID, PAGNode *> extToNewNodes;

    // Add the value nodes.
    for (auto extNodeIt : this->getValueNodes()) {
        NodeID newNodeId = pag->addDummyValNode();
        extToNewNodes[extNodeIt] = pag->getPAGNode(newNodeId);
    }

    // Add the object nodes.
    for (auto extNodeIt : this->getObjectNodes()) {
        // TODO: fix obj node - there's more to it?
        NodeID newNodeId = pag->addDummyObjNode();
        extToNewNodes[extNodeIt] = pag->getPAGNode(newNodeId);
    }

    // Add the edges.
    for (const auto &extEdgeIt : this->getEdges()) {
        NodeID extSrcId = std::get<0>(extEdgeIt);
        NodeID extDstId = std::get<1>(extEdgeIt);
        const std::string &extEdgeType = std::get<2>(extEdgeIt);
        int extOffsetOrCSId = std::get<3>(extEdgeIt);

        PAGNode *srcNode = extToNewNodes[extSrcId];
        PAGNode *dstNode = extToNewNodes[extDstId];
        NodeID srcId = srcNode->getId();
        NodeID dstId = dstNode->getId();

        if (extEdgeType == "addr") {
            pag->addAddrPE(srcId, dstId);
        } else if (extEdgeType == "copy") {
            pag->addCopyPE(srcId, dstId);
        } else if (extEdgeType == "load") {
            pag->addLoadPE(srcId, dstId);
        } else if (extEdgeType == "store") {
            pag->addStorePE(srcId, dstId, nullptr);
        } else if (extEdgeType == "gep") {
            pag->addNormalGepPE(srcId, dstId, LocationSet(extOffsetOrCSId));
        } else if (extEdgeType == "variant-gep") {
            pag->addVariantGepPE(srcId, dstId);
        } else if (extEdgeType == "call") {
            pag->addEdge(srcNode, dstNode,
                         new CallPE(srcNode, dstNode, pag, nullptr));
        } else if (extEdgeType == "ret") {
            pag->addEdge(srcNode, dstNode,
                         new RetPE(srcNode, dstNode, pag, nullptr));
        } else if (extEdgeType == "cmp") {
            pag->addCmpPE(srcId, dstId);
        } else if (extEdgeType == "binary-op") {
            pag->addBinaryOPPE(srcId, dstId);
        } else if (extEdgeType == "unary-op") {
            pag->addUnaryOPPE(srcId, dstId);
        } else {
            outs() << "Bad edge type found during extpag addition\n";
        }
    }

    // Record the arg nodes.
    Map<int, PAGNode *> argNodes;
    for (auto &argNodeIt : this->getArgNodes()) {
        int index = argNodeIt.first;
        NodeID extNodeId = argNodeIt.second;
        argNodes[index] = extToNewNodes[extNodeId];
    }

    pag->functionToExternalPAGEntries[function] = argNodes;

    // Record the return node.
    if (this->hasReturnNode()) {
        pag->functionToExternalPAGReturns[function] =
            extToNewNodes[this->getReturnNode()];
    }

    // Put it back as if nothing happened.
    SVFModule::setPagFromTXT(oldSVFModuleFileName);

    return true;
}

// Very similar implementation to the PAGBuilderFromFile.
void ExternalPAG::readFromFile(std::string &filename) {
    std::string line;
    std::ifstream pagFile(filename.c_str());

    if (!pagFile.is_open()) {
        outs() << "ExternalPAG::buildFromFile: could not open " << filename
               << "\n";
        return;
    }

    while (pagFile.good()) {
        std::getline(pagFile, line);

        Size_t tokenCount = 0;
        std::string tmps;
        std::istringstream ss(line);
        while (ss.good()) {
            ss >> tmps;
            tokenCount++;
        }

        if (tokenCount == 0) {
            // Empty line.
            continue;
        }

        if (tokenCount == 2 || tokenCount == 3) {
            // It's a node.
            NodeID nodeId;
            std::string nodeType;
            std::istringstream ss(line);
            ss >> nodeId;
            ss >> nodeType;

            if (nodeType == "v") {
                valueNodes.insert(nodeId);
            } else if (nodeType == "o") {
                objectNodes.insert(nodeId);
            } else {
                assert(false &&
                       "format not supported, please specify node type");
            }

            if (tokenCount == 3) {
                // If there are 3 tokens, it should be 0, 1, 2, ... or ret.
                std::string argNoOrRet;
                ss >> argNoOrRet;

                if (argNoOrRet == "ret") {
                    setReturnNode(nodeId);
                } else if (std::all_of(argNoOrRet.begin(), argNoOrRet.end(),
                                       ::isdigit)) {
                    int argNo = std::stoi(argNoOrRet);
                    argNodes[argNo] = nodeId;
                } else {
                    assert(false &&
                           "format not supported, not arg number or ret");
                }
            }
        } else if (tokenCount == 4) {
            // It's an edge
            NodeID nodeSrc;
            NodeID nodeDst;
            Size_t offsetOrCSId;
            std::string edge;
            std::istringstream ss(line);
            ss >> nodeSrc;
            ss >> edge;
            ss >> nodeDst;
            ss >> offsetOrCSId;

            edges.insert(std::tuple<NodeID, NodeID, std::string, int>(
                nodeSrc, nodeDst, edge, offsetOrCSId));
        } else {
            if (!line.empty()) {
                outs() << "format not supported, token count = " << tokenCount
                       << "\n";
                assert(false && "format not supported");
            }
        }
    }

    /*
    TODO: might need to include elsewhere.
    /// new gep node's id from lower bound, nodeNum may not reflect the total
    nodes u32_t lower_bound = 1000; for(u32_t i = 0; i < lower_bound; i++)
        pag->incNodeNum();
    */
}
