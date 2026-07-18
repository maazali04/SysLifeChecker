#pragma once

#include "SystemInfo.hpp"

std::vector<GPUInfo> GetGPUInfo();

const char* GPUVendorToString(GPUVendor vendor);
const char* GPUTypeToString(GPUType type);
const char* MemoryTypeToString(MemoryType type);