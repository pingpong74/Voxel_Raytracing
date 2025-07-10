#include "window.hpp"
#include <GLFW/glfw3.h>

class Application {
    public:
    void frameBufferResize(int width, int height);
    void mouseCallBack(double xpos, double ypos);
};

Window::Window(Application* app) {
    glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	handle = glfwCreateWindow(initialWidth, initialHeight, "Vulkan Render", nullptr, nullptr);
	glfwSetWindowUserPointer(handle, app);

	glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(handle, frameBufferResizeCallBack);
	glfwSetCursorPosCallback(handle, mousePositionCallBack);

	glfwSwapBuffers(handle);

	glfwGetWindowSize(handle, &width, &height);
}

void Window::frameBufferResizeCallBack(GLFWwindow* window, int width, int height) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    app->frameBufferResize(width, height);
}
void Window::mousePositionCallBack(GLFWwindow* window, double xpos, double ypos) {
    Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
    app->mouseCallBack(xpos, ypos);
}

void Window::poll() {
    glfwSwapBuffers(handle);
	glfwPollEvents();
}

Window::~Window() {
    glfwDestroyWindow(handle);
	glfwTerminate();
}
