#include "../includes/logicalDevice.hpp"
#include <GL/glext.h>
#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <vector>
#include <set>

#include "../includes/physicalDevice.hpp"

#include "../vulkanConfig.hpp"

using namespace vkf;

LogicalDevice::LogicalDevice(Instance* instance) {
    this->instance = instance;

    physicalDevice = pickPhysicalDevices(instance);

    QueueFamily indices = QueueFamily::findQueueFamilies(physicalDevice, instance->surface);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentationFamily.value(), indices.transferFamily.value(), indices.computeFamily.value() };
	float queuePriority = 1.0f;

	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// Acceleration Structure feature
	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelStructFeatures{};
	accelStructFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	accelStructFeatures.accelerationStructure = VK_TRUE;

	// Ray Tracing Pipeline feature
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rayTracingPipelineFeatures{};
	rayTracingPipelineFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
	rayTracingPipelineFeatures.rayTracingPipeline = VK_TRUE;
	rayTracingPipelineFeatures.pNext = &accelStructFeatures;

	//Needed for controlling buffer access
	VkPhysicalDeviceVulkan12Features features12 = {};
	features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
	features12.shaderInt8 = VK_TRUE;
	features12.storageBuffer8BitAccess = VK_TRUE;
	features12.bufferDeviceAddress = VK_TRUE;
	features12.pNext = &rayTracingPipelineFeatures;

	// Root device features structure
	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.features.shaderInt64 = VK_TRUE;
	deviceFeatures2.pNext = &features12;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = VK_NULL_HANDLE;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pNext = &deviceFeatures2;

	#if VALIDATION_LAYERS_ENABLED
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
       	createInfo.ppEnabledLayerNames = validationLayers.data();
    #else
        createInfo.enabledLayerCount = 0;
    #endif

	VK_CHECK(vkCreateDevice(physicalDevice, &createInfo, nullptr, &handle), "Failed to create logical device")

	VkQueue _graphicsQueue, _transferQueue, _computeQueue, _presentationQueue;

	vkGetDeviceQueue(handle, indices.graphicsFamily.value(), 0, &_graphicsQueue);
	vkGetDeviceQueue(handle, indices.presentationFamily.value(), 0, &_presentationQueue);
	vkGetDeviceQueue(handle, indices.transferFamily.value(), 0, &_transferQueue);
	vkGetDeviceQueue(handle, indices.computeFamily.value(), 0, &_computeQueue);

	graphicsQueue.handle = _graphicsQueue;
	graphicsQueue.familyIndex = indices.graphicsFamily.value();

	transferQueue.handle = _transferQueue;
	transferQueue.familyIndex = indices.transferFamily.value();

	computeQueue.handle = _computeQueue;
	computeQueue.familyIndex = indices.computeFamily.value();

	presentationQueue.handle = _presentationQueue;
	presentationQueue.familyIndex = indices.presentationFamily.value();
}

void LogicalDevice::createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& bufferHandle, VkDeviceMemory& bufferMemory) {

    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(handle, &createInfo, nullptr, &bufferHandle), "Failed to create buffer")

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(handle, bufferHandle, &memoryRequirements);


    VkMemoryAllocateFlagsInfo flagInfo{};
    flagInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    flagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateInfo memoryAllocInfo{};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memoryRequirements.size;
    memoryAllocInfo.pNext = &flagInfo;
    memoryAllocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits, properties);

    VK_CHECK(vkAllocateMemory(handle, &memoryAllocInfo, nullptr, &bufferMemory), "failed to allocate memory for buffer")

    VK_CHECK(vkBindBufferMemory(handle, bufferHandle, bufferMemory, 0), "Failed to bind buffer memory")
}

void LogicalDevice::destroyBuffer(VkBuffer buffer, VkDeviceMemory bufferMemory) {
    vkDestroyBuffer(handle, buffer, nullptr);
    vkFreeMemory(handle, bufferMemory, nullptr);
}

void LogicalDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPool& commandPool, VkCommandPoolCreateFlags flags) {
    VkCommandPoolCreateInfo poolCreateInfo{};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolCreateInfo.flags = flags;
	poolCreateInfo.queueFamilyIndex = queueFamilyIndex;

	VK_CHECK(vkCreateCommandPool(handle, &poolCreateInfo, nullptr, &commandPool), "Failed to create command pool")
}

void LogicalDevice::destroyCommandPool(VkCommandPool commandPool) {
    vkDestroyCommandPool(handle, commandPool, nullptr);
}

void LogicalDevice::createImage(VkFormat format, VkExtent2D extent, VkImage& image, VkDeviceMemory& imgMemory) {
    VkImageCreateInfo imgCreateInfo{};
    imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgCreateInfo.format = format;
    imgCreateInfo.extent = {extent.width, extent.height, 1};
    imgCreateInfo.arrayLayers = 1;
    imgCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imgCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imgCreateInfo.mipLevels = 1;
    imgCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imgCreateInfo.pQueueFamilyIndices = nullptr;
    imgCreateInfo.queueFamilyIndexCount = 0;
    imgCreateInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    imgCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imgCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imgCreateInfo.flags = 0;

    VK_CHECK(vkCreateImage(handle, &imgCreateInfo, nullptr, &image), "Faield to create Image");

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(handle, image, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits,  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(handle, &allocInfo, nullptr, &imgMemory), "Failed to allocate image memory");

    VK_CHECK(vkBindImageMemory(handle, image, imgMemory, 0), "Failed to bind Image memory");
}

void LogicalDevice::createImageView(VkImage image, VkFormat format, VkImageView& imageView) {
    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(handle, &viewCreateInfo, nullptr, &imageView), "Failed to create Image view");
}

void LogicalDevice::destroyImage(VkImage image, VkDeviceMemory imgMemory, VkImageView imageView) {
    vkDestroyImageView(handle, imageView, nullptr);
    vkDestroyImage(handle, image, nullptr);
    vkFreeMemory(handle, imgMemory, nullptr);
}

uint32_t LogicalDevice::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if( (typeFilter & (1 << i)) && ( (memProperties.memoryTypes[i].propertyFlags & properties) == properties) ) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type for the buffer");
}

LogicalDevice::~LogicalDevice() {
    vkDestroyDevice(handle, nullptr);
}
