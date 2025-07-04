#include "../../includes/VulkanFramework/queue.h"
#include <vulkan/vulkan_core.h>

using namespace vkf;

void Queue::submit(const VkSubmitInfo* submits, uint32_t submitCount, VkFence fence = VK_NULL_HANDLE) {
    VK_CHECK(vkQueueSubmit(handle, submitCount, submits, fence), "Failed to submit to queue")
}

void Queue::present(VkPresentInfoKHR* presentInfo) {
    VK_CHECK(vkQueuePresentKHR(handle, presentInfo), "Failed to present")
}

void Queue::waitIdle() {
    VK_CHECK(vkQueueWaitIdle(handle), "Failed to wait for queue")
}
