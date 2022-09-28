/*
 * ZeroSi4463 is a simple OOK TX arduino library for Si4463.
 * The ZoroSi4463 library was developed only for LightAPRS hardware.
 * 
 * Copyright (C) 2019 HAKKI CAN (TA2NHP) <hkkcan@gmail.com> www.hakkican.com
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef Si4463_H
#define Si4463_H

#include "Arduino.h"

#define Si4463_CTS_REPLY					  0xff
// Waiting time for a valid FFh CTS reading
// the typical time is 20us
#define Si4463_CTS_TIMEOUT 					  2500
// Waiting time for packet send interrupt
// this time is depended on tx length and data rate of wireless
#define Si4463_TX_TIMEOUT 					  500
// Frequency channel
#define Si4463_FREQ_CHANNEL					  0

//Commands
#define Si4463_CMD_NOP                        0x00
#define Si4463_CMD_PART_INFO                  0x01
#define Si4463_CMD_POWER_UP                   0x02
#define Si4463_CMD_PATCH_IMAGE                0x04
#define Si4463_CMD_FUNC_INFO                  0x10
#define Si4463_CMD_SET_PROPERTY               0x11
#define Si4463_CMD_GET_PROPERTY               0x12
#define Si4463_CMD_GPIO_PIN_CFG               0x13
#define Si4463_CMD_GET_ADC_READING            0x14
#define Si4463_CMD_FIFO_INFO                  0x15
#define Si4463_CMD_PACKET_INFO                0x16
#define Si4463_CMD_IRCAL                      0x17
#define Si4463_CMD_PROTOCOL_CFG               0x18
#define Si4463_CMD_GET_INT_STATUS             0x20
#define Si4463_CMD_GET_PH_STATUS              0x21
#define Si4463_CMD_GET_MODEM_STATUS           0x22
#define Si4463_CMD_GET_CHIP_STATUS            0x23
#define Si4463_CMD_START_TX                   0x31
#define Si4463_CMD_START_RX                   0x32
#define Si4463_CMD_REQUEST_DEVICE_STATE       0x33
#define Si4463_CMD_CHANGE_STATE               0x34
#define Si4463_CMD_START_MFSK                 0x35 
#define Si4463_CMD_RX_HOP                     0x36
#define Si4463_CMD_READ_BUF                   0x44
#define Si4463_CMD_FAST_RESPONSE_A            0x50
#define Si4463_CMD_FAST_RESPONSE_B            0x51
#define Si4463_CMD_FAST_RESPONSE_C            0x53
#define Si4463_CMD_FAST_RESPONSE_D            0x57
#define Si4463_CMD_TX_FIFO_WRITE              0x66
#define Si4463_CMD_RX_FIFO_READ               0x77

// Properties
//#define Si4463_CMD_SET_PROPERTY               0x11
//#define Si4463_CMD_GET_PROPERTY               0x12
#define Si4463_PROPERTY_GLOBAL_XO_TUNE                   0x0000
#define Si4463_PROPERTY_GLOBAL_CLK_CFG                   0x0001
#define Si4463_PROPERTY_GLOBAL_LOW_BATT_THRESH           0x0002
#define Si4463_PROPERTY_GLOBAL_CONFIG                    0x0003
#define Si4463_PROPERTY_GLOBAL_WUT_CONFIG                0x0004
#define Si4463_PROPERTY_GLOBAL_WUT_M_15_8                0x0005
#define Si4463_PROPERTY_GLOBAL_WUT_M_7_0                 0x0006
#define Si4463_PROPERTY_GLOBAL_WUT_R                     0x0007
#define Si4463_PROPERTY_GLOBAL_WUT_LDC                   0x0008
#define Si4463_PROPERTY_INT_CTL_ENABLE                   0x0100
#define Si4463_PROPERTY_INT_CTL_PH_ENABLE                0x0101
#define Si4463_PROPERTY_INT_CTL_MODEM_ENABLE             0x0102
#define Si4463_PROPERTY_INT_CTL_CHIP_ENABLE              0x0103
#define Si4463_PROPERTY_FRR_CTL_A_MODE                   0x0200
#define Si4463_PROPERTY_FRR_CTL_B_MODE                   0x0201
#define Si4463_PROPERTY_FRR_CTL_C_MODE                   0x0202
#define Si4463_PROPERTY_FRR_CTL_D_MODE                   0x0203
#define Si4463_PROPERTY_PREAMBLE_TX_LENGTH               0x1000
#define Si4463_PROPERTY_PREAMBLE_CONFIG_STD_1            0x1001
#define Si4463_PROPERTY_PREAMBLE_CONFIG_NSTD             0x1002
#define Si4463_PROPERTY_PREAMBLE_CONFIG_STD_2            0x1003
#define Si4463_PROPERTY_PREAMBLE_CONFIG                  0x1004
#define Si4463_PROPERTY_PREAMBLE_PATTERN_31_24           0x1005
#define Si4463_PROPERTY_PREAMBLE_PATTERN_23_16           0x1006
#define Si4463_PROPERTY_PREAMBLE_PATTERN_15_8            0x1007
#define Si4463_PROPERTY_PREAMBLE_PATTERN_7_0             0x1008
#define Si4463_PROPERTY_SYNC_CONFIG                      0x1100
#define Si4463_PROPERTY_SYNC_BITS_31_24                  0x1101
#define Si4463_PROPERTY_SYNC_BITS_23_16                  0x1102
#define Si4463_PROPERTY_SYNC_BITS_15_8                   0x1103
#define Si4463_PROPERTY_SYNC_BITS_7_0                    0x1104
#define Si4463_PROPERTY_PKT_CRC_CONFIG                   0x1200
#define Si4463_PROPERTY_PKT_CONFIG1                      0x1206
#define Si4463_PROPERTY_PKT_LEN                          0x1208
#define Si4463_PROPERTY_PKT_LEN_FIELD_SOURCE             0x1209
#define Si4463_PROPERTY_PKT_LEN_ADJUST                   0x120a
#define Si4463_PROPERTY_PKT_TX_THRESHOLD                 0x120b
#define Si4463_PROPERTY_PKT_RX_THRESHOLD                 0x120c
#define Si4463_PROPERTY_PKT_FIELD_1_LENGTH_12_8          0x120d
#define Si4463_PROPERTY_PKT_FIELD_1_LENGTH_7_0           0x120e
#define Si4463_PROPERTY_PKT_FIELD_1_CONFIG               0x120f
#define Si4463_PROPERTY_PKT_FIELD_1_CRC_CONFIG           0x1210
#define Si4463_PROPERTY_PKT_FIELD_2_LENGTH_12_8          0x1211
#define Si4463_PROPERTY_PKT_FIELD_2_LENGTH_7_0           0x1212
#define Si4463_PROPERTY_PKT_FIELD_2_CONFIG               0x1213
#define Si4463_PROPERTY_PKT_FIELD_2_CRC_CONFIG           0x1214
#define Si4463_PROPERTY_PKT_FIELD_3_LENGTH_12_8          0x1215
#define Si4463_PROPERTY_PKT_FIELD_3_LENGTH_7_0           0x1216
#define Si4463_PROPERTY_PKT_FIELD_3_CONFIG               0x1217
#define Si4463_PROPERTY_PKT_FIELD_3_CRC_CONFIG           0x1218
#define Si4463_PROPERTY_PKT_FIELD_4_LENGTH_12_8          0x1219
#define Si4463_PROPERTY_PKT_FIELD_4_LENGTH_7_0           0x121a
#define Si4463_PROPERTY_PKT_FIELD_4_CONFIG               0x121b
#define Si4463_PROPERTY_PKT_FIELD_4_CRC_CONFIG           0x121c
#define Si4463_PROPERTY_PKT_FIELD_5_LENGTH_12_8          0x121d
#define Si4463_PROPERTY_PKT_FIELD_5_LENGTH_7_0           0x121e
#define Si4463_PROPERTY_PKT_FIELD_5_CONFIG               0x121f
#define Si4463_PROPERTY_PKT_FIELD_5_CRC_CONFIG           0x1220
#define Si4463_PROPERTY_PKT_RX_FIELD_1_LENGTH_12_8       0x1221
#define Si4463_PROPERTY_PKT_RX_FIELD_1_LENGTH_7_0        0x1222
#define Si4463_PROPERTY_PKT_RX_FIELD_1_CONFIG            0x1223
#define Si4463_PROPERTY_PKT_RX_FIELD_1_CRC_CONFIG        0x1224
#define Si4463_PROPERTY_PKT_RX_FIELD_2_LENGTH_12_8       0x1225
#define Si4463_PROPERTY_PKT_RX_FIELD_2_LENGTH_7_0        0x1226
#define Si4463_PROPERTY_PKT_RX_FIELD_2_CONFIG            0x1227
#define Si4463_PROPERTY_PKT_RX_FIELD_2_CRC_CONFIG        0x1228
#define Si4463_PROPERTY_PKT_RX_FIELD_3_LENGTH_12_8       0x1229
#define Si4463_PROPERTY_PKT_RX_FIELD_3_LENGTH_7_0        0x122a
#define Si4463_PROPERTY_PKT_RX_FIELD_3_CONFIG            0x122b
#define Si4463_PROPERTY_PKT_RX_FIELD_3_CRC_CONFIG        0x122c
#define Si4463_PROPERTY_PKT_RX_FIELD_4_LENGTH_12_8       0x122d
#define Si4463_PROPERTY_PKT_RX_FIELD_4_LENGTH_7_0        0x122e
#define Si4463_PROPERTY_PKT_RX_FIELD_4_CONFIG            0x122f
#define Si4463_PROPERTY_PKT_RX_FIELD_4_CRC_CONFIG        0x1230
#define Si4463_PROPERTY_PKT_RX_FIELD_5_LENGTH_12_8       0x1231
#define Si4463_PROPERTY_PKT_RX_FIELD_5_LENGTH_7_0        0x1232
#define Si4463_PROPERTY_PKT_RX_FIELD_5_CONFIG            0x1233
#define Si4463_PROPERTY_PKT_RX_FIELD_5_CRC_CONFIG        0x1234
#define Si4463_PROPERTY_MODEM_MOD_TYPE                   0x2000
#define Si4463_PROPERTY_MODEM_MAP_CONTROL                0x2001
#define Si4463_PROPERTY_MODEM_DSM_CTRL                   0x2002
#define Si4463_PROPERTY_MODEM_DATA_RATE_2                0x2003
#define Si4463_PROPERTY_MODEM_DATA_RATE_1                0x2004
#define Si4463_PROPERTY_MODEM_DATA_RATE_0                0x2005
#define Si4463_PROPERTY_MODEM_TX_NCO_MODE_3              0x2006
#define Si4463_PROPERTY_MODEM_TX_NCO_MODE_2              0x2007
#define Si4463_PROPERTY_MODEM_TX_NCO_MODE_1              0x2008
#define Si4463_PROPERTY_MODEM_TX_NCO_MODE_0              0x2009
#define Si4463_PROPERTY_MODEM_FREQ_DEV_2                 0x200a
#define Si4463_PROPERTY_MODEM_FREQ_DEV_1                 0x200b
#define Si4463_PROPERTY_MODEM_FREQ_DEV_0                 0x200c
#define Si4463_PROPERTY_MODEM_TX_FILTER_COEFF            0x200f
#define Si4463_PROPERTY_MODEM_TX_RAMP_DELAY              0x2018
#define Si4463_PROPERTY_MODEM_MDM_CTRL                   0x2019
#define Si4463_PROPERTY_MODEM_IF_CONTROL                 0x201a
#define Si4463_PROPERTY_MODEM_IF_FREQ_2                  0x201b
#define Si4463_PROPERTY_MODEM_IF_FREQ_1                  0x201c
#define Si4463_PROPERTY_MODEM_IF_FREQ_0                  0x201d
#define Si4463_PROPERTY_MODEM_DECIMATION_CFG1            0x201e
#define Si4463_PROPERTY_MODEM_DECIMATION_CFG0            0x201f
#define Si4463_PROPERTY_MODEM_BCR_OSR_1                  0x2022
#define Si4463_PROPERTY_MODEM_BCR_OSR_0                  0x2023
#define Si4463_PROPERTY_MODEM_BCR_NCO_OFFSET_2           0x2024
#define Si4463_PROPERTY_MODEM_BCR_NCO_OFFSET_1           0x2025
#define Si4463_PROPERTY_MODEM_BCR_NCO_OFFSET_0           0x2026
#define Si4463_PROPERTY_MODEM_BCR_GAIN_1                 0x2027
#define Si4463_PROPERTY_MODEM_BCR_GAIN_0                 0x2028
#define Si4463_PROPERTY_MODEM_BCR_GEAR                   0x2029
#define Si4463_PROPERTY_MODEM_BCR_MISC1                  0x202a
#define Si4463_PROPERTY_MODEM_AFC_GEAR                   0x202c
#define Si4463_PROPERTY_MODEM_AFC_WAIT                   0x202d
#define Si4463_PROPERTY_MODEM_AFC_GAIN_1                 0x202e
#define Si4463_PROPERTY_MODEM_AFC_GAIN_0                 0x202f
#define Si4463_PROPERTY_MODEM_AFC_LIMITER_1              0x2030
#define Si4463_PROPERTY_MODEM_AFC_LIMITER_0              0x2031
#define Si4463_PROPERTY_MODEM_AFC_MISC                   0x2032
#define Si4463_PROPERTY_MODEM_AGC_CONTROL                0x2035
#define Si4463_PROPERTY_MODEM_AGC_WINDOW_SIZE            0x2038
#define Si4463_PROPERTY_MODEM_AGC_RFPD_DECAY             0x2039
#define Si4463_PROPERTY_MODEM_AGC_IFPD_DECAY             0x203a
#define Si4463_PROPERTY_MODEM_FSK4_GAIN1                 0x203b
#define Si4463_PROPERTY_MODEM_FSK4_GAIN0                 0x203c
#define Si4463_PROPERTY_MODEM_FSK4_TH1                   0x203d
#define Si4463_PROPERTY_MODEM_FSK4_TH0                   0x203e
#define Si4463_PROPERTY_MODEM_FSK4_MAP                   0x203f
#define Si4463_PROPERTY_MODEM_OOK_PDTC                   0x2040
#define Si4463_PROPERTY_MODEM_OOK_CNT1                   0x2042
#define Si4463_PROPERTY_MODEM_OOK_MISC                   0x2043
#define Si4463_PROPERTY_MODEM_RAW_SEARCH                 0x2044
#define Si4463_PROPERTY_MODEM_RAW_CONTROL                0x2045
#define Si4463_PROPERTY_MODEM_RAW_EYE_1                  0x2046
#define Si4463_PROPERTY_MODEM_RAW_EYE_0                  0x2047
#define Si4463_PROPERTY_MODEM_ANT_DIV_MODE               0x2048
#define Si4463_PROPERTY_MODEM_ANT_DIV_CONTROL            0x2049
#define Si4463_PROPERTY_MODEM_RSSI_THRESH                0x204a
#define Si4463_PROPERTY_MODEM_RSSI_JUMP_THRESH           0x204b
#define Si4463_PROPERTY_MODEM_RSSI_CONTROL               0x204c
#define Si4463_PROPERTY_MODEM_RSSI_CONTROL2              0x204d
#define Si4463_PROPERTY_MODEM_RSSI_COMP                  0x204e
#define Si4463_PROPERTY_MODEM_ANT_DIV_CONT               0x2049
#define Si4463_PROPERTY_MODEM_CLKGEN_BAND                0x2051
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE13_7_0  0x2100
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE12_7_0  0x2101
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE11_7_0  0x2102
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE10_7_0  0x2103
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE9_7_0   0x2104
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE8_7_0   0x2105
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE7_7_0   0x2106
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE6_7_0   0x2107
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE5_7_0   0x2108
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE4_7_0   0x2109
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE3_7_0   0x210a
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE2_7_0   0x210b
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE1_7_0   0x210c
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COE0_7_0   0x210d
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COEM0      0x210e
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COEM1      0x210f
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COEM2      0x2110
#define Si4463_PROPERTY_MODEM_CHFLT_RX1_CHFLT_COEM3      0x2111
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE13_7_0  0x2112
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE12_7_0  0x2113
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE11_7_0  0x2114
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE10_7_0  0x2115
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE9_7_0   0x2116
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE8_7_0   0x2117
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE7_7_0   0x2118
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE6_7_0   0x2119
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE5_7_0   0x211a
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE4_7_0   0x211b
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE3_7_0   0x211c
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE2_7_0   0x211d
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE1_7_0   0x211e
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COE0_7_0   0x211f
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COEM0      0x2120
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COEM1      0x2121
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COEM2      0x2122
#define Si4463_PROPERTY_MODEM_CHFLT_RX2_CHFLT_COEM3      0x2123
#define Si4463_PROPERTY_PA_MODE                          0x2200
#define Si4463_PROPERTY_PA_PWR_LVL                       0x2201
#define Si4463_PROPERTY_PA_BIAS_CLKDUTY                  0x2202
#define Si4463_PROPERTY_PA_TC                            0x2203
#define Si4463_PROPERTY_SYNTH_PFDCP_CPFF                 0x2300
#define Si4463_PROPERTY_SYNTH_PFDCP_CPINT                0x2301
#define Si4463_PROPERTY_SYNTH_VCO_KV                     0x2302
#define Si4463_PROPERTY_SYNTH_LPFILT3                    0x2303
#define Si4463_PROPERTY_SYNTH_LPFILT2                    0x2304
#define Si4463_PROPERTY_SYNTH_LPFILT1                    0x2305
#define Si4463_PROPERTY_SYNTH_LPFILT0                    0x2306
#define Si4463_PROPERTY_MATCH_VALUE_1                    0x3000
#define Si4463_PROPERTY_MATCH_MASK_1                     0x3001
#define Si4463_PROPERTY_MATCH_CTRL_1                     0x3002
#define Si4463_PROPERTY_MATCH_VALUE_2                    0x3003
#define Si4463_PROPERTY_MATCH_MASK_2                     0x3004
#define Si4463_PROPERTY_MATCH_CTRL_2                     0x3005
#define Si4463_PROPERTY_MATCH_VALUE_3                    0x3006
#define Si4463_PROPERTY_MATCH_MASK_3                     0x3007
#define Si4463_PROPERTY_MATCH_CTRL_3                     0x3008
#define Si4463_PROPERTY_MATCH_VALUE_4                    0x3009
#define Si4463_PROPERTY_MATCH_MASK_4                     0x300a
#define Si4463_PROPERTY_MATCH_CTRL_4                     0x300b
#define Si4463_PROPERTY_FREQ_CONTROL_INTE                0x4000
#define Si4463_PROPERTY_FREQ_CONTROL_FRAC_2              0x4001
#define Si4463_PROPERTY_FREQ_CONTROL_FRAC_1              0x4002
#define Si4463_PROPERTY_FREQ_CONTROL_FRAC_0              0x4003
#define Si4463_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_1 0x4004
#define Si4463_PROPERTY_FREQ_CONTROL_CHANNEL_STEP_SIZE_0 0x4005
#define Si4463_PROPERTY_FREQ_CONTROL_VCOCNT_RX_ADJ       0x4007
#define Si4463_PROPERTY_RX_HOP_CONTROL                   0x5000
#define Si4463_PROPERTY_RX_HOP_TABLE_SIZE                0x5001
#define Si4463_PROPERTY_RX_HOP_TABLE_ENTRY_0             0x5002

//#define Si4463_CMD_GPIO_PIN_CFG               0x13
#define Si4463_GPIO_NO_CHANGE                   0
#define Si4463_GPIO_DISABLED                    1
#define Si4463_GPIO_LOW                         2
#define Si4463_GPIO_HIGH                        3
#define Si4463_GPIO_INPUT                       4
#define Si4463_GPIO_32_KHZ_CLOCK                5
#define Si4463_GPIO_BOOT_CLOCK                  6
#define Si4463_GPIO_DIVIDED_MCU_CLOCK           7
#define Si4463_GPIO_CTS                         8
#define Si4463_GPIO_INV_CTS                     9
#define Si4463_GPIO_HIGH_ON_CMD_OVERLAP         10
#define Si4463_GPIO_SPI_DATA_OUT                11
#define Si4463_GPIO_HIGH_AFTER_RESET            12
#define Si4463_GPIO_HIGH_AFTER_CALIBRATION      13
#define Si4463_GPIO_HIGH_AFTER_WUT              14
#define Si4463_GPIO_UNUSED_0                    15
#define Si4463_GPIO_TX_DATA_CLOCK               16
#define Si4463_GPIO_RX_DATA_CLOCK               17
#define Si4463_GPIO_UNUSED_1                    18
#define Si4463_GPIO_TX_DATA                     19
#define Si4463_GPIO_RX_DATA                     20
#define Si4463_GPIO_RX_RAW_DATA                 21
#define Si4463_GPIO_ANTENNA_1_SWITCH            22
#define Si4463_GPIO_ANTENNA_2_SWITCH            23
#define Si4463_GPIO_VALID_PREAMBLE              24
#define Si4463_GPIO_INVALID_PREAMBLE            25
#define Si4463_GPIO_SYNC_DETECTED               26
#define Si4463_GPIO_RSSI_ABOVE_CAT              27
#define Si4463_GPIO_TX_STATE                    32
#define Si4463_GPIO_RX_STATE                    33
#define Si4463_GPIO_RX_FIFO_ALMOST_FULL         34
#define Si4463_GPIO_TX_FIFO_ALMOST_EMPTY        35
#define Si4463_GPIO_BATT_LOW                    36
#define Si4463_GPIO_RSSI_ABOVE_CAT_LOW          37
#define Si4463_GPIO_HOP                         38
#define Si4463_GPIO_HOP_TABLE_WRAPPED           39
#define Si4463_NIRQ_INTERRUPT_SIGNAL			39

// #define Si4463_CMD_GET_INT_STATUS             0x20
#define Si4463_INT_STATUS_CHIP_INT_STATUS                0x04
#define Si4463_INT_STATUS_MODEM_INT_STATUS               0x02
#define Si4463_INT_STATUS_PH_INT_STATUS                  0x01
#define Si4463_INT_STATUS_FILTER_MATCH                   0x80
#define Si4463_INT_STATUS_FILTER_MISS                    0x40
#define Si4463_INT_STATUS_PACKET_SENT                    0x20
#define Si4463_INT_STATUS_PACKET_RX                      0x10
#define Si4463_INT_STATUS_CRC_ERROR                      0x08
#define Si4463_INT_STATUS_TX_FIFO_ALMOST_EMPTY           0x02
#define Si4463_INT_STATUS_RX_FIFO_ALMOST_FULL            0x01
#define Si4463_INT_STATUS_INVALID_SYNC                   0x20
#define Si4463_INT_STATUS_RSSI_JUMP                      0x10
#define Si4463_INT_STATUS_RSSI                           0x08
#define Si4463_INT_STATUS_INVALID_PREAMBLE               0x04
#define Si4463_INT_STATUS_PREAMBLE_DETECT                0x02
#define Si4463_INT_STATUS_SYNC_DETECT                    0x01
#define Si4463_INT_STATUS_CAL                            0x40
#define Si4463_INT_STATUS_FIFO_UNDERFLOW_OVERFLOW_ERROR  0x20
#define Si4463_INT_STATUS_STATE_CHANGE                   0x10
#define Si4463_INT_STATUS_CMD_ERROR                      0x08
#define Si4463_INT_STATUS_CHIP_READY                     0x04
#define Si4463_INT_STATUS_LOW_BATT                       0x02
#define Si4463_INT_STATUS_WUT                            0x01

//#define Si4463_PROPERTY_INT_CTL_ENABLE                   0x0100
#define Si4463_CHIP_INT_STATUS_EN                        0x04
#define Si4463_MODEM_INT_STATUS_EN                       0x02
#define Si4463_PH_INT_STATUS_EN                          0x01

//#define Si4463_PROPERTY_FRR_CTL_A_MODE                   0x0200
//#define Si4463_PROPERTY_FRR_CTL_B_MODE                   0x0201
//#define Si4463_PROPERTY_FRR_CTL_C_MODE                   0x0202
//#define Si4463_PROPERTY_FRR_CTL_D_MODE                   0x0203
#define Si4463_FRR_MODE_DISABLED                         0
#define Si4463_FRR_MODE_GLOBAL_STATUS                    1
#define Si4463_FRR_MODE_GLOBAL_INTERRUPT_PENDING         2
#define Si4463_FRR_MODE_PACKET_HANDLER_STATUS            3
#define Si4463_FRR_MODE_PACKET_HANDLER_INTERRUPT_PENDING 4
#define Si4463_FRR_MODE_MODEM_STATUS                     5
#define Si4463_FRR_MODE_MODEM_INTERRUPT_PENDING          6
#define Si4463_FRR_MODE_CHIP_STATUS                      7
#define Si4463_FRR_MODE_CHIP_INTERRUPT_PENDING           8
#define Si4463_FRR_MODE_CURRENT_STATE                    9
#define Si4463_FRR_MODE_LATCHED_RSSI                     10

//#define Si4463_PROPERTY_PREAMBLE_CONFIG                  0x1004
#define Si4463_PREAMBLE_STANDARD_PREAM                   0x00
#define Si4463_PREAMBLE_DSA_ONLY                         0x80
#define Si4463_PREAMBLE_FIRST_1                          0x20
#define Si4463_PREAMBLE_FIRST_0                          0x00
#define Si4463_PREAMBLE_LENGTH_NIBBLES                   0x00
#define Si4463_PREAMBLE_LENGTH_BYTES                     0x10
#define Si4463_PREAMBLE_MAN_CONST                        0x08
#define Si4463_PREAMBLE_MAN_ENABLE                       0x02
#define Si4463_PREAMBLE_NON_STANDARD                     0x00
#define Si4463_PREAMBLE_STANDARD_1010                    0x01
#define Si4463_PREAMBLE_STANDARD_0101                    0x02

//#define Si4463_PROPERTY_SYNC_CONFIG                      0x1100
#define Si4463_SYNC_CONFIG_SKIP_TX                       0x80
#define Si4463_SYNC_CONFIG_RX_ERRORS_MASK                0x70
#define Si4463_SYNC_CONFIG_4FSK                          0x08
#define Si4463_SYNC_CONFIG_MANCH                         0x04
#define Si4463_SYNC_CONFIG_LENGTH_MASK                   0x03

//#define Si4463_PROPERTY_PKT_CRC_CONFIG                   0x1200
#define Si4463_CRC_SEED_ALL_0S                           0x00
#define Si4463_CRC_SEED_ALL_1S                           0x80
#define Si4463_CRC_MASK                                  0x0f
#define Si4463_CRC_NONE                                  0x00
#define Si4463_CRC_ITU_T                                 0x01
#define Si4463_CRC_IEC_16                                0x02
#define Si4463_CRC_BIACHEVA                              0x03
#define Si4463_CRC_16_IBM                                0x04
#define Si4463_CRC_CCITT                                 0x05
#define Si4463_CRC_KOOPMAN                               0x06
#define Si4463_CRC_IEEE_802_3                            0x07
#define Si4463_CRC_CASTAGNOLI                            0x08

//#define Si4463_PROPERTY_PKT_CONFIG1                      0x1206
#define Si4463_PH_FIELD_SPLIT                            0x80
#define Si4463_PH_RX_DISABLE                             0x40
#define Si4463_4FSK_EN                                   0x20
#define Si4463_RX_MULTI_PKT                              0x10
#define Si4463_MANCH_POL                                 0x08
#define Si4463_CRC_INVERT                                0x04
#define Si4463_CRC_ENDIAN                                0x02
#define Si4463_BIT_ORDER                                 0x01

//#define Si4463_PROPERTY_PKT_LEN                          0x1208
#define Si4463_INFINITE_LEN  							0x40
#define Si4463_ENDIAN_LITTLE  							0x00
#define Si4463_ENDIAN_BIG  								0x20
#define Si4463_SIZE_ENUM_0 								0x00
#define Si4463_SIZE_ENUM_1 								0x01
#define Si4463_IN_FIFO 									0x08
#define Si4463_DST_FIELD_ENUM_0 						0x00
#define Si4463_DST_FIELD_ENUM_1 						0x01
#define Si4463_DST_FIELD_ENUM_2 						0x02
#define Si4463_DST_FIELD_ENUM_3 						0x03
#define Si4463_DST_FIELD_ENUM_4 						0x04
#define Si4463_DST_FIELD_ENUM_5 						0x05
#define Si4463_DST_FIELD_ENUM_6 						0x06
#define Si4463_DST_FIELD_ENUM_7 						0x07

//#define Si4463_PROPERTY_PKT_LEN_FIELD_SOURCE             0x1209
#define Si4463_SRC_FIELD_ENUM_0 						0x00
#define Si4463_SRC_FIELD_ENUM_1 						0x01
#define Si4463_SRC_FIELD_ENUM_2 						0x02
#define Si4463_SRC_FIELD_ENUM_3 						0x03
#define Si4463_SRC_FIELD_ENUM_4 						0x04
#define Si4463_SRC_FIELD_ENUM_5 						0x05

//#define Si4463_PROPERTY_PKT_FIELD_1_CONFIG               0x120f
//#define Si4463_PROPERTY_PKT_FIELD_2_CONFIG               0x1213
//#define Si4463_PROPERTY_PKT_FIELD_3_CONFIG               0x1217
//#define Si4463_PROPERTY_PKT_FIELD_4_CONFIG               0x121b
//#define Si4463_PROPERTY_PKT_FIELD_5_CONFIG               0x121f
#define Si4463_FIELD_CONFIG_4FSK                         0x10
#define Si4463_FIELD_CONFIG_PN_START                     0x04
#define Si4463_FIELD_CONFIG_WHITEN                       0x02
#define Si4463_FIELD_CONFIG_MANCH                        0x01

//#define Si4463_PROPERTY_PKT_RX_FIELD_1_CRC_CONFIG        0x1210
//#define Si4463_PROPERTY_PKT_RX_FIELD_2_CRC_CONFIG        0x1214
//#define Si4463_PROPERTY_PKT_RX_FIELD_3_CRC_CONFIG        0x1218
//#define Si4463_PROPERTY_PKT_RX_FIELD_4_CRC_CONFIG        0x121c
//#define Si4463_PROPERTY_PKT_RX_FIELD_5_CRC_CONFIG        0x1220
#define Si4463_FIELD_CONFIG_CRC_START                     0x80
#define Si4463_FIELD_CONFIG_ALT_CRC_START                 0x40
#define Si4463_FIELD_CONFIG_SEND_CRC                      0x20
#define Si4463_FIELD_CONFIG_SEND_ALT_CRC                  0x10
#define Si4463_FIELD_CONFIG_CHECK_CRC                     0x08
#define Si4463_FIELD_CONFIG_CHECK_ALT_CRC                 0x04
#define Si4463_FIELD_CONFIG_CRC_ENABLE                    0x02
#define Si4463_FIELD_CONFIG_ALT_CRC_ENABLE                0x01

//#define Si4463_PROPERTY_MODEM_MOD_TYPE                   0x2000
#define Si4463_TX_DIRECT_MODE_TYPE_SYNCHRONOUS           0x00
#define Si4463_TX_DIRECT_MODE_TYPE_ASYNCHRONOUS          0x80
#define Si4463_TX_DIRECT_MODE_GPIO0                      0x00
#define Si4463_TX_DIRECT_MODE_GPIO1                      0x20
#define Si4463_TX_DIRECT_MODE_GPIO2                      0x40
#define Si4463_TX_DIRECT_MODE_GPIO3                      0x60
#define Si4463_MOD_SOURCE_PACKET_HANDLER                 0x00
#define Si4463_MOD_SOURCE_DIRECT_MODE                    0x08
#define Si4463_MOD_SOURCE_RANDOM_GENERATOR               0x10
#define Si4463_MOD_TYPE_CW                               0x00
#define Si4463_MOD_TYPE_OOK                              0x01
#define Si4463_MOD_TYPE_2FSK                             0x02
#define Si4463_MOD_TYPE_2GFSK                            0x03
#define Si4463_MOD_TYPE_4FSK                             0x04
#define Si4463_MOD_TYPE_4GFSK                            0x05

//#define  Si4463_PROPERTY_PA_MODE                          0x2200
#define Si4463_PA_MODE_1_GROUP                           0x04
#define Si4463_PA_MODE_2_GROUPS                          0x08
#define Si4463_PA_MODE_CLASS_E                           0x00
#define Si4463_PA_MODE_SWITCH_CURRENT                    0x01

class Si4463
{
public:
	Si4463(uint8_t nIRQPin = 4, uint8_t sdnPin = 2,uint8_t nSELPin = 0);
	bool        init();
	bool		checkDevice();
	bool		txPacket(uint8_t* sendbuf,uint8_t sendLen);
	bool		waitnIRQ();
	bool		enterStandbyMode();
	bool		setGPIOMode(uint8_t GPIO0Mode,uint8_t GPIO1Mode);
	bool		setTxPower(uint8_t power);
	bool		setCommand(uint8_t length,uint8_t command,uint8_t* paraBuf);
	bool		getCommand(uint8_t length,uint8_t command,uint8_t* paraBuf);
	bool		setProperties(uint16_t startProperty, uint8_t length ,uint8_t* paraBuf);
	bool 		getProperties(uint16_t startProperty, uint8_t length ,uint8_t* paraBuf);
	bool		clrInterrupts();
	void    	enterTxMode();
	bool    	setFrequency(uint32_t freq);
	bool    	filter_coeffs(void);
	void    	setModemOOK(void);
	
protected:
	void		spiInit();
	void		pinInit();
	void		powerOnReset();
	void		setConfig(const uint8_t* parameters,uint16_t paraLen);
	void		fifoReset();
	bool		setTxInterrupt();
	bool		checkCTS();
	void		spiWriteBuf(uint8_t writeLen,uint8_t* writeBuf);
	void		spiReadBuf(uint8_t readLen,uint8_t* readBuf);
	uint8_t		spiByte(uint8_t writeData);
	
private:
	uint8_t		_nIRQPin;
	uint8_t		_sdnPin;
	uint8_t		_nSELPin;
};

#endif
