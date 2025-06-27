#include "accelerationStructure.h"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <vulkan/vulkan_core.h>

void AccelerationStructureManager::create(VkDevice _device, VkPhysicalDevice _physicalDevice, VkQueue _buildQueue, VkCommandPool _buildPool, VkQueue _transferQueue, VkCommandPool _transferPool) {
    device = _device;
    physicalDevice = _physicalDevice;
    buildQueue = _buildQueue;
    buildPool = _buildPool;
    transferQueue = _transferQueue;
    transferPool = _transferPool;

    LOAD_FUNC(device, vkGetAccelerationStructureDeviceAddressKHR);
    LOAD_FUNC(device, vkGetAccelerationStructureBuildSizesKHR);
    LOAD_FUNC(device, vkCreateAccelerationStructureKHR);
    LOAD_FUNC(device, vkCmdBuildAccelerationStructuresKHR);
    LOAD_FUNC(device, vkDestroyAccelerationStructureKHR);

    blasScratchBuffer.createBuffer(device, physicalDevice, SCRATCH_BUFFFER_SIZE, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);

    blasBuffer.createBuffer(device, physicalDevice, BLAS_BUFFER_SIZE, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);

    boundingBoxBuffer.createBuffer(device, physicalDevice, sizeof(VkAabbPositionsKHR), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);
    boundingBoxStagingBuffer.createBuffer(device, physicalDevice, sizeof(VkAabbPositionsKHR), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, false);
    vkMapMemory(device, boundingBoxStagingBuffer.bufferMemory, 0, sizeof(VkAabbPositionsKHR), 0, &boundingBoxStagingBufferMapped);

    instanceBuffer.createBuffer(device, physicalDevice, MAX_BLAS * sizeof(InstanceData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);
    instanceStagingBuffer.createBuffer(device, physicalDevice, sizeof(InstanceData), VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, false);
    vkMapMemory(device, instanceStagingBuffer.bufferMemory, 0, sizeof(InstanceData), 0, &instanceStagingBufferMapped);

    geometryBuffer.createBuffer(device, physicalDevice, MAX_GEO_BUFFER_SIZE, VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);
    geometryStagingBuffer.createBuffer(device, physicalDevice, MAX_SINGLE_VV, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, false);
    vkMapMemory(device, geometryStagingBuffer.bufferMemory, 0, MAX_SINGLE_VV, 0, &geometryStagingBufferMapped);
}

VkDeviceAddress AccelerationStructureManager::getAccelerationStructureAddress(AccelerationStructure* accelerationStructure) {
    VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    addressInfo.accelerationStructure = accelerationStructure->handle;

    return vkGetAccelerationStructureDeviceAddressKHR(device, &addressInfo);
}

VkAccelerationStructureKHR AccelerationStructureManager::getTopLevelAccelrationStructure() {
    return topLevelStructure.handle;
}

VkBuffer AccelerationStructureManager::getInstanceBuffer() {
    return instanceBuffer.handle;
}

AccelerationStructure AccelerationStructureManager::createBottomLevelAccelerationStructure() {
    //Fill in the geometry struct which contains the data we wanna turn in to BLAS
    VkAccelerationStructureGeometryAabbsDataKHR aabbData{};
    aabbData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
    aabbData.data.deviceAddress = boundingBoxBuffer.getBufferAddress(device);
    aabbData.stride = sizeof(VkAabbPositionsKHR);
    aabbData.pNext = nullptr;

    //More info on this shit
    VkAccelerationStructureGeometryKHR geometry{};
    geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;
    geometry.geometry.aabbs = aabbData;
    geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
    geometry.pNext = nullptr;

    //Gives info on how the data is laid out
    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo{};
    buildRangeInfo.primitiveCount = 1;
    buildRangeInfo.primitiveOffset = 0;
    buildRangeInfo.transformOffset = 0;
    buildRangeInfo.firstVertex = 0;

    //Information on how this data needs to be built
    VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo{};
    buildGeometryInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildGeometryInfo.pNext = nullptr;
    buildGeometryInfo.srcAccelerationStructure = VK_NULL_HANDLE;
    buildGeometryInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildGeometryInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
    buildGeometryInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildGeometryInfo.geometryCount = 1;
    buildGeometryInfo.pGeometries = &geometry;
    buildGeometryInfo.ppGeometries = nullptr;

    //Gets the information on the size of scratch buffer and the size of accceleration structure in memory
    VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
    sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    uint32_t count = 1;
    vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &count, &sizeInfo);

    //assign the scratch buffer
    buildGeometryInfo.scratchData.deviceAddress = blasScratchBuffer.getBufferAddress(device);

    if(sizeInfo.buildScratchSize > SCRATCH_BUFFFER_SIZE) throw runtime_error("SCRATCH BUFFER SIZE EXCEDDED");
    if(sizeInfo.accelerationStructureSize + blasOffset > BLAS_BUFFER_SIZE) throw runtime_error("BLAS BUFFER SIZE EXCEDDED");

    //create the acceleration struture
    VkAccelerationStructureCreateInfoKHR accelerationCreateInfo;
    accelerationCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelerationCreateInfo.buffer = blasBuffer.handle;
    accelerationCreateInfo.size = sizeInfo.accelerationStructureSize;
    accelerationCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    accelerationCreateInfo.pNext = VK_NULL_HANDLE;
    accelerationCreateInfo.offset = blasOffset;
    accelerationCreateInfo.createFlags = 0;
    accelerationCreateInfo.deviceAddress = 0;

    VkAccelerationStructureKHR accelerationStructure;
    vkCreateAccelerationStructureKHR(device, &accelerationCreateInfo, nullptr, &accelerationStructure);

    buildGeometryInfo.dstAccelerationStructure = accelerationStructure;

    //FINALLY send the acceleration strucuture off the the gpu where it is built and will stay for the rest of its life before it is destoryed
    VkAccelerationStructureBuildRangeInfoKHR* infos[] = { &buildRangeInfo };

    CommandBuffer commandBuffer;
    commandBuffer.createCommandBuffer(device, buildPool);
    commandBuffer.beginRecording(true);

    vkCmdBuildAccelerationStructuresKHR(commandBuffer.handle, 1, &buildGeometryInfo, (VkAccelerationStructureBuildRangeInfoKHR* const*)&infos);

    commandBuffer.endRecording();

    //submit to the queue
    commandBuffer.flush(buildQueue);

    blasOffset += 1280;

    AccelerationStructure as{};
    as.handle = accelerationStructure;
    as.isTLAS = false;

    return as;
}

