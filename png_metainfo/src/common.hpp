/*
 * common.hpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#ifndef COMMON_HPP_
#define COMMON_HPP_

//#define DEBUG
#include <cstdlib>
#include <string>
#include <cstring>
#include <cassert>
#include <vector>
#include <cstdio>
#include <iostream>

typedef unsigned char	byte;
typedef unsigned short	word;
typedef unsigned long	dword;

typedef std::pair<std::string, std::string> pair_t;
typedef std::vector<pair_t>					pairs_t;

enum ECODE{
	OPEN_ERROR		= 1,
	TYPE_ERROR		= 2,
	ARG_ERROR		= 3,
	CHUNK_ERROR		= 4,
	WRONG_PNG		= 5,
};

enum TxtChunkType {
	tEXt_Chunk = 0,
    zTXt_Chunk = 1,
    iTXt_Chunk = 2
};

#endif /* COMMON_HPP_ */
