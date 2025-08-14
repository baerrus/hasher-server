//
// Copyright (c) 2024 Vlad Troyanker
//
#ifndef HASHER_BUFFER_H
#define HASHER_BUFFER_H

#include <memory>
#include <vector>
#include <cstdlib>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <deque>
/*
 * Creates a self contained copy of a buffer. Suitable to 
 * work as an underlying storage for the asio::buffer as well
 * as passing around (even across thread boundaries).
 */


using Buffer = std::shared_ptr<std::vector<char>>;


Buffer make_buffer(unsigned size);


using WriteCallback  = std::function<void (Buffer buffer)>;

using ComputeHandler = std::function<void (Buffer buffer, WriteCallback wrcb)>;

/*
 A minimal thread-safe buffer queue
*/
class BufferQueue
{
	using size_type = unsigned int;

	mutable std::mutex mutex_;
	std::condition_variable cond_; // has data condition
	std::deque<Buffer> queue_;

public:
	void enqueue(Buffer buf);
	Buffer dequeue();
	size_type size() const;
	bool empty() const;
};

#endif