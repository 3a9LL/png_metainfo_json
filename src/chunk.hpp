/*
 * chunk.hpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#ifndef CHUNK_HPP_
#define CHUNK_HPP_

#include <cstdlib>
#include <cstdio>

#include "common.hpp"
#include "DataBuf.h"
#include "utils.hpp"
#include <zlib.h>

void decodeIHDRChunk(const DataBuf& data, int* outWidth, int* outHeight);

DataBuf keyTXTChunk(const DataBuf& data, bool stripHeader = false);
DataBuf parseTXTChunk(const DataBuf& data, int keysize, TxtChunkType type);
DataBuf decodeTXTChunk(const DataBuf& data, TxtChunkType type);
bool zlibToDataBuf(const byte* bytes, long length, DataBuf& result);
void zlibUncompress(const byte*  compressedText, unsigned int compressedTextSize, DataBuf& arr);

#endif /* CHUNK_HPP_ */
