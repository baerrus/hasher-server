//
// Copyright (c) 2024 Vlad Troyanker
//

#include "hasher-stream.h"

#include <iostream>
#include <openssl/md5.h>

HasherStream::HasherStream()
{
    reset_context();
}

void HasherStream::proc_bytes(BufferQueue& bq, WriteCallback wrcb)
{
    auto buffer = bq.dequeue();
    parse_bytes(buffer, wrcb);
}

void HasherStream::finalize_bytes(WriteCallback wrcb)
{
    finalize(wrcb); // process any remaining bytes in the context
    // and return the final hash result
}

void HasherStream::reset_context()
{
    MD5_Init(&ctx); 
    pending_data_ = false; // reset pending data flag
}

void HasherStream::finalize(WriteCallback wrcb)
{
    unsigned char digest[MD5_DIGEST_LENGTH];
    if (!pending_data_)
        return; // nothing to finalize
    if (MD5_Final(digest, &ctx) == 0)
        throw std::runtime_error("MD5 error");
    reset_context(); // reset context for next use
    auto res_buffer = format(digest);
    wrcb(res_buffer);
}

void HasherStream::parse_bytes(Buffer buffer, WriteCallback wrcb)
{
    char *pb = buffer->data(),
         *chunk = buffer->data();

    for (; pb < buffer->data() + buffer->size(); pb++) {
        if (*pb == delim) {
            process_block(chunk, pb); // exclude the delim
            finalize(wrcb);
            chunk = pb + 1; // skip the delimiter
        }
    }
    if (pb > chunk) // tail of the buffer is not delimited
        process_block(chunk, pb);
}

void HasherStream::process_block(char* start, char* end)
{
    if (MD5_Update(&ctx, start, end - start) == 0)
        throw std::runtime_error("MD5 error");
    pending_data_ = true; 
}

Buffer HasherStream::format(unsigned char digest[])
{
    auto buffer = make_buffer(result_size);
    hexencode(digest, &digest[MD5_DIGEST_LENGTH], (uint8_t*)buffer->data());
    buffer->at(result_size - 1) = delim;
    return buffer;
}

uint8_t* HasherStream::hexencode(uint8_t* first, uint8_t* last, uint8_t* out)
{
    // static_assert(sizeof *first == 1, "unsupported type");
    while (first != last) {
        uint8_t const byte = *first++;
        char const nib1 = (byte & 0xf0) >> 4;
        char const nib2 = byte & 0x0f;
        *out++ = nib1 > 9 ? nib1 + 'a' - 10 : nib1 + '0';
        *out++ = nib2 > 9 ? nib2 + 'a' - 10 : nib2 + '0';
    }
    return out;
}
