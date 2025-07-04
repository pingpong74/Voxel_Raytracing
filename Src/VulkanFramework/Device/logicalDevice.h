#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>
#include <set>

#include "queueFamilies.h"
#include "physicalDevice.h"

#include "../../config.h"
#include "../buffer.h"
#include "../commandBuffer.h"

namespace vkf {

    class LogicalDevice {
        public:

        VkDevice handle;
        VkPhysicalDevice physicalDevice;

        VkQueue graphicsQueue, transferQueue, computeQueue, presentationQueue;

        void createLogicalDevice(VkInstance instance, VkSurfaceKHR surface);

        //Buffer
        Buffer createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
        VkDeviceAddress getBufferAddress(Buffer buffer);
        void destroyBuffer(Buffer buffer);

        //Command Buffer
        CommandBuffer createCommandBuffer(VkCommandPool commandPool);
        void destroyCommandBuffer(CommandBuffer commandBuffer);

        //Image

        void destroy();

        private:

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
