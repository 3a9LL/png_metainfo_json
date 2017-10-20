/*
 * utils.cpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */
#include "utils.hpp"

dword getULong(const byte* data) {
	dword r = 0;
	byte* r_ptr = (byte*) &r;
	r_ptr[0] = data[3];
	r_ptr[1] = data[2];
	r_ptr[2] = data[1];
	r_ptr[3] = data[0];
	return r;
}

word getUShort(const byte* data) {
	word r = 0;
	r |= data[1];
	r |= data[0] << 8;
	return r;
}

void chunk_error()
{
    fprintf(stderr, "Wrong chunk format\n");
    exit(ECODE::CHUNK_ERROR);
}

std::string dateToString(date_t& date) {
	return std::to_string(date.day) + "/" + std::to_string(date.month) + "/" + std::to_string(date.year) +
			", " + std::to_string(date.hour) + ":" + std::to_string(date.minute) + ":" + std::to_string(date.second) + " UTC";
}
