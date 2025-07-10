#pragma once

#include "../VulkanFramework/includes/pipelines.hpp"
#include "../VulkanFramework/includes/image.hpp"
#include "../Scene/scene.hpp"
#include "../camera.hpp"
#include <vulkan/vulkan_core.h>

class Raytracer {
    public:

    Raytracer(vkf::LogicalDevice*,vkf::CommandPool* , vkf::CommandPool*,vkf::CommandPool*, VkFormat, VkExtent2D);
    ~Raytracer() = default;

    void setScene(Scene* scene);
    void traceRays(CameraConstants camConstants, VkCommandBuffer commandBuffer, vkf::Image* swapchainImage);

    void frameBufferResize(int width, int height);

    private:

    vkf::LogicalDevice* logicalDevice;

    vkf::CommandPool* transferPool;
    vkf::CommandPool* graphicsPool;

    vkf::DescriptorPool descrictorPool;
    vkf::DescriptorSetLayout setLayout;
    vkf::DescriptorSet raytracingSet;

    vkf::Image frameImage;
    vkf::RayTracingPipeline raytracingPipeline;
};
