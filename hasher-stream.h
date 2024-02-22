//
// Copyright (c) 2024 Vlad Troyanker
//

#include "buffer.h"

#include <openssl/md5.h>

/*
 * Data is delimited by a single character '\n'
 *
 * TODO: parametrize the class to support any hashing function
 */

class HasherStream {

    static const auto delim = '\n';

    enum { result_size = MD5_DIGEST_LENGTH * 2 + 1 };

public:
    HasherStream();

    void work(BufferQueue& bq, WriteCallback wrcb);

private:
    void reset_context();
    void parse_block(Buffer buffer, std::function<void(char* start, char* end)> finalize);
    void process_block(char* start, char* end);
    Buffer format(unsigned char digest[]);

    uint8_t* hexencode(uint8_t* first, uint8_t* last, uint8_t* out);

    MD5_CTX ctx;
};
