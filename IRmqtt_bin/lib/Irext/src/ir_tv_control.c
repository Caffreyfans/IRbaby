/**************************************************************************************
Filename:       ir_lib.c
Revised:        Date: 2016-10-21
Revision:       Revision: 1.0

Description:    This file provides algorithms for IR decode (compressed command type)

Revision log:
* 2016-10-21: created by strawmanbobi
**************************************************************************************/

#include <string.h>

#include "../include/ir_defs.h"
#include "../include/ir_decode.h"
#include "../include/ir_tv_control.h"


struct buffer
{
    UINT8 *data;
    UINT16 len;
    UINT16 offset;
} ir_file;


static struct buffer *pbuffer = &ir_file;

static UINT8 *prot_cycles_num = NULL;
static t_ir_cycles *prot_cycles_data[IRDA_MAX];
static UINT8 prot_items_cnt = 0;
static t_ir_data *prot_items_data = NULL;
static t_ir_data_tv *remote_p;
static UINT8 *remote_pdata = NULL;

static UINT16 time_index = 0;
static UINT8 ir_level = IRDA_LEVEL_LOW;
static UINT8 ir_toggle_bit = FALSE;
static UINT8 ir_decode_flag = IRDA_DECODE_1_BIT;
static UINT8 cycles_num_size = 0;


static BOOL get_ir_protocol(UINT8 encode_type);

static BOOL get_ir_keymap(void);

static void print_ir_time(t_ir_data *data, UINT8 key_index, UINT16 *ir_time);

static void process_decode_number(UINT8 keycode, t_ir_data *data, UINT8 valid_bits, UINT16 *ir_time);

static void convert_to_ir_time(UINT8 value, UINT16 *ir_time);

static void replace_with(t_ir_cycles *pcycles_num, UINT16 *ir_time);


INT8 tv_binary_open(UINT8 *binary, UINT16 binary_length)
{
    // load binary to buffer
    pbuffer->data = binary;
    pbuffer->len = binary_length;
    pbuffer->offset = 0;
    return IR_DECODE_SUCCEEDED;
}

BOOL tv_binary_parse(UINT8 encode_type)
{
    if (FALSE == get_ir_protocol(encode_type))
    {
        return FALSE;
    }

    return get_ir_keymap();
}

UINT16 tv_binary_decode(UINT8 key, UINT16 *user_data)
{
    UINT16 i = 0;

    time_index = 0;
    ir_level = IRDA_LEVEL_LOW;

    for (i = 0; i < prot_items_cnt; i++)
    {
        print_ir_time(&prot_items_data[i], key, user_data);
    }

    // next flip
    if (2 == prot_cycles_num[IRDA_FLIP])
    {
        ir_toggle_bit = (ir_toggle_bit == FALSE) ? TRUE : FALSE;
    }

    return time_index;
}


static BOOL get_ir_protocol(UINT8 encode_type)
{
    UINT8 i = 0;
    UINT8 name_size = 20;
    UINT8 *prot_cycles = NULL;
    UINT8 cycles_sum = 0;

    if (pbuffer->data == NULL)
    {
        return FALSE;
    }

    pbuffer->offset = 0;

    /* t_ac_protocol name */
    pbuffer->offset += name_size;

    /* cycles number */
    prot_cycles_num = pbuffer->data + pbuffer->offset;

    if (encode_type == 0)
    {
        cycles_num_size = 8;      /* "BOOT", "STOP", "SEP", "ONE", "ZERO", "FLIP", "TWO", "THREE" */
        if (prot_cycles_num[IRDA_TWO] == 0 && prot_cycles_num[IRDA_THREE] == 0)
        {
            ir_decode_flag = IRDA_DECODE_1_BIT;
        }
        else
        {
            ir_decode_flag = IRDA_DECODE_2_BITS;
        }
    }
    else if (encode_type == 1)
    {
        cycles_num_size = IRDA_MAX;
        ir_decode_flag = IRDA_DECODE_4_BITS;
    }
    else
    {
        return FALSE;
    }
    pbuffer->offset += cycles_num_size;

    /* cycles data */
    prot_cycles = pbuffer->data + pbuffer->offset;
    for (i = 0; i < cycles_num_size; i++)
    {
        if (0 != prot_cycles_num[i])
        {
            prot_cycles_data[i] = (t_ir_cycles *) (&prot_cycles[sizeof(t_ir_cycles) * cycles_sum]);
        }
        else
        {
            prot_cycles_data[i] = NULL;
        }
        cycles_sum += prot_cycles_num[i];
    }
    pbuffer->offset += sizeof(t_ir_cycles) * cycles_sum;

    /* items count */
    prot_items_cnt = pbuffer->data[pbuffer->offset];
    pbuffer->offset += sizeof(UINT8);

    /* items data */
    prot_items_data = (t_ir_data *) (pbuffer->data + pbuffer->offset);
    pbuffer->offset += prot_items_cnt * sizeof(t_ir_data);

    ir_toggle_bit = FALSE;

    return TRUE;
}

