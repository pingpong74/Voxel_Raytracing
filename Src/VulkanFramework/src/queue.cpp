#include "../includes/queue.hpp"
#include <vulkan/vulkan_core.h>

#include "../vulkanConfig.hpp"

using namespace vkf;

void Queue::flushCommandBuffer(VkCommandBuffer commandBuffer) {
    VkSubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    vkQueueSubmit(handle, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(handle);
}

void Queue::submit(const VkSubmitInfo* submits, uint32_t submitCount, VkFence fence = VK_NULL_HANDLE) {
    VK_CHECK(vkQueueSubmit(handle, submitCount, submits, fence), "Failed to submit to queue")
}

void Queue::present(VkPresentInfoKHR* presentInfo) {
    VK_CHECK(vkQueuePresentKHR(handle, presentInfo), "Failed to present")
}

void Queue::waitIdle() {
    VK_CHECK(vkQueueWaitIdle(handle), "Failed to wait for queue")
}
