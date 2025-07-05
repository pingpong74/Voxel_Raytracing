#pragma once

#include <vulkan/vulkan_core.h>
#include <vector>

namespace vkf {
    class RayTracingPipeline {
        public:


        private:

        std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
        VkPipelineLayout rayTracingPipelineLayout;
        VkPipeline rayTracingPipeline;

        void createPipeline();
        void createShaderBindingTable();

    };
}
