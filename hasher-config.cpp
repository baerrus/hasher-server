//
// Copyright (c) 2024 Vlad Troyanker
//

#include "hasher-config.h"

#include <cstdlib>
#include <stdexcept>
#include <sstream>
#include <iostream>


Configuration::Configuration()
{
    if (auto* v = getenv("HASHER_SERVER_PORT")) {
        port = convert_int("HASHER_SERVER_PORT", v, [this](int v){ validator_greater(v, 1024); });
    }
    if (auto* v = getenv("HASHER_SERVER_SOCK_LISTEN")) {
        socket_listen_capacity = convert_int("HASHER_SERVER_SOCK_LISTEN", v, [this](int v){ validator_greater(v, 1); });
    }
    if (auto* v = getenv("HASHER_SERVER_ADDRESS")) {
        addr = v;
    }
    if (auto* v = getenv("HASHER_SERVER_CONN_CAP")) {
        conn_pool_capacity = convert_int("HASHER_SERVER_CONN_CAP", v, [this](int v){ validator_intrange(v, 1, 256); });
     }
    if (auto* v = getenv("HASHER_SERVER_COMPUTE_CAP")) {
        compute_pool_capacity = convert_int("HASHER_SERVER_COMPUTE_CAP", v, [this](int v){ validator_intrange(v, 1, 256); });
    }
}

int Configuration::convert_int(const char * name, char * value, std::function<void (int)> validate) {
    try {
        auto v = std::stoi(value);
        validate(v);
        std::cout << name << " = " << v << std::endl;
        return v;
    }
    catch (std::exception& e){
        std::ostringstream oss;
        oss << "configuration: " << name << " : " << value <<  " : " << e.what();
        throw std::runtime_error(oss.str());
    }
}

void Configuration::validator_greater (int value, int min_value) {
    if(value < min_value)
        throw std::range_error("validator greater failed");
}

void Configuration::validator_intrange (int value, int min_value, int max_value) {
    if(min_value > max_value || value < min_value || value > max_value)
        throw std::range_error("validator integer range failed");
}