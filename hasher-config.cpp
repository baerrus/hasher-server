//
// Copyright (c) 2024 Vlad Troyanker
//

#include "hasher-config.h"

#include <cstdlib>


Configuration::Configuration()
{
    if (auto* v = getenv("HASHER_SERVER_PORT")) {
        port = std::stoi(v);
    }
    if (auto* v = getenv("HASHER_SERVER_ADDRESS")) {
        addr = v;
    }
    if (auto* v = getenv("HASHER_SERVER_CONN_CAP")) {
        conn_pool_capacity = std::min(std::stoi(v), 1);
    }
    if (auto* v = getenv("HASHER_SERVER_COMPUTE_CAP")) {
        compute_pool_capacity = std::min(std::stoi(v), 1);
    }
}
