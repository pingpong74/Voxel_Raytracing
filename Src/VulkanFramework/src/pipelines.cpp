#include "../includes/pipelines.hpp"
#include "../vulkanConfig.hpp"
#include <fstream>
#include <vector>
#include <vulkan/vulkan_core.h>
#include <cstring>

#include "../includes/loadedFunctions.hpp"

using namespace vkf;

VkShaderModule vkf::createShaderModule(const std::string& filePath, LogicalDevice* logicalDevice) {
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);

	if(!file.is_open()) throw std::runtime_error("Failed to open shader file");

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> code(fileSize);

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

VkPipelineShaderStageCreateInfo vkf::createShaderStageCreateInfo(VkShaderModule shaderModule, VkShaderStageFlagBits flags) {
	VkPipelineShaderStageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.module = shaderModule;
	createInfo.stage = flags;
	createInfo.pName = "main";

	return createInfo;
}

RayTracingPipeline::RayTracingPipeline(LogicalDevice* logicalDevice) : sbtBuffer(logicalDevice), rayTracingPipelineProperties() {
    this->logicalDevice = logicalDevice;


    std::cout << " oinke" << std::endl;

    rayTracingPipelineProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	VkPhysicalDeviceProperties2 deviceProperties2{};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &rayTracingPipelineProperties;
	vkGetPhysicalDeviceProperties2(logicalDevice->physicalDevice, &deviceProperties2);
	std::cout << " oinke" << std::endl;
}

void RayTracingPipeline::create(DescriptorSetLayout** layouts, uint32_t descriptorSetCount, uint32_t pushConstantsSize, std::string raygen, std::string miss, std::string closestHit, std::string intersection) {
    enum ShaderIndices {
        iRaygen,
        iMiss,
        iClosestHit,
        iIntersection,
        ShaderCount
    };

    //0 for raygen, 1 for miss, 2 for closestHit, 3 for intersection(optional)
    std::vector<VkPipelineShaderStageCreateInfo> shaderCreateInfos;

    VkShaderModule raygenMod = createShaderModule(raygen, logicalDevice);
    shaderCreateInfos.push_back(createShaderStageCreateInfo(raygenMod, VK_SHADER_STAGE_RAYGEN_BIT_KHR));

    VkShaderModule missMod = createShaderModule(miss, logicalDevice);
    shaderCreateInfos.push_back(createShaderStageCreateInfo(missMod, VK_SHADER_STAGE_MISS_BIT_KHR));

    VkShaderModule closestHitMod = createShaderModule(closestHit, logicalDevice);
    shaderCreateInfos.push_back(createShaderStageCreateInfo(closestHitMod, VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR));

    VkShaderModule intersectionMod = VK_NULL_HANDLE;
    if(intersection != "^") {
        intersectionMod = createShaderModule(intersection, logicalDevice);
        shaderCreateInfos.push_back(createShaderStageCreateInfo(intersectionMod, VK_SHADER_STAGE_INTERSECTION_BIT_KHR));
    }

    VkRayTracingShaderGroupCreateInfoKHR group{};
    group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
    group.anyHitShader       = VK_SHADER_UNUSED_KHR;
    group.closestHitShader   = VK_SHADER_UNUSED_KHR;
    group.generalShader      = VK_SHADER_UNUSED_KHR;
    group.intersectionShader = VK_SHADER_UNUSED_KHR;

    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;

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
    if(intersectionMod != VK_NULL_HANDLE) group.intersectionShader = static_cast<uint32_t>(iIntersection);
    else group.intersectionShader = VK_SHADER_UNUSED_KHR;
    shaderGroups.push_back(group);

    shaderGroupCount = shaderGroups.size();

    VkPipelineLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;

    //Push constants

    if(pushConstantsSize > 0) {
        VkPushConstantRange pushConstantsRange{};
        pushConstantsRange.offset = 0;
        pushConstantsRange.size = pushConstantsSize;
        layoutCreateInfo.pushConstantRangeCount = 1;
        layoutCreateInfo.pPushConstantRanges = &pushConstantsRange;
    }
    else {
        layoutCreateInfo.pushConstantRangeCount = 0;
        layoutCreateInfo.pPushConstantRanges = nullptr;
    }

    VkDescriptorSetLayout l[descriptorSetCount];

    //Set the descriptor sets
    if(descriptorSetCount > 0) {
        layoutCreateInfo.setLayoutCount = descriptorSetCount;

        for(int i = 0; i < descriptorSetCount; i++) {
            l[i] = (layouts[i]->handle);
        }

        layoutCreateInfo.pSetLayouts = (VkDescriptorSetLayout*)l;
    }
    else {
        layoutCreateInfo.setLayoutCount = 0;
        layoutCreateInfo.pSetLayouts = nullptr;
    }


    VK_CHECK(vkCreatePipelineLayout(logicalDevice->handle, &layoutCreateInfo, nullptr, &pipelineLayout), "Failed to create rayTracing pipeline layout");

    //create the pipeline
    VkRayTracingPipelineCreateInfoKHR pipelineCreateInfo{};
    pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
    pipelineCreateInfo.stageCount = shaderCreateInfos.size();
    pipelineCreateInfo.pStages = shaderCreateInfos.data();

    pipelineCreateInfo.groupCount = shaderGroups.size();
    pipelineCreateInfo.pGroups = shaderGroups.data();
    pipelineCreateInfo.maxPipelineRayRecursionDepth = 2;
    pipelineCreateInfo.layout = pipelineLayout;

    VK_CHECK(vkf::vkCreateRayTracingPipelinesKHR(logicalDevice->handle, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &handle), "Failed to create ray tracing pipeline");

    vkDestroyShaderModule(logicalDevice->handle, raygenMod, nullptr);
    vkDestroyShaderModule(logicalDevice->handle, missMod, nullptr);
    vkDestroyShaderModule(logicalDevice->handle, closestHitMod, nullptr);
    if(intersectionMod != VK_NULL_HANDLE) vkDestroyShaderModule(logicalDevice->handle, intersectionMod, nullptr);
}

