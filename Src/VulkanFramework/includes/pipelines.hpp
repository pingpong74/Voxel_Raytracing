#pragma once

#include "logicalDevice.hpp"
#include "descriptor.hpp"
#include "buffer.hpp"
#include "commandPool.hpp"
#include "loadedFunctions.hpp"

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

        inline void bindPipeline(VkCommandBuffer commandBuffer) {  vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, handle); }
        inline void bindDescriptorSet(VkCommandBuffer commandBuffer, DescriptorSet* desSet) { vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipelineLayout, 0, 1, &desSet->handle, 0, 0); }
        inline void traceRays(VkCommandBuffer commandBuffer, VkExtent2D extent) { vkCmdTraceRaysKHR(commandBuffer, &rgenRegion, &missRegion, &hitRegion, &callRegion, extent.width, extent.height, 1); }
        inline void pushConstants(VkCommandBuffer commandBuffer, void* values, uint32_t size) { vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_ALL, 0, size, values); }

        ~RayTracingPipeline();

        private:

        Buffer sbtBuffer;
        uint32_t shaderGroupCount;
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;

        VkStridedDeviceAddressRegionKHR rgenRegion{};
        VkStridedDeviceAddressRegionKHR missRegion{};
        VkStridedDeviceAddressRegionKHR hitRegion{};
        VkStridedDeviceAddressRegionKHR callRegion{};

        VkPipeline handle;
        VkPipelineLayout pipelineLayout;
        LogicalDevice* logicalDevice;
    };

    class RasterizationPipeline {

    };

    class ComputePipeline {

    };
}
