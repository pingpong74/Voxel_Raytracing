#include "window.hpp"

Window::Window() {
    glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	handle = glfwCreateWindow(initialWidth, initialHeight, "Vulkan Render", nullptr, nullptr);
	glfwSetWindowUserPointer(handle, this);

	glfwSetInputMode(handle, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(handle, frameBufferResizeCallBack);
	glfwSetCursorPosCallback(handle, mousePositionCallBack);

	glfwSwapBuffers(handle);

	glfwGetWindowSize(handle, &width, &height);
}

void Window::frameBufferResizeCallBack(GLFWwindow*, int, int) {

}
void Window::mousePositionCallBack(GLFWwindow*, double, double) {

}

Window::~Window() {
    glfwDestroyWindow(handle);
	glfwTerminate();
}
