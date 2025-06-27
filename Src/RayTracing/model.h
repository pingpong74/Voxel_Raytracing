#include "../DataStructures/voxel.h"
#include <fstream>
#include <cstdint>
#include <string>
#include <vector>

struct VoxelModel {
    int side;
    std::vector<uint8_t> voxels;

    static VoxelModel loadModel(const std::string& filepath);
};
