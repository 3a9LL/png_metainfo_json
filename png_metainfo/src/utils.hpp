/*
 * utils.hpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#ifndef UTILS_HPP_
#define UTILS_HPP_

#include "common.hpp"
#include "types.hpp"

dword 			getULong(const byte* data);
word 			getUShort(const byte* data);
void 			chunk_error();
std::string 	dateToString(date_t& date);

#endif /* UTILS_HPP_ */
