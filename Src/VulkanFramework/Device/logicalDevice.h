#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>
#include <set>

#include "../../config.h"
#include "queueFamilies.h"
#include "physicalDevice.h"
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

        //Command Buffer
        CommandBuffer createCommandBuffer(VkCommandPool commandPool);

        void destroy();

        private:

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
