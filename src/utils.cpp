/*
 * utils.cpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */
#include "utils.hpp"

dword getLong(const byte* data) {
	dword r = 0;
	byte* r_ptr = (byte*) &r;
	r_ptr[0] = data[3];
	r_ptr[1] = data[2];
	r_ptr[2] = data[1];
	r_ptr[3] = data[0];
	return r;
}

void chunk_error()
{
    printf("Wrong chunk format\n");
    exit(ECODE::CHUNK_ERROR);
}
