/*
 * DataBuf.h
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#ifndef DATABUF_H_
#define DATABUF_H_

#include "common.hpp"

class DataBuf {
public:
	DataBuf() :
			pData(0), size(0) {
	}

	DataBuf(long sz) :
			pData(new byte[sz]), size(sz) {
	}

	DataBuf(const byte* pData, long size);

	DataBuf(DataBuf& rhs);

	~DataBuf() {
		delete[] pData;
	}

	DataBuf& operator=(DataBuf& rhs);

	void alloc(long size);

	std::pair<byte*, long> release();

	void reset(std::pair<byte*, long> = std::make_pair((byte*) (0), long(0)));

	byte* pData;
	long size;
};

#endif /* DATABUF_H_ */
