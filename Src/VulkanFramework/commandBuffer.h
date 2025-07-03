#pragma once

#include <vulkan/vulkan_core.h>

struct CommandBuffer {
    VkCommandBuffer handle;

    void beginRecording(VkCommandBufferUsageFlags flags) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flags;

        vkBeginCommandBuffer(handle, &beginInfo);
    }

    void endRecording() {
        vkEndCommandBuffer(handle);
    }

    void flush() {

    }
};