void AccelerationStructureManager::addBottomLevelAccelerationStructure(DataType type, glm::vec3 bMin, int side, float voxelWidth, uint8_t voxels[], int size) {
    InstanceData data{};
    data.voxelWidth = voxelWidth;
    data.bMin = bMin;
    data.side = side;
    data.type = type;
    data.geometryData = geometryBuffer.getBufferAddress(device) + (uint64_t)geometryOffset;

    VkAabbPositionsKHR boundingBox = {
        0 , 0 , 0 ,
        voxelWidth * side, voxelWidth *  side, voxelWidth * side,
    };

    CommandBuffer commandBuffer;
    commandBuffer.createCommandBuffer(device, transferPool);
    commandBuffer.beginRecording(true);

    if(geometryOffset + size > MAX_GEO_BUFFER_SIZE || size > MAX_SINGLE_VV) {
        throw runtime_error(" Size of geo buffer excceded");
    }
    memcpy(geometryStagingBufferMapped, (void*)voxels, size * sizeof(uint8_t));
    Buffer::copyBuffer(geometryStagingBuffer, geometryBuffer, sizeof(uint8_t) * size, commandBuffer, 0, geometryOffset);

    memcpy(instanceStagingBufferMapped, &data, sizeof(InstanceData));
    Buffer::copyBuffer(instanceStagingBuffer, instanceBuffer, sizeof(InstanceData), commandBuffer, 0, instanceOffset);

    memcpy(boundingBoxStagingBufferMapped, &boundingBox, sizeof(VkAabbPositionsKHR));
    Buffer::copyBuffer(boundingBoxStagingBuffer, boundingBoxBuffer, sizeof(VkAabbPositionsKHR), commandBuffer);

    commandBuffer.endRecording();
    commandBuffer.flush(transferQueue);
    commandBuffer.freeCommandBuffer(device, transferPool);

    geometryOffset += size * sizeof(uint8_t);
    instanceOffset += sizeof(InstanceData);

    bottomLevelStructures.push_back(createBottomLevelAccelerationStructure());

    VkTransformMatrixKHR transform = {
        1, 0, 0, bMin.x,
        0, 1, 0, bMin.y,
        0, 0, 1, bMin.z
    };

    transforms.push_back(transform);
}

