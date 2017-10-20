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
#include <zlib.h>
#include <json-c/json.h>

#include "common.hpp"
#include "DataBuf.h"
#include "types.hpp"
#include "utils.hpp"

void decodeIHDRChunk(const DataBuf& data, int* outWidth, int* outHeight);
void decodeTIMEChunk(const DataBuf& data, date_t* date);

DataBuf keyTXTChunk(const DataBuf& data, bool stripHeader = false);
DataBuf parseTXTChunk(const DataBuf& data, int keysize, TxtChunkType type);
pair_t decodeTXTChunk(const DataBuf& data, TxtChunkType type);
bool zlibToDataBuf(const byte* bytes, long length, DataBuf& result);
void zlibUncompress(const byte*  compressedText, unsigned int compressedTextSize, DataBuf& arr);

#endif /* CHUNK_HPP_ */
