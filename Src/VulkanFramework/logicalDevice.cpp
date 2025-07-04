#include "../../includes/VulkanFramework/logicalDevice.h"
#include <vulkan/vulkan_core.h>

using namespace vkf;

void LogicalDevice::createLogicalDevice(VkInstance instance, VkSurfaceKHR surface) {
    physicalDevice = vkf::pickPhysicalDevices(instance,  surface);

    QueueFamily indices = QueueFamily::findQueueFamilies(physicalDevice, surface);
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

	vkGetDeviceQueue(handle, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(handle, indices.presentationFamily.value(), 0, &presentationQueue);
	vkGetDeviceQueue(handle, indices.transferFamily.value(), 0, &transferQueue);
	vkGetDeviceQueue(handle, indices.computeFamily.value(), 0, &computeQueue);
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

void LogicalDevice::destroy() {
    vkDestroyDevice(handle, nullptr);
}
