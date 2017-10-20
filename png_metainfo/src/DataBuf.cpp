/*
 * DataBuf.cpp
 *
 *  Created on: Oct 8, 2017
 *      Author: root
 */

#include "DataBuf.h"

void DataBuf::alloc(long sz)
    {
        if (sz > size) {
            delete[] pData;
            pData = 0;
            size = 0;
            pData = new byte[sz];
            size = sz;
        }
    }

DataBuf::DataBuf(const byte* d, long sz)
        : pData(0), size(0)
    {
        if (sz > 0) {
            pData = new byte[sz];
            memcpy(pData, d, sz);
            size = sz;
        }
    }

void DataBuf::release()
    {
        pData = 0;
        size = 0;
    }
