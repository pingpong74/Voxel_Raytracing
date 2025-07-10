#pragma once

#include "logicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace vkf {
    class Image {
        public:
        VkImage handle;
        VkImageView view;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
        VkFormat format;
        VkExtent2D extent;

        Image() = default;
        Image(LogicalDevice* ,VkFormat, VkExtent2D);

        void transitionLayout(VkCommandBuffer commandBuffer,VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange, VkPipelineStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

        ~Image();

        private:
        LogicalDevice* logicalDevice;
    };
}
