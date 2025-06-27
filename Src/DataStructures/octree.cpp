#include "dataStructures.h"

void Octree::buildOctree(uint8_t voxels[], int side, vector<uint8_t>& octree, int offsetX, int offsetY, int offsetZ, int reccursionDepth) {

    int check = voxels[side * side * offsetX + side * offsetY + offsetZ];
    bool isLeaf = true;

    for(int x = offsetX; x < side + offsetX; x++) {
        for(int y = offsetY; y < side + offsetY; y++) {
            for(int z = offsetZ; z < side + offsetZ; z++) {
                if(voxels[side * side * x + side * y + z] != check) {
                    isLeaf = false;
                    break;
                }
            }
        }
    }

    if(isLeaf) {

    }
    else {

    }
}
