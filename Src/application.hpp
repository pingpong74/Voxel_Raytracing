#pragma once


#define GLFW_INCLUDE_VULKAN

#include "Renderer/renderer.hpp"
#include "window.hpp"

#define FRAME_IN_FLIGHT 2

using namespace std;

class Application {
    public:

    Application();
    ~Application() = default;

    void run();

    void frameBufferResize();
    void mouseCallBack();

    private:

    Window window;
    Renderer renderer;
};
