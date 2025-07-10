#include "../includes/image.hpp"
#include <vulkan/vulkan_core.h>

using namespace vkf;

Image::Image(LogicalDevice* logicalDevice, VkFormat format, VkExtent2D extent) {
    this->logicalDevice = logicalDevice;
    this->format = format;
    this->extent = extent;

    logicalDevice->createImage(format, extent, handle, imageMemory);
    logicalDevice->createImageView(handle, format, view);
}

void Image::transitionLayout(VkCommandBuffer commandBuffer,VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subResourcesRange, VkPipelineStageFlags srcFlags, VkPipelineStageFlags dstFlags) {
    VkImageMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    memoryBarrier.oldLayout = oldLayout;
    memoryBarrier.newLayout = newLayout;

    memoryBarrier.image = handle;
    memoryBarrier.subresourceRange = subResourcesRange;

    switch (oldLayout) {

        case VK_IMAGE_LAYOUT_UNDEFINED:
            memoryBarrier.srcAccessMask = 0;
            break;

        case VK_IMAGE_LAYOUT_PREINITIALIZED:
            memoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            memoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
            memoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            break;

        case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
            memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
            break;

        default:
            break;
    }

    switch(newLayout) {
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            memoryBarrier.dstAccessMask = 0; // No access needed after present
            break;

        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			memoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			memoryBarrier.dstAccessMask = memoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			if (memoryBarrier.srcAccessMask == 0) {
				memoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
			}
			memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;

        default:
            break;
    }

    memoryBarrier.pNext = nullptr;
    vkCmdPipelineBarrier(commandBuffer, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
}

Image::~Image() {
    logicalDevice->destroyImage(handle, imageMemory, view);
}
