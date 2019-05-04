/**************************************************************************************
Filename:       ir_ac_control.h
Revised:        Date: 2016-12-31
Revision:       Revision: 1.0

Description:    This file provides methods for AC IR control

Revision log:
* 2016-10-12: created by strawmanbobi
**************************************************************************************/
#ifndef IRDA_DECODER_IR_AC_CONTROL_H
#define IRDA_DECODER_IR_AC_CONTROL_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "ir_defs.h"


#define TAG_COUNT_FOR_PROTOCOL 29

#define TAG_INVALID 0xffff

#define MAX_DELAYCODE_NUM 16
#define MAX_BITNUM 16

#define AC_PARAMETER_TYPE_1 0
#define AC_PARAMETER_TYPE_2 1

typedef enum
{
    AC_POWER_ON = 0,
    AC_POWER_OFF,
    AC_POWER_MAX
} t_ac_power;

typedef enum
{
    AC_TEMP_16 = 0,
    AC_TEMP_17,
    AC_TEMP_18,
    AC_TEMP_19,
    AC_TEMP_20,
    AC_TEMP_21,
    AC_TEMP_22,
    AC_TEMP_23,
    AC_TEMP_24,
    AC_TEMP_25,
    AC_TEMP_26,
    AC_TEMP_27,
    AC_TEMP_28,
    AC_TEMP_29,
    AC_TEMP_30,
    AC_TEMP_MAX
} t_ac_temperature;

typedef enum
{
    AC_MODE_COOL = 0,
    AC_MODE_HEAT,
    AC_MODE_AUTO,
    AC_MODE_FAN,
    AC_MODE_DRY,
    AC_MODE_MAX
} t_ac_mode;

typedef enum
{
    AC_FUNCTION_POWER = 1,
    AC_FUNCTION_MODE,
    AC_FUNCTION_TEMPERATURE_UP,
    AC_FUNCTION_TEMPERATURE_DOWN,
    AC_FUNCTION_WIND_SPEED,
    AC_FUNCTION_WIND_SWING,
    AC_FUNCTION_WIND_FIX,
    AC_FUNCTION_MAX,
} t_ac_function;

typedef enum
{
    AC_WS_AUTO = 0,
    AC_WS_LOW,
    AC_WS_MEDIUM,
    AC_WS_HIGH,
    AC_WS_MAX
} t_ac_wind_speed;

typedef enum
{
    AC_SWING_ON = 0,
    AC_SWING_OFF,
    AC_SWING_MAX
} t_ac_swing;

typedef enum
{
    SWING_TYPE_SWING_ONLY = 0,
    SWING_TYPE_NORMAL,
    SWING_TYPE_NOT_SPECIFIED,
    SWING_TYPE_MAX
} swing_type;

typedef enum
{
    TEMP_TYPE_DYNAMIC = 0,
    TEMP_TYPE_STATIC,
    TEMP_TYPE_MAX,
} t_temp_type;

// enumeration for application polymorphism
typedef enum
{
    AC_APPLY_POWER = 0,
    AC_APPLY_MODE,
    AC_APPLY_TEMPERATURE_UP,
    AC_APPLY_TEMPERATURE_DOWN,
    AC_APPLY_WIND_SPEED,
    AC_APPLY_WIND_SWING,
    AC_APPLY_WIND_FIX,
    AC_APPLY_MAX
} t_ac_apply;

typedef struct _ac_hex
{
    UINT8 len;
    UINT8 *data;
} t_ac_hex;

typedef struct _ac_level
{
    UINT16 low;
    UINT16 high;
} t_ac_level;

typedef struct _ac_bootcode
{
    UINT16 len;
    UINT16 data[16];
} t_ac_bootcode;

typedef struct _ac_delaycode
{
    INT16 pos;
    UINT16 time[8];
    UINT16 time_cnt;
} t_ac_delaycode;

/*
 * the array of tag_100X application data
 * seg_len : length for each segment
 * byte_pos : the position of update byte
 * byte_value : the value to be updated to position
 */
typedef struct _tag_comp_type_1
{
    UINT8 seg_len;
    UINT8 *segment;
} t_tag_comp;

