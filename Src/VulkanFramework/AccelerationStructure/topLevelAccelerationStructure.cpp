#include "accelerationStructure.h"
#include <vulkan/vulkan_core.h>

using namespace vkf;

TopLevelAccelerationStructure TopLevelAccelerationStructure::createTopLevelAccelerationStructure(LogicalDevice* logicalDevice, VkCommandPool buildPool, std::vector<BottomLevelAccelerationStructure> bottomLevelStructures, std::vector<VkTransformMatrixKHR> transforms) {
    std::vector<VkAccelerationStructureInstanceKHR> instances;

    if(bottomLevelStructures.size() == 0) throw std::runtime_error("No bottom level structures");

    for(int i = 0; i < bottomLevelStructures.size(); i++) {
        VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transforms[i];
		instance.instanceCustomIndex = i;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = bottomLevelStructures[i].getAddress(logicalDevice);
        instances.push_back(instance);
    }

    //create a buffer on the gpu which has the data in the exact format wanted by vulkan
    Buffer blasInstanceBuffer = logicalDevice->createBuffer(sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    Buffer blasInstanceStagingBuffer = logicalDevice->createBuffer(sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    //Fill the data related to the instances
    VkAccelerationStructureGeometryInstancesDataKHR instanceData{};
    instanceData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instanceData.data.deviceAddress = logicalDevice->getBufferAddress(blasInstanceBuffer);

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
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
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
    Buffer scratchBuffer = logicalDevice->createBuffer(sizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //assign the scratch buffer
    buildGeometryInfo.scratchData.deviceAddress = logicalDevice->getBufferAddress(scratchBuffer);

    //make the buffer in which the acceleration strcuture will reside
    Buffer accelerationStructureBuffer = logicalDevice->createBuffer(sizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

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
    vkCreateAccelerationStructureKHR(logicalDevice->handle, &accelerationCreateInfo, nullptr, &accelerationStructure);

    buildGeometryInfo.dstAccelerationStructure = accelerationStructure;

    //FINALLY send the acceleration strucuture off the the gpu where it is built and will stay for the rest of its life before it is destoryed
    VkAccelerationStructureBuildRangeInfoKHR* infos[] = { &buildRangeInfo };

    CommandBuffer commandBuffer = logicalDevice->createCommandBuffer(buildPool);
    commandBuffer.beginRecording(0);

    Buffer::copyBuffer(blasInstanceStagingBuffer, blasInstanceBuffer, sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), commandBuffer);
    vkCmdBuildAccelerationStructuresKHR(commandBuffer.handle, 1, &buildGeometryInfo, (VkAccelerationStructureBuildRangeInfoKHR* const*)&infos);

    commandBuffer.endRecording();

    //submit to the queue
    commandBuffer.flush(logicalDevice->computeQueue);

    return {
        accelerationStructure,
        accelerationStructureBuffer,
        scratchBuffer,
        bottomLevelStructures,
        transforms,
        commandBuffer
    };
}
