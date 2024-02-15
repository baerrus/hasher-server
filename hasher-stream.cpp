//
// Copyright (c) 2024 Vlad Troyanker
//

#include "hasher-stream.h"

#include <openssl/md5.h>

	HasherStream::HasherStream() {
		reset();
	}

	void HasherStream::work(Buffer buffer, WriteCallback wrcb) {

		auto hash_finalize = [this, wrcb](char * start, char * end){

			unsigned char digest[MD5_DIGEST_LENGTH];

			MD5_Update(&ctx, start, end - start);
			if(MD5_Final(digest, &ctx) == 0)
				throw std::runtime_error("MD5 error");
			auto res_buffer = format(digest);
			wrcb(res_buffer);
		};

		parse(buffer, hash_finalize);
	}


	void HasherStream::reset() {
		MD5_Init(&ctx);
	}

	void HasherStream::parse(Buffer buffer, std::function<void (char * start, char * end)> finalize){
		char * pb = buffer->data(), 
		     * chunk = buffer->data();

		for(; pb < buffer->data()+buffer->size(); pb++) {
			if(*pb == delim) {
				finalize(chunk, pb); // exclude the delim
				reset();
				chunk = pb + 1; // skip the delimiter
			}
		}
		if(pb > chunk)
			process(chunk, pb);
	}


	void HasherStream::process(char * start, char * end) {

		if(MD5_Update(&ctx, start, end - start) == 0)
			throw std::runtime_error("MD5 error");
	}

	Buffer HasherStream::format(unsigned char digest[]) {
		auto buffer = make_buffer(result_size);
		hexencode(digest, &digest[MD5_DIGEST_LENGTH], (uint8_t*)buffer->data());
		buffer->at(result_size-1) = delim;
		return buffer;
	}

    uint8_t* HasherStream::hexencode(uint8_t* first, uint8_t* last, uint8_t* out)
    {
        //static_assert(sizeof *first == 1, "unsupported type");
        while (first != last) {
            uint8_t const byte = *first++;
            char const nib1 = (byte & 0xf0) >> 4;
            char const nib2 = byte & 0x0f;
            *out++ = nib1 > 9 ? nib1 + 'a' - 10 : nib1 + '0';
            *out++ = nib2 > 9 ? nib2 + 'a' - 10 : nib2 + '0';
        }
        return out;
    }


