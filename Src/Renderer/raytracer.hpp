#pragma once

#include "../VulkanFramework/includes/pipelines.hpp"
#include "../VulkanFramework/includes/image.hpp"
#include "../camera.hpp"
#include <vulkan/vulkan_core.h>

class Raytracer {
    public:

    Raytracer(vkf::LogicalDevice*,vkf::CommandPool* , vkf::CommandPool*, VkFormat, VkExtent2D);
    ~Raytracer() = default;

    void traceRays();

    private:

    vkf::CommandPool* transferPool;
    vkf::CommandPool* graphicsPool;
    Camera cam;

    vkf::DescriptorPool descrictorPool;
    vkf::DescriptorSetLayout setLayout;
    vkf::DescriptorSet raytracingSet;

    vkf::Image frameImage;
    vkf::RayTracingPipeline raytracingPipeline;
};
