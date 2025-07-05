#pragma once


#define GLFW_INCLUDE_VULKAN

#include "Renderer/renderer.hpp"
#include "window.hpp"

#define FRAME_IN_FLIGHT 2

using namespace std;

class Application {
    public:

    Application();
    void run();
    ~Application() = default;

    private:

    Window window;
    Renderer renderer;
};
