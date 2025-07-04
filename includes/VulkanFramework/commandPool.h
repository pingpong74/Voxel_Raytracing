#pragma once

#include "logicalDevice.h"
#include <vulkan/vulkan_core.h>


namespace vkf {

    class CommandPool {
        public:

        VkCommandPool handle;

        CommandPool(LogicalDevice*, uint32_t, VkCommandPoolCreateFlags);

        VkCommandBuffer allocateCommandBuffer(VkCommandBufferLevel);
        void freeCommandBuffer(VkCommandBuffer);
        void reset(VkCommandPoolResetFlags);

        ~CommandPool();

        private:

        uint32_t queueFamilyIndex;
        LogicalDevice* logicalDevice;
    };
}
