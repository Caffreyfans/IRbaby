/**************************************************************************************
Filename:       ir_parse_frame_parameter.c
Revised:        Date: 2016-10-11
Revision:       Revision: 1.0

Description:    This file provides algorithms for IR decode for AC frame parameters

Revision log:
* 2016-10-11: created by strawmanbobi
**************************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/ir_utils.h"
#include "../include/ir_ac_parse_frame_info.h"


INT8 parse_boot_code(struct tag_head *tag)
{
    UINT8 buf[16] = { 0 };
    UINT8 *p = NULL;
    UINT16 pos = 0;
    UINT16 cnt = 0, index = 0;

    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }
    p = tag->p_data;

    if (NULL == p)
    {
        return IR_DECODE_FAILED;
    }

    while (index <= tag->len)
    {
        while ((index != (tag->len)) && (*(p++) != ','))
        {
            index++;
        }
        ir_memcpy(buf, tag->p_data + pos, index - pos);
        pos = (UINT16) (index + 1);
        index = pos;
        context->boot_code.data[cnt++] = (UINT16) (atoi((char *) buf));
        ir_memset(buf, 0, 16);
    }
    context->boot_code.len = cnt;
    return IR_DECODE_SUCCEEDED;
}

INT8 parse_zero(struct tag_head *tag)
{
    UINT8 low[16] = { 0 };
    UINT8 high[16] = { 0 };
    UINT16 index = 0;
    UINT8 *p = NULL;

    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }
    p = tag->p_data;

    if (NULL == p)
    {
        return IR_DECODE_FAILED;
    }

    while (*(p++) != ',')
    {
        index++;
    }

    ir_memcpy(low, tag->p_data, index);
    ir_memcpy(high, tag->p_data + index + 1, (size_t) (tag->len - index - 1));

    context->zero.low = (UINT16) (atoi((char *) low));
    context->zero.high = (UINT16) (atoi((char *) high));
    return IR_DECODE_SUCCEEDED;
}

INT8 parse_one(struct tag_head *tag)
{
    UINT8 low[16] = { 0 };
    UINT8 high[16] = { 0 };
    UINT16 index = 0;
    UINT8 *p = NULL;

    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }
    p = tag->p_data;

    if (NULL == p)
    {
        return IR_DECODE_FAILED;
    }

    while (*(p++) != ',')
    {
        index++;
    }
    ir_memcpy(low, tag->p_data, index);
    ir_memcpy(high, tag->p_data + index + 1, (size_t) (tag->len - index - 1));

    context->one.low = (UINT16) (atoi((char *) low));
    context->one.high = (UINT16) (atoi((char *) high));

    return IR_DECODE_SUCCEEDED;
}

INT8 parse_delay_code_data(UINT8 *pdata)
{
    UINT8 buf[16] = { 0 };
    UINT8 *p = NULL;
    UINT16 pos = 0;
    UINT16 cnt = 0, index = 0;

    if (NULL == pdata)
    {
        return IR_DECODE_FAILED;
    }
    p = pdata;

    while (index <= ir_strlen((char *) pdata))
    {
        while ((index != ir_strlen((char *) pdata)) && (*(p++) != ','))
        {
            index++;
        }
        ir_memcpy(buf, pdata + pos, index - pos);
        pos = (UINT16) (index + 1);
        index = pos;
        context->dc[context->dc_cnt].time[cnt++] = (UINT16) (atoi((char *) buf));
        context->dc[context->dc_cnt].time_cnt = cnt;
        ir_memset(buf, 0, 16);
    }

    return IR_DECODE_SUCCEEDED;
}

INT8 parse_delay_code_pos(UINT8 *buf)
{
    UINT16 i = 0;
    UINT8 data[64] = { 0 }, start[8] = { 0 };

    if (NULL == buf)
    {
        return IR_DECODE_FAILED;
    }

    for (i = 0; i < ir_strlen((char *) buf); i++)
    {
        if (buf[i] == '&')
        {
            ir_memcpy(start, buf, i);
            ir_memcpy(data, buf + i + 1, ir_strlen((char *) buf) - i - 1);
            break;
        }
    }
    parse_delay_code_data(data);
    context->dc[context->dc_cnt].pos = (UINT16) (atoi((char *) start));

    context->dc_cnt++;
    return IR_DECODE_SUCCEEDED;
}

INT8 parse_delay_code(struct tag_head *tag)
{
    UINT8 buf[64] = { 0 };
    UINT16 i = 0;
    UINT16 preindex = 0;
    preindex = 0;

    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }

    for (i = 0; i < tag->len; i++)
    {
        if (tag->p_data[i] == '|')
        {
            ir_memcpy(buf, tag->p_data + preindex, i - preindex);
            preindex = (UINT16) (i + 1);
            parse_delay_code_pos(buf);
            ir_memset(buf, 0, 64);
        }

    }
    ir_memcpy(buf, tag->p_data + preindex, i - preindex);
    parse_delay_code_pos(buf);
    ir_memset(buf, 0, 64);

    return IR_DECODE_SUCCEEDED;
}

INT8 parse_frame_len(struct tag_head *tag, UINT16 len)
{
    UINT8 *temp = NULL;

    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }

    temp = (UINT8 *) ir_malloc(len + 1);

    if (NULL == temp)
    {
        return IR_DECODE_FAILED;
    }

    ir_memset(temp, 0x00, len + 1);

    ir_memcpy(temp, tag->p_data, len);
    temp[len] = '\0';

    context->frame_length = (UINT16) (atoi((char *) temp));

    ir_free(temp);
    return IR_DECODE_SUCCEEDED;
}

INT8 parse_endian(struct tag_head *tag)
{
    UINT8 buf[8] = { 0 };

    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }
    ir_memcpy(buf, tag->p_data, tag->len);
    context->endian = (UINT8) (atoi((char *) buf));
    return IR_DECODE_SUCCEEDED;
}

INT8 parse_lastbit(struct tag_head *tag)
{
    UINT8 buf[8] = { 0 };

    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }
    ir_memcpy(buf, tag->p_data, tag->len);
    context->last_bit = (UINT8) (atoi((char *) buf));
    return IR_DECODE_SUCCEEDED;
}

INT8 parse_repeat_times(struct tag_head *tag)
{
    char asc_code[8] = { 0 };
    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }

    ir_memcpy(asc_code, tag->p_data, tag->len);

    context->repeat_times = (UINT16) (atoi((char *) asc_code));

    return IR_DECODE_SUCCEEDED;
}

INT8 parse_delay_code_tag48_pos(UINT8 *buf)
{
    UINT16 i = 0;
    UINT8 data[64] = { 0 }, start[8] = { 0 };

    if (NULL == buf)
    {
        return IR_DECODE_FAILED;
    }

    for (i = 0; i < ir_strlen((char *) buf); i++)
    {
        if (buf[i] == '&')
        {
            ir_memcpy(start, buf, i);
            ir_memcpy(data, buf + i + 1, ir_strlen((char *) buf) - i - 1);
            break;
        }
    }

    context->bit_num[context->bit_num_cnt].pos = (UINT16) (atoi((char *) start));
    context->bit_num[context->bit_num_cnt].bits = (UINT16) (atoi((char *) data));
    context->bit_num_cnt++;
    return IR_DECODE_SUCCEEDED;
}

INT8 parse_bit_num(struct tag_head *tag)
{
    UINT16 i = 0;
    UINT16 preindex = 0;
    UINT8 buf[64] = { 0 };

    if (NULL == tag)
    {
        return IR_DECODE_FAILED;
    }

    preindex = 0;
    for (i = 0; i < tag->len; i++)
    {
        if (tag->p_data[i] == '|')
        {
            ir_memcpy(buf, tag->p_data + preindex, i - preindex);
            preindex = (UINT16) (i + 1);
            parse_delay_code_tag48_pos(buf);
            ir_memset(buf, 0, 64);
        }

    }
    ir_memcpy(buf, tag->p_data + preindex, i - preindex);
    parse_delay_code_tag48_pos(buf);
    ir_memset(buf, 0, 64);

    for (i = 0; i < context->bit_num_cnt; i++)
    {
        if (context->bit_num[i].pos == -1)
            context->bit_num[i].pos = (UINT16) (context->default_code.len - 1); //convert -1 to last data pos
    }
    return IR_DECODE_SUCCEEDED;
}
