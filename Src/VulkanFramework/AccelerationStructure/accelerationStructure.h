#pragma once

#include <vulkan/vulkan_core.h>

#include "../buffer.h"
#include "../commandBuffer.h"

namespace vkf {
    static PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
    static PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
    static PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
    static PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
    static PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = nullptr;

    struct BottomLevelAccelerationStructure {
        VkAccelerationStructureKHR handle;

        static BottomLevelAccelerationStructure createBottomLevelAccelerationStructure();
        void destroy();
    };

    struct TopLevelAccelerationStructure {
        VkAccelerationStructureKHR handle;
        Buffer tlasBuffer;
        Buffer scratchBuffer;

        static TopLevelAccelerationStructure createTopLevelAccelerationStructure();
        void updateTopLevelAccelerationStructure();
        void destroy();
    };
}
