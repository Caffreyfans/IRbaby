/**************************************************************************************
Filename:       ir_lib.h
Revised:        Date: 2016-02-23
Revision:       Revision: 1.0

Description:    This file provides algorithms for IR decode (compressed command type)

Revision log:
* 2016-10-21: created by strawmanbobi
**************************************************************************************/

#ifndef _IRDA_LIB_H_
#define _IRDA_LIB_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ir_defs.h"

#define STB_CHANNEL_OFFSET              14

#define IRDA_FLAG_NORMAL                0
#define IRDA_FLAG_INVERSE               1

#define IRDA_LEVEL_LOW                  0
#define IRDA_LEVEL_HIGH                 1

#define IRDA_LSB                        0
#define IRDA_MSB                        1

enum
{
    IRDA_DECODE_1_BIT = 0,
    IRDA_DECODE_2_BITS,
    IRDA_DECODE_4_BITS,
};

/*
 * global type definitions
 */
typedef enum ir_flags
{
    IRDA_BOOT = 0,
    IRDA_STOP,
    IRDA_SEP,
    IRDA_ONE,
    IRDA_ZERO,
    IRDA_FLIP,
    IRDA_TWO,
    IRDA_THREE = 7,
    IRDA_FOUR,
    IRDA_FIVE,
    IRDA_SIX,
    IRDA_SEVEN,
    IRDA_EIGHT,
    IRDA_NINE,
    IRDA_A,
    IRDA_B,
    IRDA_C,
    IRDA_D,
    IRDA_E,
    IRDA_F,
    IRDA_MAX = 20,
} t_ir_flags;

typedef struct ir_data
{
    UINT8 bits;
    UINT8 lsb;
    UINT8 mode;
    UINT8 index;
} t_ir_data;

#if !defined BOARD_51 && !defined BOARD_STM8
#pragma pack(1)
#endif
typedef struct ir_cycles
{
    UINT8 flag;
    UINT16 mask;
    UINT16 space;
} t_ir_cycles;

#if !defined BOARD_51 && !defined BOARD_STM8
#pragma pack()
#endif

typedef enum tv_key_value
{
    TV_POWER = 0,
    TV_MUTE,
    TV_UP,
    TV_DOWN,
    TV_LEFT,
    TV_RIGHT,
    TV_OK,
    TV_VOL_UP,
    TV_VOL_DOWN,
    TV_BACK,
    TV_INPUT,
    TV_MENU,
    TV_HOME,
    TV_SET,
    TV_0,
    TV_1,
    TV_2,
    TV_3,
    TV_4,
    TV_5,
    TV_6,
    TV_7,
    TV_8,
    TV_9,
    TV_KEY_MAX,
} t_tv_key_value;


typedef enum stb_key_value
{
    STB_POWER = 0,
    STB_MUTE,
    STB_UP,
    STB_DOWN,
    STB_LEFT,
    STB_RIGHT,
    STB_OK,
    STB_VOL_UP,
    STB_VOL_DOWN,
    STB_BACK,
    STB_INPUT,
    STB_MENU,
    STB_PAGE_UP,
    STB_PAGE_DOWN,
    STB_0,
    STB_1,
    STB_2,
    STB_3,
    STB_4,
    STB_5,
    STB_6,
    STB_7,
    STB_8,
    STB_9,
    STB_KEY_MAX,
} t_stb_key_value;

typedef enum nw_key_value
{
    NW_POWER = 0,
    NW_UP,
    NW_DOWN,
    NW_LEFT,
    NW_RIGHT,
    NW_OK,
    NW_VOL_UP,
    NW_VOL_DOWN,
    NW_BACK,
    NW_MENU,
    NW_HOME,
    NW_0,
    NW_1,
    NW_2,
    NW_3,
    NW_4,
    NW_5,
    NW_6,
    NW_7,
    NW_8,
    NW_9,
    NW_KEY_MAX,
} t_nw_key_value;

typedef enum cm_key_value
{
    CM_POWER = 0,
    CM_UP,
    CM_DOWN,
    CM_LEFT,
    CM_RIGHT,
    CM_OK,
    CM_VOL_UP,
    CM_VOL_DOWN,
    CM_FUNC_1,
    CM_FUNC_2,
    CM_FUNC_3,
    CM_BACK,
    CM_HOME,
    CM_MENU,
    CM_MODE,
    CM_KEY_MAX,
} t_cm_key_value;

typedef struct ir_data_tv
{
    char magic[4];
    UINT8 per_keycode_bytes;
} t_ir_data_tv;


extern INT8 tv_binary_open(UINT8 *binary, UINT16 binary_length);

extern BOOL tv_binary_parse(UINT8 encode_type);

extern UINT16 tv_binary_decode(UINT8 key, UINT16 *user_data);

extern UINT8 tv_lib_close();

#ifdef __cplusplus
}
#endif

#endif /* _IRDA_LIB_H_ */

