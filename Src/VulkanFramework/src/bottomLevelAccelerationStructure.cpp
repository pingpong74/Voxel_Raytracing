#include "../includes/accelerationStructure.hpp"
#include <vulkan/vulkan_core.h>

#include "../vulkanConfig.hpp"
#include "../includes/loadedFunctions.hpp"

using namespace vkf;

std::vector<BottomLevelAccelerationStructure> BottomLevelAccelerationStructure::createBottomLevelAccelerationStructure(VkCommandBuffer commandBuffer, LogicalDevice* logicalDevice, BottomLevelAccelerationStructureBuildInfo* blasBuldInfo, uint32_t count) {

    VkAccelerationStructureBuildGeometryInfoKHR geoInfos[count];
    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfos[count];
    std::vector<BottomLevelAccelerationStructure> accelerationStructures(count);

    for(int i = 0; i < count; i++) {
        VkAccelerationStructureGeometryAabbsDataKHR aabbData{};
        aabbData.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
        aabbData.data.deviceAddress = blasBuldInfo[i].boundingBoxBufferAddress;
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
        buildRangeInfos[i].primitiveCount = 1;
        buildRangeInfos[i].primitiveOffset = 0;
        buildRangeInfos[i].transformOffset = 0;
        buildRangeInfos[i].firstVertex = 0;

        //Information on how this data needs to be built
        geoInfos[i].sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        geoInfos[i].pNext = nullptr;
        geoInfos[i].srcAccelerationStructure = VK_NULL_HANDLE;
        geoInfos[i].type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        geoInfos[i].flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        geoInfos[i].mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        geoInfos[i].geometryCount = 1;
        geoInfos[i].pGeometries = &geometry;
        geoInfos[i].ppGeometries = nullptr;

        //Gets the information on the size of scratch buffer and the size of accceleration structure in memory
        VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
        sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        uint32_t count = 1;
        vkGetAccelerationStructureBuildSizesKHR(logicalDevice->handle, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &geoInfos[i], &count, &sizeInfo);

        //assign the scratch buffer
        geoInfos[i].scratchData.deviceAddress = blasBuldInfo[i].scratchBufferAddress;

        //create the acceleration struture
        VkAccelerationStructureCreateInfoKHR accelerationCreateInfo;
        accelerationCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        accelerationCreateInfo.buffer = blasBuldInfo[i].blasBuffer->handle;
        accelerationCreateInfo.size = sizeInfo.accelerationStructureSize;
        accelerationCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
        accelerationCreateInfo.pNext = VK_NULL_HANDLE;
        accelerationCreateInfo.offset = blasBuldInfo[i].blasOffset;
        accelerationCreateInfo.createFlags = 0;
        accelerationCreateInfo.deviceAddress = 0;

        VK_CHECK(vkCreateAccelerationStructureKHR(logicalDevice->handle, &accelerationCreateInfo, nullptr, &accelerationStructures[i].handle), "Failed to build blas")

        geoInfos[i].dstAccelerationStructure = accelerationStructures[i].handle;
        accelerationStructures[i].logicalDevice = logicalDevice;
    }

    VkAccelerationStructureBuildRangeInfoKHR* infos[count];

    for(int i = 0; i < count; i++) {
        infos[i] = &buildRangeInfos[i];
    }

    vkCmdBuildAccelerationStructuresKHR(commandBuffer, count, geoInfos, (VkAccelerationStructureBuildRangeInfoKHR* const*)&infos);

    return accelerationStructures;
}

VkDeviceAddress BottomLevelAccelerationStructure::getAddress() {
    VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
    addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
    addressInfo.accelerationStructure = handle;

    return vkGetAccelerationStructureDeviceAddressKHR(logicalDevice->handle, &addressInfo);
}

BottomLevelAccelerationStructure::~BottomLevelAccelerationStructure() {
    if(handle != VK_NULL_HANDLE) vkDestroyAccelerationStructureKHR(logicalDevice->handle, handle, nullptr);
}
