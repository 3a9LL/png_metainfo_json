/*
 * DataBuf.h
 *
 *  Created on: Oct 8, 2017
 *      Author: root
 */

#ifndef DATABUF_H_
#define DATABUF_H_

#include "common.hpp"

struct DataBuf {
public:
    DataBuf() : pData(0), size(0) {}
    explicit DataBuf(long sz) : pData(new byte[sz]), size(sz) {}
	DataBuf(const byte* pData, long size);
    ~DataBuf() { delete[] pData; }

	void alloc(long size);
	void release();

	byte* 	pData;
	size_t	size;
};

#endif /* DATABUF_H_ */
