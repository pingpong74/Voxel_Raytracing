#include "model.h"
#include <stdexcept>

VoxelModel VoxelModel::loadModel(const std::string &filepath) {
    struct magicVoxel {
        uint8_t x, y, z, colourIndex;
    };

    std::ifstream file(filepath, std::ios::binary);
    if(!file.is_open()) throw std::runtime_error("failed to open file");

    int side;

    auto read_uint32 = [&]() {
        uint32_t val;
        file.read(reinterpret_cast<char*>(&val), 4);
        return val;
    };

    char magic[4];
    file.read(magic, 4);
    assert(std::string(magic, 4) == "VOX " && "Invalid VOX file");

    uint32_t version = read_uint32(); // usually 150

    int sizeX = -1, sizeY = -1, sizeZ = -1;
    std::vector<magicVoxel> voxels;

    while (file) {
        char chunkId[4];
        file.read(chunkId, 4);
        if (file.eof()) break;

        uint32_t chunkSize = read_uint32();
        uint32_t childSize = read_uint32();
        std::string id(chunkId, 4);

        if (id == "SIZE") {
            sizeX = read_uint32();
            sizeY = read_uint32();
            sizeZ = read_uint32();
            file.seekg(chunkSize - 12, std::ios::cur);
        } else if (id == "XYZI") {
            uint32_t numVoxels = read_uint32();
            voxels.resize(numVoxels);
            file.read(reinterpret_cast<char*>(voxels.data()), numVoxels * 4);
        } else {
            file.seekg(chunkSize, std::ios::cur);
        }
    }

    assert(sizeX == sizeY && sizeY == sizeZ && "Model must be cubic");
    side = sizeX;

    std::vector<uint8_t> flat(side * side * side, 0);
    auto index = [&](int x, int y, int z) {
        return side * side * x + side * y + z;
    };

    for (const auto& v : voxels) {
        flat[index(v.x, v.z, side - 1 - v.y)] = v.colourIndex;
    }

    return {side, flat };
}
