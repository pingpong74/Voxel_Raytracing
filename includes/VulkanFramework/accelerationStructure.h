#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <glm/vec3.hpp>

#include "../Device/logicalDevice.h"
#include "../buffer.h"
#include "../commandBuffer.h"

namespace vkf {
    static PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
    static PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
    static PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
    static PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
    static PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = nullptr;

    struct BottomLevelAccelerationStructureBuildInfo {
        VkDeviceAddress boundingBoxBufferAddress;
        VkDeviceAddress scratchBufferAddress;
        Buffer blasBuffer;
        uint32_t blasOffset;
    };

    struct BottomLevelAccelerationStructure {
        VkAccelerationStructureKHR handle;

        static std::vector<BottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(CommandBuffer commandBuffer, LogicalDevice* logicalDevice, BottomLevelAccelerationStructureBuildInfo* blasBuldInfo, uint32_t count);
        VkDeviceAddress getAddress(LogicalDevice* logicalDevice);
        void destroy(LogicalDevice* logicalDevice);
    };

    struct TopLevelAccelerationStructure {
        VkAccelerationStructureKHR handle;
        Buffer tlasBuffer;
        Buffer scratchBuffer;

        Buffer instanceBuffer;
        Buffer instanceStagingBuffer;

        std::vector<BottomLevelAccelerationStructure> blases;
        std::vector<VkTransformMatrixKHR> transforms;

        CommandBuffer commandBuffer;

        static TopLevelAccelerationStructure createTopLevelAccelerationStructure(LogicalDevice* logicalDevice, VkCommandPool buildPool, std::vector<BottomLevelAccelerationStructure>, std::vector<VkTransformMatrixKHR> transforms);
        void updateTopLevelAccelerationStructure(LogicalDevice* logicalDevice, VkCommandPool buildPool, std::vector<BottomLevelAccelerationStructure> bottomLevelStructures, std::vector<VkTransformMatrixKHR> transforms);
        void destroy();
    };
}
