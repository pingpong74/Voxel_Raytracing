#include "swapchain.h"
#include <vulkan/vulkan_core.h>

using namespace vkf;

void Swapchain::createSwapchain(LogicalDevice* _logicalDevice, VkSurfaceKHR surface, int width, int height) {
    logicalDevice = _logicalDevice;

    SwapChainSupportDetails supportDetails = querySwapChainSupport(logicalDevice->physicalDevice, surface);

	swapchainFormat = chooseSurfaceFormat(supportDetails.formats);
	swapchainPresentMode = choosePresentMode(supportDetails.presetMode);
	swapchainExtent = chooseSwapChainExtent(supportDetails.capabilities, width, height);

	uint32_t imageCount = supportDetails.capabilities.minImageCount;

	if(imageCount < supportDetails.capabilities.maxImageCount && supportDetails.capabilities.maxImageCount > 0) imageCount = supportDetails.capabilities.maxImageCount;

	VkSwapchainCreateInfoKHR createInfo{};

	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.imageFormat = swapchainFormat.format;
	createInfo.imageColorSpace = swapchainFormat.colorSpace;
	createInfo.imageExtent = swapchainExtent;
	createInfo.presentMode = swapchainPresentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;

	QueueFamily family = QueueFamily::findQueueFamilies(logicalDevice->physicalDevice, surface);
	uint32_t queues[] = {family.graphicsFamily.value(), family.presentationFamily.value()};

	if(queues[0] != queues[1]) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
       	createInfo.queueFamilyIndexCount = 2;
       	createInfo.pQueueFamilyIndices = queues;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
       	createInfo.queueFamilyIndexCount = 0;
       	createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = supportDetails.capabilities.currentTransform;

	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if(vkCreateSwapchainKHR(logicalDevice->handle, &createInfo, nullptr, &handle) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create swapchain");
	}

	vkGetSwapchainImagesKHR(logicalDevice->handle, handle, &imageCount,nullptr);
	swapchainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice->handle, handle, &imageCount, swapchainImages.data());

	swapchainImageViews.resize(swapchainImages.size());

	for(size_t i = 0; i < swapchainImages.size(); i++) {
		VkImageViewCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapchainFormat.format;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if(vkCreateImageView(logicalDevice->handle, &createInfo, nullptr, &swapchainImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view");
		}

	}
}

VkSurfaceFormatKHR Swapchain::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
	for(const auto& format : formats) {
		if(format.format == VK_FORMAT_B8G8R8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return format;
	}

	return formats[0];
}

VkPresentModeKHR Swapchain::choosePresentMode(const std::vector<VkPresentModeKHR>& presentModes) {
	for(const auto& presentationMode: presentModes) {
		if(presentationMode == VK_PRESENT_MODE_MAILBOX_KHR) return presentationMode;
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Swapchain::chooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities, int width, int height) {

	if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

	VkExtent2D extent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

	extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
	extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

	return extent;
}

void Swapchain::recreateSwapchain() {

}

void Swapchain::destroy() {
	for(auto imageView: swapchainImageViews) {
		vkDestroyImageView(logicalDevice->handle, imageView, nullptr);
	}

	vkDestroySwapchainKHR(logicalDevice->handle, handle, nullptr);
}
