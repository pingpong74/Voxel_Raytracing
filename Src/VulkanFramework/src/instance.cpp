#include "../includes/instance.hpp"

#include <stdexcept>
#include <vector>
#include <string.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "../vulkanConfig.hpp"

using namespace vkf;

Instance::Instance(GLFWwindow* window) {
    uint32_t extensionPropertiesCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, nullptr);
	std::vector<VkExtensionProperties> extensionsProperties(extensionPropertiesCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, extensionsProperties.data());

	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulkan Renderer";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.apiVersion = VK_API_VERSION_1_4;

	//get the extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo instanceCreateInfo{};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pApplicationInfo = &app_info;

	//get the required extensions
	auto extensions = getRequiredExtensions();
	instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

	//check for validation layers and put them in the strust if enabled
	#if VALIDATION_LAYERS_ENABLED
	    if(!checkValidationLayerSupport()) throw std::runtime_error("Validation layers requested but not available");
		instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
	#else
	    create_info.enabledLayerCount = 0;
	#endif

	VK_CHECK(vkCreateInstance(&instanceCreateInfo, nullptr, &handle), "Failed to create instance")
	VK_CHECK(glfwCreateWindowSurface(handle, window, nullptr, &surface), "Failed to create surface")

	setupDebugMessenger();
}

bool Instance::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> layerProperties(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, layerProperties.data());

	for(const char* name : validationLayers) {

		bool layerFound = false;

		for(const auto& layerProperty : layerProperties) {
			if(strcmp(name, layerProperty.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if(!layerFound) return false;
	}

	return true;
}

std::vector<const char*> Instance::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	#if VALIDATION_LAYERS_ENABLED
	    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	#endif

	return extensions;
}

void Instance::setupDebugMessenger() {
	#if VALIDATION_LAYERS_ENABLED

	    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	    createInfo.pfnUserCallback = debugCallback;
	    createInfo.pUserData = nullptr;

	    //since this is an extension function we need to load it ourselves as extension functions are not loaded by default
	    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(handle, "vkCreateDebugUtilsMessengerEXT");

	    if(func == nullptr || func(handle, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) throw std::runtime_error("Failed to setuo debug messenger");

	#else
	    return;
	#endif
}

Instance::~Instance() {
    #if VALIDATION_LAYERS_ENABLED
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(handle, "vkDestroyDebugUtilsMessengerEXT");
		if(func != nullptr) {
			func(handle, debugMessenger, nullptr);
		}
	#endif

	vkDestroySurfaceKHR(handle, surface, nullptr);
	vkDestroyInstance(handle, nullptr);
}
