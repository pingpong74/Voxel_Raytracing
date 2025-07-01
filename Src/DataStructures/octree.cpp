#include "dataStructures.h"

vector<uint8_t> Octree::buildOctree(uint8_t voxels[], int side) {
    vector<uint8_t> octree;

    bool isDivided = false;

    for(int x = 0; x < side; x++) {
        for(int y = 0; y < side; y++) {
            for(int z = 0; z < side; z++) {
                if(voxels[side * side * x + side * y + z] != voxels[0]) {
                    isDivided = true;
                    break;
                }
            }
        }
    }

    if(isDivided) {
        traverseOctree(voxels, side, octree, true, 0, 0, 0, 0);
    }
    else {
        octree = {0, 0, voxels[0]};
    }

    return octree;
}

void Octree::traverseOctree(uint8_t voxels[], int side, vector<uint8_t>& octree, bool isSubdivided, int offsetX, int offsetY, int offsetZ, int recursuionDepth) {
    uint8_t childDivisionMask = 0;
    uint8_t childValidityMask = 0;

    for(int i = 0; i < 8; i++) {
        bool isNodeDivided = false;
        uint8_t check = voxels[side * side * offsetX + side * offsetY + offsetZ];

        for(int x = offsetX; x < side + offsetX; x++) {
            for(int y = offsetY; y < side + offsetY; y++) {
                for(int z = offsetZ; z < side + offsetZ; z++) {

                }
            }
        }
    }
}
