#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>
#include <optional>

#include "instance.hpp"

namespace vkf {

    struct QueueFamily {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentationFamily;
        std::optional<uint32_t> transferFamily;
        std::optional<uint32_t> computeFamily;

        inline bool isComplete() {
            return graphicsFamily.has_value() && presentationFamily.has_value() && transferFamily.has_value() && computeFamily.has_value();
        }

        static QueueFamily findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presetMode;
    };

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice dev, VkSurfaceKHR surface);

    bool checkDeviceExtensionsSupport(VkPhysicalDevice dev);

    int rateSuitability(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    VkPhysicalDevice pickPhysicalDevices(Instance*);
};
