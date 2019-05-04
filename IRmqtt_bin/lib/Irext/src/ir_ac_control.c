/**************************************************************************************
Filename:       ir_ac_control.c
Revised:        Date: 2017-01-02
Revision:       Revision: 1.0

Description:    This file provides methods for AC IR control

Revision log:
* 2016-10-12: created by strawmanbobi
**************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir_ac_control.h"
#include "../include/ir_ac_binary_parse.h"
#include "../include/ir_decode.h"
#include "../include/ir_ac_parse_parameter.h"
#include "../include/ir_ac_parse_forbidden_info.h"
#include "../include/ir_ac_parse_frame_info.h"
#include "../include/ir_utils.h"


#if defined USE_DYNAMIC_TAG
extern struct tag_head *tags;
#else
extern struct tag_head tags[];
#endif

extern UINT8 tag_count;

static INT8 ir_context_init();


static INT8 ir_context_init()
{
    ir_memset(context, 0, sizeof(t_ac_protocol));
    return IR_DECODE_SUCCEEDED;
}


INT8 ir_ac_lib_parse()
{
    UINT8 i = 0;
    // suggest not to call init function here for de-couple purpose
    ir_context_init();

    if (IR_DECODE_FAILED == binary_parse_offset())
    {
        return IR_DECODE_FAILED;
    }

    if (IR_DECODE_FAILED == binary_parse_len())
    {
        return IR_DECODE_FAILED;
    }

    if (IR_DECODE_FAILED == binary_parse_data())
    {
        return IR_DECODE_FAILED;
    }

    binary_tags_info();

    context->endian = 0;
    context->last_bit = 0;
    context->repeat_times = 1;

    for (i = 0; i < N_MODE_MAX; i++)
    {
        context->n_mode[i].enable = TRUE;
        context->n_mode[i].all_speed = FALSE;
        context->n_mode[i].all_temp = FALSE;
        ir_memset(context->n_mode[i].speed, 0x00, AC_WS_MAX);
        context->n_mode[i].speed_cnt = 0;
        ir_memset(context->n_mode[i].temp, 0x00, AC_TEMP_MAX);
        context->n_mode[i].temp_cnt = 0;
    }

    // parse TAG 46 in first priority
    for (i = 0; i < tag_count; i++)
    {
        if (tags[i].tag == TAG_AC_SWING_INFO)
        {
            if (tags[i].len != 0)
            {
                parse_swing_info(&tags[i], &(context->si));
            }
            else
            {
                context->si.type = SWING_TYPE_NORMAL;
                context->si.mode_count = 2;
            }
            context->si.dir_index = 0;
            break;
        }
    }

    for (i = 0; i < tag_count; i++)
    {
        if (tags[i].len == 0)
        {
            continue;
        }
        // then parse TAG 26 or 33
        if (context->si.type == SWING_TYPE_NORMAL)
        {
            UINT16 swing_space_size = 0;
            if (tags[i].tag == TAG_AC_SWING_1)
            {
                context->swing1.count = context->si.mode_count;
                context->swing1.len = (UINT8) tags[i].len >> 1;
                swing_space_size = sizeof(t_tag_comp) * context->si.mode_count;
                context->swing1.comp_data = (t_tag_comp *) ir_malloc(swing_space_size);
                if (NULL == context->swing1.comp_data)
                {
                    return IR_DECODE_FAILED;
                }

                ir_memset(context->swing1.comp_data, 0x00, swing_space_size);
                if (IR_DECODE_FAILED == parse_common_ac_parameter(&tags[i],
                                                                  context->swing1.comp_data,
                                                                  context->si.mode_count,
                                                                  AC_PARAMETER_TYPE_1))
                {
                    return IR_DECODE_FAILED;
                }
            }
            else if (tags[i].tag == TAG_AC_SWING_2)
            {
                context->swing2.count = context->si.mode_count;
                context->swing2.len = (UINT8) tags[i].len >> 1;
                swing_space_size = sizeof(t_tag_comp) * context->si.mode_count;
                context->swing2.comp_data = (t_tag_comp *) ir_malloc(swing_space_size);
                if (NULL == context->swing2.comp_data)
                {
                    return IR_DECODE_FAILED;
                }
                ir_memset(context->swing2.comp_data, 0x00, swing_space_size);
                if (IR_DECODE_FAILED == parse_common_ac_parameter(&tags[i],
                                                                  context->swing2.comp_data,
                                                                  context->si.mode_count,
                                                                  AC_PARAMETER_TYPE_2))
                {
                    return IR_DECODE_FAILED;
                }
            }
        }

        if (tags[i].tag == TAG_AC_DEFAULT_CODE) // default code TAG
        {
            context->default_code.data = (UINT8 *) ir_malloc(((size_t) tags[i].len - 2) >> 1);
            if (NULL == context->default_code.data)
            {
                return IR_DECODE_FAILED;
            }
            if (IR_DECODE_FAILED == parse_default_code(&tags[i], &(context->default_code)))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_POWER_1) // power tag
        {
            context->power1.len = (UINT8) tags[i].len >> 1;
            if (IR_DECODE_FAILED == parse_common_ac_parameter(&tags[i],
                                                              context->power1.comp_data,
                                                              AC_POWER_MAX,
                                                              AC_PARAMETER_TYPE_1))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_TEMP_1) // temperature tag type 1
        {
            if (IR_DECODE_FAILED == parse_temp_1(&tags[i], &(context->temp1)))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_MODE_1) // mode tag
        {
            context->mode1.len = (UINT8) tags[i].len >> 1;
            if (IR_DECODE_FAILED == parse_common_ac_parameter(&tags[i],
                                                              context->mode1.comp_data,
                                                              AC_MODE_MAX,
                                                              AC_PARAMETER_TYPE_1))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_SPEED_1) // wind speed tag
        {
            context->speed1.len = (UINT8) tags[i].len >> 1;
            if (IR_DECODE_FAILED == parse_common_ac_parameter(&tags[i],
                                                              context->speed1.comp_data,
                                                              AC_WS_MAX,
                                                              AC_PARAMETER_TYPE_1))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_CHECKSUM_TYPE)
        {
            if (IR_DECODE_FAILED == parse_checksum(&tags[i], &(context->checksum)))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_MODE_2)
        {
            context->mode2.len = (UINT8) tags[i].len >> 1;
            if (IR_DECODE_FAILED ==
                parse_common_ac_parameter(&tags[i],
                                          context->mode2.comp_data, AC_MODE_MAX, AC_PARAMETER_TYPE_1))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_SPEED_2)
        {
            context->speed2.len = (UINT8) tags[i].len >> 1;
            if (IR_DECODE_FAILED ==
                parse_common_ac_parameter(&tags[i],
                                          context->speed2.comp_data, AC_WS_MAX, AC_PARAMETER_TYPE_1))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_TEMP_2)
        {
            if (IR_DECODE_FAILED == parse_temp_2(&tags[i], &(context->temp2)))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_SOLO_FUNCTION)
        {
            if (IR_DECODE_FAILED == parse_solo_code(&tags[i], &(context->sc)))
            {
                return IR_DECODE_FAILED;
            }
            context->solo_function_mark = 1;
        }
        else if (tags[i].tag == TAG_AC_FUNCTION_1)
        {
            if (IR_DECODE_FAILED == parse_function_1_tag29(&tags[i], &(context->function1)))
            {
                ir_printf("\nfunction code parse error\n");
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_FUNCTION_2)
        {
            if (IR_DECODE_FAILED == parse_function_2_tag34(&tags[i], &(context->function2)))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_FRAME_LENGTH)
        {
            if (IR_DECODE_FAILED == parse_frame_len(&tags[i], tags[i].len))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_ZERO)
        {
            if (IR_DECODE_FAILED == parse_zero(&tags[i]))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_ONE)
        {
            if (IR_DECODE_FAILED == parse_one(&tags[i]))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_BOOT_CODE)
        {
            if (IR_DECODE_FAILED == parse_boot_code(&tags[i]))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_REPEAT_TIMES)
        {
            if (IR_DECODE_FAILED == parse_repeat_times(&tags[i]))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_BIT_NUM)
        {
            if (IR_DECODE_FAILED == parse_bit_num(&tags[i]))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_ENDIAN)
        {
            if (IR_DECODE_FAILED == parse_endian(&tags[i]))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_BAN_FUNCTION_IN_COOL_MODE)
        {
            if (IR_DECODE_FAILED == parse_nmode(&tags[i], N_COOL))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_BAN_FUNCTION_IN_HEAT_MODE)
        {
            if (IR_DECODE_FAILED == parse_nmode(&tags[i], N_HEAT))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_BAN_FUNCTION_IN_AUTO_MODE)
        {
            if (IR_DECODE_FAILED == parse_nmode(&tags[i], N_AUTO))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_BAN_FUNCTION_IN_FAN_MODE)
        {
            if (IR_DECODE_FAILED == parse_nmode(&tags[i], N_FAN))
            {
                return IR_DECODE_FAILED;
            }
        }
        else if (tags[i].tag == TAG_AC_BAN_FUNCTION_IN_DRY_MODE)
        {
            if (IR_DECODE_FAILED == parse_nmode(&tags[i], N_DRY))
            {
                return IR_DECODE_FAILED;
            }
        }
    }

    for (i = 0; i < tag_count; i++)
    {
        if (tags[i].len == 0)
        {
            continue;
        }
        if (tags[i].tag == TAG_AC_DELAY_CODE)
        {
            if (IR_DECODE_FAILED == parse_delay_code(&tags[i]))
            {
                return IR_DECODE_FAILED;
            }
        }
        if (tags[i].tag == TAG_AC_LAST_BIT)
        {
            if (IR_DECODE_FAILED == parse_lastbit(&tags[i]))
            {
                return IR_DECODE_FAILED;
            }
        }
    }

#if defined USE_DYNAMIC_TAG
    if (NULL != tags)
    {
        ir_free(tags);
        tags = NULL;
    }
#endif

    ir_hex_code = (UINT8 *) ir_malloc(context->default_code.len);
    if (NULL == ir_hex_code)
    {
        // warning: this AC bin contains no default code
        return IR_DECODE_FAILED;
    }

    ir_hex_len = context->default_code.len;
    ir_memset(ir_hex_code, 0x00, ir_hex_len);

    // pre-calculate solo function status after parse phase
    if (1 == context->solo_function_mark)
    {
        context->solo_function_mark = 0x00;
        // bit order from right to left : power, mode, temp+, temp-, wind_speed, swing, fix
        for (i = AC_FUNCTION_POWER; i < AC_FUNCTION_MAX; i++)
        {
            if (is_in(context->sc.solo_function_codes, i, context->sc.solo_func_count))
            {
                context->solo_function_mark |= (1 << (i - 1));
            }
        }
    }

    // it is strongly recommended that we free p_ir_buffer
    // or make global buffer shared in extreme memory case
    /* in case of running with test - begin */
