//
// Copyright (c) 2024 Vlad Troyanker
//

#include "buffer.h"

#include <openssl/md5.h>

/*
 * Data is delimited by a single character '\n'
 */


class HasherStream {

	static const auto delim = '\n';

    enum { result_size = MD5_DIGEST_LENGTH*2+1 };

public:
	HasherStream();

	void work(Buffer buffer, WriteCallback wrcb);


private:

	void reset();
	void parse(Buffer buffer, std::function<void (char * start, char * end)> finalize);
	void process(char * start, char * end);
	Buffer format(unsigned char digest[]);
	
    uint8_t* hexencode(uint8_t* first, uint8_t* last, uint8_t* out);


    MD5_CTX ctx;
};