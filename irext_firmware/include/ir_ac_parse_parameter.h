/**************************************************************************************
Filename:       ir_parse_ac_parameter.h
Revised:        Date: 2016-10-12
Revision:       Revision: 1.0

Description:    This file provides algorithms for IR decode for AC functionality parameters

Revision log:
* 2016-10-12: created by strawmanbobi
**************************************************************************************/

#ifndef _IRDA_PARSE_AC_PARAMETER_H_
#define _IRDA_PARSE_AC_PARAMETER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "ir_decode.h"

extern INT8 parse_common_ac_parameter(t_tag_head *tag, t_tag_comp *comp_data, UINT8 with_end, UINT8 type);

extern INT8 parse_default_code(struct tag_head *tag, t_ac_hex *default_code);

extern INT8 parse_power_1(struct tag_head *tag, t_power_1 *power1);

extern INT8 parse_temp_1(struct tag_head *tag, t_temp_1 *temp1);

extern INT8 parse_mode_1(struct tag_head *tag, t_mode_1 *mode1);

extern INT8 parse_speed_1(struct tag_head *tag, t_speed_1 *speed1);

extern INT8 parse_swing_1(struct tag_head *tag, t_swing_1 *swing1, UINT16 swing_count);

extern INT8 parse_checksum(struct tag_head *tag, t_checksum *checksum);

extern INT8 parse_function_1_tag29(struct tag_head *tag, t_function_1 *function1);

extern INT8 parse_temp_2(struct tag_head *tag, t_temp_2 *temp2);

extern INT8 parse_mode_2(struct tag_head *tag, t_mode_2 *mode2);

extern INT8 parse_speed_2(struct tag_head *tag, t_speed_2 *speed2);

extern INT8 parse_swing_2(struct tag_head *tag, t_swing_2 *swing2, UINT16 swing_count);

extern INT8 parse_function_2_tag34(struct tag_head *tag, t_function_2 *function2);

extern INT8 parse_swing_info(struct tag_head *tag, t_swing_info *si);

extern INT8 parse_solo_code(struct tag_head *tag, t_solo_code *sc);

#ifdef __cplusplus
}
#endif

#endif // _IRDA_PARSE_AC_PARAMETER_H_