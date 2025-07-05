#pragma once

#include "logicalDevice.hpp"
#include <vulkan/vulkan_core.h>

namespace vkf {
    class Queue {
        public:

        VkQueue handle;

        Queue(LogicalDevice* , uint32_t);

        void submit(const VkSubmitInfo*, uint32_t, VkFence);
        void present(VkPresentInfoKHR*);
        void waitIdle();

        ~Queue() = default;

        private:

        uint32_t familyIndex = 0;
        VkQueueFlags capabilities = 0;

    };
}
