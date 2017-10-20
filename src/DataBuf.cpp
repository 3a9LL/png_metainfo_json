/*
 * DataBuf.cpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#include "DataBuf.h"

DataBuf::DataBuf(DataBuf& rhs) :
		pData(rhs.pData), size(rhs.size) {
	rhs.release();
}

DataBuf::DataBuf(const byte* pD, long sz) :
		pData(0), size(0) {
	if (size > 0) {
		pData = new byte[sz];
		memcpy(pData, pD, sz);
		size = sz;
	}
}

DataBuf& DataBuf::operator=(DataBuf& rhs) {
	if (this == &rhs)
		return *this;
	reset(rhs.release());
	return *this;
}

void DataBuf::alloc(long sz) {
	if (sz > size) {
		delete[] pData;
		pData = 0;
		size = 0;
		pData = new byte[sz];
		size = sz;
	}
}

std::pair<byte*, long> DataBuf::release() {
	std::pair<byte*, long> p = std::make_pair(pData, size);
	pData = 0;
	size = 0;
	return p;
}

void DataBuf::reset(std::pair<byte*, long> p) {
	if (pData != p.first) {
		delete[] pData;
		pData = p.first;
	}
	size = p.second;
}
