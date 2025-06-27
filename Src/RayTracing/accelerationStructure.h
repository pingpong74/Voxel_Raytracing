#include <cstdint>
#include <vulkan/vulkan_core.h>
#include <vector>
#include "../buffer.h"
#include "../DataStructures/voxel.h"

using namespace std;

#define LOAD_FUNC(device, name) \
name = (PFN_##name)vkGetDeviceProcAddr(device, #name); \
if(!name) { throw std::runtime_error("Failed to load function"); }

//Scratch buffer size: 2688
//Acceleratiion StructureBuffer size: 1152 -> round to 1280

#define MAX_BLAS 10
#define SCRATCH_BUFFFER_SIZE MAX_BLAS * 2688
#define BLAS_BUFFER_SIZE MAX_BLAS * 1280

#define MAX_GEO_BUFFER_SIZE 10 * 1000000
#define MAX_SINGLE_VV 1000000

struct AccelerationStructure {
    VkAccelerationStructureKHR handle;
    Buffer buffer;
    bool isTLAS = false;
};

class AccelerationStructureManager {
    public:

    void create(VkDevice _device, VkPhysicalDevice _physicalDevice, VkQueue _buildQueue, VkCommandPool _buildPool, VkQueue transferQueue, VkCommandPool transferPool);

    VkBuffer getInstanceBuffer();
    VkAccelerationStructureKHR getTopLevelAccelrationStructure();

    void addBottomLevelAccelerationStructure(DataType type, glm::vec3 bMin, int side, float voxelWidth, uint8_t voxels[], int size);
    void createTopLevelAccelerationStructure();
    void updateTopLevelAccelerationStructure();

    void cleanup();

    private:

    VkDevice device;
    VkPhysicalDevice physicalDevice;

    VkQueue buildQueue;
    VkCommandPool buildPool;

    VkQueue transferQueue;
    VkCommandPool transferPool;

    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = nullptr;
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = nullptr;
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = nullptr;
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = nullptr;
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = nullptr;

    Buffer blasScratchBuffer;

    Buffer blasBuffer;
    int blasOffset = 0;

    Buffer boundingBoxBuffer;
    Buffer boundingBoxStagingBuffer;
    void* boundingBoxStagingBufferMapped;

    Buffer geometryBuffer;
    Buffer geometryStagingBuffer;
    void* geometryStagingBufferMapped;
    int geometryOffset = 0;

    Buffer instanceBuffer;
    Buffer instanceStagingBuffer;
    void* instanceStagingBufferMapped;
    int instanceOffset = 0;

    vector<AccelerationStructure> bottomLevelStructures;
    vector<VkTransformMatrixKHR> transforms;
    AccelerationStructure topLevelStructure;

    VkDeviceAddress getAccelerationStructureAddress(AccelerationStructure* acccelerationStructure);

    AccelerationStructure createBottomLevelAccelerationStructure();

    void destroyAccelerationStructure(AccelerationStructure as);
};
