#pragma once

#include "../VulkanFramework/includes/logicalDevice.hpp"
#include "../VulkanFramework/includes/swapchain.hpp"
#include "../Scene/scene.hpp"
#include "raytracer.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

struct FrameData {
    VkSemaphore renderFinishSemaphore;
    VkSemaphore imageSemaphore;
    VkFence inFlightFence;
    VkCommandBuffer commandBuffer;
};

class Renderer {
    public:

    Renderer(vkf::LogicalDevice* logicalDevice, vkf::CommandPool* transferPool, vkf::CommandPool* graphicsPool, vkf::CommandPool* computePool, vkf::Swapchain* swapchain);

    void frameBufferResize(int width, int height);

    void setScene(Scene* scene);
    void drawFrame();

    ~Renderer();

    private:

    vkf::LogicalDevice* logicalDevice;
    vkf::Swapchain* swapchain;
    vkf::CommandPool* graphicsPool, *transferPool, *computePool;
    Raytracer raytracer;
    Scene* currentScene;
    FrameData frameData;
};
