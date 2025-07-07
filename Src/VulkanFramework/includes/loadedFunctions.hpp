#pragma once

#include <vulkan/vulkan_core.h>

namespace vkf {
    //Acceleration Structures
    inline PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
    inline PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
    inline PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
    inline PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
    inline PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = nullptr;

    //Raytracing pipeline
    inline PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = nullptr;
    inline PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = nullptr;
}
