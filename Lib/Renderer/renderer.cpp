#include "../../includes/Renderer/renderer.hpp"

#include "../../includes/VulkanFramework/buffer.hpp"
#include "../../includes/VulkanFramework/commandPool.hpp"
#include "../../includes/VulkanFramework/descriptor.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

Renderer::Renderer(GLFWwindow* window, int width, int height)
    : instance(window),
      logicalDevice(&instance),
      swapchain(&logicalDevice, width, height, instance.surface) {

    vkf::Buffer test(8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &logicalDevice);

    vkf::CommandPool commandPool(&logicalDevice, 0);

    vkf::DescriptorSetLayout layout(&logicalDevice);
    layout.addBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    layout.build();

    vkf::DescriptorPool despritorPool(&logicalDevice);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1);
    despritorPool.build(1);

    vkf::DescriptorSet set = despritorPool.allocateDescriptorSet(&layout);
}

void Renderer::drawFrame() {

}
