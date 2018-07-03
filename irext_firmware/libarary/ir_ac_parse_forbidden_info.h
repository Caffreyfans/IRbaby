/**************************************************************************************
Filename:       ir_parse_forbidden_info.h
Revised:        Date: 2016-10-05
Revision:       Revision: 1.0

Description:    This file provides algorithms for forbidden area of AC code

Revision log:
* 2016-10-05: created by strawmanbobi
**************************************************************************************/

#ifndef _IRDA_PARSE_PARSE_H_
#define _IRDA_PARSE_PARSE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ir_decode.h"

extern INT8 parse_nmode(struct tag_head *tag, t_ac_n_mode index);

#ifdef __cplusplus
}
#endif

#endif // _IRDA_PARSE_PARSE_H_

