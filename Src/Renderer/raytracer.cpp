#include "raytracer.hpp"
#include <vulkan/vulkan_core.h>

Raytracer::Raytracer(vkf::LogicalDevice* logicalDevice, vkf::CommandPool* transferPool, vkf::CommandPool* graphicsPool, VkFormat format, VkExtent2D extent):
frameImage(logicalDevice, format, extent),
raytracingPipeline(logicalDevice),
descrictorPool(logicalDevice),
setLayout(logicalDevice){

    this->transferPool = transferPool;
    this->graphicsPool = graphicsPool;

    setLayout.addBinding(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 0, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    setLayout.addBinding(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, VK_SHADER_STAGE_RAYGEN_BIT_KHR, 1);
    setLayout.build();

    descrictorPool.addPoolSize(VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1);
    descrictorPool.addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1);
    descrictorPool.build(1);

    raytracingSet = descrictorPool.allocateDescriptorSet(&setLayout);

    vkf::DescriptorSetLayout* layouts[] = { &setLayout };

    raytracingPipeline.create(layouts, 1, sizeof(CameraConstants), "Shaders/raygen.spv", "Shaders/miss.spv", "Shaders/closestHit.spv", "Shaders/intersection.spv");
    raytracingPipeline.createShaderBindingTable(transferPool);
}
