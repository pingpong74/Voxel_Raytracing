#include "renderer.hpp"

#include "../VulkanFramework/includes/accelerationStructure.hpp"
#include "../VulkanFramework/includes/buffer.hpp"
#include "../VulkanFramework/includes/commandPool.hpp"
#include "../VulkanFramework/includes/descriptor.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include<cstring>

Renderer::Renderer(GLFWwindow *window, int width, int height)
    : instance(window),
    logicalDevice(&instance),
    swapchain(&logicalDevice, width, height, instance.surface),
    graphicsPool(&logicalDevice, logicalDevice.graphicsQueue.familyIndex),
    transferPool(&logicalDevice, logicalDevice.transferQueue.familyIndex),
    computePool(&logicalDevice, logicalDevice.computeQueue.familyIndex),
    rayTracingPipeline(&logicalDevice) {

    vkf::DescriptorSetLayout layout(&logicalDevice);
    layout.addBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    layout.addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    layout.addBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    layout.addBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3,VK_SHADER_STAGE_INTERSECTION_BIT_KHR, 1);
    layout.build();

    vkf::DescriptorPool despritorPool(&logicalDevice);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    despritorPool.build(2);

    vkf::DescriptorSet set = despritorPool.allocateDescriptorSet(&layout);
    vkf::DescriptorSetLayout *layouts[] = {&layout};
    rayTracingPipeline.create(layouts, 1, 0, "Shaders/raygen.spv", "Shaders/miss.spv", "Shaders/closestHit.spv", "Shaders/intersection.spv");
    rayTracingPipeline.createShaderBindingTable(&transferPool);

    vkf::Buffer blasBuffer(1500, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT_KHR | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &logicalDevice);
    vkf::Buffer scratchBuffer(2688, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &logicalDevice);

    vkf::Buffer boundingBoxBuffer(sizeof(VkAabbPositionsKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &logicalDevice);
    vkf::Buffer boundingBoxStagingBuffer(sizeof(VkAabbPositionsKHR), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &logicalDevice);

    void* boundingBoxStagingBufferMapped;
    vkMapMemory(logicalDevice.handle, boundingBoxStagingBuffer.bufferMemory, 0, sizeof(VkAabbPositionsKHR), 0, &boundingBoxStagingBufferMapped);
    VkAabbPositionsKHR aabb = { 0, 0, 0, 1, 1, 1};
    std::memcpy(boundingBoxStagingBufferMapped, &aabb, sizeof(VkAabbPositionsKHR));
    vkUnmapMemory(logicalDevice.handle, boundingBoxStagingBuffer.bufferMemory);

    VkCommandBuffer cd = transferPool.allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cd, &beginInfo);
    vkf::Buffer::copyBuffer(&boundingBoxStagingBuffer, &boundingBoxBuffer, sizeof(VkAabbPositionsKHR), cd);
    vkEndCommandBuffer(cd);

    logicalDevice.transferQueue.flushCommandBuffer(cd);
    transferPool.freeCommandBuffer(cd);

    vkf::BottomLevelAccelerationStructureBuildInfo buildInfo{};
    buildInfo.blasBuffer = &blasBuffer;
    buildInfo.blasOffset = 0;
    buildInfo.boundingBoxBufferAddress = boundingBoxBuffer.getBufferAddress();
    buildInfo.scratchBufferAddress = scratchBuffer.getBufferAddress();

    cd = computePool.allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    vkBeginCommandBuffer(cd, &beginInfo);
    std::vector<vkf::BottomLevelAccelerationStructure> blas = vkf::BottomLevelAccelerationStructure::createBottomLevelAccelerationStructure(cd, &logicalDevice, &buildInfo, 1);
    vkEndCommandBuffer(cd);

    std::cout << blas[0].handle << std::endl;

    logicalDevice.computeQueue.flushCommandBuffer(cd);
    computePool.freeCommandBuffer(cd);

    std::vector<VkTransformMatrixKHR> transforms = {{
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
    }};

    vkf::TopLevelAccelerationStructure tlas(&logicalDevice, &computePool);

    tlas.createTopLevelAccelerationStructure(&blas, &transforms);

    std::cout << tlas.handle << std::endl;
}

void Renderer::drawFrame() {}
