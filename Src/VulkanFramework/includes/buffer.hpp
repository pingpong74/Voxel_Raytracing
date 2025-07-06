#pragma once

#include <vulkan/vulkan_core.h>

#include "logicalDevice.hpp"

namespace vkf {
    class Buffer {
        public:
        VkBuffer handle = VK_NULL_HANDLE;
        VkDeviceMemory bufferMemory = VK_NULL_HANDLE;

        Buffer(LogicalDevice*);
        Buffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, LogicalDevice* logicalDevice);

        void create(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        VkDeviceAddress getBufferAddress();
        inline uint32_t getSize() { return size; }

        static void copyBuffer(Buffer* src, Buffer* dst, int size, VkCommandBuffer commandBuffer, int srcOffset = 0, int dstOffset = 0);

        ~Buffer();

        private:

        LogicalDevice* logicalDevice;
        uint32_t size;
        VkBufferUsageFlags usage;
        VkMemoryPropertyFlags properties;
    };
}
