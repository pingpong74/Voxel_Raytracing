#pragma once

#include "logicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace vkf {
    class Image {
        public:
        VkImage handle;
        VkImageView view;

        Image(LogicalDevice* ,VkFormat, VkExtent2D);

        void transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout, VkImageSubresourceRange, VkPipelineStageFlags, VkPipelineStageFlags);

        ~Image();

        private:
        LogicalDevice* logicalDevice;
        VkDeviceMemory imageMemory;
        VkImageLayout currentLayout;
        VkFormat format;
        VkExtent2D extent;
    };
}
