#include "raytracer.h"
#include <cstdint>
#include <glm/detail/compute_common.hpp>
#include <glm/matrix.hpp>
#include <iostream>
#include <stdexcept>
#include <sys/types.h>
#include <vector>
#include <vulkan/vulkan_core.h>

void RayTracer::createRayTracer(vkf::LogicalDevice* _logicalDevice, VkCommandPool _graphicsPool, VkCommandPool _transferPool, VkSurfaceFormatKHR format, VkExtent2D extent, GLFWwindow* _window) {
    logicalDevice = _logicalDevice;

    graphicsPool = _graphicsPool;
    transferPool = _transferPool;
    window = _window;

    loadFunctions();
    accelerationStructureManager.create(device, physicalDevice, graphicsQueue, graphicsPool, transferQueue, transferPool);
    cam.Initialize();

    // Get ray tracing pipeline properties, which will be used later on in the sample
	rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProperties2{};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &rayTracingPipelineProperties;
	vkGetPhysicalDeviceProperties2(logicalDevice->physicalDevice, &deviceProperties2);

	// Get acceleration structure properties, which will be used later on in the sample
	accelerationStructureFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
	VkPhysicalDeviceFeatures2 deviceFeatures2{};
	deviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
	deviceFeatures2.pNext = &accelerationStructureFeatures;
	vkGetPhysicalDeviceFeatures2(logicalDevice->physicalDevice, &deviceFeatures2);

	VoxelModel monu1 = VoxelModel::loadModel("Models/monu1.vox");
	VoxelModel monu2 = VoxelModel::loadModel("Models/monu2.vox");
	VoxelModel monu3 = VoxelModel::loadModel("Models/monu3.vox");
	VoxelModel test = VoxelModel::loadModel("Models/test.vox");

	vector<uint8_t> monu1BrickMap = BrickMap::buildBrickmap(monu1.voxels.data(), monu1.side);
    vector<uint8_t> monu2BrickMap = BrickMap::buildBrickmap(monu2.voxels.data(),  monu2.side);
    vector<uint8_t> monu3BrickMap = BrickMap::buildBrickmap(monu3.voxels.data(), monu3.side);
    vector<uint8_t> testBrickMap = BrickMap::buildBrickmap(test.voxels.data(), test.side);

    accelerationStructureManager.addBottomLevelAccelerationStructure(BRICKMAP, glm::vec3(-70, 0, 0), monu1.side, 0.1f, monu1BrickMap.data(), monu1BrickMap.size());
    accelerationStructureManager.addBottomLevelAccelerationStructure(BRICKMAP, glm::vec3(0), monu2.side, 0.1f, monu2BrickMap.data(), monu2BrickMap.size());
    accelerationStructureManager.addBottomLevelAccelerationStructure(BRICKMAP, glm::vec3(-25, 0, 0), monu3.side, 0.1f, monu3BrickMap.data(), monu3BrickMap.size());
    accelerationStructureManager.addBottomLevelAccelerationStructure(BRICKMAP, glm::vec3(30, 0, 0), test.side, 0.1f, testBrickMap.data(), testBrickMap.size());

    accelerationStructureManager.createTopLevelAccelerationStructure();

    createImage(format, extent);
    createUBOBuffer();
    createDescritorSets();
    createRayTracingPipeline();
    createShaderBindingTable();
}

void RayTracer::loadFunctions() {
    LOAD_FUNC(logicalDevice->handle, vkCreateRayTracingPipelinesKHR);
    LOAD_FUNC(logicalDevice->handle, vkGetRayTracingShaderGroupHandlesKHR);
    LOAD_FUNC(logicalDevice->handle, vkCmdTraceRaysKHR);
}

