#pragma once
#include "RayTracing/raytracer.h"
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <limits>
#include <algorithm>
#include <fstream>

#include "VulkanFramework/Device/logicalDevice.h"

#define FRAME_IN_FLIGHT 2

//REMEMBER TO HAVE A COMPUTE QUEUE AND OFF LOAD BUILDING ACCELERATION STRUCTURE TO THAT QUEUE

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

    void mouseInput(double xpos, double ypos) {
        raytracer.cam.MouseInput(window, xpos, ypos);
    }

    void rayTracerResize() {
        recreateSwapchain();
        raytracer.handleResize(swapchainFormat, swapchainExtent);
    }

    private:

    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;


    VkSurfaceKHR surface;

    vkf::LogicalDevice logicalDevice;

    VkSurfaceFormatKHR swapchainFormat;
    VkPresentModeKHR swapchainPresentMode;
    VkExtent2D swapchainExtent;
    VkSwapchainKHR swapchain;

    VkCommandPool graphicsPool;
    VkCommandPool transferPool;

    RayTracer raytracer;

    vector<VkImage> swapchainImages;
    vector<VkImageView> swapchainImageViews;



    static void frameBufferResizeCallBack(GLFWwindow* window, int width, int height) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->rayTracerResize();
    }

    static void mousePosCallBack(GLFWwindow *window, double xpos, double ypos) {
        Application* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->mouseInput( xpos, ypos);
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

    //swapchain creation
    VkSurfaceFormatKHR chooseSurfaceFormat(const vector<VkSurfaceFormatKHR>& formats);
    VkPresentModeKHR choosePresentMode(const vector<VkPresentModeKHR>& presentMode);
    VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities);
    void createSwapchain();
    void recreateSwapchain();

    void createImageViews();
    void createFramebuffers();

    void createCommandPools();

    //Creating the shaders shit
    VkShaderModule createShaderModule(const string& filePath);
    VkPipelineShaderStageCreateInfo createShaderStageCreateInfo(VkShaderModule shaderModule, VkShaderStageFlagBits flags);

    void main_loop();

    void cleanupSwapchain();
    void cleanup();
};
