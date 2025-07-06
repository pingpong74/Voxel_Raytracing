#pragma once

#include "logicalDevice.hpp"
#include <vulkan/vulkan_core.h>


namespace vkf {

    class CommandPool {
        public:

        VkCommandPool handle;

        CommandPool(LogicalDevice*, uint32_t, VkCommandPoolCreateFlags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        VkCommandBuffer allocateCommandBuffer(VkCommandBufferLevel);
        void freeCommandBuffer(VkCommandBuffer);
        void reset(VkCommandPoolResetFlags);

        ~CommandPool();

        private:

        uint32_t queueFamilyIndex;
        LogicalDevice* logicalDevice;
    };
}
