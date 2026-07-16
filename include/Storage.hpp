#pragma once

#include "SystemInfo.hpp"

StorageInfo GetStorageInfo();
const char* PartitionStyleToString(PartitionStyle style);
const char* StorageDeviceTypeToString(StorageDeviceType type);
const char* StorageBusTypeToString(StorageBusType bus);