#if (defined BOARD_PC || defined BOARD_PC_DLL)
    ir_lib_free_inner_buffer();
    ir_printf("AC parse done\n");
#endif
    /* in case of running with test - end */

    return IR_DECODE_SUCCEEDED;
}


INT8 free_ac_context()
{
    UINT16 i = 0;

    if (ir_hex_code != NULL)
    {
        ir_free(ir_hex_code);
        ir_hex_code = NULL;
    }
    ir_hex_len = 0;

    if (context->default_code.data != NULL)
    {
        ir_free(context->default_code.data);
        context->default_code.data = NULL;
        context->default_code.len = 0;
    }

    for (i = 0; i < AC_POWER_MAX; i++)
    {
        if (context->power1.comp_data[i].segment != NULL)
        {
            ir_free(context->power1.comp_data[i].segment);
            context->power1.comp_data[i].segment = NULL;
            context->power1.comp_data[i].seg_len = 0;
        }
    }

    for (i = 0; i < AC_TEMP_MAX; i++)
    {
        if (context->temp1.comp_data[i].segment != NULL)
        {
            ir_free(context->temp1.comp_data[i].segment);
            context->temp1.comp_data[i].segment = NULL;
            context->temp1.comp_data[i].seg_len = 0;
        }
        if (context->temp2.comp_data[i].segment != NULL)
        {
            ir_free(context->temp2.comp_data[i].segment);
            context->temp2.comp_data[i].segment = NULL;
            context->temp2.comp_data[i].seg_len = 0;
        }
    }

    for (i = 0; i < AC_MODE_MAX; i++)
    {
        if (context->mode1.comp_data[i].segment != NULL)
        {
            ir_free(context->mode1.comp_data[i].segment);
            context->mode1.comp_data[i].segment = NULL;
            context->mode1.comp_data[i].seg_len = 0;
        }
        if (context->mode2.comp_data[i].segment != NULL)
        {
            ir_free(context->mode2.comp_data[i].segment);
            context->mode2.comp_data[i].segment = NULL;
            context->mode2.comp_data[i].seg_len = 0;
        }
    }
    for (i = 0; i < AC_WS_MAX; i++)
    {
        if (context->speed1.comp_data[i].segment != NULL)
        {
            ir_free(context->speed1.comp_data[i].segment);
            context->speed1.comp_data[i].segment = NULL;
            context->speed1.comp_data[i].seg_len = 0;
        }
        if (context->speed2.comp_data[i].segment != NULL)
        {
            ir_free(context->speed2.comp_data[i].segment);
            context->speed2.comp_data[i].segment = NULL;
            context->speed2.comp_data[i].seg_len = 0;
        }
    }

    for (i = 0; i < context->si.mode_count; i++)
    {
        if (context->swing1.comp_data != NULL &&
            context->swing1.comp_data[i].segment != NULL)
        {
            ir_free(context->swing1.comp_data[i].segment);
            context->swing1.comp_data[i].segment = NULL;
            context->swing1.comp_data[i].seg_len = 0;
        }
        if (context->swing2.comp_data != NULL &&
            context->swing2.comp_data[i].segment != NULL)
        {
            ir_free(context->swing2.comp_data[i].segment);
            context->swing2.comp_data[i].segment = NULL;
            context->swing2.comp_data[i].seg_len = 0;
        }
    }

    for (i = 0; i < AC_FUNCTION_MAX - 1; i++)
    {
        if (context->function1.comp_data[i].segment != NULL)
        {
            ir_free(context->function1.comp_data[i].segment);
            context->function1.comp_data[i].segment = NULL;
            context->function1.comp_data[i].seg_len = 0;
        }
        if (context->function2.comp_data[i].segment != NULL)
        {
            ir_free(context->function2.comp_data[i].segment);
            context->function2.comp_data[i].segment = NULL;
            context->function2.comp_data[i].seg_len = 0;
        }
    }

    // free composite data for swing1 and swing 2
    if (context->swing1.comp_data != NULL)
    {
        ir_free(context->swing1.comp_data);
        context->swing1.comp_data = NULL;
    }
    if (context->swing2.comp_data != NULL)
    {
        ir_free(context->swing2.comp_data);
        context->swing2.comp_data = NULL;
    }

    for (i = 0; i < context->checksum.count; i++)
    {
        if (context->checksum.checksum_data != NULL &&
            context->checksum.checksum_data[i].spec_pos != NULL)
        {
            ir_free(context->checksum.checksum_data[i].spec_pos);
            context->checksum.checksum_data[i].len = 0;
            context->checksum.checksum_data[i].spec_pos = NULL;
        }
    }
    if (context->checksum.checksum_data != NULL)
    {
        ir_free(context->checksum.checksum_data);
        context->checksum.checksum_data = NULL;
    }

    return IR_DECODE_SUCCEEDED;
}

BOOL is_solo_function(UINT8 function_code)
{
    return (((context->solo_function_mark >> (function_code - 1)) & 0x01) == 0x01) ? TRUE : FALSE;
}
