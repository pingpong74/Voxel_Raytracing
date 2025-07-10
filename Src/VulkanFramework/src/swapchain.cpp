#include "../includes/swapchain.hpp"
#include "../includes/physicalDevice.hpp"
#include "../vulkanConfig.hpp"

#include <vulkan/vulkan_core.h>
#include <limits>
#include <algorithm>

using namespace vkf;

Swapchain::Swapchain(LogicalDevice* _logicalDevice, int width, int height, VkSurfaceKHR _surface) {
    this->logicalDevice = _logicalDevice;
    this->surface = _surface;

    create(width, height);
}

void Swapchain::create(int width, int height) {
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

	VK_CHECK(vkCreateSwapchainKHR(logicalDevice->handle, &createInfo, nullptr, &handle), "Failed to create swapchain")

	std::vector<VkImage> img;
	std::vector<VkImageView> imgView;

	vkGetSwapchainImagesKHR(logicalDevice->handle, handle, &imageCount,nullptr);
	swapchainImages.resize(imageCount);
	img.resize(imageCount);
	imgView.resize(imageCount);

	vkGetSwapchainImagesKHR(logicalDevice->handle, handle, &imageCount, img.data());

	for(int i = 0; i < imageCount; i++) {
        logicalDevice->createImageView(img[i], swapchainFormat.format, imgView[i]);
        swapchainImages[i].view = imgView[i];
        swapchainImages[i].handle = img[i];
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

void Swapchain::recreateSwapchain(int width, int height) {
    vkDeviceWaitIdle(logicalDevice->handle);
    cleanup();

    create(width, height);
}

void Swapchain::cleanup() {
	vkDestroySwapchainKHR(logicalDevice->handle, handle, nullptr);
}

Swapchain::~Swapchain() {
    cleanup();
}
