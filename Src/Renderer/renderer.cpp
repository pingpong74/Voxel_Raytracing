#include "renderer.hpp"

#include "raytracer.hpp"
#include <GLFW/glfw3.h>

#include <cstdint>
#include <vulkan/vulkan_core.h>

Renderer::Renderer(vkf::LogicalDevice* logicalDevice, vkf::CommandPool* transferPool, vkf::CommandPool* graphicsPool, vkf::CommandPool* computePool, vkf::Swapchain* swapchain) :
logicalDevice(logicalDevice),
swapchain(swapchain),
graphicsPool(graphicsPool),
transferPool(transferPool),
computePool(computePool),
raytracer(logicalDevice, transferPool, graphicsPool, computePool, swapchain->swapchainFormat.format, swapchain->swapchainExtent) {

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    vkCreateFence(logicalDevice->handle, &fenceCreateInfo, nullptr, &frameData.inFlightFence);
    vkCreateSemaphore(logicalDevice->handle, &semaphoreCreateInfo, nullptr, &frameData.imageSemaphore);
    vkCreateSemaphore(logicalDevice->handle, &semaphoreCreateInfo, nullptr, &frameData.renderFinishSemaphore);
    frameData.commandBuffer = graphicsPool->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}


Renderer::~Renderer() {
    vkDestroyFence(logicalDevice->handle, frameData.inFlightFence, nullptr);
    vkDestroySemaphore(logicalDevice->handle, frameData.imageSemaphore, nullptr);
    vkDestroySemaphore(logicalDevice->handle, frameData.renderFinishSemaphore, nullptr);
    graphicsPool->freeCommandBuffer(frameData.commandBuffer);
}

void Renderer::setScene(Scene* scene) {
    currentScene = scene;
    raytracer.setScene(scene);
}

void Renderer::drawFrame() {
    vkWaitForFences(logicalDevice->handle, 1, &frameData.inFlightFence, VK_TRUE, UINT64_MAX);
    vkResetFences(logicalDevice->handle, 1, &frameData.inFlightFence);
    vkResetCommandPool(logicalDevice->handle, graphicsPool->handle, 0);

    uint32_t imageIndex;
    vkAcquireNextImageKHR(logicalDevice->handle, swapchain->handle, UINT64_MAX, frameData.imageSemaphore, VK_NULL_HANDLE, &imageIndex);

    CameraConstants camConstants = currentScene->getCameraMatricies();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(frameData.commandBuffer, &beginInfo);

    raytracer.traceRays(camConstants, frameData.commandBuffer, &swapchain->swapchainImages[imageIndex]);

    vkEndCommandBuffer(frameData.commandBuffer);


    VkPipelineStageFlags stageFlags[] = {VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR};

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &frameData.renderFinishSemaphore;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &frameData.imageSemaphore;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &frameData.commandBuffer;
	submitInfo.pWaitDstStageMask = stageFlags;

	logicalDevice->graphicsQueue.submit(&submitInfo, 1, frameData.inFlightFence);

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain->handle;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &frameData.renderFinishSemaphore;

	logicalDevice->presentationQueue.present(&presentInfo);

	//logicalDevice->presentationQueue.waitIdle();
}

void Renderer::frameBufferResize(int width, int height) {
    raytracer.frameBufferResize(width, height);
}
