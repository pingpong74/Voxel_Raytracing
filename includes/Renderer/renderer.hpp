#pragma once

#include "../../includes/VulkanFramework/instance.hpp"
#include "../../includes/VulkanFramework/logicalDevice.hpp"
#include "../../includes/VulkanFramework/swapchain.hpp"
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

};
