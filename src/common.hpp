/*
 * common.hpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#ifndef COMMON_HPP_
#define COMMON_HPP_

#define DEBUG
#include <cstdlib>
#include <string>
#include <cstring>
#include <cassert>
#include <cstdio>
#include <iostream>

#define byte	unsigned char
#define dword	unsigned long


enum ECODE{
	OPEN_ERROR		= 1,
	TYPE_ERROR		= 2,
	ARG_ERROR		= 3,
	CHUNK_ERROR		= 4,

};

enum TxtChunkType {
	tEXt_Chunk = 0,
    zTXt_Chunk = 1,
    iTXt_Chunk = 2
};

#endif /* COMMON_HPP_ */
