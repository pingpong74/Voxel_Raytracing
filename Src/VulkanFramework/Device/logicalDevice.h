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

        static LogicalDevice createLogicalDevice(VkInstance instance, VkSurfaceKHR surface);
        VkQueue getGraphicsQueue();
        VkQueue getTransferQueue();
        VkQueue getComputeQueue();
        VkQueue getPresentationQueue();

        //Buffer
        Buffer createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

        //Command Buffer
        VkCommandBuffer createCommandBuffer();

        private:

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
