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
    uint8_t childDivisionMask; //Tells which of the following children are actually divided. If they are divided, its a uint32_t which will poit to more child data, otherwise it is uint8_t voxel data.
    uint8_t childMask; //Tells which of the following childs have data. 1 means some data and 0 means air?.
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


    vector<uint8_t> buildOctree(uint8_t voxels[], int side);
    void traverseOctree(uint8_t voxels[], int side, vector<uint8_t>& octree, bool isSubdivided, int offsetX, int offsetY, int offsetZ, int recursionDepth);
}
