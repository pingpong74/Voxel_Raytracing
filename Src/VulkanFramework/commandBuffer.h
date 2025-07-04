#pragma once

#include <vulkan/vulkan_core.h>
#include "../config.h"

struct CommandBuffer {
    VkCommandBuffer handle;
    VkCommandPool commandPool;

    void reset() {
        vkResetCommandBuffer(handle, 0);
    }

    void beginRecording(VkCommandBufferUsageFlags flags) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flags;

        vkBeginCommandBuffer(handle, &beginInfo);
    }

    void endRecording() {
        vkEndCommandBuffer(handle);
    }

    void flush(VkQueue queue) {
        VkSubmitInfo submitInfo{};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &handle;
        submitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;

        VK_CHECK(vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE), "Failed to submit command buffer")
    }
};
