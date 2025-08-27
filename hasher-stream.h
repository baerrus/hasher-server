//
// Copyright (c) 2024 Vlad Troyanker
//
#ifndef HASHER_STREAM_H
#define HASHER_STREAM_H

#include "buffer.h"

#include <openssl/md5.h>

/*
 * Input data is character based stream of lines where lines delimited by a single character '\n'
 *
 * Output is a MD5 hash of each input line (not including th delimiter char).
 * 
 * TODO: parametrize the class to support any hashing function
 */

class HasherStream {

    static const auto delim = '\n';

    enum { result_size = MD5_DIGEST_LENGTH * 2 + 1 };

public:
    HasherStream();

    void proc_bytes(Buffer buffer, WriteCallback wrcb);
    void finalize_bytes(WriteCallback wrcb);

private:
    void reset_context();
    void finalize(WriteCallback wrcb);
    void parse_bytes(Buffer buffer, WriteCallback wrcb);
    void process_block(char* start, char* end);
    Buffer format(unsigned char digest[]);

    uint8_t* hexencode(uint8_t* first, uint8_t* last, uint8_t* out);

    MD5_CTX ctx;
    bool pending_data_ = false; // true if there is a pending data to process
    // this is required to avoid finalizing an empty context
};

#endif