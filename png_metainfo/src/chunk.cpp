/*
 * chunk.cpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#include "chunk.hpp"

void decodeIHDRChunk(const DataBuf& data, int* outWidth, int* outHeight) {
	// Extract image width and height from IHDR chunk.
	*outWidth 	= getULong(data.pData);
	*outHeight 	= getULong(data.pData + 4);
}

void decodeTIMEChunk(const DataBuf& data, date_t* date) {
	// Extract image width and height from IHDR chunk.
	date->year 		= getUShort(data.pData);
	date->month 	= data.pData[2];
	date->day 		= data.pData[3];
	date->hour 		= data.pData[4];
	date->minute 	= data.pData[5];
	date->second	= data.pData[6];
}

pair_t decodeTXTChunk(const DataBuf& data,
                                     TxtChunkType   type)
{
	DataBuf key = keyTXTChunk(data);

#ifdef DEBUG
	std::cout << "decodeTXTChunk: TXT chunk key: "
			<< std::string((const char*)key.pData, key.size) << "\n";
#endif
	DataBuf value = parseTXTChunk(data, key.size, type);
	return {std::string((const char*)key.pData, 	key.size),
			std::string((const char*)value.pData, 	value.size)};
}

DataBuf keyTXTChunk(const DataBuf& data, bool stripHeader)
{
	// From a tEXt, zTXt, or iTXt chunk,
	// we get the key, it's a null terminated string at the chunk start
	if (data.size <= (stripHeader ? 8 : 0))
		chunk_error();
	const byte *key = data.pData + (stripHeader ? 8 : 0);

	// Find null string at end of key.
	int keysize=0;
	for ( ; key[keysize] != 0 ; keysize++)
    {
    	// look if keysize is valid.
    	if (keysize >= data.size)
    		chunk_error();
	}
	return DataBuf(key, keysize);
}

DataBuf parseTXTChunk(const DataBuf& data, int keysize, TxtChunkType type) {
	DataBuf arr;

	if (type == zTXt_Chunk) {
		// Extract a deflate compressed Latin-1 text chunk
		// we get the compression method after the key
		const byte* compressionMethod = data.pData + keysize + 1;
		if (*compressionMethod != 0x00) {
			// then it isn't zlib compressed and we are sunk
#ifdef DEBUG
			std::cerr << "parseTXTChunk: Non-standard zTXt compression method.\n";
#endif
    		chunk_error();
		}

		// compressed string after the compression technique spec
		const byte* compressedText 		= data.pData + keysize + 2;
		unsigned int compressedTextSize = data.size - keysize - 2;

		zlibUncompress(compressedText, compressedTextSize, arr);
		return arr;
	}
    else if(type == tEXt_Chunk)
    {
        // Extract a non-compressed Latin-1 text chunk

        // the text comes after the key, but isn't null terminated
        const byte* text = data.pData + keysize + 1;
        long textsize    = data.size  - keysize - 1;

        return DataBuf(text, textsize);
    }
    else if (type == iTXt_Chunk) {
		// Extract a deflate compressed or uncompressed UTF-8 text chunk

		// we get the compression flag after the key
		const byte* compressionFlag 	= data.pData + keysize + 1;
		// we get the compression method after the compression flag
		const byte* compressionMethod	= data.pData + keysize + 2;
		// language description string after the compression technique spec
		std::string languageText((const char*) (data.pData + keysize + 3));
		unsigned int languageTextSize =
				static_cast<unsigned int>(languageText.size());
		// translated keyword string after the language description
		std::string translatedKeyText(
				(const char*) (data.pData + keysize + 3 + languageTextSize + 1));
		unsigned int translatedKeyTextSize =
				static_cast<unsigned int>(translatedKeyText.size());

		if (compressionFlag[0] == 0x00) {
			// then it's an uncompressed iTXt chunk
#ifdef DEBUG
			std::cout << "parseTXTChunk: We found an uncompressed iTXt field\n";
#endif

			// the text comes after the translated keyword, but isn't null terminated
			const byte* text = data.pData + keysize + 3 + languageTextSize + 1
					+ translatedKeyTextSize + 1;
			long textsize = data.size
					- (keysize + 3 + languageTextSize + 1
							+ translatedKeyTextSize + 1);
			return DataBuf(text, textsize);
		} else if (compressionFlag[0] == 0x01 && compressionMethod[0] == 0x00) {
			// then it's a zlib compressed iTXt chunk
#ifdef DEBUG
			std::cout << "parseTXTChunk: We found a zlib compressed iTXt field\n";
#endif

			// the compressed text comes after the translated keyword, but isn't null terminated
			const byte* compressedText = data.pData + keysize + 3
					+ languageTextSize + 1 + translatedKeyTextSize + 1;
			long compressedTextSize = data.size
					- (keysize + 3 + languageTextSize + 1
							+ translatedKeyTextSize + 1);

			zlibUncompress(compressedText, compressedTextSize, arr);
			return arr;
		} else {
			// then it isn't zlib compressed and we are sunk
#ifdef DEBUG
			std::cerr << "parseTXTChunk: Non-standard iTXt compression method.\n";
#endif
    		chunk_error();
		}
	}
#ifdef DEBUG
		std::cerr << "parseTXTChunk: We found a field, not expected though\n";
#endif
	chunk_error();
	return arr;
} // parsePngChunk

bool zlibToDataBuf(const byte* bytes, long length, DataBuf& result) {
	uLongf uncompressedLen = length * 2;
	int zlibResult;

	do {
		result.alloc(uncompressedLen);
		zlibResult = uncompress((Bytef*) result.pData, &uncompressedLen, bytes,
				length);
		// if result buffer is large than necessary, redo to fit perfectly.
		if (zlibResult == Z_OK && (long) uncompressedLen < result.size) {
			result.release();
			result.alloc(uncompressedLen);
			zlibResult = uncompress((Bytef*) result.pData, &uncompressedLen,
					bytes, length);
		}
		if (zlibResult == Z_BUF_ERROR) {
			// the uncompressed buffer needs to be larger
			result.release();

			// Sanity - never bigger than 16mb
			if (uncompressedLen > 16 * 1024 * 1024)
				zlibResult = Z_DATA_ERROR;
			else
				uncompressedLen *= 2;
		}
	} while (zlibResult == Z_BUF_ERROR);

	return zlibResult == Z_OK;
}

void zlibUncompress(const byte*  compressedText, unsigned int compressedTextSize, DataBuf& arr)
    {
        uLongf uncompressedLen = compressedTextSize * 2; // just a starting point
        int zlibResult;
        int dos = 0;

        do {
            arr.alloc(uncompressedLen);
            zlibResult = uncompress((Bytef*)arr.pData,
                                    &uncompressedLen,
                                    compressedText,
                                    compressedTextSize);
            if (zlibResult == Z_OK) {
                assert((uLongf)arr.size >= uncompressedLen);
                arr.size = uncompressedLen;
            }
            else if (zlibResult == Z_BUF_ERROR) {
                // the uncompressedArray needs to be larger
                uncompressedLen *= 2;
                // DoS protection. can't be bigger than 64k
                if (uncompressedLen > 131072) {
                    if (++dos > 1) break;
                    uncompressedLen = 131072;
                }
            }
            else {
                // something bad happened
        		chunk_error();
            }
        }
        while (zlibResult == Z_BUF_ERROR);

        if (zlibResult != Z_OK)
    		chunk_error();
    } // PngChunk::zlibUncompress
