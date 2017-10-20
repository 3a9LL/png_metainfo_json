/*
 * main.cpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#include <sys/stat.h>
#include <json-c/json.h>
#include <png.h>

#include "common.hpp"
#include "utils.hpp"
#include "chunk.hpp"
#include "types.hpp"

// Signature from front of PNG file
const unsigned char pngSignature[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A,
		0x1A, 0x0A };

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

	printf("main() at 0x%x\n", main);

	FILE* png_file;
	struct image_t img;
	bool isIHDR, isTIME, istEXt, iszTXt, isiTXt;
	isIHDR = isTIME = istEXt = iszTXt = isiTXt = false;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <png file>\n", argv[0]);
		return ECODE::ARG_ERROR;
	}
	png_file = fopen(argv[1], "rb");
	if (!png_file) {
		fprintf(stderr, "Can't open file %s\n", argv[1]);
		return ECODE::OPEN_ERROR;
	}
	if (!isPngType(png_file, true)) {
		fprintf(stderr, "Is not the correct PNG file\n");
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
		if (bufRead != cheaderBuf.size)
			chunk_error();

		// Decode chunk data length.
		dword dataOffset = getULong(cheaderBuf.pData);
		long pos = ftell(png_file);
		if (pos == -1 || dataOffset > uint32_t(0x7FFFFFFF)
				|| static_cast<long>(dataOffset) > imgSize - pos)
			chunk_error();

		// Perform a chunk triage for item that we need.

		if (!memcmp(cheaderBuf.pData + 4, "IEND", 4)
				|| !memcmp(cheaderBuf.pData + 4, "IHDR", 4)
				|| !memcmp(cheaderBuf.pData + 4, "tEXt", 4)
				|| !memcmp(cheaderBuf.pData + 4, "zTXt", 4)
				|| !memcmp(cheaderBuf.pData + 4, "iTXt", 4)
				|| !memcmp(cheaderBuf.pData + 4, "tIME", 4)) {
			// Extract chunk data.

			DataBuf cdataBuf(dataOffset);
			bufRead = fread(cdataBuf.pData, sizeof(byte), dataOffset, png_file);
			if (bufRead != (long) dataOffset)
				chunk_error();

			if (!memcmp(cheaderBuf.pData + 4, "IEND", 4)) {
				// Last chunk found: we stop parsing.
#ifdef DEBUG
				std::cout << "readMetadata: Found IEND chunk (length: "
				<< dataOffset << ")\n";
#endif
				break;
			} else if (!memcmp(cheaderBuf.pData + 4, "IHDR", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found IHDR chunk (length: "
				<< dataOffset << ")\n";
#endif
				decodeIHDRChunk(cdataBuf, &(img.width), &(img.height));
				isIHDR = true;
			} else if (!memcmp(cheaderBuf.pData + 4, "tIME", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found tIME chunk (length: "
				<< dataOffset << ")\n";
#endif
				decodeTIMEChunk(cdataBuf, &(img.date));
				isTIME = true;
			} else if (!memcmp(cheaderBuf.pData + 4, "tEXt", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found tEXt chunk (length: "
				<< dataOffset << ")\n";
#endif
				img.tEXt.push_back(
						decodeTXTChunk(cdataBuf, TxtChunkType::tEXt_Chunk));
				istEXt = true;
			} else if (!memcmp(cheaderBuf.pData + 4, "zTXt", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found zTXt chunk (length: "
				<< dataOffset << ")\n";
#endif
				img.zTXt.push_back(
						decodeTXTChunk(cdataBuf, TxtChunkType::zTXt_Chunk));
				iszTXt = true;
			} else if (!memcmp(cheaderBuf.pData + 4, "iTXt", 4)) {
#ifdef DEBUG
				std::cout << "readMetadata: Found iTXt chunk (length: "
				<< dataOffset << ")\n";
#endif
				img.iTXt.push_back(
						decodeTXTChunk(cdataBuf, TxtChunkType::iTXt_Chunk));
				isiTXt = true;
			}

			// Set dataOffset to null like chunk data have been extracted previously.
			dataOffset = 0;
		}
		// Move to the next chunk: chunk data size + 4 CRC bytes.
#ifdef DEBUG
		std::cout << "readMetadata: Seek to offset: " << dataOffset + 4 << "\n";
#endif
		fseek(png_file, dataOffset + 4, SEEK_CUR);
		if (ferror(png_file) || feof(png_file))
			chunk_error();
	}
// ------------------------- TO JSON ------------------
	/* Main JSON object */
	json_object * jobj = json_object_new_object();

	/* Add width and height */
	if (isIHDR) {
		json_object *jWidth = json_object_new_int(img.width);
		json_object *jHeight = json_object_new_int(img.height);
		json_object_object_add(jobj, "width",	jWidth);
		json_object_object_add(jobj, "height", jHeight);
	}
	/* Modification time */
	if (isTIME) {
		std::string s = dateToString(img.date);
		json_object *jModTime = json_object_new_string(s.c_str());
		json_object_object_add(jobj, "mod_time", jModTime);
	}
	/* Add tEXt */
	if (istEXt) {
		json_object * jtEXt = json_object_new_object();
		for (pair_t p : img.tEXt) {
			json_object *jtEXt_e = json_object_new_string(p.second.c_str());
			json_object_object_add(jtEXt, p.first.c_str(), jtEXt_e);
		}
		json_object_object_add(jobj, "tEXt", jtEXt);
	}
	/* Add zTXt */
	if (iszTXt) {
		json_object * jzTXt = json_object_new_object();
		for (pair_t p : img.zTXt) {
			json_object *jzTXt_e = json_object_new_string(p.second.c_str());
			json_object_object_add(jzTXt, p.first.c_str(), jzTXt_e);
		}
		json_object_object_add(jobj, "zTXt", jzTXt);
	}
	/* Add iTXt */
	if (isiTXt) {
		json_object * jiTXt = json_object_new_object();
		for (pair_t p : img.iTXt) {
			json_object *jiTXt_e = json_object_new_string(p.second.c_str());
			json_object_object_add(jiTXt, p.first.c_str(), jiTXt_e);
		}
		json_object_object_add(jobj, "iTXt", jiTXt);
	}

	int x, y;

	int width, height;
	png_byte color_type;
	png_byte bit_depth;

	png_structp png_ptr;
	png_infop info_ptr;
	int number_of_passes;
	png_bytep row_pointers[5000];
	png_byte png_idat[5000][4*5000];

	byte header[8];    // 8 is the maximum size that can be checked

	/* open file and test for it being a png */
	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "Can't open file %s\n", argv[1]);
		return ECODE::OPEN_ERROR;
	}
	fread(header, 1, 8, fp);
	if (png_sig_cmp(header, 0, 8))
		chunk_error();

	/* initialize stuff */
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		chunk_error();

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		chunk_error();

	if (setjmp(png_jmpbuf(png_ptr)))
		chunk_error();

	png_init_io(png_ptr, fp);
	png_set_sig_bytes(png_ptr, 8);

	png_read_info(png_ptr, info_ptr);

	width		= img.width;
	height		= img.height;
	color_type	= png_get_color_type(png_ptr, info_ptr);
	bit_depth	= png_get_bit_depth(png_ptr, info_ptr);

	number_of_passes = png_set_interlace_handling(png_ptr);
	png_read_update_info(png_ptr, info_ptr);

	/* read file */
	if (setjmp(png_jmpbuf(png_ptr)))
		chunk_error();

	for (y = 0; y < height; y++)
	{
		row_pointers[y] = png_idat[y];
	}

	png_colorp plte;
	int num_palette;
	bool isPLTE = false;
	isPLTE = (bool) png_get_PLTE(png_ptr, info_ptr, &plte, &num_palette);
	png_read_image(png_ptr, row_pointers);

	fclose(fp);

	/* create file */
	fp = fopen(argv[1], "wb");
	if (!fp) {
		fprintf(stderr, "Can't open file %s\n", argv[1]);
		return ECODE::OPEN_ERROR;
	}

	/* initialize stuff */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		chunk_error();

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		chunk_error();

	if (setjmp(png_jmpbuf(png_ptr)))
		chunk_error();

	png_init_io(png_ptr, fp);

	/* write header */
	if (setjmp(png_jmpbuf(png_ptr)))
		chunk_error();

	png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
			PNG_INTERLACE_NONE,
			PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

	if(isPLTE)
		png_set_PLTE(png_ptr, info_ptr, plte, num_palette);
	png_write_info(png_ptr, info_ptr);

	/* write bytes */
	if (setjmp(png_jmpbuf(png_ptr)))
		chunk_error();

	png_write_image(png_ptr, row_pointers);

	/* end write */
	if (setjmp(png_jmpbuf(png_ptr)))
		chunk_error();

	png_write_end(png_ptr, NULL);


	fclose(fp);

	printf("%s\n", json_object_to_json_string(jobj));
	return 0;
}