VkShaderModule RayTracer::createShaderModule(const string& filePath) {
	ifstream file(filePath, std::ios::ate | std::ios::binary);

	if(!file.is_open()) throw runtime_error("Failed to open shader file");

	size_t fileSize = (size_t) file.tellg();
	vector<char> code(fileSize);

	file.seekg(0);
	file.read(code.data(), fileSize);

	file.close();

	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;

    VK_CHECK(vkCreateShaderModule(logicalDevice->handle, &createInfo, nullptr, &shaderModule) , "Failed to create shader module");

	return shaderModule;
}

VkPipelineShaderStageCreateInfo RayTracer::createShaderStageCreateInfo(VkShaderModule shaderModule, VkShaderStageFlagBits flags) {
	VkPipelineShaderStageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.module = shaderModule;
	createInfo.stage = flags;
	createInfo.pName = "main";

	return createInfo;
}

uint32_t RayTracer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(logicalDevice->physicalDevice, &memProperties);

    for(uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if( (typeFilter & (1 << i)) && ( (memProperties.memoryTypes[i].propertyFlags & properties) == properties) ) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type for the buffer");
}

void RayTracer::setImgLayout(CommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subResourcesRange, VkPipelineStageFlags srcFlags , VkPipelineStageFlags dstFlags) {
    VkImageMemoryBarrier memoryBarrier;
    memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    memoryBarrier.oldLayout = oldLayout;
    memoryBarrier.newLayout = newLayout;

    memoryBarrier.image = image;
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
    vkCmdPipelineBarrier(commandBuffer.handle, srcFlags, dstFlags, 0, 0, nullptr, 0, nullptr, 1, &memoryBarrier);
}