static BOOL get_ir_keymap(void)
{
    remote_p = (t_ir_data_tv *) (pbuffer->data + pbuffer->offset);
    pbuffer->offset += sizeof(t_ir_data_tv);

    if (strncmp(remote_p->magic, "irda", 4) == 0)
    {
        remote_pdata = pbuffer->data + pbuffer->offset;
        return TRUE;
    }

    return FALSE;
}

static void print_ir_time(t_ir_data *data, UINT8 key_index, UINT16 *ir_time)
{
    UINT8 i = 0;
    UINT8 cycles_num = 0;
    t_ir_cycles *pcycles = NULL;
    UINT8 key_code = 0;

    if (NULL == data || NULL == ir_time)
    {
        ir_printf("data or ir_time is null\n");
        return;
    }

    pcycles = prot_cycles_data[data->index];
    key_code = remote_pdata[remote_p->per_keycode_bytes * key_index + data->index - 1];

    if (prot_cycles_num[IRDA_ONE] != 1 || prot_cycles_num[IRDA_ZERO] != 1)
    {
        ir_printf("logical 1 or 0 is invalid\n");
        return;
    }

    if (time_index >= USER_DATA_SIZE)
    {
        ir_printf("time index exceeded\n");
        return;
    }

    if (data->bits == 1)
    {
        if (pcycles == NULL)
        {
            ir_printf("pcycles is null\n");
            return;
        }

        cycles_num = prot_cycles_num[data->index];
        if (cycles_num > 5)
        {
            ir_printf("cycles number exceeded\n");
            return;
        }

        for (i = cycles_num; i > 0; i--)
        {
            if (cycles_num == 2 && data->index == IRDA_FLIP)
            {
                if (ir_toggle_bit == TRUE)
                {
                    pcycles += 1;
                }
            }

            if (pcycles->mask && pcycles->space)
            {
                if (pcycles->flag == IRDA_FLAG_NORMAL)
                {
                    if (ir_level == IRDA_LEVEL_HIGH && time_index != 0)
                    {
                        time_index--;
                        ir_time[time_index++] += pcycles->mask;
                    }
                    else if (ir_level == IRDA_LEVEL_LOW)
                    {
                        ir_time[time_index++] = pcycles->mask;
                    }
                    ir_time[time_index++] = pcycles->space;
                    ir_level = IRDA_LEVEL_LOW;
                }
                else if (pcycles->flag == IRDA_FLAG_INVERSE)
                {
                    if (ir_level == IRDA_LEVEL_LOW && time_index != 0)
                    {
                        time_index--;
                        ir_time[time_index++] += pcycles->space;
                    }
                    else if (ir_level == IRDA_LEVEL_HIGH)
                    {
                        ir_time[time_index++] = pcycles->space;
                    }
                    ir_time[time_index++] = pcycles->mask;
                    ir_level = IRDA_LEVEL_HIGH;
                }
            }
            else if (0 == pcycles->mask && 0 != pcycles->space)
            {
                if (ir_level == IRDA_LEVEL_LOW && time_index != 0)
                {
                    time_index--;
                    ir_time[time_index++] += pcycles->space;
                }
                else if (ir_level == IRDA_LEVEL_HIGH)
                {
                    ir_time[time_index++] = pcycles->space;
                }
                ir_level = IRDA_LEVEL_LOW;
            }
            else if (0 == pcycles->space && 0 != pcycles->mask)
            {
                if (ir_level == IRDA_LEVEL_HIGH && time_index != 0)
                {
                    time_index--;
                    ir_time[time_index++] += pcycles->mask;
                }
                else if (ir_level == IRDA_LEVEL_LOW)
                {
                    ir_time[time_index++] = pcycles->mask;
                }
                ir_level = IRDA_LEVEL_HIGH;
            }
            else
            {
                // do nothing
            }

            if (cycles_num == 2 && data->index == IRDA_FLIP)
            {
                break;
            }
            pcycles++;
        }
    }
    else
    {
        // mode: inverse
        if (data->mode == 1)
            key_code = ~key_code;

        if (ir_decode_flag == IRDA_DECODE_1_BIT)
        {
            // for binary formatted code
            process_decode_number(key_code, data, 1, ir_time);
        }
        else if (ir_decode_flag == IRDA_DECODE_2_BITS)
        {
            // for quaternary formatted code
            process_decode_number(key_code, data, 2, ir_time);
        }
        else if (ir_decode_flag == IRDA_DECODE_4_BITS)
        {
            // for hexadecimal formatted code
            process_decode_number(key_code, data, 4, ir_time);
        }
    }
}

