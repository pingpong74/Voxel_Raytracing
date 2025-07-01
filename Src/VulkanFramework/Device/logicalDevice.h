#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>
#include <set>

#include "../../config.h"
#include "queueFamilies.h"

namespace vkf {
    struct Buffer {
        VkBuffer handle;
        VkDeviceMemory bufferMemory;
    };

    struct Image {
        VkImage handle;
        VkImageView view;
    };

    class LogicalDevice {
        public:

        static LogicalDevice createLogicalDevice(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, std::vector<const char*> deviceExtensions, std::vector<const char*> validationLayers);
        VkQueue getGraphicsQueue();

        //Buffer
        Buffer createBuffer(uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);

        //Command Buffer
        VkCommandBuffer createCommandBuffer();

        //Image
        Image createImage();

        private:

        VkDevice handle;
        VkPhysicalDevice physicalDevice;

        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    };
}
