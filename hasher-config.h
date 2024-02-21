//
// Copyright (c) 2024 Vlad Troyanker
//

#ifndef HASHER_CONFIGURATION_H

#include <string>
#include <optional>
#include <functional>


struct Configuration {

    std::optional<short> port;
    std::optional<std::string> addr;
    std::optional<unsigned> conn_pool_capacity;
    std::optional<unsigned> compute_pool_capacity;
    std::optional<unsigned> socket_listen_capacity;

    Configuration();


    int convert_int(const char * name, char * value, std::function<void (int)> validate);


    void validator_intrange(int value, int low, int high);
    void validator_greater (int value, int low);
    void validator_ipaddr  (const char* value);
};


#define HASHER_CONFIGURATION_H
#endif