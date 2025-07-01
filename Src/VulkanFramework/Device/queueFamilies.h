#include <optional>
#include <stdint.h>
#include <vector>
#include <vulkan/vulkan_core.h>

struct QueueFamily {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentationFamily;
    std::optional<uint32_t> transferFamily;
    std::optional<uint32_t> computeFamily;

    inline bool isComplete() {
        return graphicsFamily.has_value() && presentationFamily.has_value() && transferFamily.has_value() && computeFamily.has_value();
    }

    static QueueFamily findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
        QueueFamily queueFamily;

        uint32_t queueFamilyCount;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

        int i = 0;

        for(const auto& properties : queueFamilyProperties) {
            VkBool32 presetationQueuePresent = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presetationQueuePresent);

            if(properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) queueFamily.graphicsFamily = i;

            if(presetationQueuePresent) queueFamily.presentationFamily = i;

            if(properties.queueFlags & VK_QUEUE_TRANSFER_BIT && !(properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)) queueFamily.transferFamily = i;

            if(properties.queueFlags & VK_QUEUE_COMPUTE_BIT && !(properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)) queueFamily.computeFamily = i;

            if(queueFamily.isComplete()) break;

            i++;

        }

        return queueFamily;
    }
};
