#pragma once

#include "logicalDevice.hpp"
#include "descriptor.hpp"
#include "buffer.hpp"
#include "commandPool.hpp"

#include <vulkan/vulkan_core.h>
#include <string>

namespace vkf {

    VkShaderModule createShaderModule(const std::string& filePath, LogicalDevice* logicalDevice);
    VkPipelineShaderStageCreateInfo createShaderStageCreateInfo(VkShaderModule shaderModule, VkShaderStageFlagBits flags);


    class RayTracingPipeline {
        public:
        RayTracingPipeline(LogicalDevice* logicalDevice);

        void create(DescriptorSetLayout** layouts, uint32_t descriptorSetCount, uint32_t pushConstantsSize, std::string raygen, std::string miss, std::string closestHit, std::string intersection = "^");
        void createShaderBindingTable(CommandPool* transferPool);

        ~RayTracingPipeline();

        private:

        Buffer sbtBuffer;
        uint32_t shaderGroupCount;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;

        VkStridedDeviceAddressRegionKHR rgenRegion{};
        VkStridedDeviceAddressRegionKHR missRegion{};
        VkStridedDeviceAddressRegionKHR hitRegion{};
        VkStridedDeviceAddressRegionKHR callRegion{};

        PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = nullptr;
        PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = nullptr;

        VkPipeline handle;
        VkPipelineLayout pipelineLayout;
        LogicalDevice* logicalDevice;
    };

    class RasterizationPipeline {

    };

    class ComputePipeline {

    };
}
