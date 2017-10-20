/*
 * main.cpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#include <sys/stat.h>

#include "common.hpp"
#include "utils.hpp"
#include "image.hpp"
#include "chunk.hpp"

// Signature from front of PNG file
const unsigned char pngSignature[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };

bool isPngType(FILE* f, bool advance) {
	const dword len = 8;
	byte buf[len];
	int res;
	if (!f)
		return false;
	res = fread(buf, sizeof(byte), len, f);
	if (res != len) {
		return false;
	}
	int rc = memcmp(buf, pngSignature, 8);
	if (!advance || rc != 0) {
		fseek(f, -len, SEEK_CUR);
	}
	return rc == 0;
}

int main(int argc, char** argv) {
	FILE* png_file;
	struct image_t img;

	if (argc != 2) {
		printf("Usage: %s <png file>", argv[0]);
		return ECODE::ARG_ERROR;
	}
	png_file = fopen(argv[1], "rb");
	if (!png_file) {
		printf("Can't open file %s\n", argv[1]);
		return ECODE::OPEN_ERROR;
	}
	if (!isPngType(png_file, true)) {
		printf("Is not the correct PNG file\n");
		return ECODE::TYPE_ERROR;
	}

	struct stat st;
	stat(argv[1], &st);
	long imgSize = st.st_size;

	DataBuf cheaderBuf(8); // Chunk header size : 4 bytes (data size) + 4 bytes (chunk type).

	while (!feof(png_file)) {
		// Read chunk header.

#ifdef DEBUG
		std::cout << "readMetadata: Position: " << ftell(png_file) << "\n";
#endif
		memset(cheaderBuf.pData, 0x0, cheaderBuf.size);
		long bufRead = fread(cheaderBuf.pData, sizeof(byte), cheaderBuf.size,
				png_file);
		if (bufRead != cheaderBuf.size) {
			printf("Chunk header read error\n");
			return ECODE::CHUNK_ERROR;
		}

		// Decode chunk data length.
		dword dataOffset = getLong(cheaderBuf.pData);
		long pos = ftell(png_file);
		if (pos == -1 || dataOffset > uint32_t(0x7FFFFFFF)
				|| static_cast<long>(dataOffset) > imgSize - pos) {
			printf("Wrong chunk format\n");
			return ECODE::CHUNK_ERROR;
		}

		// Perform a chunk triage for item that we need.

		if (	!memcmp(cheaderBuf.pData + 4, "IEND", 4)
			||	!memcmp(cheaderBuf.pData + 4, "IHDR", 4)
			||	!memcmp(cheaderBuf.pData + 4, "tEXt", 4)
			||	!memcmp(cheaderBuf.pData + 4, "zTXt", 4)
			||	!memcmp(cheaderBuf.pData + 4, "iTXt", 4)
			||	!memcmp(cheaderBuf.pData + 4, "iCCP", 4)) {
			// Extract chunk data.

			DataBuf cdataBuf(dataOffset);
			bufRead = fread(cdataBuf.pData, sizeof(byte), dataOffset, png_file);
			if (bufRead != (long) dataOffset) {
				printf("Wrong chunks data\n");
				return ECODE::CHUNK_ERROR;
			}

			if (!memcmp(cheaderBuf.pData + 4, "IEND", 4)) {
				// Last chunk found: we stop parsing.
#ifdef DEBUG
				std::cout << "readMetadata: Found IEND chunk (length: " << dataOffset << ")\n";
#endif
				break;
			} else if (!memcmp(cheaderBuf.pData + 4, "IHDR", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found IHDR chunk (length: " << dataOffset << ")\n";
#endif
				decodeIHDRChunk(cdataBuf, &(img.pixelWidth), &(img.pixelHeight));
			} else if (!memcmp(cheaderBuf.pData + 4, "tEXt", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found tEXt chunk (length: " << dataOffset << ")\n";
#endif
				decodeTXTChunk(cdataBuf, TxtChunkType::tEXt_Chunk);
			} else if (!memcmp(cheaderBuf.pData + 4, "zTXt", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found zTXt chunk (length: " << dataOffset << ")\n";
#endif
				decodeTXTChunk(cdataBuf, TxtChunkType::zTXt_Chunk);
			} else if (!memcmp(cheaderBuf.pData + 4, "iTXt", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found iTXt chunk (length: " << dataOffset << ")\n";
#endif
				decodeTXTChunk(cdataBuf, TxtChunkType::iTXt_Chunk);
			} else if (!memcmp(cheaderBuf.pData + 4, "iCCP", 4)) {
			    DataBuf iccProfile_;
				zlibToDataBuf(cdataBuf.pData + 12 + 1, dataOffset - 13, iccProfile_); // +1 = 'compressed' flag
#ifdef DEBUG
						std::cout << "Exiv2::PngImage::readMetadata: Found iCCP chunk length: " << dataOffset << std::endl;
						std::cout << "Exiv2::PngImage::readMetadata: iccProfile.size_ : " << iccProfile_.size << std::endl;
#endif
			}

			// Set dataOffset to null like chunk data have been extracted previously.
			dataOffset = 0;
		}

		// Move to the next chunk: chunk data size + 4 CRC bytes.
#ifdef DEBUG
		std::cout << "Exiv2::PngImage::readMetadata: Seek to offset: " << dataOffset + 4 << "\n";
#endif
		fseek(png_file, dataOffset + 4, SEEK_CUR);
		if (ferror(png_file) || feof(png_file)) {
			printf("Wrong chunk format\n");
			return ECODE::CHUNK_ERROR;
		}
	}
}

