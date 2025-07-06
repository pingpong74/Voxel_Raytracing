#pragma once

#include "../VulkanFramework/includes/instance.hpp"
#include "../VulkanFramework/includes/logicalDevice.hpp"
#include "../VulkanFramework/includes/swapchain.hpp"
#include "../VulkanFramework/includes/pipelines.hpp"
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
