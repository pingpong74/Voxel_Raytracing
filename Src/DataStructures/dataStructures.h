#pragma once

#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vector>

using namespace std;

namespace BrickMap {
    vector<uint8_t> buildBrickmap(uint8_t voxels[], int side);
}

struct OctreeNode {
    uint8_t childDivisionMask; //tells which of the child is subdivided. 1 means leaf chils, 0 means non leaf
    uint8_t childMask; //if the child is a leaf node then 1 means that this voxel stores some data and 0 means that the pointer is just null and hence stores no data. No meaning if the node is divided.
    uint16_t childAddres;

    uint32_t to_uint32_t() {
        uint32_t res;
        memcpy(&res, this, 4);
        return res;
    }
};

namespace Octree {

    inline bool isPowerOfTwo(unsigned int n) {
        return n && !(n & (n - 1));
    }


    void buildOctree(uint8_t voxels[], int side, vector<uint8_t>& octree, int offsetX = 0, int offsetY = 0, int offsetZ = 0, int reccursionDepth = 0);
}
