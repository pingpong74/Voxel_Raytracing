#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <stdexcept>
#include <vector>
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../VulkanFramework/vulkanFramework.h"

#include "../DataStructures/dataStructures.h"
#include "accelerationStructure.h"
#include "../Camera.h"
#include "model.h"

using namespace std;

struct CameraConstants {
    glm::mat4 inverseView;
    glm::mat4 inverseProj;
};

class RayTracer {
    public:

    Camera cam;

    void createRayTracer(vkf::LogicalDevice* _logicalDevice, VkCommandPool _graphicsPool, VkCommandPool _transferPool, VkSurfaceFormatKHR format, VkExtent2D extent, GLFWwindow* window);
    void drawFrame(CommandBuffer commandBuffer, VkImage swapchainImage, float deltaTime);
    void cleanup();

    //Handling resize
    void handleResize(VkSurfaceFormatKHR format, VkExtent2D extent);

    private:

    vkf::LogicalDevice* logicalDevice;

    VkCommandPool graphicsPool;
    VkCommandPool transferPool;

    GLFWwindow* window;

    //Pipeline creation function
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = nullptr;

    //Get Properties
    PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = nullptr;

    //trace rays
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = nullptr;

    //functions for pipeline creation
    VkShaderModule createShaderModule(const string& filePath);
    VkPipelineShaderStageCreateInfo createShaderStageCreateInfo(VkShaderModule shaderModule, VkShaderStageFlagBits flags);

    //Descriptor sets
    VkDescriptorPool descriptorPool;
    VkDescriptorSet set0;
    VkDescriptorSetLayout set0Layout;

    //Raytracing pipeline
    vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
    VkPipelineLayout rayTracingPipelineLayout;
    VkPipeline rayTracingPipeline;

    //Pipeline and acceleration strucutre properties
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties{};
    VkPhysicalDeviceAccelerationStructureFeaturesKHR accelerationStructureFeatures{};

    AccelerationStructureManager accelerationStructureManager;

    //Images shit
    VkImage frame;
    VkDeviceMemory imgMemory;
    VkImageView frameView;
    VkExtent2D imgExtent;

    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void setImgLayout(CommandBuffer commandBuffer, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subResourcesRange, VkPipelineStageFlags srcFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VkPipelineStageFlags dstFlags = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);

    //Shader Binding table
    vkf::Buffer sbtBuffer;

    VkStridedDeviceAddressRegionKHR rgenRegion{};
    VkStridedDeviceAddressRegionKHR missRegion{};
    VkStridedDeviceAddressRegionKHR hitRegion{};
    VkStridedDeviceAddressRegionKHR callRegion{};

    //Loads all the functions for extensions
    void loadFunctions();

    AccelerationStructure buildBLAS();
    AccelerationStructure buildTLAS(std::vector<AccelerationStructure> blases);

    //The storage image which the pipeline will write too
    void createImage(VkSurfaceFormatKHR format, VkExtent2D extent);

    //Creaete the camera buffers for the descriptor
    vkf::Buffer ubo;
    void createUBOBuffer();

    //Creation of descriptor sets
    void createDescritorSets();
    void updateDescriptorSets(float deltaTime);

    //Pipeline and binidng table. Binding table is used for fast look up of shaders
    void createRayTracingPipeline();
    void createShaderBindingTable();

    //Rendering code
    void recordCommandBuffer(CommandBuffer commandBuffer, VkImage swapchainImage);

    //FINISHHHHHH
    void destroyAccelerationStructure(AccelerationStructure acccelerationStructure);

};
