//===- NodeIDAllocator.cpp -- Allocates node IDs on request
//------------------------//

#include "Util/NodeIDAllocator.h"
#include "Util/Options.h"

namespace SVF {
const NodeID NodeIDAllocator::blackHoleObjectId = 0;
const NodeID NodeIDAllocator::constantObjectId = 1;
const NodeID NodeIDAllocator::blackHolePointerId = 2;
const NodeID NodeIDAllocator::nullPointerId = 3;

// Initialise counts to 4 because that's how many special nodes we have.

NodeIDAllocator::NodeIDAllocator()
    : numObjects(4), numValues(4), numSymbols(4), numNodes(4), numTypes(0),
      strategy(Options::NodeAllocStrat) {}

NodeIDAllocator::NodeIDAllocator(const NodeIDAllocator &nia)
    : numObjects(nia.numObjects), numValues(nia.numValues),
      numSymbols(nia.numSymbols), numNodes(nia.numNodes),
      numTypes(nia.numTypes), strategy(nia.strategy) {}

NodeIDAllocator &NodeIDAllocator::operator=(const NodeIDAllocator &nia) {

    if (this != &nia) {
        numObjects = nia.numObjects;
        numValues = nia.numValues;
        numSymbols = nia.numSymbols;
        numNodes = nia.numNodes;
        numTypes = nia.numTypes;
        strategy = nia.strategy;
    }

    return *this;
}

NodeID NodeIDAllocator::allocateObjectId() {
    NodeID id = 0;
    if (strategy == Strategy::DENSE) {
        // We allocate objects from 0(-ish, considering the special nodes) to #
        // of objects.
        id = numObjects;
    } else if (strategy == Strategy::SEQ) {
        // Everything is sequential and intermixed.
        id = numNodes;
    } else if (strategy == Strategy::DEBUG) {
        // Non-GEPs just grab the next available ID.
        // We may have "holes" because GEPs increment the total
        // but allocate far away. This is not a problem because
        // we don't care about the relative distances between nodes.
        id = numNodes;
    } else {
        assert(false && "NodeIDAllocator::allocateObjectId: unimplemented node "
                        "allocation strategy.");
    }

    ++numObjects;
    ++numNodes;

    assert(id != 0 && "NodeIDAllocator::allocateObjectId: ID not allocated");
    return id;
}

NodeID NodeIDAllocator::allocateGepObjectId(NodeID base, u32_t offset,
                                            u32_t maxFieldLimit) {
    NodeID id = 0;
    if (strategy == Strategy::DENSE) {
        // Nothing different to the other case.
        id = numObjects;
    } else if (strategy == Strategy::SEQ) {
        // Everything is sequential and intermixed.
        id = numNodes;
    } else if (strategy == Strategy::DEBUG) {
        // For a gep id, base id is set at lower bits, and offset is set at
        // higher bits e.g., 1100050 denotes base=50 and offset=10 The offset is
        // 10, not 11, because we add 1 to the offset to ensure that the high
        // bits are never 0. For example, we do not want the gep id to be 50
        // when the base is 50 and the offset is 0.
        NodeID gepMultiplier =
            pow(10, ceil(log10(numSymbols > maxFieldLimit ? numSymbols
                                                          : maxFieldLimit)));
        id = (offset + 1) * gepMultiplier + base;
        assert(id > numNodes && "NodeIDAllocator::allocateGepObjectId: GEP "
                                "allocation clashing with other nodes");
    } else {
        assert(false && "NodeIDAllocator::allocateGepObjectId: unimplemented "
                        "node allocation strategy");
    }

    ++numObjects;
    ++numNodes;

    assert(id != 0 && "NodeIDAllocator::allocateGepObjectId: ID not allocated");
    return id;
}

NodeID NodeIDAllocator::allocateValueId() {
    NodeID id = 0;
    if (strategy == Strategy::DENSE) {
        // We allocate values from UINT_MAX to UINT_MAX - # of values.
        // TODO: UINT_MAX does not allow for an easily changeable type
        //       of NodeID (though it is already in use elsewhere).
        id = UINT_MAX - numValues;
    } else if (strategy == Strategy::SEQ) {
        // Everything is sequential and intermixed.
        id = numNodes;
    } else if (strategy == Strategy::DEBUG) {
        id = numNodes;
    } else {
        assert(false && "NodeIDAllocator::allocateValueId: unimplemented node "
                        "allocation strategy");
    }

    ++numValues;
    ++numNodes;

    assert(id != 0 && "NodeIDAllocator::allocateValueId: ID not allocated");
    return id;
}

NodeID NodeIDAllocator::allocateTypeId() { return numTypes++; }

void NodeIDAllocator::endSymbolAllocation() { numSymbols = numNodes; }

}; // namespace SVF.