typedef struct _tag_swing_info
{
    swing_type type;
    UINT8 mode_count;
    UINT8 dir_index;
} t_swing_info;

typedef struct _tag_power_1
{
    UINT8 len;
    t_tag_comp comp_data[AC_POWER_MAX];
} t_power_1;

typedef struct _tag_temp_1
{
    UINT8 len;
    UINT8 type;
    t_tag_comp comp_data[AC_TEMP_MAX];
} t_temp_1;

typedef struct tag_mode_1
{
    UINT8 len;
    t_tag_comp comp_data[AC_MODE_MAX];
} t_mode_1;

typedef struct tag_speed_1
{
    UINT8 len;
    t_tag_comp comp_data[AC_WS_MAX];
} t_speed_1;

typedef struct tag_swing_1
{
    UINT8 len;
    UINT16 count;
    t_tag_comp *comp_data;
} t_swing_1;

typedef struct tag_temp_2
{
    UINT8 len;
    UINT8 type;
    t_tag_comp comp_data[AC_TEMP_MAX];
} t_temp_2;

typedef struct tag_mode_2
{
    UINT8 len;
    t_tag_comp comp_data[AC_MODE_MAX];
} t_mode_2;

typedef struct tag_speed_2
{
    UINT8 len;
    t_tag_comp comp_data[AC_WS_MAX];
} t_speed_2;

typedef struct tag_swing_2
{
    UINT8 len;
    UINT16 count;
    t_tag_comp *comp_data;
} t_swing_2;

#if defined SUPPORT_HORIZONTAL_SWING
typedef struct tag_horiswing_1
{
    UINT16 len;
    t_tag_comp comp_data[AC_HORI_SWING_MAX];
} hori_swing_1;
#endif

typedef struct _tag_checksum_data
{
    UINT8 len;
    UINT8 type;
    UINT8 start_byte_pos;
    UINT8 end_byte_pos;
    UINT8 checksum_byte_pos;
    UINT8 checksum_plus;
    UINT8 *spec_pos;
} t_tag_checksum_data;

typedef struct tag_checksum
{
    UINT8 len;
    UINT16 count;
    t_tag_checksum_data *checksum_data;
} t_checksum;

typedef struct tag_function_1
{
    UINT8 len;
    t_tag_comp comp_data[AC_FUNCTION_MAX - 1];
} t_function_1;

typedef struct tag_function_2
{
    UINT8 len;
    t_tag_comp comp_data[AC_FUNCTION_MAX - 1];
} t_function_2;

typedef struct tag_solo_code
{
    UINT8 len;
    UINT8 solo_func_count;
    UINT8 solo_function_codes[AC_FUNCTION_MAX - 1];
} t_solo_code;

typedef struct _ac_bitnum
{
    INT16 pos;
    UINT16 bits;
} t_ac_bit_num;

typedef enum
{
    N_COOL = 0,
    N_HEAT,
    N_AUTO,
    N_FAN,
    N_DRY,
    N_MODE_MAX,
} t_ac_n_mode;

typedef enum
{
    CHECKSUM_TYPE_BYTE = 1,
    CHECKSUM_TYPE_BYTE_INVERSE,
    CHECKSUM_TYPE_HALF_BYTE,
    CHECKSUM_TYPE_HALF_BYTE_INVERSE,
    CHECKSUM_TYPE_SPEC_HALF_BYTE,
    CHECKSUM_TYPE_SPEC_HALF_BYTE_INVERSE,
    CHECKSUM_TYPE_SPEC_HALF_BYTE_ONE_BYTE,
    CHECKSUM_TYPE_SPEC_HALF_BYTE_INVERSE_ONE_BYTE,
    CHECKSUM_TYPE_MAX,
} t_checksum_type;

typedef struct _ac_n_mode_info
{
    UINT8 enable;
    UINT8 all_speed;
    UINT8 all_temp;
    UINT8 temp[AC_TEMP_MAX];
    UINT8 temp_cnt;
    UINT8 speed[AC_WS_MAX];
    UINT8 speed_cnt;
} t_ac_n_mode_info;

