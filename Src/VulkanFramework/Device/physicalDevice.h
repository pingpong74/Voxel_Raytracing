#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <set>
#include <string>
#include <map>
#include <stdexcept>

#include "../../config.h"
#include "queueFamilies.h"

namespace vkf {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presetMode;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice dev, VkSurfaceKHR surface);

    bool checkDeviceExtensionsSupport(VkPhysicalDevice dev);

    int rateSuitability(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    VkPhysicalDevice pickPhysicalDevices(VkInstance instance, VkSurfaceKHR surface);
};