void AccelerationStructureManager::createTopLevelAccelerationStructure() {
    std::vector<VkAccelerationStructureInstanceKHR> instances;

    if(bottomLevelStructures.size() == 0) throw runtime_error("No bottom level structures");

    for(int i = 0; i < bottomLevelStructures.size(); i++) {
        VkAccelerationStructureInstanceKHR instance{};
		instance.transform = transforms[i];
		instance.instanceCustomIndex = i;
		instance.mask = 0xFF;
		instance.instanceShaderBindingTableRecordOffset = 0;
		instance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
		instance.accelerationStructureReference = getAccelerationStructureAddress(&bottomLevelStructures[i]);
        instances.push_back(instance);
    }

    //create a buffer on the gpu which has the data in the exact format wanted by vulkan
    Buffer blasInstanceBuffer;
    blasInstanceBuffer.createBuffer(device, physicalDevice, sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);
    blasInstanceBuffer.populateBuffer(device, physicalDevice, instances.data(), sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), transferPool, transferQueue);

    //Fill the data related to the instances
    VkAccelerationStructureGeometryInstancesDataKHR instanceData{};
    instanceData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    instanceData.data.deviceAddress = blasInstanceBuffer.getBufferAddress(device);

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
    vkGetAccelerationStructureBuildSizesKHR(device, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometryInfo, &count, &sizeInfo);

    //Make a scratch buffer from the info gathered
    Buffer scratchBuffer;
    scratchBuffer.createBuffer(device, physicalDevice, sizeInfo.buildScratchSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);

    //assign the scratch buffer
    buildGeometryInfo.scratchData.deviceAddress = scratchBuffer.getBufferAddress(device);

    //make the buffer in which the acceleration strcuture will reside
    Buffer accelerationStructureBuffer;
    accelerationStructureBuffer.createBuffer(device, physicalDevice, sizeInfo.accelerationStructureSize, VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false);

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
    vkCreateAccelerationStructureKHR(device, &accelerationCreateInfo, nullptr, &accelerationStructure);

    buildGeometryInfo.dstAccelerationStructure = accelerationStructure;

    //FINALLY send the acceleration strucuture off the the gpu where it is built and will stay for the rest of its life before it is destoryed
    VkAccelerationStructureBuildRangeInfoKHR* infos[] = { &buildRangeInfo };

    CommandBuffer commandBuffer;
    commandBuffer.createCommandBuffer(device, buildPool);
    commandBuffer.beginRecording(true);

    vkCmdBuildAccelerationStructuresKHR(commandBuffer.handle, 1, &buildGeometryInfo, (VkAccelerationStructureBuildRangeInfoKHR* const*)&infos);

    commandBuffer.endRecording();

    //submit to the queue
    commandBuffer.flush(buildQueue);
    commandBuffer.freeCommandBuffer(device, buildPool);

    scratchBuffer.destroy(device);
    blasInstanceBuffer.destroy(device);

    topLevelStructure = {accelerationStructure, accelerationStructureBuffer, true};
}

void AccelerationStructureManager::destroyAccelerationStructure(AccelerationStructure as) {
    if(as.isTLAS == true) as.buffer.destroy(device);
    vkDestroyAccelerationStructureKHR(device, as.handle, nullptr);
}

void AccelerationStructureManager::cleanup() {
    vkUnmapMemory(device, boundingBoxStagingBuffer.bufferMemory);
    boundingBoxStagingBuffer.destroy(device);
    boundingBoxBuffer.destroy(device);

    vkUnmapMemory(device, instanceStagingBuffer.bufferMemory);
    instanceStagingBuffer.destroy(device);
    instanceBuffer.destroy(device);

    vkUnmapMemory(device, geometryStagingBuffer.bufferMemory);
    geometryStagingBuffer.destroy(device);
    geometryBuffer.destroy(device);

    blasBuffer.destroy(device);
    blasScratchBuffer.destroy(device);

    for(int i = 0; i < bottomLevelStructures.size(); i++) {
        destroyAccelerationStructure(bottomLevelStructures[i]);
    }

    destroyAccelerationStructure(topLevelStructure);
}
