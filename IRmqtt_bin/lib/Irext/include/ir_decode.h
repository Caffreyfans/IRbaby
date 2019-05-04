/**************************************************************************************
Filename:       ir_decode.h
Revised:        Date: 2016-10-01
Revision:       Revision: 1.0

Description:    This file provides algorithms for IR decode

Revision log:
* 2016-10-01: created by strawmanbobi
**************************************************************************************/

#ifndef _IRDA_DECODE_H_
#define _IRDA_DECODE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include "ir_defs.h"
#include "ir_ac_control.h"
#include "ir_tv_control.h"

#define IR_DECODE_FAILED             (-1)
#define IR_DECODE_SUCCEEDED          (0)

#define IR_CATEGORY_AC               1
#define IR_CATEGORY_TV               2

#define IR_TYPE_STATUS               0
#define IR_TYPE_COMMANDS             1

#define SUB_CATEGORY_QUATERNARY      0
#define SUB_CATEGORY_HEXADECIMAL     1

// exported functions
/**
 * function     ir_file_open
 *
 * description: open IR binary code from file
 *
 * parameters:  category (in) - category ID get from indexing API
 *              sub_category (in) - subcategory ID get from indexing API
 *              file_name (in) - file name of IR binary
 *
 * returns:     IR_DECODE_SUCCEEDED / IR_DECODE_FAILED
 *
 */
extern INT8 ir_file_open(const UINT8 category, const UINT8 sub_category, const char* file_name);

/**
 * function     ir_binary_open
 *
 * description: open IR binary code from buffer
 *
 * parameters:  category (in) - category ID get from indexing API
 *              sub_category (in) - subcategory ID get from indexing API
 *              binary (in) - pointer to binary buffer
 *              binary_length (in) - binary buffer size
 *
 * returns:     IR_DECODE_SUCCEEDED / IR_DECODE_FAILED
 */
extern INT8 ir_binary_open(const UINT8 category, const UINT8 sub_category, UINT8* binary, UINT16 binary_length);

/**
 * function     ir_decode
 *
 * description: decode IR binary into INT16 array which indicates the IR levels
 *
 * parameters:  key_code (in) - the code of pressed key
 *              user_data (out) - output decoded data in INT16 array format
 *              ac_status(in) - pointer to AC status (optional)
 *              change_wind_direction (in) - if control changes wind direction for AC (for AC only)
 *
 * returns:     length of decoded data (0 indicates decode failure)
 */
extern UINT16 ir_decode(UINT8 key_code, UINT16* user_data, t_remote_ac_status* ac_status, BOOL change_wind_direction);

/**
 * function     ir_close
 *
 * description: close IR binary code
 *
 * parameters:  N/A
 *
 * returns:     IR_DECODE_SUCCEEDED / IR_DECODE_FAILED
 */
extern INT8 ir_close();

/**
 * function     get_temperature_range
 *
 * description: get the supported temperature range [min, max] for the opened AC IR binary
 *
 * parameters:  ac_mode (in) specify in which AC mode the application need to get temperature info
 *              temp_min (out) the min temperature supported in a specified AC mode
 *              temp_max (out) the max temperature supported in a specified AC mode
 *
 * returns:     IR_DECODE_SUCCEEDED / IR_DECODE_FAILED
 */
extern INT8 get_temperature_range(UINT8 ac_mode, INT8 *temp_min, INT8 *temp_max);

/**
 * function     get_supported_mode
 *
 * description: get supported mode for the opened AC IR binary
 *
 * parameters:  supported_mode (out) mode supported by the remote in lower 5 bits
 *
 * returns:     IR_DECODE_SUCCEEDED / IR_DECODE_FAILED
 */
extern INT8 get_supported_mode(UINT8 *supported_mode);

/**
 * function     get_supported_wind_speed
 *
 * description: get supported wind speed levels for the opened AC IR binary in certain mode
 *
 * parameters:  ac_mode (in) specify in which AC mode the application need to get wind speed info
 *              supported_wind_speed (out) wind speed supported by the remote in lower 4 bits
 *
 * returns:     IR_DECODE_SUCCEEDED / IR_DECODE_FAILED
 */
extern INT8 get_supported_wind_speed(UINT8 ac_mode, UINT8 *supported_wind_speed);

/**
 * function     get_supported_swing
 *
 * description: get supported swing functions for the opened AC IR binary in certain mode
 *
 * parameters:  ac_mode (in) specify in which AC mode the application need to get swing info
 *              supported_swing (out) swing supported by the remote in lower 2 bits
 *
 * returns:     IR_DECODE_SUCCEEDED / IR_DECODE_FAILED
 */
extern INT8 get_supported_swing(UINT8 ac_mode, UINT8 *supported_swing);

/**
 * function     get_supported_wind_direction
 *
 * description: get supported wind directions for the opened AC IR binary in certain mode
 *
 * parameters:  supported_wind_direction (out) swing supported by the remote in lower 2 bits
 *
 * returns:     IR_DECODE_SUCCEEDED / IR_DECODE_FAILED
 */
extern INT8 get_supported_wind_direction(UINT8 *supported_wind_direction);


// private extern function
#if (defined BOARD_PC || defined BOARD_PC_DLL)
extern void ir_lib_free_inner_buffer();
#endif

// this function is preferred being called by JNI only
UINT16 ir_decode_combo(const UINT8 category, const UINT8 sub_category,
                       UINT8* binary, UINT16 binary_length,
                       UINT8 key_code, UINT16* user_data,
                       t_remote_ac_status* ac_status, BOOL change_wind_direction);

#ifdef __cplusplus
}
#endif

#endif // _IRDA_DECODE_H_
