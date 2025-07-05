#pragma once

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "instance.hpp"


namespace vkf {

    class LogicalDevice {
        public:

        VkDevice handle;
        VkPhysicalDevice physicalDevice;

        VkQueue graphicsQueue, transferQueue, computeQueue, presentationQueue;

        LogicalDevice(Instance*);
        ~LogicalDevice();

        //Buffer
        void createBuffer(uint32_t , VkBufferUsageFlags , VkMemoryPropertyFlags , VkBuffer& , VkDeviceMemory&);
        void destroyBuffer(VkBuffer , VkDeviceMemory);

        //Image
        void createImage(VkFormat, VkExtent2D, VkImage&, VkDeviceMemory&);
        void createImageView(VkImage, VkFormat, VkImageView&);
        void destroyImage(VkImage, VkDeviceMemory, VkImageView);

        //CommandPool
        void createCommandPool(uint32_t, VkCommandPool&, VkCommandPoolCreateFlags);
        void destroyCommandPool(VkCommandPool);

        private:

        Instance* instance;

        uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags);
    };
}
