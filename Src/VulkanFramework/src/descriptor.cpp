#include "../includes/descriptor.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include "../vulkanConfig.hpp"

using namespace vkf;

DescriptorSetLayout::DescriptorSetLayout(LogicalDevice* logicalDevice) {
    this->logicalDevice = logicalDevice;
}

void DescriptorSetLayout::addBinding(VkDescriptorType descriptorType, uint32_t binding, VkShaderStageFlags stageFlags, uint32_t count, VkSampler* sampler) {
    if(handle != VK_NULL_HANDLE) {
        throw std::runtime_error("Layout already constructed");
    }

    VkDescriptorSetLayoutBinding bindingInfo{};
    bindingInfo.descriptorType = descriptorType;
    bindingInfo.binding = binding;
    bindingInfo.stageFlags = stageFlags;
    bindingInfo.descriptorCount = count;
    bindingInfo.pImmutableSamplers = sampler;

    bindings.push_back(bindingInfo);
}

void DescriptorSetLayout::build() {
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
    layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutCreateInfo.bindingCount = bindings.size();
    layoutCreateInfo.pBindings = bindings.data();
    layoutCreateInfo.pNext = nullptr;

    VK_CHECK(vkCreateDescriptorSetLayout(logicalDevice->handle, &layoutCreateInfo, nullptr, &handle), "Failed to create descriptor set layout");
}

DescriptorSetLayout::~DescriptorSetLayout() {
    vkDestroyDescriptorSetLayout(logicalDevice->handle, handle, nullptr);
}

DescriptorPool::DescriptorPool(LogicalDevice* logicalDevice) {
    this->logicalDevice = logicalDevice;
}

void DescriptorPool::addPoolSize(VkDescriptorType descriptorType, uint32_t count) {
    if(handle != VK_NULL_HANDLE) {
        throw std::runtime_error("Pool alreaady constructucted");
    }

    VkDescriptorPoolSize poolSize{};
    poolSize.type = descriptorType;
    poolSize.descriptorCount = count;

    poolSizes.push_back(poolSize);
}

void DescriptorPool::build(uint32_t maxSets) {
    VkDescriptorPoolCreateInfo poolCreateInfo{};
    poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolCreateInfo.pNext = nullptr;
    poolCreateInfo.maxSets = maxSets;
    poolCreateInfo.poolSizeCount = poolSizes.size();
    poolCreateInfo.pPoolSizes = poolSizes.data();

    VK_CHECK(vkCreateDescriptorPool(logicalDevice->handle, &poolCreateInfo, nullptr, &handle), "Failed to make descriptor pool");
}

DescriptorSet DescriptorPool::allocateDescriptorSet(DescriptorSetLayout* layout) {
    if(handle == VK_NULL_HANDLE) {
        throw std::runtime_error("Tried to allocate descriptor set but descriptor pool isnt built yet");
    }

    DescriptorSet set;
    set.logicalDevice = logicalDevice;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = handle;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pNext = nullptr;
    allocInfo.pSetLayouts = &(layout->handle);

    VK_CHECK(vkAllocateDescriptorSets(logicalDevice->handle, &allocInfo, &set.handle), "Failed to allocate descriptor set");

    return set;
}

DescriptorPool::~DescriptorPool() {
    vkDestroyDescriptorPool(logicalDevice->handle, handle, nullptr);

    std::cout << "3" << std::endl;
}

void DescriptorSet::write(VkWriteDescriptorSet* writeInfo, uint32_t count) {
    vkUpdateDescriptorSets(logicalDevice->handle, count, writeInfo, 0, nullptr);
}
