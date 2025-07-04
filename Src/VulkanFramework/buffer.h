#pragma once

#include <vulkan/vulkan_core.h>

#include "commandBuffer.h"

namespace vkf {
    struct Buffer {
        VkBuffer handle;
        VkDeviceMemory bufferMemory;

        static void copyBuffer(Buffer src, Buffer dst, int size, CommandBuffer commandBuffer, int srcOffset = 0, int dstOffset = 0) {
            VkBufferCopy copyRegion{};
            copyRegion.dstOffset = dstOffset;
            copyRegion.srcOffset = srcOffset;
            copyRegion.size = size;

            vkCmdCopyBuffer(commandBuffer.handle, src.handle, dst.handle, 1, &copyRegion);
        }
    };
}
