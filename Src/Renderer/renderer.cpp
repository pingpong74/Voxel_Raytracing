#include "renderer.hpp"

#include "../VulkanFramework/includes/buffer.hpp"
#include "../VulkanFramework/includes/commandPool.hpp"
#include "../VulkanFramework/includes/descriptor.hpp"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>

Renderer::Renderer(GLFWwindow* window, int width, int height)
    : instance(window),
      logicalDevice(&instance),
      swapchain(&logicalDevice, width, height, instance.surface),
      rayTracingPipeline(&logicalDevice) {

    vkf::Buffer test(8, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &logicalDevice);

    vkf::CommandPool transferPool(&logicalDevice, logicalDevice.transferQueue.familyIndex);

    vkf::DescriptorSetLayout layout(&logicalDevice);
    layout.addBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    layout.addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    layout.addBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    layout.addBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3, VK_SHADER_STAGE_INTERSECTION_BIT_KHR, 1);
    layout.build();

    vkf::DescriptorPool despritorPool(&logicalDevice);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);
    despritorPool.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1);
    despritorPool.build(1);

    vkf::DescriptorSet set = despritorPool.allocateDescriptorSet(&layout);

    vkf::DescriptorSetLayout* layouts[] = {&layout};

    rayTracingPipeline.create(layouts, 1, 0, "Shaders/raygen.spv", "Shaders/miss.spv", "Shaders/closestHit.spv", "Shaders/intersection.spv");
    rayTracingPipeline.createShaderBindingTable(&transferPool);
}

void Renderer::drawFrame() {

}
