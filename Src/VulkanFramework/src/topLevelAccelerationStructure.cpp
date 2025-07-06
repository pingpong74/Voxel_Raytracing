#include "../includes/accelerationStructure.hpp"
#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include "../vulkanConfig.hpp"

using namespace vkf;

TopLevelAccelerationStructure::TopLevelAccelerationStructure(LogicalDevice* logicalDevice, CommandPool* buildPool) : tlasBuffer(logicalDevice), scratchBuffer(logicalDevice), instanceBuffer(logicalDevice), instanceStagingBuffer(logicalDevice) {
    this->logicalDevice = logicalDevice;
    this->buildPool = buildPool;
    commandBuffer = buildPool->allocateCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY);
}

TopLevelAccelerationStructure::~TopLevelAccelerationStructure() {
    vkDestroyAccelerationStructureKHR(logicalDevice->handle, handle, nullptr);
    buildPool->freeCommandBuffer(commandBuffer);
}

void TopLevelAccelerationStructure::createTopLevelAccelerationStructure(std::vector<BottomLevelAccelerationStructure> bottomLevelStructures, std::vector<VkTransformMatrixKHR> transforms) {
    std::vector<VkAccelerationStructureInstanceKHR> instances;

    if(bottomLevelStructures.size() == 0) throw std::runtime_error("No bottom level structures");

    for(int i = 0; i < bottomLevelStructures.size(); i++) {
        VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transforms[i];
		instance.instanceCustomIndex = i;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = bottomLevelStructures[i].getAddress();
        instances.push_back(instance);
    }

    //create a buffer on the gpu which has the data in the exact format wanted by vulkan
    instanceBuffer.create(sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    instanceStagingBuffer.create(sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    //Fill the data related to the instances
    VkAccelerationStructureGeometryInstancesDataKHR instanceData{};
    instanceData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instanceData.data.deviceAddress = instanceBuffer.getBufferAddress();

    VkAccelerationStructureGeometryKHR geometry{};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.geometry.instances = instanceData;

    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount = bottomLevelStructures.size();
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.transformOffset = 0;

    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.pNext = nullptr;
    buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &geometry;
    buildGeometryInfo.ppGeometries = nullptr;

    //Get info about the size of the acceleration structure and also the scratch buffer required
    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
    sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    uint32_t count = bottomLevelStructures.size();
    vkGetAccelerationStructureBuildSizesKHR(logicalDevice->handle, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &count, &sizeInfo);

    //Make a scratch buffer from the info gathered
    Buffer scratchBuffer(sizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, logicalDevice);

    //assign the scratch buffer
    buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getBufferAddress();

    //make the buffer in which the acceleration strcuture will reside
    Buffer accelerationStructureBuffer(sizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, logicalDevice);

    //create acceleeration structure
    VkAccelerationStructureCreateInfoKHR accelerationCreateInfo;
    accelerationCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationCreateInfo.buffer = accelerationStructureBuffer.handle;
    accelerationCreateInfo.size = sizeInfo.accelerationStructureSize;
    accelerationCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    accelerationCreateInfo.pNext = VK_NULL_HANDLE;
    accelerationCreateInfo.offset = 0;
    accelerationCreateInfo.createFlags = 0;
    accelerationCreateInfo.deviceAddress = 0;

    VkAccelerationStructureKHR accelerationStructure;
    VK_CHECK(vkCreateAccelerationStructureKHR(logicalDevice->handle, &accelerationCreateInfo, nullptr, &accelerationStructure), "Failed to create accleleration structure");

    buildGeometryInfo.dstAccelerationStructure = accelerationStructure;

    //FINALLY send the acceleration strucuture off the the gpu where it is built and will stay for the rest of its life before it is destoryed
    VkAccelerationStructureBuildRangeInfoKHR* infos[] = { &buildRangeInfo };

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Failed to begin command buffer inside tlas")

    Buffer::copyBuffer(instanceStagingBuffer, instanceBuffer, sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), commandBuffer);
    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, (VkAccelerationStructureBuildRangeInfoKHR* const*)&infos);

    VK_CHECK(vkEndCommandBuffer(commandBuffer), "Failed to end command buffer inside tlas build");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VK_CHECK(vkQueueSubmit(logicalDevice->computeQueue, 1, &submitInfo, VK_NULL_HANDLE), "Failed to submit to queue");
}

void TopLevelAccelerationStructure::updateTopLevelAccelerationStructure(std::vector<BottomLevelAccelerationStructure> bottomLevelStructures, std::vector<VkTransformMatrixKHR> transforms) {
    std::vector<VkAccelerationStructureInstanceKHR> instances;

    if(bottomLevelStructures.size() == 0) throw std::runtime_error("No bottom level structures");

    for(int i = 0; i < bottomLevelStructures.size(); i++) {
        VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transforms[i];
		instance.instanceCustomIndex = i;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = bottomLevelStructures[i].getAddress();
        instances.push_back(instance);
    }

    if(blases.size() < bottomLevelStructures.size()) {

    }

    //Check old buffers and ensure they are large enough
    if(blases.size() > bottomLevelStructures.size()) {
        //Add resize code here...  for now just throw a runtime error XD
        throw std::runtime_error("Toooo many bottom level acceleration structures");
    }

    //Fill the data related to the instances
    VkAccelerationStructureGeometryInstancesDataKHR instanceData{};
    instanceData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instanceData.data.deviceAddress = instanceBuffer.getBufferAddress();

    VkAccelerationStructureGeometryKHR geometry{};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.geometry.instances = instanceData;

    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount = bottomLevelStructures.size();
    buildRangeInfo.firstVertex = 0;
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.transformOffset = 0;

    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.pNext = nullptr;
    buildGeometryInfo.srcAccelerationStructure = handle;
    buildGeometryInfo.dstAccelerationStructure = handle;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR | VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_UPDATE_BIT_KHR;
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_UPDATE_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &geometry;
    buildGeometryInfo.ppGeometries = nullptr;

    //Get info about the size of the acceleration structure and also the scratch buffer required
    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
    sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    uint32_t count = bottomLevelStructures.size();
    vkGetAccelerationStructureBuildSizesKHR(logicalDevice->handle, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &count, &sizeInfo);

    //Check whether the current buffers are enough for updating the acceleration structure
    // Need to add resize code in future
    if(scratchBuffer.getSize() < sizeInfo.updateScratchSize) {
        throw std::runtime_error("Small scratch buffer");
    }
    if(tlasBuffer.getSize() < sizeInfo.accelerationStructureSize) {
        throw std::runtime_error("Small accleeration structure buffer");
    }


    //FINALLY send the acceleration strucuture off the the gpu where it is built and will stay for the rest of its life before it is destoryed
    VkAccelerationStructureBuildRangeInfoKHR* infos[] = { &buildRangeInfo };

    Buffer::copyBuffer(instanceBuffer, instanceStagingBuffer, sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), commandBuffer);
    vkCmdBuildAccelerationStructuresKHR(commandBuffer, 1, &buildGeometryInfo, (VkAccelerationStructureBuildRangeInfoKHR* const*)&infos);
}
