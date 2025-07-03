#pragma once

#define VK_CHECK(name, err) \
if(name != VK_SUCCESS) { throw runtime_error(err); }

#include "Device/logicalDevice.h"
#include "Swapchain/swapchain.h"
#include "buffer.h"
#include "commandBuffer.h"
