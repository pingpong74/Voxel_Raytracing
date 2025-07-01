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

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice dev, VkSurfaceKHR surface) {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(dev, surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface,&formatCount, nullptr);

        if(formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(dev, surface, &formatCount, details.formats.data());
        }

        uint32_t presentCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentCount, nullptr);

        if(presentCount != 0) {
            details.presetMode.resize(presentCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(dev, surface, &presentCount, details.presetMode.data());
        }

        return details;
    }

    bool checkDeviceExtensionsSupport(VkPhysicalDevice dev) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(dev, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(dev, nullptr,&extensionCount, extensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for(const auto& extension: extensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    int rateSuitability(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);

        QueueFamily qf = QueueFamily::findQueueFamilies(physicalDevice, surface);

        if(!qf.isComplete()) return 0;
        if(!checkDeviceExtensionsSupport(physicalDevice)) return 0;


        SwapChainSupportDetails details = querySwapChainSupport(physicalDevice, surface);

        if(details.presetMode.empty() || details.formats.empty()) return 0;

        int score = 0;

        if(deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 100;

        return score;
    }

    VkPhysicalDevice pickPhysicalDevices(VkInstance instance, VkSurfaceKHR surface) {
        uint32_t deviceCount;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        std::vector<VkPhysicalDevice> physicalDevices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, physicalDevices.data());

        std::multimap<int, VkPhysicalDevice> scoreTable;

        for(const auto& devices : physicalDevices) {
            int score = rateSuitability(devices, surface);
            scoreTable.insert(std::make_pair(score, devices));
        }

        if(scoreTable.rbegin()->first > 0) {
            return scoreTable.rbegin()->second;
        }
        else {
            throw std::runtime_error("Failed to find a physical device");
        }
    }
};
