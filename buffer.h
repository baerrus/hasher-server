//
// Copyright (c) 2024 Vlad Troyanker
//
#include <memory>
#include <vector>
#include <cstdlib>
#include <functional>
/*
 * Creates a self contained copy of a buffer. Suitable to 
 * work as an underlying storage for the asio::buffer as well
 * as passing around (even across thread boundaries).
 */


using Buffer = std::shared_ptr<std::vector<char>>;


Buffer make_buffer(unsigned size);


using WriteCallback  = std::function<void (Buffer buffer)>;

using ComputeHnadler = std::function<void (Buffer buffer, WriteCallback wrcb)>;
