#pragma once

#include <vector>
#include <limits>
#include <algorithm>
#include <vulkan/vulkan_core.h>

#include "../Device/logicalDevice.h"

namespace vkf {
    class Swapchain {
        public:

        VkSwapchainKHR handle;

        VkSurfaceFormatKHR swapchainFormat;
        VkPresentModeKHR swapchainPresentMode;
        VkExtent2D swapchainExtent;

        std::vector<VkImage> swapchainImages;

        void createSwapchain(LogicalDevice* logicalDevice, VkSurfaceKHR surface, int width, int height);
        void recreateSwapchain();
        void destroy();

        private:

        std::vector<VkImageView> swapchainImageViews;

        LogicalDevice* logicalDevice;

        static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& presentMode);
        static VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities, int width, int height);
    };
}
