#include "../../includes/VulkanFramework/buffer.h"

using namespace vkf;

Buffer::Buffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, LogicalDevice* logicalDevice) {
    logicalDevice->createBuffer(size, usage, properties, handle, bufferMemory);

    this->size = size;
    this->usage = usage;
    this->properties = properties;
    this->logicalDevice = logicalDevice;
}

VkDeviceAddress Buffer::getBufferAddress() {
    VkBufferDeviceAddressInfo addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    addressInfo.buffer = handle;

    return vkGetBufferDeviceAddress(logicalDevice->handle, &addressInfo);
}

void Buffer::copyBuffer(Buffer src, Buffer dst, int size, VkCommandBuffer commandBuffer, int srcOffset, int dstOffset) {
    VkBufferCopy copyRegion{};
    copyRegion.dstOffset = dstOffset;
    copyRegion.srcOffset = srcOffset;
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer, src.handle, dst.handle, 1, &copyRegion);
}

Buffer::~Buffer() {
    logicalDevice->destroyBuffer(handle, bufferMemory);
}
