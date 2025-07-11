#pragma once

#include <vector>
#include <vulkan/vulkan_core.h>

#include "logicalDevice.hpp"
#include "image.hpp"

namespace vkf {
    class Swapchain {
        public:

        Swapchain(LogicalDevice*, int, int, VkSurfaceKHR);

        VkSwapchainKHR handle;

        VkSurfaceFormatKHR swapchainFormat;
        VkPresentModeKHR swapchainPresentMode;
        VkExtent2D swapchainExtent;

        std::vector<Image> swapchainImages;

        void recreateSwapchain(int width, int height);

        ~Swapchain();

        private:

        LogicalDevice* logicalDevice;
        VkSurfaceKHR surface;

        void create(int width, int height);
        void cleanup();

        static VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        static VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& presentMode);
        static VkExtent2D chooseSwapChainExtent(VkSurfaceCapabilitiesKHR capabilities, int width, int height);
    };
}
