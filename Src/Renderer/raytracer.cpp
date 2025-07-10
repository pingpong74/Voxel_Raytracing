#include "raytracer.hpp"
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>

Raytracer::Raytracer(vkf::LogicalDevice* logicalDevice, vkf::CommandPool* transferPool, vkf::CommandPool* graphicsPool, vkf::CommandPool* computePool, VkFormat format, VkExtent2D extent):
frameImage(logicalDevice, format, extent),
raytracingPipeline(logicalDevice),
descrictorPool(logicalDevice),
setLayout(logicalDevice),
transferPool(transferPool),
graphicsPool(graphicsPool),
logicalDevice(logicalDevice){

    VkCommandBuffer imgLayoutBuffer = graphicsPool->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(imgLayoutBuffer, &beginInfo);

    frameImage.transitionLayout(imgLayoutBuffer,VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    vkEndCommandBuffer(imgLayoutBuffer);

    //Test
    logicalDevice->graphicsQueue.flushCommandBuffer(imgLayoutBuffer);
    //

    setLayout.addBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    setLayout.addBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    setLayout.build();

    descrictorPool.addPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1);
    descrictorPool.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);
    descrictorPool.build(1);

    raytracingSet = descrictorPool.allocateDescriptorSet(&setLayout);

    vkf::DescriptorSetLayout* layouts[] = { &setLayout };

    raytracingPipeline.create(layouts, 1, sizeof(CameraConstants), "Shaders/spv/raygen.spv", "Shaders/spv/miss.spv", "Shaders/spv/closestHit.spv", "Shaders/spv/intersection.spv");
    raytracingPipeline.createShaderBindingTable(transferPool);

    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = frameImage.view;

    VkWriteDescriptorSet imgWrite{};
    imgWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imgWrite.descriptorCount = 1;
    imgWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imgWrite.dstBinding = 1;
    imgWrite.dstSet = raytracingSet.handle;
    imgWrite.pImageInfo = &imgInfo;

    raytracingSet.write(&imgWrite, 1);
}

void Raytracer::setScene(Scene* scene) {
    VkWriteDescriptorSetAccelerationStructureKHR desASInfo{};
    desASInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
    desASInfo.accelerationStructureCount = 1;
    VkAccelerationStructureKHR tlas = scene->getTopLevelAccelerationStructure();
    desASInfo.pAccelerationStructures = &tlas;
    desASInfo.pNext = nullptr;

    VkWriteDescriptorSet asWrite{};
    asWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    asWrite.descriptorCount = 1;
    asWrite.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
    asWrite.dstBinding = 0;
    asWrite.dstSet = raytracingSet.handle;
    asWrite.pNext = &desASInfo;

    raytracingSet.write(&asWrite, 1);
}

void Raytracer::frameBufferResize(int width, int height) {
    VkFormat format = frameImage.getFormat();
    VkExtent2D extent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

    logicalDevice->destroyImage(frameImage.handle, frameImage.imageMemory, frameImage.view);

    logicalDevice->createImage(format, extent, frameImage.handle, frameImage.imageMemory);
    logicalDevice->createImageView(frameImage.handle, format, frameImage.view);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VkCommandBuffer imgLayoutBuffer = graphicsPool->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    vkBeginCommandBuffer(imgLayoutBuffer, &beginInfo);

    frameImage.transitionLayout(imgLayoutBuffer,VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    vkEndCommandBuffer(imgLayoutBuffer);

    //Test
    logicalDevice->graphicsQueue.flushCommandBuffer(imgLayoutBuffer);
    //
    //
    VkDescriptorImageInfo imgInfo{};
    imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    imgInfo.imageView = frameImage.view;

    VkWriteDescriptorSet imgWrite{};
    imgWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    imgWrite.descriptorCount = 1;
    imgWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
    imgWrite.dstBinding = 1;
    imgWrite.dstSet = raytracingSet.handle;
    imgWrite.pImageInfo = &imgInfo;

    raytracingSet.write(&imgWrite, 1);
}

void Raytracer::traceRays(CameraConstants camConstants, VkCommandBuffer commandBuffer, vkf::Image* swapchainImage) {
    raytracingPipeline.bindPipeline(commandBuffer);
    raytracingPipeline.bindDescriptorSet(commandBuffer, &raytracingSet);
    raytracingPipeline.pushConstants(commandBuffer, &camConstants, sizeof(CameraConstants));

    //Now we shall begin copying the generated img to the swapchain.
    VkImageSubresourceRange subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };

    raytracingPipeline.traceRays(commandBuffer, frameImage.getExtent());

    //Change the layouts of the images to help copying
    swapchainImage->transitionLayout(commandBuffer,VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
    //Change the layout of the created image
    frameImage.transitionLayout(commandBuffer,VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, subresourceRange);

    //Now we shall do the actual copy operation
    VkImageCopy copyRegion{};
	copyRegion.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
	copyRegion.dstOffset = { 0, 0, 0 };
	VkExtent2D extent2d = frameImage.getExtent();
	copyRegion.extent = {extent2d.width, extent2d.height, 1};

    vkCmdCopyImage(commandBuffer, frameImage.handle, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, swapchainImage->handle, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);

    //Set the layout of the images back
    swapchainImage->transitionLayout(commandBuffer,VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, subresourceRange);
    frameImage.transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL, subresourceRange);
}