typedef struct ac_protocol
{
    UINT8 endian;
    // t_ac_hex default_code;
    t_ac_hex default_code;
    t_ac_level zero;
    t_ac_level one;
    t_ac_bootcode boot_code;
    t_ac_delaycode dc[MAX_DELAYCODE_NUM];
    t_power_1 power1;
    t_temp_1 temp1;
    t_mode_1 mode1;
    t_speed_1 speed1;
    t_swing_1 swing1;
    t_checksum checksum;

    t_function_1 function1;
    t_function_2 function2;

    t_temp_2 temp2;
    t_mode_2 mode2;
    t_speed_2 speed2;
    t_swing_2 swing2;

    t_swing_info si;
    t_solo_code sc;

    UINT8 swing_status;

    BOOL change_wind_direction;

    UINT16 dc_cnt;
    t_ac_bit_num bit_num[MAX_BITNUM];
    UINT16 bit_num_cnt;
    UINT16 repeat_times;
    t_ac_n_mode_info n_mode[N_MODE_MAX];
    UINT16 code_cnt;
    UINT8 last_bit;
    UINT16 *time;
    UINT8 solo_function_mark;

    UINT16 frame_length;
} t_ac_protocol;

typedef struct tag_head
{
    UINT16 tag;
    UINT16 len;
    UINT16 offset;
    UINT8 *p_data;
} t_tag_head;

struct ir_bin_buffer
{
    UINT8 *data;
    UINT16 len;
    UINT16 offset;
};

typedef struct REMOTE_AC_STATUS
{
    t_ac_power ac_power;
    t_ac_temperature ac_temp;
    t_ac_mode ac_mode;
    t_ac_swing ac_wind_dir;
    t_ac_wind_speed ac_wind_speed;
    UINT8 ac_display;
    UINT8 ac_sleep;
    UINT8 ac_timer;
} t_remote_ac_status;

// function polymorphism
typedef INT8 (*lp_apply_ac_parameter)(t_remote_ac_status ac_status, UINT8 function_code);

#define TAG_AC_BOOT_CODE                  1
#define TAG_AC_ZERO                       2
#define TAG_AC_ONE                        3
#define TAG_AC_DELAY_CODE                 4
#define TAG_AC_FRAME_LENGTH               5
#define TAG_AC_ENDIAN                     6
#define TAG_AC_LAST_BIT                   7

#define TAG_AC_POWER_1                    21
#define TAG_AC_DEFAULT_CODE               22
#define TAG_AC_TEMP_1                     23
#define TAG_AC_MODE_1                     24
#define TAG_AC_SPEED_1                    25
#define TAG_AC_SWING_1                    26
#define TAG_AC_CHECKSUM_TYPE              27
#define TAG_AC_SOLO_FUNCTION              28
#define TAG_AC_FUNCTION_1                 29
#define TAG_AC_TEMP_2                     30
#define TAG_AC_MODE_2                     31
#define TAG_AC_SPEED_2                    32
#define TAG_AC_SWING_2                    33
#define TAG_AC_FUNCTION_2                 34

#define TAG_AC_BAN_FUNCTION_IN_COOL_MODE  41
#define TAG_AC_BAN_FUNCTION_IN_HEAT_MODE  42
#define TAG_AC_BAN_FUNCTION_IN_AUTO_MODE  43
#define TAG_AC_BAN_FUNCTION_IN_FAN_MODE   44
#define TAG_AC_BAN_FUNCTION_IN_DRY_MODE   45
#define TAG_AC_SWING_INFO                 46
#define TAG_AC_REPEAT_TIMES               47
#define TAG_AC_BIT_NUM                    48


// definition about size

#define PROTOCOL_SIZE (sizeof(t_ac_protocol))

/* exported variables */
extern UINT8 *ir_hex_code;
extern UINT8 ir_hex_len;
extern t_ac_protocol *context;


extern INT8 ir_ac_lib_parse();

extern INT8 free_ac_context();

extern BOOL is_solo_function(UINT8 function_code);

#ifdef __cplusplus
}
#endif

#endif //IRDA_DECODER_IR_AC_CONTROL_H
