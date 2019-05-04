/**************************************************************************************
Filename:       ir_decode.c
Revised:        Date: 2016-10-01
Revision:       Revision: 1.0

Description:    This file provides algorithms for IR decode (status type)

Revision log:
* 2016-10-01: created by strawmanbobi
**************************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include "../include/ir_decode.h"
#include "../include/ir_utils.h"
#include "../include/ir_ac_build_frame.h"
#include "../include/ir_ac_apply.h"

struct ir_bin_buffer binary_file;
struct ir_bin_buffer *p_ir_buffer = &binary_file;

const char* release = "0.2.0";

#if defined USE_DYNAMIC_TAG
struct tag_head *tags;
#else
struct tag_head tags[TAG_COUNT_FOR_PROTOCOL];
#endif

UINT8 *ir_hex_code = NULL;
UINT8 ir_hex_len = 0;

UINT8 byteArray[PROTOCOL_SIZE] = { 0 };

size_t binary_length = 0;
UINT8 *binary_content = NULL;

UINT8 ir_binary_type = IR_TYPE_STATUS;
UINT8 ir_hexadecimal = SUB_CATEGORY_QUATERNARY;

t_ac_protocol *context = (t_ac_protocol *) byteArray;

lp_apply_ac_parameter apply_table[AC_APPLY_MAX] =
{
    apply_power,
    apply_mode,
    apply_temperature,
    apply_temperature,
    apply_wind_speed,
    apply_swing,
    apply_swing
};

// static functions declarations
static INT8 ir_ac_file_open(const char *file_name);
static INT8 ir_ac_binary_open(UINT8 *binary, UINT16 binary_length);
static UINT16 ir_ac_control(t_remote_ac_status ac_status, UINT16 *user_data, UINT8 function_code,
                            BOOL change_wind_direction);
static INT8 ir_ac_binary_close();
static INT8 ir_tv_file_open(const char *file_name);
static INT8 ir_tv_binary_open(UINT8 *binary, UINT16 binary_length);
static INT8 ir_tv_binary_parse(UINT8 ir_hex_encode);
static UINT16 ir_tv_control(UINT8 key, UINT16 *l_user_data);
static INT8 ir_tv_binary_close();


void noprint(const char *fmt, ...)
{
    return;
}

// pubic function definitions
#if (!defined BOARD_51 && !defined BOARD_CC26XX)
INT8 ir_file_open(const UINT8 category, const UINT8 sub_category, const char* file_name)
{
    INT8 ret = IR_DECODE_SUCCEEDED;
    if (category == IR_CATEGORY_AC)
    {
        ir_binary_type = IR_TYPE_STATUS;
        ret = ir_ac_file_open(file_name);
        if (IR_DECODE_SUCCEEDED == ret)
        {
            return ir_ac_lib_parse();
        }
        else
        {
            return ret;
        }
    }
    else
    {
        ir_binary_type = IR_TYPE_COMMANDS;
        if (1 == sub_category)
        {
            ir_hexadecimal = SUB_CATEGORY_QUATERNARY;
        }
        else if (2 == sub_category)
        {
            ir_hexadecimal = SUB_CATEGORY_HEXADECIMAL;
        }
        else
        {
            return IR_DECODE_FAILED;
        }

        ret = ir_tv_file_open(file_name);
        if (IR_DECODE_SUCCEEDED == ret)
        {
            return ir_tv_binary_parse(ir_hexadecimal);
        }
        else
        {
            return ret;
        }
    }
}
#else
INT8 ir_file_open(const UINT8 category, const UINT8 sub_category, const char* file_name)
{
    return IR_DECODE_SUCCEEDED;
}
#endif

INT8 ir_binary_open(const UINT8 category, const UINT8 sub_category, UINT8* binary, UINT16 binary_length)
{
    INT8 ret = IR_DECODE_SUCCEEDED;

    if (category == IR_CATEGORY_AC)
    {
        ir_binary_type = IR_TYPE_STATUS;
        ret = ir_ac_binary_open(binary, binary_length);
        if (IR_DECODE_SUCCEEDED == ret)
        {
            return ir_ac_lib_parse();
        }
        else
        {
            return ret;
        }
    }
    else
    {
        ir_binary_type = IR_TYPE_COMMANDS;
        if (1 == sub_category)
        {
            ir_hexadecimal = SUB_CATEGORY_QUATERNARY;
        }
        else if (2 == sub_category)
        {
            ir_hexadecimal = SUB_CATEGORY_HEXADECIMAL;
        }
        else
        {
            return IR_DECODE_FAILED;
        }

        ret = ir_tv_binary_open(binary, binary_length);
        if (IR_DECODE_SUCCEEDED == ret)
        {
            return ir_tv_binary_parse(ir_hexadecimal);
        }
        else
        {
            return ret;
        }
    }
}


UINT16 ir_decode(UINT8 key_code, UINT16* user_data,
        t_remote_ac_status* ac_status, BOOL change_wind_direction)
{
    if (IR_TYPE_COMMANDS == ir_binary_type)
    {
        return ir_tv_control(key_code, user_data);
    }
    else
    {
        if (NULL == ac_status)
        {
            return 0;
        }
        ir_printf("ac status is not null in decode core : power = %d, mode = %d, "
                  "temp = %d, wind_dir = %d, wind_speed = %d, "
                  "keycode = %d, change_wind_direction = %d\n",
                  ac_status->ac_power, ac_status->ac_mode,
                  ac_status->ac_temp, ac_status->ac_wind_dir,
                  ac_status->ac_wind_speed,
                  key_code, change_wind_direction);
        return ir_ac_control(*ac_status, user_data, key_code, change_wind_direction);
    }
}


INT8 ir_close()
{
    if (IR_TYPE_COMMANDS == ir_binary_type)
    {
        ir_printf("tv binary close\n");
        return ir_tv_binary_close();
    }
    else
    {
        ir_printf("ac binary close\n");
        return ir_ac_binary_close();
    }
}


#if (defined BOARD_PC || defined BOARD_PC_DLL)
void ir_lib_free_inner_buffer();
#endif


// static function definitions

//////// AC Begin ////////
static INT8 ir_ac_file_open(const char *file_name)
{
#if !defined NO_FS
    size_t ret = 0;
#if !defined WIN32
    FILE *stream = fopen(file_name, "rb");
#else
    FILE *stream;
    fopen_s(&stream, file_name, "rb");
#endif
    if (NULL == stream)
    {
        ir_printf("\nfile open failed\n");
        return IR_DECODE_FAILED;
    }

    fseek(stream, 0, SEEK_END);
    binary_length = (size_t) ftell(stream);
    binary_content = (UINT8 *) ir_malloc(binary_length);

    if (NULL == binary_content)
    {
        ir_printf("\nfailed to alloc memory for binary\n");
        fclose(stream);
        return IR_DECODE_FAILED;
    }

    fseek(stream, 0, SEEK_SET);
    ret = fread(binary_content, binary_length, 1, stream);

    if (ret <= 0)
    {
        fclose(stream);
        ir_free(binary_content);
        binary_length = 0;
        return IR_DECODE_FAILED;
    }

    fclose(stream);

    if (IR_DECODE_FAILED == ir_ac_binary_open(binary_content, (UINT16) binary_length))
    {
        ir_free(binary_content);
        binary_length = 0;
        return IR_DECODE_FAILED;
    }
#endif
    return IR_DECODE_SUCCEEDED;
}

static INT8 ir_ac_binary_open(UINT8 *binary, UINT16 binary_length)
{
    // it is recommended that the parameter binary pointing to
    // a global memory block in embedded platform environment
    p_ir_buffer->data = binary;
    p_ir_buffer->len = binary_length;
    p_ir_buffer->offset = 0;
    return IR_DECODE_SUCCEEDED;
}

static UINT16 ir_ac_control(t_remote_ac_status ac_status, UINT16 *user_data, UINT8 function_code,
                            BOOL change_wind_direction)
{
    UINT16 time_length = 0;

#if defined BOARD_PC
    UINT16 i = 0;
#endif

    if (0 == context->default_code.len)
    {
        ir_printf("\ndefault code is empty\n");
        return 0;
    }

    // pre-set change wind direction flag here
    context->change_wind_direction = change_wind_direction;

    context->time = user_data;

    // generate temp buffer for frame calculation
    ir_memcpy(ir_hex_code, context->default_code.data, context->default_code.len);

#if defined USE_APPLY_TABLE
    if(ac_status.ac_power != AC_POWER_OFF)
    {
        for (i = AC_APPLY_POWER; i < AC_APPLY_MAX; i++)
        {
            apply_table[i](context, parameter_array[i]);
        }
    }
#else
    if (ac_status.ac_power == AC_POWER_OFF)
    {
        // otherwise, power should always be applied
        apply_power(ac_status, function_code);
    }
    else
    {
        // check the mode as the first priority, despite any other status
        if (TRUE == context->n_mode[ac_status.ac_mode].enable)
        {
            if (is_solo_function(function_code))
            {
                // this key press function needs to send solo code
                apply_table[function_code - 1](ac_status, function_code);
            }
            else
            {
                if (!is_solo_function(AC_FUNCTION_POWER))
                {
                    apply_power(ac_status, function_code);
                }

                if (!is_solo_function(AC_FUNCTION_MODE))
                {
                    if (IR_DECODE_FAILED == apply_mode(ac_status, function_code))
                    {
                        return 0;
                    }
                }

                if (!is_solo_function(AC_FUNCTION_WIND_SPEED))
                {
                    if (IR_DECODE_FAILED == apply_wind_speed(ac_status, function_code))
                    {
                        return 0;
                    }
                }

                if (!is_solo_function(AC_FUNCTION_WIND_SWING) &&
                    !is_solo_function(AC_FUNCTION_WIND_FIX))
                {
                    if (IR_DECODE_FAILED == apply_swing(ac_status, function_code))
                    {
                        return 0;
                    }
                }

                if (!is_solo_function(AC_FUNCTION_TEMPERATURE_UP) &&
                    !is_solo_function(AC_FUNCTION_TEMPERATURE_DOWN))
                {
                    if (IR_DECODE_FAILED == apply_temperature(ac_status, function_code))
                    {
                        return 0;
                    }
                }
            }
        }
        else
        {
            return 0;
        }
    }
#endif
    apply_function(context, function_code);
    // checksum should always be applied
    apply_checksum(context);

    time_length = create_ir_frame();

#if (defined BOARD_PC)
#if (defined BOARD_PC_JNI)
    ir_printf("code count = %d\n", context->code_cnt);
#else
    for (i = 0; i < context->code_cnt; i++)
    {
        ir_printf("%d,", context->time[i]);
    }
#endif
    ir_printf("\n");
#endif

    return time_length;
}

static INT8 ir_ac_binary_close()
{
#if defined USE_DYNAMIC_TAG
    // free context
    if (NULL != tags)
    {
        ir_free(tags);
        tags = NULL;
    }
#endif

    free_ac_context();

    return IR_DECODE_SUCCEEDED;
}

// utils
INT8 get_temperature_range(UINT8 ac_mode, INT8 *temp_min, INT8 *temp_max)
{
    UINT8 i = 0;

    if (ac_mode >= AC_MODE_MAX)
    {
        return IR_DECODE_FAILED;
    }
    if (NULL == temp_min || NULL == temp_max)
    {
        return IR_DECODE_FAILED;
    }

    if (1 == context->n_mode[ac_mode].all_temp)
    {
        *temp_min = *temp_max = -1;
        return IR_DECODE_SUCCEEDED;
    }

    *temp_min = -1;
    *temp_max = -1;
    for (i = 0; i < AC_TEMP_MAX; i++)
    {
        if (is_in(context->n_mode[ac_mode].temp, i, context->n_mode[ac_mode].temp_cnt) ||
            (context->temp1.len != 0 && 0 == context->temp1.comp_data[i].seg_len) ||
            (context->temp2.len != 0 && 0 == context->temp2.comp_data[i].seg_len))
        {
            continue;
        }
        if (-1 == *temp_min)
        {
            *temp_min = i;
        }
        if (-1 == *temp_max || i > *temp_max)
        {
            *temp_max = i;
        }
    }
    return IR_DECODE_SUCCEEDED;
}

INT8 get_supported_mode(UINT8 *supported_mode)
{
    UINT8 i = 0;
    if (NULL == supported_mode)
    {
        return IR_DECODE_FAILED;
    }
    *supported_mode = 0x1F;

    for (i = 0; i < AC_MODE_MAX; i++)
    {
        if (0 == context->n_mode[i].enable ||
            (context->mode1.len != 0 && 0 == context->mode1.comp_data[i].seg_len) ||
            (context->mode2.len != 0 && 0 == context->mode2.comp_data[i].seg_len))
        {
            *supported_mode &= ~(1 << i);
        }
    }

    return IR_DECODE_SUCCEEDED;
}

INT8 get_supported_wind_speed(UINT8 ac_mode, UINT8 *supported_wind_speed)
{
    UINT8 i = 0;
    if (ac_mode >= AC_MODE_MAX)
    {
        return IR_DECODE_FAILED;
    }

    if (NULL == supported_wind_speed)
    {
        return IR_DECODE_FAILED;
    }

    if (1 == context->n_mode[ac_mode].all_speed)
    {
        *supported_wind_speed = 0;
        return IR_DECODE_SUCCEEDED;
    }

    *supported_wind_speed = 0x0F;

    for (i = 0; i < AC_WS_MAX; i++)
    {
        if (is_in(context->n_mode[ac_mode].speed, i, context->n_mode[ac_mode].speed_cnt) ||
            (context->speed1.len != 0 && 0 == context->speed1.comp_data[i].seg_len) ||
            (context->speed2.len != 0 && 0 == context->speed2.comp_data[i].seg_len))
        {
            *supported_wind_speed &= ~(1 << i);
        }
    }

    return IR_DECODE_SUCCEEDED;
}

INT8 get_supported_swing(UINT8 ac_mode, UINT8 *supported_swing)
{
    if (ac_mode >= AC_MODE_MAX)
    {
        return IR_DECODE_FAILED;
    }

    if (NULL == supported_swing)
    {
        return IR_DECODE_FAILED;
    }

    if (context->si.type == SWING_TYPE_NORMAL)
    {
        *supported_swing = 0x03;
    }
    else if (context->si.type == SWING_TYPE_SWING_ONLY)
    {
        *supported_swing = 0x02;
    }
    else if (context->si.type == SWING_TYPE_NOT_SPECIFIED)
    {
        *supported_swing = 0x00;
    }
    else
    {
        *supported_swing = 0x01;
    }
    return IR_DECODE_SUCCEEDED;
}

INT8 get_supported_wind_direction(UINT8 *supported_wind_direction)
{
    if (NULL != context)
    {
        *supported_wind_direction = (UINT8) (context->si.mode_count - 1);
        return IR_DECODE_SUCCEEDED;
    }
    else
    {
        return IR_DECODE_FAILED;
    }
}

//////// AC End ////////

//////// TV Begin ////////
static INT8 ir_tv_file_open(const char *file_name)
{
#if !defined NO_FS
    size_t ret = 0;

#if !defined WIN32
    FILE *stream = fopen(file_name, "rb");
#else
    FILE *stream;
    fopen_s(&stream, file_name, "rb");
#endif

    if (stream == NULL)
    {
        ir_printf("\nfile open failed\n");
        return IR_DECODE_FAILED;
    }

    fseek(stream, 0, SEEK_END);
    binary_length = (size_t) ftell(stream);

    binary_content = (UINT8 *) ir_malloc(binary_length);
    if (NULL == binary_content)
    {
        ir_printf("\nfailed to alloc memory for binary\n");
        fclose(stream);
        return IR_DECODE_FAILED;
    }

    fseek(stream, 0, SEEK_SET);
    ret = fread(binary_content, binary_length, 1, stream);
    if (ret <= 0)
    {
        fclose(stream);
        ir_free(binary_content);
        binary_length = 0;
        return IR_DECODE_FAILED;
    }

    fclose(stream);

    if (IR_DECODE_FAILED == ir_tv_binary_open(binary_content, (UINT16) binary_length))
    {
        ir_free(binary_content);
        binary_length = 0;
        return IR_DECODE_FAILED;
    }
#endif
    return IR_DECODE_SUCCEEDED;
}

static INT8 ir_tv_binary_open(UINT8 *binary, UINT16 binary_length)
{
    return tv_binary_open(binary, binary_length);
}

static INT8 ir_tv_binary_parse(UINT8 ir_hex_encode)
{
    if (FALSE == tv_binary_parse(ir_hex_encode))
    {
        ir_printf("parse irda binary failed\n");
        return IR_DECODE_FAILED;
    }
    return IR_DECODE_SUCCEEDED;
}

static UINT16 ir_tv_control(UINT8 key, UINT16 *l_user_data)
{
#if defined BOARD_PC
    UINT16 print_index = 0;
#endif
    UINT16 ir_code_length = 0;
    memset(l_user_data, 0x00, USER_DATA_SIZE);
    ir_code_length = tv_binary_decode(key, l_user_data);

#if defined BOARD_PC
    // have some debug
    ir_printf("length of IR code = %d\n", ir_code_length);
    for (print_index = 0; print_index < ir_code_length; print_index++)
    {
        ir_printf("%d ", l_user_data[print_index]);
    }
#endif

    return ir_code_length;
}

static INT8 ir_tv_binary_close()
{
#if (defined BOARD_PC || defined BOARD_PC_DLL)
    ir_lib_free_inner_buffer();
#endif
    return IR_DECODE_SUCCEEDED;
}
//////// TV End ////////

// combo decode for JNI which means call open, decode and then close in one JNI call
UINT16 ir_decode_combo(const UINT8 category, const UINT8 sub_category,
                       UINT8* binary, UINT16 binary_length,
                       UINT8 key_code, UINT16* user_data,
                       t_remote_ac_status* ac_status, BOOL change_wind_direction)
{
    UINT16 decoded_length = 0;
    if (IR_DECODE_SUCCEEDED ==
        ir_binary_open(category, sub_category, binary, binary_length))
    {
        decoded_length = ir_decode(key_code, user_data, ac_status, change_wind_direction);
        ir_close();
        return decoded_length;
    }
    else
    {
        return 0;
    }
}

#if (defined BOARD_PC || defined BOARD_PC_DLL)
void ir_lib_free_inner_buffer()
{
    if (NULL != binary_content)
    {
        ir_free(binary_content);
        binary_content = NULL;
    }
}
#endif