void RayTracingPipeline::createShaderBindingTable(CommandPool* transferPool) {
    const uint32_t handleSize = rayTracingPipelineProperties.shaderGroupHandleSize;
    const uint32_t handleAlignemt = rayTracingPipelineProperties.shaderGroupHandleAlignment;
    const uint32_t baseAligment = rayTracingPipelineProperties.shaderGroupBaseAlignment;

    const uint32_t handleSizeAligned = (handleSize + handleAlignemt - 1) & ~(handleAlignemt - 1);

    std::vector<uint8_t> shaderHandles(shaderGroupCount * handleSize);
    VK_CHECK(vkGetRayTracingShaderGroupHandlesKHR(logicalDevice->handle, handle, 0, shaderGroupCount, shaderHandles.size(), shaderHandles.data()), "Failed to get shader handles");

    sbtBuffer.create(shaderGroupCount * baseAligment, VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkf::Buffer sbtStagingBuffer(shaderGroupCount * baseAligment, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, logicalDevice);

    VkDeviceAddress sbtAddress = sbtBuffer.getBufferAddress();
    //Put the data in the buffer
    void* mappedData = nullptr;
    vkMapMemory(logicalDevice->handle, sbtStagingBuffer.bufferMemory, 0, shaderGroupCount * handleAlignemt, 0, &mappedData);

    uint8_t* pData = reinterpret_cast<uint8_t*>(mappedData);

    for (uint32_t i = 0; i < shaderGroupCount; i++) {
        std::memcpy(pData + i * baseAligment, shaderHandles.data() + i * handleSize, handleSize);
    }

    vkUnmapMemory(logicalDevice->handle, sbtStagingBuffer.bufferMemory);

    VkCommandBuffer commandBuffer = transferPool->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    vkf::Buffer::copyBuffer(&sbtStagingBuffer, &sbtBuffer, shaderGroupCount * baseAligment, commandBuffer);

    vkEndCommandBuffer(commandBuffer);

    logicalDevice->transferQueue.flushCommandBuffer(commandBuffer);
    transferPool->freeCommandBuffer(commandBuffer);

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

RayTracingPipeline::~RayTracingPipeline() {
    vkDestroyPipeline(logicalDevice->handle, handle, nullptr);
    vkDestroyPipelineLayout(logicalDevice->handle, pipelineLayout, nullptr);
}
