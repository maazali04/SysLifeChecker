#pragma once

#include "SystemInfo.hpp"

const char* NetworkAdapterTypeToString(NetworkAdapterType type);

const char* NetworkStatusToString(NetworkStatus status);

std::vector<NetworkAdapter> GetNetworkAdapters();