#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <glm/vec3.hpp>

#include "logicalDevice.hpp"
#include "commandPool.hpp"
#include "buffer.hpp"

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

    class BottomLevelAccelerationStructure {
        public:
        VkAccelerationStructureKHR handle;

        BottomLevelAccelerationStructure() = default;
        ~BottomLevelAccelerationStructure();

        //Does not start recording or submit the command buffer, need to do it yourself
        static std::vector<BottomLevelAccelerationStructure> createBottomLevelAccelerationStructure(VkCommandBuffer commandBuffer, LogicalDevice* logicalDevice, BottomLevelAccelerationStructureBuildInfo* blasBuldInfo, uint32_t count);

        VkDeviceAddress getAddress();

        private:
        LogicalDevice* logicalDevice;
    };

    class TopLevelAccelerationStructure {
        public:
        VkAccelerationStructureKHR handle;
        std::vector<BottomLevelAccelerationStructure> blases;
        std::vector<VkTransformMatrixKHR> transforms;
        VkCommandBuffer commandBuffer;

        TopLevelAccelerationStructure(LogicalDevice* logicalDevice, CommandPool* buildPool);
        ~TopLevelAccelerationStructure();

        void createTopLevelAccelerationStructure(std::vector<BottomLevelAccelerationStructure>, std::vector<VkTransformMatrixKHR> transforms);
        //Doesnt not start recording or submit the command buffer, meed to do it yourself
        void updateTopLevelAccelerationStructure(std::vector<BottomLevelAccelerationStructure> bottomLevelStructures, std::vector<VkTransformMatrixKHR> transforms);

        private:
        LogicalDevice* logicalDevice;
        CommandPool* buildPool;

        Buffer tlasBuffer;
        Buffer scratchBuffer;

        Buffer instanceBuffer;
        Buffer instanceStagingBuffer;
    };
}
