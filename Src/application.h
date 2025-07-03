#pragma once


#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>

#include "VulkanFramework/vulkanFramework.h"
#include "RayTracing/raytracer.h"

#define FRAME_IN_FLIGHT 2

using namespace std;

const uint32_t width = 800;
const uint32_t height = 600;

const bool enableValidationLayers = true;

struct FrameData {
    VkFence inFlightFence;
    VkSemaphore renderFinishSemaphore;
    VkSemaphore imageSemaphore;
};

class Application {
    public:

    void run();

    private:

    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkSurfaceKHR surface;

    vkf::LogicalDevice logicalDevice;

    vkf::Swapchain swapchain;

    VkCommandPool graphicsPool;
    VkCommandPool transferPool;

    RayTracer raytracer;

    static void frameBufferResizeCallBack(GLFWwindow* window, int width, int height) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->swapchain.recreateSwapchain(width, height);
        app->raytracer.handleResize(app->swapchain.swapchainFormat, app->swapchain.swapchainExtent);
    }

    static void mousePosCallBack(GLFWwindow *window, double xpos, double ypos) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->raytracer.cam.MouseInput(window,  xpos, ypos);
    }

    //Sends the error msg to console
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			std::cerr << "validationlayer:" << pCallbackData->pMessage << endl;
		}

		return VK_FALSE;
	}

    //functions for validations layers
    bool checkValidationLayerSupport();
    vector<const char*> getRequiredExtensions();

    void init_window();

    void create_instance();
    void setupDebugMessenger();

    void createSurface();

    void createCommandPools();

    void main_loop();

    void cleanup();
};
