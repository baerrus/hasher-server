//
// Copyright (c) 2024 Vlad Troyanker
//

#include <cassert>
#include <iostream>
#include <utility>

#include "buffer.h"

Buffer make_buffer(unsigned size)
{

    assert(size > 0);
    auto trackingDeleter = [](std::vector<char>* p) { // debug use only
        std::cout << "Call delete for object size =" << p->size() << '\n';
        delete p;
    };
    return std::shared_ptr<std::vector<char>>(new std::vector<char>(size));
}

void BufferQueue::enqueue(Buffer buf)
{
    std::lock_guard lg(mutex_);
    queue_.push_back(buf);
    cond_.notify_one();
}

Buffer BufferQueue::dequeue()
{
    std::unique_lock lock(mutex_);

    while (queue_.empty())
        cond_.wait(lock);

    auto v = queue_.front();
    queue_.pop_front();
    return v;
}
