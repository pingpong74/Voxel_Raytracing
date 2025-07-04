#pragma once

#include "logicalDevice.h"
#include <vulkan/vulkan_core.h>

namespace vkf {
    class Queue {
        public:

        VkQueue handle;

        Queue();

        void submit(const VkSubmitInfo*, uint32_t, VkFence);
        void present(VkPresentInfoKHR*);
        void waitIdle();

        private:

        uint32_t familyIndex = 0;
        VkQueueFlags capabilities = 0;

    };
}
