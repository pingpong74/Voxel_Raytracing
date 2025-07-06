#pragma once

#include "logicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace vkf {
    class Queue {
        public:

        VkQueue handle;

        Queue() = default;

        void flushCommandBuffer(VkCommandBuffer);
        void submit(const VkSubmitInfo*, uint32_t, VkFence);
        void present(VkPresentInfoKHR*);
        void waitIdle();

        ~Queue() = default;
    };
}
