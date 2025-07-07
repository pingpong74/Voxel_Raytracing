#include "window.hpp"
#include <GLFW/glfw3.h>

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
    //app->frameBufferResize();
}
void Window::mousePositionCallBack(GLFWwindow*, double, double) {

}

Window::~Window() {
    glfwDestroyWindow(handle);
	glfwTerminate();
}
