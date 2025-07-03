#pragma once

#include <vulkan/vulkan_core.h>

#include "commandBuffer.h"

namespace vkf {
    struct Buffer {
        VkBuffer handle;
        VkDeviceMemory bufferMemory;

        VkDeviceAddress getBufferAddress(VkDevice device) {
            VkBufferDeviceAddressInfo addressInfo{};
            addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
            addressInfo.buffer = handle;

            return vkGetBufferDeviceAddress(device, &addressInfo);
        }

        static void copyBuffer(Buffer src, Buffer dst, int size, CommandBuffer commandBuffer, int srcOffset = 0, int dstOffset = 0) {
            VkBufferCopy copyRegion{};
            copyRegion.dstOffset = dstOffset;
            copyRegion.srcOffset = srcOffset;
            copyRegion.size = size;

            vkCmdCopyBuffer(commandBuffer.handle, src.handle, dst.handle, 1, &copyRegion);
        }
    };
}
