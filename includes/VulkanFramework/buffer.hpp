#pragma once

#include <vulkan/vulkan_core.h>

#include "logicalDevice.hpp"

namespace vkf {
    class Buffer {
        public:
        VkBuffer handle;
        VkDeviceMemory bufferMemory;

        Buffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, LogicalDevice* logicalDevice);

        VkDeviceAddress getBufferAddress();

        static void copyBuffer(Buffer src, Buffer dst, int size, VkCommandBuffer commandBuffer, int srcOffset = 0, int dstOffset = 0);

        ~Buffer();

        private:

        LogicalDevice* logicalDevice;
        uint32_t size;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags properties;
    };
}
