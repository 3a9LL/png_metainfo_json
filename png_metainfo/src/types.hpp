/*
 * image.hpp
 *
 *  Created on: Oct 7, 2017
 *      Author: root
 */

#ifndef TYPES_HPP_
#define TYPES_HPP_

#include "common.hpp"

struct date_t
{
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
};

struct image_t
{
	pairs_t tEXt;
	pairs_t zTXt;
	pairs_t iTXt;

	date_t 	date;
	int 	width;
	int		height;
};

#endif /* TYPES_HPP_ */
