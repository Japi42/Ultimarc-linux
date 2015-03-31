/*
 ============================================================================
 Name        : ipacseries.h
 Author      : Katie Snow
 Version     :
 Copyright   : Copyright 2014 Robert Abram, Katie Snow
 Description : Ultimarc 2015 PAC board configuration library
 ============================================================================
*/

#ifndef PACSERIES_H_
#define PACSERIES_H_

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define PACSERIES_PRODUCT_STR ""
#define PACSERIES_VERSION 2

typedef struct json_object json_object;

/*
 * Determine if the JSON configuration file is one of the PAC Series (2015 PAC boards)
 */
bool validateIPACSeriesData(json_object* jobj);

/**
 * 2015 board and the pre-2015 Ultimate I/O board.  Key value to hex value.
 */
unsigned char convertIPACSeries (json_object* jobj);

/**
 * Update one of the PAC Series boards.  These boards are the 2015 boards from Ultimarc.
 */
bool updatePACSeriesBoard(json_object* jobj);

#ifdef __cplusplus
}
#endif

#endif /* PACSERIES_H_ */