static void process_decode_number(UINT8 keycode, t_ir_data *data, UINT8 valid_bits, UINT16 *ir_time)
{
    UINT8 i = 0;
    UINT8 value = 0;
    UINT8 bit_num = data->bits / valid_bits;
    UINT8 valid_value = 0;

    valid_value = (UINT8) ((valid_bits == 1) ? 1 : (valid_bits * valid_bits - 1));

    if (data->lsb == IRDA_LSB)
    {
        for (i = 0; i < bit_num; i++)
        {
            value = (keycode >> (valid_bits * i)) & valid_value;
            convert_to_ir_time(value, ir_time);
        }
    }
    else if (data->lsb == IRDA_MSB)
    {
        for (i = 0; i < bit_num; i++)
        {
            value = (keycode >> (data->bits - valid_bits * (i + 1))) & valid_value;
            convert_to_ir_time(value, ir_time);
        }
    }
}

static void convert_to_ir_time(UINT8 value, UINT16 *ir_time)
{
    switch (value)
    {
        case 0:
            replace_with(prot_cycles_data[IRDA_ZERO], ir_time);
            break;
        case 1:
            replace_with(prot_cycles_data[IRDA_ONE], ir_time);
            break;
        case 2:
            replace_with(prot_cycles_data[IRDA_TWO], ir_time);
            break;
        case 3:
            replace_with(prot_cycles_data[IRDA_THREE], ir_time);
            break;
        case 4:
            replace_with(prot_cycles_data[IRDA_FOUR], ir_time);
            break;
        case 5:
            replace_with(prot_cycles_data[IRDA_FIVE], ir_time);
            break;
        case 6:
            replace_with(prot_cycles_data[IRDA_SIX], ir_time);
            break;
        case 7:
            replace_with(prot_cycles_data[IRDA_SEVEN], ir_time);
            break;
        case 8:
            replace_with(prot_cycles_data[IRDA_EIGHT], ir_time);
            break;
        case 9:
            replace_with(prot_cycles_data[IRDA_NINE], ir_time);
            break;
        case 0x0A:
            replace_with(prot_cycles_data[IRDA_A], ir_time);
            break;
        case 0x0B:
            replace_with(prot_cycles_data[IRDA_B], ir_time);
            break;
        case 0x0C:
            replace_with(prot_cycles_data[IRDA_C], ir_time);
            break;
        case 0x0D:
            replace_with(prot_cycles_data[IRDA_D], ir_time);
            break;
        case 0x0E:
            replace_with(prot_cycles_data[IRDA_E], ir_time);
            break;
        case 0x0F:
            replace_with(prot_cycles_data[IRDA_F], ir_time);
            break;
        default:
            break;
    }
}

static void replace_with(t_ir_cycles *pcycles_num, UINT16 *ir_time)
{
    if (NULL == pcycles_num || NULL == ir_time)
    {
        return;
    }

    if (pcycles_num->flag == IRDA_FLAG_NORMAL)
    {
        if (ir_level == IRDA_LEVEL_HIGH && time_index != 0)
        {
            time_index--;
            ir_time[time_index++] += pcycles_num->mask;
        }
        else if (ir_level == IRDA_LEVEL_LOW)
        {
            ir_time[time_index++] = pcycles_num->mask;
        }
        ir_time[time_index++] = pcycles_num->space;
        ir_level = IRDA_LEVEL_LOW;
    }
    else if (pcycles_num->flag == IRDA_FLAG_INVERSE)
    {
        if (ir_level == IRDA_LEVEL_LOW && time_index != 0)
        {
            time_index--;
            ir_time[time_index++] += pcycles_num->space;
        }
        else if (ir_level == IRDA_LEVEL_HIGH)
        {
            ir_time[time_index++] = pcycles_num->space;
        }
        ir_time[time_index++] = pcycles_num->mask;
        ir_level = IRDA_LEVEL_HIGH;
    }
}