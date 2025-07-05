#include "../../includes/Renderer/renderer.hpp"
#include <GLFW/glfw3.h>

Renderer::Renderer(GLFWwindow* window, int width, int height)
    : instance(window),
      logicalDevice(&instance),
      swapchain(&logicalDevice, width, height, instance.surface) {

}