void RayTracer::createImage(VkSurfaceFormatKHR format, VkExtent2D extent) {
    VkImageCreateInfo imgCreateInfo{};
    imgCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imgCreateInfo.format = format.format;
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

    VK_CHECK(vkCreateImage(logicalDevice->handle, &imgCreateInfo, nullptr, &frame), "Faield to create Image");

    VkMemoryRequirements memoryRequirements{};
    vkGetImageMemoryRequirements(logicalDevice->handle, frame, &memoryRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memoryRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memoryRequirements.memoryTypeBits,  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VK_CHECK(vkAllocateMemory(logicalDevice->handle, &allocInfo, nullptr, &imgMemory), "Failed to allocate image memory");

    VK_CHECK(vkBindImageMemory(logicalDevice->handle, frame, imgMemory, 0), "Failed to bind Image memory");

    VkImageViewCreateInfo viewCreateInfo{};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = frame;
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewCreateInfo.format = format.format;
	viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.baseMipLevel = 0;
	viewCreateInfo.subresourceRange.levelCount = 1;
	viewCreateInfo.subresourceRange.baseArrayLayer = 0;
	viewCreateInfo.subresourceRange.layerCount = 1;

    VK_CHECK(vkCreateImageView(logicalDevice->handle, &viewCreateInfo, nullptr, &frameView), "Failed to create Image view");

    CommandBuffer commandBuffer = logicalDevice->createCommandBuffer(graphicsPool);
    commandBuffer.beginRecording(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

    setImgLayout(commandBuffer, frame, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    commandBuffer.endRecording();

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer.handle;

	// Create fence to ensure that the command buffer has finished executing
	VkFenceCreateInfo fenceInfo{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO};
	VkFence fence;
	VK_CHECK(vkCreateFence(logicalDevice->handle, &fenceInfo, nullptr, &fence), "Failed to create fence");

	// Submit to the queue
	VK_CHECK(vkQueueSubmit(logicalDevice->graphicsQueue, 1, &submitInfo, fence), "Failed to submit to queue");
	// Wait for the fence to signal that command buffer has finished executing
	VK_CHECK(vkWaitForFences(logicalDevice->handle, 1, &fence, VK_TRUE, UINT64_MAX), "Queeus failed to comepelte");

	vkDestroyFence(logicalDevice->handle, fence, nullptr);

    imgExtent = extent;
}

void RayTracer::createUBOBuffer() {
    ubo = logicalDevice->createBuffer(sizeof(CameraConstants), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    CameraConstants camCons{};
    camCons.inverseView = glm::inverse(glm::mat4(1));
    camCons.inverseProj = glm::inverse(glm::perspective(glm::radians(45.0f), (float)imgExtent.width / (float) imgExtent.height, 0.1f, 100.0f));

    void* mapped;
    vkMapMemory(logicalDevice->handle, ubo.bufferMemory, 0, sizeof(camCons), 0, &mapped);
    memcpy(mapped, &camCons, sizeof(camCons));
    vkUnmapMemory(logicalDevice->handle, ubo.bufferMemory);
}

void RayTracer::createDescritorSets() {
    VkDescriptorSetLayoutBinding asBindings{};
    asBindings.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    asBindings.binding = 0;
    asBindings.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    asBindings.descriptorCount = 1;
    asBindings.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding imgBindings{};
    imgBindings.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imgBindings.binding = 1;
    imgBindings.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    imgBindings.descriptorCount = 1;
    imgBindings.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding camBindings{};
    camBindings.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    camBindings.binding = 2;
    camBindings.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    camBindings.descriptorCount = 1;
    camBindings.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding storage{};
    storage.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storage.binding = 3;
    storage.stageFlags = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    storage.descriptorCount = 1;
    storage.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutBinding bindingInfo[] = {asBindings, imgBindings, camBindings, storage};

    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = 4;
    layoutCreateInfo.pBindings = bindingInfo;
    layoutCreateInfo.pNext = nullptr;

    VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice->handle, &layoutCreateInfo, nullptr, &set0Layout), "Failed to create descriptor set layout");

    VkDescriptorPoolSize asPoolSize{};
    asPoolSize.descriptorCount = 1;
    asPoolSize.type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;

    VkDescriptorPoolSize imgPoolSize{};
    imgPoolSize.descriptorCount = 1;
    imgPoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

    VkDescriptorPoolSize camPoolSize{};
    camPoolSize.descriptorCount = 1;
    camPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

    VkDescriptorPoolSize storagePoolSize{};
    storagePoolSize.descriptorCount = 1;
    storagePoolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;

    VkDescriptorPoolSize poolSizes[] = { asPoolSize, imgPoolSize, camPoolSize, storagePoolSize};

    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.pNext = nullptr;
    poolCreateInfo.maxSets = 1;
    poolCreateInfo.poolSizeCount = 4;
    poolCreateInfo.pPoolSizes = poolSizes;

    VK_CHECK(vkCreateDescriptorPool(logicalDevice->handle, &poolCreateInfo, nullptr, &descriptorPool), "Failed to make descriptor pool");

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pNext = nullptr;
    allocInfo.pSetLayouts = &set0Layout;

    VK_CHECK(vkAllocateDescriptorSets(logicalDevice->handle, &allocInfo, &set0), "Failed to allocate descriptor set");

    VkWriteDescriptorSetAccelerationStructureKHR desASInfo{};
    desASInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    desASInfo.accelerationStructureCount = 1;
    VkAccelerationStructureKHR tlas = accelerationStructureManager.getTopLevelAccelrationStructure();
    desASInfo.pAccelerationStructures = &tlas;
    desASInfo.pNext = nullptr;

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = frameView;

    VkDescriptorBufferInfo camInfo{};
    camInfo.buffer = ubo.handle;
    camInfo.offset = 0;
    camInfo.range = sizeof(CameraConstants);

    VkDescriptorBufferInfo storageInfo{};
    storageInfo.buffer = accelerationStructureManager.getInstanceBuffer();
    storageInfo.offset = 0;
    storageInfo.range = MAX_BLAS * sizeof(InstanceData);

    VkWriteDescriptorSet asWrite{};
    asWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    asWrite.descriptorCount = 1;
    asWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    asWrite.dstBinding = 0;
    asWrite.dstSet = set0;
    asWrite.pNext = &desASInfo;

    VkWriteDescriptorSet imgWrite{};
    imgWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imgWrite.descriptorCount = 1;
    imgWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imgWrite.dstBinding = 1;
    imgWrite.dstSet = set0;
    imgWrite.pImageInfo = &imgInfo;

    VkWriteDescriptorSet camWrite{};
    camWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    camWrite.descriptorCount = 1;
    camWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    camWrite.dstBinding = 2;
    camWrite.dstSet = set0;
    camWrite.pBufferInfo = &camInfo;

    VkWriteDescriptorSet storageWrite{};
    storageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    storageWrite.descriptorCount = 1;
    storageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    storageWrite.dstBinding = 3;
    storageWrite.dstSet = set0;
    storageWrite.pBufferInfo = &storageInfo;

    VkWriteDescriptorSet writeInfo[] = {asWrite, imgWrite, camWrite, storageWrite};

    vkUpdateDescriptorSets(logicalDevice->handle, 4, writeInfo, 0, VK_NULL_HANDLE);
}

void RayTracer::updateDescriptorSets(float deltaTime) {
    CameraConstants camCons{};
    camCons.inverseProj = glm::inverse(glm::perspective(glm::radians(45.0f), (float)imgExtent.width / (float) imgExtent.height, 0.1f, 100.0f));

    glm::mat4 view;

    cam.UpdateCamera(deltaTime, window, &view);

    camCons.inverseView = glm::inverse(view);

    void* mapped;
    vkMapMemory(logicalDevice->handle, ubo.bufferMemory, 0, sizeof(camCons), 0, &mapped);
    memcpy(mapped, &camCons, sizeof(camCons));
    vkUnmapMemory(logicalDevice->handle, ubo.bufferMemory);


    VkDescriptorBufferInfo camInfo{};
    camInfo.buffer = ubo.handle;
    camInfo.offset = 0;
    camInfo.range = sizeof(CameraConstants);


    VkWriteDescriptorSet camWrite{};
    camWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    camWrite.descriptorCount = 1;
    camWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    camWrite.dstBinding = 2;
    camWrite.dstSet = set0;
    camWrite.pBufferInfo = &camInfo;

    VkWriteDescriptorSet writeInfo[] = {camWrite};

    vkUpdateDescriptorSets(logicalDevice->handle, 1, writeInfo, 0, VK_NULL_HANDLE);
}

void RayTracer::createRayTracingPipeline() {
    enum ShaderIndices {
        iRaygen,
        iMiss,
        iClosestHit,
        iIntersection,
        ShaderCount
    };

    vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos(ShaderCount);

    VkShaderModule raygenMod = createShaderModule("Shaders/raygen.spv");
    shaderCreateInfos[iRaygen] = createShaderStageCreateInfo(raygenMod, VK_SHADER_STAGE_RAYGEN_BIT_KHR);

    VkShaderModule missMod = createShaderModule("Shaders/miss.spv");
    shaderCreateInfos[iMiss] = createShaderStageCreateInfo(missMod, VK_SHADER_STAGE_MISS_BIT_KHR);

    VkShaderModule closestHitMod = createShaderModule("Shaders/closestHit.spv");
    shaderCreateInfos[iClosestHit] = createShaderStageCreateInfo(closestHitMod, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);

    VkShaderModule intersectionMod = createShaderModule("Shaders/intersection.spv");
    shaderCreateInfos[iIntersection] = createShaderStageCreateInfo(intersectionMod, VK_SHADER_STAGE_INTERSECTION_BIT_KHR);

    VkRayTracingShaderGroupCreateInfoKHR group{};
    group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    group.anyHitShader       = VK_SHADER_UNUSED_KHR;
    group.closestHitShader   = VK_SHADER_UNUSED_KHR;
    group.generalShader      = VK_SHADER_UNUSED_KHR;
    group.intersectionShader = VK_SHADER_UNUSED_KHR;

    // Raygen
    group.type          = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group.generalShader = static_cast<uint32_t>(iRaygen);
    shaderGroups.push_back(group);

    // Miss
    group.type          = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
    group.generalShader = static_cast<uint32_t>(iMiss);
    shaderGroups.push_back(group);

    // Closest hit
    group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
    group.generalShader = VK_SHADER_UNUSED_KHR;
    group.closestHitShader = static_cast<uint32_t>(iClosestHit);
    group.intersectionShader = static_cast<uint32_t>(iIntersection);
    shaderGroups.push_back(group);

    VkPipelineLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    //Push constants
    VkPushConstantRange pushConstanctRange{};
    pushConstanctRange.offset = 0;
    pushConstanctRange.size = sizeof(CameraConstants);

    layoutCreateInfo.pushConstantRangeCount = 1;
    layoutCreateInfo.pPushConstantRanges = &pushConstanctRange;

    //Set the descriptor sets
    layoutCreateInfo.setLayoutCount = 1;
    layoutCreateInfo.pSetLayouts = &set0Layout;

    VK_CHECK(vkCreatePipelineLayout(logicalDevice->handle, &layoutCreateInfo, nullptr, &rayTracingPipelineLayout), "Failed to create rt pipeline layout");

    //create the pipeline
    VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineCreateInfo.stageCount = shaderCreateInfos.size();
    pipelineCreateInfo.pStages = shaderCreateInfos.data();

    pipelineCreateInfo.groupCount = shaderGroups.size();
    pipelineCreateInfo.pGroups = shaderGroups.data();
    pipelineCreateInfo.maxPipelineRayRecursionDepth = 2;
    pipelineCreateInfo.layout = rayTracingPipelineLayout;

    VK_CHECK(vkCreateRayTracingPipelinesKHR(logicalDevice->handle, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &rayTracingPipeline), "Failed to create ray tracing pipeline");

    vkDestroyShaderModule(logicalDevice->handle, raygenMod, nullptr);
    vkDestroyShaderModule(logicalDevice->handle, missMod, nullptr);
    vkDestroyShaderModule(logicalDevice->handle, closestHitMod, nullptr);
    vkDestroyShaderModule(logicalDevice->handle, intersectionMod, nullptr);
}

void RayTracer::createShaderBindingTable() {
    const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
    const uint32_t handleAlignemt = rayTracingPipelineProperties.shaderGroupHandleAlignment;
    const uint32_t baseAligment = rayTracingPipelineProperties.shaderGroupBaseAlignment;
    const uint32_t groupCount = shaderGroups.size();

    const uint32_t handleSizeAligned = (handleSize + handleAlignemt - 1) & ~(handleAlignemt - 1);

    vector<uint8_t> shaderHandles(groupCount * handleSize);
    VK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(logicalDevice->handle, rayTracingPipeline, 0, groupCount, shaderHandles.size(), shaderHandles.data()), "Failed to get shader handles");

    sbtBuffer = logicalDevice->createBuffer(groupCount * baseAligment, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkf::Buffer sbtStagingBuffer = logicalDevice->createBuffer(groupCount * baseAligment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    //Put the data in the buffer
    void* mappedData = nullptr;
    vkMapMemory(logicalDevice->handle, sbtStagingBuffer.bufferMemory, 0, groupCount * handleAlignemt, 0, &mappedData);

    uint8_t* pData = reinterpret_cast<uint8_t*>(mappedData);

    for (uint32_t i = 0; i < groupCount; i++) {
        memcpy(pData + i * baseAligment, shaderHandles.data() + i * handleSize, handleSize);
    }

    vkUnmapMemory(logicalDevice->handle, sbtStagingBuffer.bufferMemory);

    CommandBuffer commandBuffer = logicalDevice->createCommandBuffer(transferPool);
    vkf::Buffer::copyBuffer(sbtStagingBuffer, sbtBuffer, groupCount * baseAligment, commandBuffer);

    VkDeviceAddress sbtAddress = logicalDevice->getBufferAddress(sbtBuffer);

    //Finally get the device addresses and shit idk wtf
    rgenRegion.deviceAddress = sbtAddress;
    rgenRegion.size = handleSizeAligned;
    rgenRegion.stride = handleSizeAligned;

    missRegion.deviceAddress = sbtAddress + baseAligment;
    missRegion.size = handleSizeAligned;
    missRegion.stride = handleSizeAligned;

    hitRegion.deviceAddress = sbtAddress + 2 * baseAligment;
    hitRegion.size = handleSizeAligned;
    hitRegion.stride = handleSizeAligned;
}

void RayTracer::handleResize(VkSurfaceFormatKHR format, VkExtent2D extent) {
    vkDestroyImage(logicalDevice->handle, frame, nullptr);
    vkDestroyImageView(logicalDevice->handle, frameView, nullptr);
    vkFreeMemory(logicalDevice->handle, imgMemory, nullptr);

    createImage(format, extent);

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = frameView;

    VkWriteDescriptorSet imgWrite{};
    imgWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imgWrite.descriptorCount = 1;
    imgWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imgWrite.dstBinding = 1;
    imgWrite.dstSet = set0;
    imgWrite.pImageInfo = &imgInfo;

    vkUpdateDescriptorSets(logicalDevice->handle, 1, &imgWrite, 0, VK_NULL_HANDLE);
}

void RayTracer::recordCommandBuffer(CommandBuffer commandBuffer, VkImage swapchainImage, float deltaTime) {
    commandBuffer.beginRecording(0);

    CameraConstants camCons{};
    camCons.inverseProj = glm::inverse(glm::perspective(glm::radians(45.0f), (float)imgExtent.width / (float) imgExtent.height, 0.1f, 100.0f));
    glm::mat4 view;
    cam.UpdateCamera(deltaTime, window, &view);
    camCons.inverseView = glm::inverse(view);

    vkCmdBindPipeline(commandBuffer.handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipeline);
    vkCmdPushConstants(commandBuffer.handle, rayTracingPipelineLayout, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, sizeof(CameraConstants), &camCons);
    vkCmdBindDescriptorSets(commandBuffer.handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, rayTracingPipelineLayout, 0, 1, &set0, 0, 0);

    vkCmdTraceRaysKHR(commandBuffer.handle, &rgenRegion, &missRegion, &hitRegion, &callRegion, imgExtent.width, imgExtent.height, 1);

    //Now we shall begin copying the generated img to the swapchain.
    VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    //Change the layouts of the images to help copying
    setImgLayout(commandBuffer, swapchainImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
    //Change the layout of the created image
    setImgLayout(commandBuffer, frame, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

    //Now we shall do the actual copy operation
    VkImageCopy copyRegion{};
	copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent = { imgExtent.width, imgExtent.height, 1 };

    vkCmdCopyImage(commandBuffer.handle, frame, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    //Set the layout of the images back
    setImgLayout(commandBuffer, swapchainImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subresourceRange);
    setImgLayout(commandBuffer, frame, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, subresourceRange);

    commandBuffer.endRecording();
}

void RayTracer::drawFrame(CommandBuffer commandBuffer, VkImage swapchainImage, float deltaTime) {

    updateDescriptorSets(deltaTime);

    recordCommandBuffer(commandBuffer, swapchainImage, deltaTime);

}

void RayTracer::cleanup() {
    accelerationStructureManager.cleanup();

    vkDestroyImage(logicalDevice->handle, frame, nullptr);
    vkDestroyImageView(logicalDevice->handle, frameView, nullptr);
    vkFreeMemory(logicalDevice->handle, imgMemory, nullptr);

    vkDestroyDescriptorSetLayout(logicalDevice->handle, set0Layout, nullptr);
    vkDestroyDescriptorPool(logicalDevice->handle, descriptorPool, nullptr);

    logicalDevice->destroyBuffer(sbtBuffer);

    vkDestroyPipeline(logicalDevice->handle, rayTracingPipeline, nullptr);
    vkDestroyPipelineLayout(logicalDevice->handle, rayTracingPipelineLayout, nullptr);
}
