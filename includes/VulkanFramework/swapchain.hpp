#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "logicalDevice.hpp"

namespace vkf {
    class Swapchain {
        public:

        VkSwapchainKHR handle;

        VkSurfaceFormatKHR swapchainFormat;
        VkPresentModeKHR swapchainPresentMode;
        VkExtent2D swapchainExtent;

        std::vector<VkImage> swapchainImages;

        void createSwapchain(LogicalDevice* logicalDevice, int width, int height, VkSurfaceKHR surface = 0);
        void recreateSwapchain(int width, int height);
        void destroy();

        private:

        std::vector<VkImageView> swapchainImageViews;

        LogicalDevice* logicalDevice;
        VkSurfaceKHR surface = 0;

        static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& presentMode);
        static VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities, int width, int height);
    };
}
