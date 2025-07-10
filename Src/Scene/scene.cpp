#include "scene.hpp"
#include <vulkan/vulkan_core.h>

Scene::Scene(vkf::LogicalDevice* logicalDevice, vkf::CommandPool* buildPool, GLFWwindow* window):
topLevelStructure(logicalDevice, buildPool),
blasBuffer(MAX_BLAS * BLAS_SIZE, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, logicalDevice),
blasScratchBuffer(MAX_CONCURENT_BUILD * BLAS_SCRATCH_SIZE, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, logicalDevice),
boundingBoxBuffer(MAX_BLAS * sizeof(VkAabbPositionsKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, logicalDevice),
boundingBoxStagingBuffer(MAX_BLAS * sizeof(VkAabbPositionsKHR), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, logicalDevice),
cam(window) {

    VkAabbPositionsKHR aabb = {
        0, 0, 0,
        1, 1, 1
    };

    void* data = boundingBoxStagingBuffer.map();
    memcpy(data, &aabb, sizeof(VkAabbPositionsKHR));
    boundingBoxStagingBuffer.unmap();

    VkCommandBuffer buildCmdBuffer = buildPool->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    vkf::BottomLevelAccelerationStructureBuildInfo buildInfo{};
    buildInfo.blasBuffer = &blasBuffer;
    buildInfo.blasOffset = 0;
    buildInfo.boundingBoxBufferAddress = boundingBoxBuffer.getBufferAddress();
    buildInfo.scratchBufferAddress = blasScratchBuffer.getBufferAddress();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(buildCmdBuffer, &beginInfo);

    vkf::Buffer::copyBuffer(&boundingBoxStagingBuffer, &boundingBoxBuffer, sizeof(VkAabbPositionsKHR), buildCmdBuffer);
    bottomLevelStructures = vkf::BottomLevelAccelerationStructure::createBottomLevelAccelerationStructure(buildCmdBuffer, logicalDevice,  &buildInfo, 1);

    vkEndCommandBuffer(buildCmdBuffer);

    logicalDevice->computeQueue.flushCommandBuffer(buildCmdBuffer);

    std::vector<VkTransformMatrixKHR> transforms = {{
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0
    }};

    topLevelStructure.createTopLevelAccelerationStructure(&bottomLevelStructures, &transforms);
}

void Scene::mouseCallBack(double xpos, double ypos) {
    cam.mouseCallback(xpos, ypos);
}


void Scene::frameBufferResize(int width, int height) {
    cam.frameBufferResize(width, height);
}

void Scene::update(double deltaTime) {
    cam.updateCamera(deltaTime);
}
