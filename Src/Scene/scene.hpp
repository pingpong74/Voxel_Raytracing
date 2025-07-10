#pragma once

#include "../camera.hpp"
#include "../VulkanFramework/includes/accelerationStructure.hpp"
#include <GLFW/glfw3.h>
#include <vector>
#include <vulkan/vulkan_core.h>

#define BLAS_SIZE 1280
#define BLAS_SCRATCH_SIZE 2688

#define MAX_BLAS 1
#define MAX_CONCURENT_BUILD 1

class Scene {
    public:

    Scene(vkf::LogicalDevice* logicalDevice, vkf::CommandPool* buildPool, GLFWwindow* window);

    void mouseCallBack(double xpos, double ypos);
    void frameBufferResize(int width, int height);

    inline VkAccelerationStructureKHR getTopLevelAccelerationStructure() { return topLevelStructure.handle; }

    void update(double deltaTime);
    inline CameraConstants getCameraMatricies() { return cam.getCamMats(); }
    ~Scene() = default;

    private:

    Camera cam;

    std::vector<vkf::BottomLevelAccelerationStructure> bottomLevelStructures;
    vkf::TopLevelAccelerationStructure topLevelStructure;

    vkf::Buffer blasBuffer;
    vkf::Buffer blasScratchBuffer;

    vkf::Buffer boundingBoxBuffer;
    vkf::Buffer boundingBoxStagingBuffer;
};
