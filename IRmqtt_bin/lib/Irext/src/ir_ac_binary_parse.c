/**************************************************************************************
Filename:       ir_ac_binary_parse.c
Revised:        Date: 2017-01-03
Revision:       Revision: 1.0

Description:    This file provides methods for AC binary parse

Revision log:
* 2017-01-03: created by strawmanbobi
**************************************************************************************/

#include "../include/ir_ac_binary_parse.h"
#include "../include/ir_decode.h"

#define BOARD_ESP8266

UINT16 tag_head_offset = 0;

extern struct ir_bin_buffer *p_ir_buffer;

#if defined USE_DYNAMIC_TAG
extern struct tag_head* tags;
#else
extern struct tag_head tags[];
#endif

UINT8 tag_count = 0;
const UINT16 tag_index[TAG_COUNT_FOR_PROTOCOL] =
{
    1, 2, 3, 4, 5, 6, 7,
    21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
    41, 42, 43, 44, 45, 46, 47, 48
};

INT8 binary_parse_offset()
{
    int i = 0;
#if defined BOARD_ESP8266
	UINT8 *phead = (UINT8 *)&p_ir_buffer->data[1];
#else
	UINT16 *phead = (UINT16 *)&p_ir_buffer->data[1];
#endif // BOARD_ESP8266

    tag_count = p_ir_buffer->data[0];
    if (TAG_COUNT_FOR_PROTOCOL != tag_count)
    {
        return IR_DECODE_FAILED;
    }

    tag_head_offset = (UINT16) ((tag_count << 1) + 1);

#if defined USE_DYNAMIC_TAG
    tags = (t_tag_head *) ir_malloc(tag_count * sizeof(t_tag_head));

    if (NULL == tags)
    {
        return IR_DECODE_FAILED;
    }
#endif

    for (i = 0; i < tag_count; i++)
    {
        tags[i].tag = tag_index[i];

#if defined BOARD_STM8 && defined COMPILER_IAR
        UINT16 offset = *(phead + i);
        tags[i].offset = (offset >> 8) | (offset << 8);
#elif defined BOARD_ESP8266
        UINT16 tmp_a = *(phead + i * 2);
        UINT16 tmp_b = *(phead + i * 2 + 1);
        tags[i].offset = tmp_b << 8 | tmp_a;
#else
        tags[i].offset = *(phead + i);
#endif

        if (tags[i].offset == TAG_INVALID)
        {
            tags[i].len = 0;
        }
    }
    return IR_DECODE_SUCCEEDED;
}

INT8 binary_parse_len()
{
    UINT16 i = 0, j = 0;
    for (i = 0; i < (tag_count - 1); i++)
    {
        if (tags[i].offset == TAG_INVALID)
        {
            continue;
        }

        for (j = (UINT16) (i + 1); j < tag_count; j++)
        {
            if (tags[j].offset != TAG_INVALID)
            {
                break;
            }
        }
        if (j < tag_count)
        {
            tags[i].len = tags[j].offset - tags[i].offset;
        }
        else
        {
            tags[i].len = p_ir_buffer->len - tags[i].offset - tag_head_offset;
            return IR_DECODE_SUCCEEDED;
        }
    }
    if (tags[tag_count - 1].offset != TAG_INVALID)
    {
        tags[tag_count - 1].len = p_ir_buffer->len - tag_head_offset - tags[tag_count - 1].offset;
    }

    return IR_DECODE_SUCCEEDED;
}

void binary_tags_info()
{
#if defined BOARD_PC && defined DEBUG
    UINT16 i = 0;
    for (i = 0; i < tag_count; i++)
    {
        if (tags[i].len == 0)
        {
            continue;
        }
        ir_printf("tag(%d).len = %d\n", tags[i].tag, tags[i].len);
    }
#endif
}

INT8 binary_parse_data()
{
    UINT16 i = 0;
    for (i = 0; i < tag_count; i++)
    {
        tags[i].p_data = p_ir_buffer->data + tags[i].offset + tag_head_offset;
    }

    return IR_DECODE_SUCCEEDED;
}