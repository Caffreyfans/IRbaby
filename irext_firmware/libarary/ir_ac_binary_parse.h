/**************************************************************************************
Filename:       ir_ac_binary_parse.h
Revised:        Date: 2017-01-03
Revision:       Revision: 1.0

Description:    This file provides methods for AC binary parse

Revision log:
* 2017-01-03: created by strawmanbobi
**************************************************************************************/

#ifndef IRDA_DECODER_IR_AC_BINARY_PARSE_H
#define IRDA_DECODER_IR_AC_BINARY_PARSE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ir_defs.h"

extern INT8 binary_parse_offset();

extern INT8 binary_parse_len();

extern void binary_tags_info();

extern INT8 binary_parse_data();

#ifdef __cplusplus
}
#endif


#endif //IRDA_DECODER_IR_AC_BINARY_PARSE_H
