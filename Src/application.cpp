#include "application.h"

void Application::run() {

    init_window();

	//All functions here will initialize vulkan
	create_instance();
	createSurface();
	setupDebugMessenger();

	logicalDevice.createLogicalDevice(instance,  surface);

	int width, height;
	glfwGetWindowSize(window, &width, &height);
	swapchain.createSwapchain(&logicalDevice, width, height, surface);

	createCommandPools();

	//raytracer.createRayTracer(&logicalDevice, graphicsPool,  transferPool, swapchain.swapchainFormat, swapchain.swapchainExtent, window);

    //main_loop();

    cleanup();
}

void Application::init_window() {
    glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(width, height, "Vulkan Render", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glfwSetFramebufferSizeCallback(window, frameBufferResizeCallBack);
	glfwSetCursorPosCallback(window, mousePosCallBack);

	glfwSwapBuffers(window);
}

bool Application::checkValidationLayerSupport() {
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	vector<VkLayerProperties> layerProperties(layerCount);
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

vector<const char*> Application::getRequiredExtensions() {
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void Application::create_instance() {
	uint32_t extensionPropertiesCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, nullptr);
	vector<VkExtensionProperties> extensionsProperties(extensionPropertiesCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionPropertiesCount, extensionsProperties.data());


	VkApplicationInfo app_info{};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "Vulkan Renderer";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName = "No Engine";
	app_info.apiVersion = VK_API_VERSION_1_2;

	//get the extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfw_extensions;
	glfw_extensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	create_info.pApplicationInfo = &app_info;

	//get the required extensions
	auto extensions = getRequiredExtensions();
	create_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	create_info.ppEnabledExtensionNames = extensions.data();

	//check for validation layers and put them in the strust if enabled
	if(enableValidationLayers && !checkValidationLayerSupport()) throw runtime_error("Validation layers requested but not available");

	if(enableValidationLayers) {
		create_info.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		create_info.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		create_info.enabledLayerCount = 0;
	}

	if(vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create instance");
	}
}

void Application::setupDebugMessenger() {
	if(!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;

	//since this is an extension function we need to load it ourselves as extension functions are not loaded by default
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if(func == nullptr || func(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) throw runtime_error("Failed to setuo debug messenger");
}

void Application::createSurface() {
	if(glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw runtime_error("Failed to create surface");
	}
}

void Application::createCommandPools() {
	QueueFamily familes = QueueFamily::findQueueFamilies(logicalDevice.physicalDevice, surface);

	VkCommandPoolCreateInfo graphicPoolInfo{};
	graphicPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	graphicPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	graphicPoolInfo.queueFamilyIndex = familes.graphicsFamily.value();

	if(vkCreateCommandPool(logicalDevice.handle, &graphicPoolInfo, nullptr, &graphicsPool) != VK_SUCCESS) {
		throw runtime_error("Failed to create graphics pool");
	}

	VkCommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	transferPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	transferPoolInfo.queueFamilyIndex = familes.transferFamily.value();

	if(vkCreateCommandPool(logicalDevice.handle, &transferPoolInfo, nullptr, &transferPool) != VK_SUCCESS) {
		throw runtime_error("Failed to create transfer pool");
	}
}

////////////////////////////////////////// IMPROVE SRNCRONIZATION THIS SHIT IS ASSS IMPROVE THIS PLEASEE REMBER TO IMPROVE THIS HAHAHAHAHHAHAHAHAHAHH H HH FU FENUFNFE JFE FUCKKKKKKKKKKKKKKKKKKKKKKKKKKKK ///////////////////////////////////////////////////
/*void Application::main_loop() {
    FrameData framedata[FRAME_IN_FLIGHT];

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    fenceCreateInfo.pNext = nullptr;

    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCreateInfo.flags = 0;
    semaphoreCreateInfo.pNext = nullptr;

    for(int i = 0; i < FRAME_IN_FLIGHT; i++) {
        VK_CHECK(vkCreateFence(logicalDevice.handle, &fenceCreateInfo, nullptr, &framedata[i].inFlightFence), "Failed to create fence");

        VK_CHECK(vkCreateSemaphore(logicalDevice.handle, &semaphoreCreateInfo, nullptr, &framedata[i].imageSemaphore), "Failed to create Semaphore");
        VK_CHECK(vkCreateSemaphore(logicalDevice.handle, &semaphoreCreateInfo, nullptr, &framedata[i].renderFinishSemaphore), "Failed to create Semaphore");

        framedata[i].commandBuffer = logicalDevice.createCommandBuffer(graphicsPool);
    }

	float lastFrame = 0;
	uint32_t frameIndex = 0;

	VkResult resize;

	while(!glfwWindowShouldClose(window)){
		float deltaTime = glfwGetTime() - lastFrame;
		lastFrame = glfwGetTime();

		vkWaitForFences(logicalDevice.handle, 1, &framedata[frameIndex].inFlightFence, VK_TRUE, UINT64_MAX);
		vkResetFences(logicalDevice.handle, 1, &framedata[frameIndex].inFlightFence);

		framedata[frameIndex].commandBuffer.reset();

		uint32_t imageIndex;
    	resize = vkAcquireNextImageKHR(logicalDevice.handle, swapchain.handle, UINT64_MAX, framedata[frameIndex].imageSemaphore, VK_NULL_HANDLE, &imageIndex);

		raytracer.drawFrame(framedata[frameIndex].commandBuffer, swapchain.swapchainImages[imageIndex], deltaTime);

		VkPipelineStageFlags stageFlags[] = {VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR};

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &framedata[frameIndex].renderFinishSemaphore;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &framedata[frameIndex].imageSemaphore;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &framedata[frameIndex].commandBuffer.handle;
		submitInfo.pWaitDstStageMask = stageFlags;

		VK_CHECK(vkQueueSubmit(logicalDevice.graphicsQueue, 1, &submitInfo, framedata[frameIndex].inFlightFence), "Failed to submit to queue");

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &swapchain.handle;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &framedata[frameIndex].renderFinishSemaphore;

		resize = vkQueuePresentKHR(logicalDevice.presentationQueue, &presentInfo);

		cout << "\rFPS : " << (1 / deltaTime);

		frameIndex = frameIndex % FRAME_IN_FLIGHT;

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	vkDeviceWaitIdle(logicalDevice.handle);

	for(int i = 0; i < FRAME_IN_FLIGHT; i++) {
        vkDestroyFence(logicalDevice.handle, framedata[i].inFlightFence, nullptr);
	    vkDestroySemaphore(logicalDevice.handle, framedata[i].imageSemaphore, nullptr);
	    vkDestroySemaphore(logicalDevice.handle, framedata[i].renderFinishSemaphore, nullptr);
		logicalDevice.destroyCommandBuffer(framedata[i].commandBuffer);
	}
	}*/

void Application::cleanup() {
    //raytracer.cleanup();
	vkDestroyCommandPool(logicalDevice.handle, graphicsPool, nullptr);
	vkDestroyCommandPool(logicalDevice.handle, transferPool, nullptr);

	swapchain.destroy();

	logicalDevice.destroy();
	vkDestroySurfaceKHR(instance,surface, nullptr);

	if(enableValidationLayers) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if(func != nullptr) {
			func(instance, debugMessenger, nullptr);
		}
	}

	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);
	glfwTerminate();
}
