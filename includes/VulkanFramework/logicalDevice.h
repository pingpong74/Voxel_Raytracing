#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>
#include <set>

#include "physicalDevice.h"

#include "../../config.h"


namespace vkf {

    struct CommandBuffer;

    class LogicalDevice {
        public:

        VkDevice handle;
        VkPhysicalDevice physicalDevice;

        VkQueue graphicsQueue, transferQueue, computeQueue, presentationQueue;

        void createLogicalDevice(VkInstance , VkSurfaceKHR);

        //Buffer
        void createBuffer(uint32_t , VkBufferUsageFlags , VkMemoryPropertyFlags , VkBuffer& , VkDeviceMemory&);
        void destroyBuffer(VkBuffer , VkDeviceMemory);

        //Image
        void createImage();
        void destroyImage();

        //CommandPool
        void createCommandPool(uint32_t, VkCommandPool&, VkCommandPoolCreateFlags);
        void destroyCommandPool(VkCommandPool);

        //Image

        void destroy();

        private:

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
