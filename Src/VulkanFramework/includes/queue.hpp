#pragma once

#include <vulkan/vulkan_core.h>

namespace vkf {
    class Queue {
        public:

        VkQueue handle;
        uint32_t familyIndex;

        Queue() = default;

        void flushCommandBuffer(VkCommandBuffer);
        void submit(const VkSubmitInfo*, uint32_t, VkFence);
        void present(VkPresentInfoKHR*);
        void waitIdle();

        ~Queue() = default;
    };
}
