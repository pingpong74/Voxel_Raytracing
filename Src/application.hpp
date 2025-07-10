#pragma once


#include "Scene/scene.hpp"
#define GLFW_INCLUDE_VULKAN

#include "Renderer/renderer.hpp"
#include "window.hpp"

constexpr uint32_t FRAME_IN_FLIGHT = 2;
constexpr uint32_t width = 800;
constexpr uint32_t height = 600;

using namespace std;

class Application {
    public:

    Application();
    ~Application() = default;

    void run();

    void frameBufferResize(int width, int height);
    void mouseCallBack(double xpos, double ypos);

    private:

    Window window;

    //Vulkan Objects
    vkf::Instance instance;
    vkf::LogicalDevice logicalDevice;
    vkf::Swapchain swapchain;
    vkf::CommandPool graphicsPool, transferPool, computePool;

    Renderer renderer;
    Scene scene;
};
