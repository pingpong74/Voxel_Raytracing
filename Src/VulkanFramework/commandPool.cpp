#include "../../includes/VulkanFramework/commandPool.h"
#include <vulkan/vulkan_core.h>

using namespace vkf;

CommandPool::CommandPool(LogicalDevice* logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT) {
    this->logicalDevice = logicalDevice;
    this->queueFamilyIndex = queueFamilyIndex;

    logicalDevice->createCommandPool(queueFamilyIndex, handle, flags);
}

VkCommandBuffer CommandPool::allocateCommandBuffer(VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY) {
    VkCommandBuffer commandBuffer;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = handle;
    allocInfo.level = level;
    allocInfo.pNext = nullptr;

    VK_CHECK(vkAllocateCommandBuffers(logicalDevice->handle, &allocInfo, &commandBuffer), "Failed to create command Buffer")

    return commandBuffer;
}

void CommandPool::freeCommandBuffer(VkCommandBuffer commandBuffer) {
    vkFreeCommandBuffers(logicalDevice->handle, handle, 1, &commandBuffer);
}

void CommandPool::reset(VkCommandPoolResetFlags flags = 0){
    VK_CHECK(vkResetCommandPool(logicalDevice->handle, handle, flags), "failed to reset command pool");
}

CommandPool::~CommandPool() {
    logicalDevice->destroyCommandPool(handle);
}
