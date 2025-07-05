#pragma once

#include <GLFW/glfw3.h>

const int initialWidth = 800;
const int initialHeight = 600;

class Window {
    public:

    GLFWwindow* handle;
    int width, height;

    Window();
    ~Window();

    private:

    static void frameBufferResizeCallBack(GLFWwindow*, int, int);
    static void mousePositionCallBack(GLFWwindow*, double, double);
};
