/**************************************************************************************
Filename:       ir_irframe.c
Revised:        Date: 2016-10-01
Revision:       Revision: 1.0

Description:    This file provides algorithms for IR frame build

Revision log:
* 2016-10-01: created by strawmanbobi
**************************************************************************************/

#include "../include/ir_ac_build_frame.h"
#include "../include/ir_decode.h"

extern t_ac_protocol *context;


//return bit number per byte,default value is 8
UINT8 bits_per_byte(UINT8 index)
{
    UINT8 i = 0;
    UINT8 size = 0;

    if (context->bit_num_cnt == 0)
        return 8; //defaut value

    if (context->bit_num_cnt >= MAX_BITNUM)
        size = MAX_BITNUM;
    else
        size = (UINT8) context->bit_num_cnt;

    for (i = 0; i < size; i++)
    {
        if (context->bit_num[i].pos == index)
            return (UINT8) context->bit_num[i].bits;
        if (context->bit_num[i].pos > index)
            return 8;
    }
    return 8;
}

UINT16 add_delaycode(UINT8 index)
{
    UINT8 i = 0, j = 0;
    UINT8 size = 0;
    UINT8 tail_delaycode = 0;
    UINT16 tail_pos = 0;

    if (context->dc_cnt != 0)
    {
        size = (UINT8) context->dc_cnt;

        for (i = 0; i < size; i++)
        {
            if (context->dc[i].pos == index)
            {
                for (j = 0; j < context->dc[i].time_cnt; j++)
                {
                    context->time[context->code_cnt++] = context->dc[i].time[j];
                }
            }
            else if (context->dc[i].pos == -1)
            {
                tail_delaycode = 1;
                tail_pos = i;
            }
        }
    }

    if ((context->last_bit == 0) && (index == (ir_hex_len - 1)))
    {
        context->time[context->code_cnt++] = context->one.low; //high
    }

    if (context->dc_cnt != 0)
    {
        if ((index == (ir_hex_len - 1)) && (tail_delaycode == 1))
        {
            for (i = 0; i < context->dc[tail_pos].time_cnt; i++)
            {
                context->time[context->code_cnt++] = context->dc[tail_pos].time[i];
            }
        }
    }

    return context->dc[i].time_cnt;
}

UINT16 create_ir_frame()
{
    UINT16 i = 0, j = 0;
    UINT8 bitnum = 0;
    UINT8 *irdata = ir_hex_code;
    UINT8 mask = 1;
    UINT16 framelen = 0;

    context->code_cnt = 0;

    // boot code
    for (i = 0; i < context->boot_code.len; i++)
    {
        context->time[context->code_cnt++] = context->boot_code.data[i];
    }
    //code_cnt += context->boot_code.len;

    for (i = 0; i < ir_hex_len; i++)
    {
        bitnum = bits_per_byte((UINT8) i);
        for (j = 0; j < bitnum; j++)
        {
            if (context->endian == 0)
                mask = (UINT8) ((1 << (bitnum - 1)) >> j);
            else
                mask = (UINT8) (1 << j);

            if (irdata[i] & mask)
            {
                //ir_printf("%d,%d,", context->one.low, context->one.high);
                context->time[context->code_cnt++] = context->one.low;
                context->time[context->code_cnt++] = context->one.high;
            }
            else
            {
                //ir_printf("%d,%d,", context->zero.low, context->zero.high);
                context->time[context->code_cnt++] = context->zero.low;
                context->time[context->code_cnt++] = context->zero.high;
            }
        }
        add_delaycode((UINT8) i);
    }

    framelen = context->code_cnt;

    for (i = 0; i < (context->repeat_times - 1); i++)
    {
        for (j = 0; j < framelen; j++)
        {
            context->time[context->code_cnt++] = context->time[j];
        }
    }

    return context->code_cnt;
}