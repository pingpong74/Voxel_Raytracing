#pragma once

#include "logicalDevice.hpp"
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vkf {
    class DescriptorPool;

    class DescriptorSetLayout {
        public:
        VkDescriptorSetLayout handle = VK_NULL_HANDLE;

        DescriptorSetLayout(LogicalDevice* logicalDevice);

        void addBinding(VkDescriptorType, uint32_t, VkShaderStageFlags, uint32_t, VkSampler* = nullptr);
        void build();

        ~DescriptorSetLayout();

        private:

        LogicalDevice* logicalDevice;
        std::vector<VkDescriptorSetLayoutBinding> bindings;
    };

    class DescriptorSet {
        public:
        VkDescriptorSet handle;
        LogicalDevice* logicalDevice;

        DescriptorSet() = default;

        void write(VkWriteDescriptorSet*, uint32_t);

        ~DescriptorSet() = default;
    };

    class DescriptorPool {
        public:
        VkDescriptorPool handle = VK_NULL_HANDLE;

        DescriptorPool(LogicalDevice*);

        void addPoolSize(VkDescriptorType, uint32_t);
        void build(uint32_t);

        DescriptorSet allocateDescriptorSet(DescriptorSetLayout*);

        ~DescriptorPool();

        private:

        LogicalDevice* logicalDevice;
        std::vector<VkDescriptorPoolSize> poolSizes;
    };
}
