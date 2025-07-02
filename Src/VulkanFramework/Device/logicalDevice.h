#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>
#include <set>

#include "../../config.h"
#include "queueFamilies.h"
#include "physicalDevice.h"

namespace vkf {
    struct Buffer {
        VkBuffer handle;
        VkDeviceMemory bufferMemory;
    };

    struct Image {
        VkImage handle;
        VkImageView view;
        VkDeviceMemory imgMemory;
    };

    class LogicalDevice {
        public:

        static LogicalDevice createLogicalDevice(VkInstance instance, VkSurfaceKHR surface);
        VkQueue getGraphicsQueue();

        //Buffer
        Buffer createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

        //Command Buffer
        VkCommandBuffer createCommandBuffer();

        //Image
        Image createImage();

        private:

        VkDevice handle;
        VkPhysicalDevice physicalDevice;

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
