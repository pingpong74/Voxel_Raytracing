#pragma once

#define VALIDATION_LAYERS_ENABLED true
#include <vector>
#include <stdexcept>

const std::vector<const char*> validationLayers = { "VK_LAYER_KHRONOS_validation"};
const std::vector<const char*> deviceExtensions = { "VK_KHR_swapchain",
    "VK_KHR_acceleration_structure",
    "VK_KHR_ray_tracing_pipeline",
    "VK_KHR_ray_query",
    "VK_KHR_pipeline_library",
    "VK_KHR_deferred_host_operations",
    "VK_KHR_8bit_storage"
};

#define LOAD_FUNC(device, name) \
name = (PFN_##name)vkGetDeviceProcAddr(device->handle, #name); \
if(!name) { throw std::runtime_error("Failed to load function"); }

#if VALIDATION_LAYERS_ENABLED

    #define VK_CHECK(name, err) \
    if(name != VK_SUCCESS) { throw std::runtime_error(err); }

#else
    #define VK_CHECK(name, err) \
    name;

#endif
