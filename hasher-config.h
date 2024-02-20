//
// Copyright (c) 2024 Vlad Troyanker
//

#ifndef HASHER_CONFIGURATION_H

#include <string>
#include <optional>


struct Configuration {

    std::optional<short> port;
    std::optional<std::string> addr;
    std::optional<unsigned> conn_pool_capacity;
    std::optional<unsigned> compute_pool_capacity;

    Configuration();
};


#define HASHER_CONFIGURATION_H
#endif