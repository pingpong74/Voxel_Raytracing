#include "dataStructures.h"
#include <cstdint>

vector<uint8_t> BrickMap::buildBrickmap(uint8_t voxels[], int side) {
    if(side % 4 != 0) throw std::runtime_error("Wrong side width for a brickmap");
    int brickSize = side / 4;

    uint64_t mask = 0;

    vector<uint8_t> brickMap = {0, 0, 0, 0, 0, 0, 0, 0};
    vector<uint8_t> data;

    int address = 0;

    for (int _x = 0; _x < 4; _x++) {
        for (int _y = 0; _y < 4; _y++ ) {
            for (int _z = 0; _z < 4; _z++) {

                int x = brickSize * _x;
                int y = brickSize * _y;
                int z = brickSize * _z;

                bool isBrick = true;
                uint8_t check = voxels[side * side * x + side * y + z];

                for(int dx = x; dx < brickSize + x; dx++) {
                    for(int dy = y; dy < brickSize + y; dy++) {
                        for(int dz = z; dz < brickSize + z; dz++) {
                            if(voxels[side * side * dx + side * dy + dz] != check) {
                                isBrick = false;
                                break;
                            }
                        }
                    }
                }

                if(isBrick) {
                    mask = mask | ( (uint64_t)1 << (4 * 4 * _x + 4 * _y + _z) );
                    brickMap.push_back(check);
                }

                else {
                    for(int dx = x; dx < brickSize + x; dx++) {
                        for(int dy = y; dy < brickSize + y; dy++) {
                            for(int dz = z; dz < brickSize + z; dz++) {
                                data.push_back(voxels[side * side * dx + side * dy + dz]);
                            }
                        }
                    }

                    brickMap.push_back(address);
                    address += 1;
                }
            }
        }
    }

    for(int i = 0; i < 8; i++) {
        brickMap[i] = static_cast<uint8_t>( (mask >> (i * 8)) & 0xFF);
    }

    for(int i : data) {
        brickMap.push_back(i);
    }

    return brickMap;
}
