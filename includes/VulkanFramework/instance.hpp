#pragma once

#include <vulkan/vulkan_core.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

namespace vkf {
    class Instance {
        public:
        VkInstance handle;
        VkSurfaceKHR surface;

        Instance(GLFWwindow* window);

        ~Instance();

        private:

        VkDebugUtilsMessengerEXT debugMessenger;

        bool checkValidationLayerSupport();
        std::vector<const char*> getRequiredExtensions();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
            if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			    std::cerr << "validationlayer:" << pCallbackData->pMessage << std::endl;
		    }

		    return VK_FALSE;
        }

        void setupDebugMessenger();
    };
}
