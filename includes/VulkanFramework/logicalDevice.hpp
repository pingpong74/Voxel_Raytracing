#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>
#include <set>

#include "physicalDevice.hpp"

#include "../../config.h"


namespace vkf {

    class LogicalDevice {
        public:

        VkDevice handle;
        VkPhysicalDevice physicalDevice;

        VkQueue graphicsQueue, transferQueue, computeQueue, presentationQueue;

        void createLogicalDevice(VkInstance , VkSurfaceKHR);
        void destroy();

        //Buffer
        void createBuffer(uint32_t , VkBufferUsageFlags , VkMemoryPropertyFlags , VkBuffer& , VkDeviceMemory&);
        void destroyBuffer(VkBuffer , VkDeviceMemory);

        //Image
        void createImage(VkFormat, VkExtent2D, VkImage&, VkDeviceMemory&);
        void createImageView(VkImage, VkFormat, VkImageView&);
        void destroyImage(VkImage, VkDeviceMemory);

        //CommandPool
        void createCommandPool(uint32_t, VkCommandPool&, VkCommandPoolCreateFlags);
        void destroyCommandPool(VkCommandPool);

        private:

        uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);
    };
}
