/**************************************************************************************
Filename:       ir_utils.c
Revised:        Date: 2016-10-26
Revision:       Revision: 1.0

Description:    This file provides generic utils for IRDA algorithms

Revision log:
* 2016-10-01: created by strawmanbobi
**************************************************************************************/

#ifndef _IRDA_UTILS_H_
#define _IRDA_UTILS_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ir_defs.h"
#include "ir_decode.h"

#include <stdio.h>

extern void string_to_hex(UINT8 *p, t_ac_hex *pac_hex);

extern void string_to_hex_common(UINT8 *p, UINT8 *hex_data, UINT16 len);

extern BOOL is_in(const UINT8 *array, UINT8 value, UINT8 len);

extern void hex_byte_to_double_char(char *dest, UINT8 length, UINT8 src);

#ifdef __cplusplus
}
#endif
#endif // _IRDA_UTILS_H_