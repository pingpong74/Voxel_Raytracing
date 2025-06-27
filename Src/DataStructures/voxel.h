#pragma once

#include <glm/vec3.hpp>
#include <vulkan/vulkan_core.h>

struct Voxel {
    uint8_t colour;
};

enum DataType {
    GRID = 0,
    BRICKMAP = 1,
    OCTREE = 2,
};

struct InstanceData {
    DataType type;
    alignas(16)glm::vec3 bMin;
    int side;
    float voxelWidth;
    uint32_t padding = 0;
    VkDeviceAddress geometryData;
};
