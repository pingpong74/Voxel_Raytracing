#pragma once

#include "../VulkanFramework/instance.hpp"
#include "../VulkanFramework/logicalDevice.hpp"
#include "../VulkanFramework/swapchain.hpp"
#include "../VulkanFramework/pipelines.hpp"
#include <GLFW/glfw3.h>

class Renderer {
    public:

    Renderer(GLFWwindow*, int, int);

    void drawFrame();

    ~Renderer() = default;

    private:

    vkf::Instance instance;
    vkf::LogicalDevice logicalDevice;
    vkf::Swapchain swapchain;
    vkf::RayTracingPipeline rayTracingPipeline;
};
