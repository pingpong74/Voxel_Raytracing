#include "renderer.hpp"

#include "../VulkanFramework/includes/commandPool.hpp"
#include "raytracer.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

Renderer::Renderer(GLFWwindow *window, int width, int height)
    : instance(window),
    logicalDevice(&instance),
    swapchain(&logicalDevice, width, height, instance.surface),
    graphicsPool(&logicalDevice, logicalDevice.graphicsQueue.familyIndex),
    transferPool(&logicalDevice, logicalDevice.transferQueue.familyIndex),
    computePool(&logicalDevice, logicalDevice.computeQueue.familyIndex),
    raytracer(&logicalDevice, &transferPool, &graphicsPool, swapchain.swapchainFormat.format, swapchain.swapchainExtent) {
}

void Renderer::drawFrame() {}
