/**************************************************************************************
Filename:       ir_ac_apply.h
Revised:        Date: 2016-10-12
Revision:       Revision: 1.0

Description:    This file provides methods for AC IR applying functionalities

Revision log:
* 2016-10-12: created by strawmanbobi
**************************************************************************************/

#ifndef _IRDA_APPLY_H_
#define _IRDA_APPLY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ir_decode.h"

#define MIN_TAG_LENGTH_TYPE_1   4
#define MIN_TAG_LENGTH_TYPE_2   6

INT8 apply_power(t_remote_ac_status ac_status, UINT8 function_code);

INT8 apply_mode(t_remote_ac_status ac_status, UINT8 function_code);

INT8 apply_wind_speed(t_remote_ac_status ac_status, UINT8 function_code);

INT8 apply_swing(t_remote_ac_status ac_status, UINT8 function_code);

INT8 apply_temperature(t_remote_ac_status ac_status, UINT8 function_code);

INT8 apply_function(struct ac_protocol *protocol, UINT8 function);

INT8 apply_checksum(struct ac_protocol *protocol);

#ifdef __cplusplus
}
#endif

#endif //_IRDA_APPLY_H_