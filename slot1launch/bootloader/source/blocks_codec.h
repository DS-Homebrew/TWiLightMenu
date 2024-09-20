// SPDX-License-Identifier: Zlib
// SPDX-FileNotice: Modified from the original version by the BlocksDS project.
//
// Copyright (C) 2017 fincs

#ifndef LIBNDS_NDS_ARM7_CODEC_H__
#define LIBNDS_NDS_ARM7_CODEC_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ARM7
#error DSi TSC is only available on the ARM7
#endif

/// @file nds/arm7/codec.h
///
/// @brief DSi "codec" Touchscreen/Sound Controller control for ARM7

#include <nds/arm7/serial.h>
#include <nds/arm7/touch.h>
#include <nds/memory.h>
#include <nds/system.h>
#include <nds/touch.h>

// TODO: These lists are incomplete.

enum cdcControlRegister
{
    CDC_CONTROL_RESET = 0x01,
    CDC_CONTROL_CLOCK_MUX = 0x04,
    CDC_CONTROL_PLL_PR = 0x05,
    CDC_CONTROL_PLL_J = 0x06,
    CDC_CONTROL_PLL_D_16 = 0x07,
    CDC_CONTROL_DAC_NDAC = 0x0B,
    CDC_CONTROL_DAC_MDAC = 0x0C,
    CDC_CONTROL_ADC_NADC = 0x12,
    CDC_CONTROL_ADC_MADC = 0x13,
    CDC_CONTROL_CLKOUT_MUX = 0x19,
    CDC_CONTROL_DAC_CTRL = 0x3F,
    CDC_CONTROL_DAC_VOLUME = 0x40,
    CDC_CONTROL_DAC_VOLUME_LEFT = 0x41,
    CDC_CONTROL_DAC_VOLUME_RIGHT = 0x42,
    CDC_CONTROL_DAC_BEEP1 = 0x47,
    CDC_CONTROL_DAC_BEEP2 = 0x48,
    CDC_CONTROL_DAC_BEEP_LEN_24 = 0x49,
    CDC_CONTROL_DAC_BEEP_SIN_16 = 0x4C,
    CDC_CONTROL_DAC_BEEP_COS_16 = 0x4E,
    CDC_CONTROL_ADC_MIC = 0x51,
    CDC_CONTROL_ADC_VOL_FINE = 0x52,
    CDC_CONTROL_ADC_VOL_COARSE = 0x53
};

enum cdcSoundRegister
{
    CDC_SOUND_MIC_BIAS = 0x2E,
    CDC_SOUND_MIC_GAIN = 0x2F
};

enum cdcTouchCntRegister
{
    CDC_TOUCHCNT_SAR_ADC_CTRL1 = 0x02,
    CDC_TOUCHCNT_SAR_ADC_CTRL2 = 0x03,
    CDC_TOUCHCNT_PRECHARGE_SENSE = 0x04,
    CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION = 0x05,
    CDC_TOUCHCNT_STATUS = 0x09,
    CDC_TOUCHCNT_TWL_PEN_DOWN = 0x0E,
    CDC_TOUCHCNT_SCAN_MODE_TIMER = 0x0F,
    CDC_TOUCHCNT_SCAN_MODE_TIMER_CLOCK = 0x10,
    CDC_TOUCHCNT_SAR_ADC_CLOCK = 0x11,
    CDC_TOUCHCNT_DEBOUNCE_PENUP = 0x12,
    CDC_TOUCHCNT_DEBOUNCE_PENDOWN = 0x14
};

#define CDC_CONTROL_CLOCK_DISABLE                   (0)
#define CDC_CONTROL_CLOCK_ENABLE(n)                 (0x80 + (n))

// CDC_CONTROL_CLOCK_MUX register values
#define CDC_CONTROL_CLOCK_PLL_IN_MCLK               (0)
#define CDC_CONTROL_CLOCK_PLL_IN_BCLK               (1 << 2)
#define CDC_CONTROL_CLOCK_PLL_IN_GPIO1              (2 << 2)
#define CDC_CONTROL_CLOCK_PLL_IN_SDIN               (3 << 2)
#define CDC_CONTROL_CLOCK_PLL_IN_MASK               (3 << 2)
#define CDC_CONTROL_CLOCK_CODEC_IN_MCLK             (0)
#define CDC_CONTROL_CLOCK_CODEC_IN_BCLK             (1)
#define CDC_CONTROL_CLOCK_CODEC_IN_GPIO1            (2)
#define CDC_CONTROL_CLOCK_CODEC_IN_PLL              (3)
#define CDC_CONTROL_CLOCK_CODEC_IN_MASK             (3)

// CDC_TOUCHCNT_SAR_ADC_CTRL1 register values
#define CDC_TOUCHCNT_SAR_ADC_STOP                   (1 << 7)
#define CDC_TOUCHCNT_SAR_ADC_RES_12_BIT             (0)
#define CDC_TOUCHCNT_SAR_ADC_RES_8_BIT              (1 << 5)
#define CDC_TOUCHCNT_SAR_ADC_RES_10_BIT             (2 << 5)
#define CDC_TOUCHCNT_SAR_ADC_RES_MASK               (3 << 5)
#define CDC_TOUCHCNT_SAR_ADC_CLOCK_DIV_1            (0)
#define CDC_TOUCHCNT_SAR_ADC_CLOCK_DIV_2            (1 << 3)
#define CDC_TOUCHCNT_SAR_ADC_CLOCK_DIV_4            (2 << 3)
#define CDC_TOUCHCNT_SAR_ADC_CLOCK_DIV_8            (3 << 3)
#define CDC_TOUCHCNT_SAR_ADC_CLOCK_DIV_MASK         (3 << 3)
#define CDC_TOUCHCNT_SAR_ADC_FILTER_MEDIAN          (1 << 2)
#define CDC_TOUCHCNT_SAR_ADC_FILTER_AVERAGE_4       (1)
#define CDC_TOUCHCNT_SAR_ADC_FILTER_AVERAGE_8       (2)
#define CDC_TOUCHCNT_SAR_ADC_FILTER_AVERAGE_16      (3)
#define CDC_TOUCHCNT_SAR_ADC_FILTER_AVERAGE_MASK    (3)

// CDC_TOUCHCNT_SAR_ADC_CTRL2 register values
#define CDC_TOUCHCNT_SAR_ADC_CONVERSION_SELF    (1 << 7)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_NONE          (0)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_XY            (1 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_XYZ           (2 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_X             (3 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_Y             (4 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_Z             (5 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_VBAT          (6 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_AUX2          (7 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_AUX1          (8 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_AUTO          (9 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_TEMP1         (10 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_PORT          (11 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_TEMP2         (12 << 2)
#define CDC_TOUCHCNT_SAR_ADC_SCAN_MASK          (15 << 2)
#define CDC_TOUCHCNT_SAR_ADC_IRQ_PEN_LOW        (0)
#define CDC_TOUCHCNT_SAR_ADC_IRQ_DATA_LOW       (1)
#define CDC_TOUCHCNT_SAR_ADC_IRQ_PEN_HIGH       (2)
#define CDC_TOUCHCNT_SAR_ADC_IRQ_MASK           (3)

// CDC_TOUCHCNT_PRECHARGE_SENSE register values
#define CDC_TOUCHCNT_PRECHARGE_TIME_0_25US      (0)
#define CDC_TOUCHCNT_PRECHARGE_TIME_1US         (1 << 4)
#define CDC_TOUCHCNT_PRECHARGE_TIME_3US         (2 << 4)
#define CDC_TOUCHCNT_PRECHARGE_TIME_10US        (3 << 4)
#define CDC_TOUCHCNT_PRECHARGE_TIME_30US        (4 << 4)
#define CDC_TOUCHCNT_PRECHARGE_TIME_100US       (5 << 4)
#define CDC_TOUCHCNT_PRECHARGE_TIME_300US       (6 << 4)
#define CDC_TOUCHCNT_PRECHARGE_TIME_1MS         (7 << 4)
#define CDC_TOUCHCNT_PRECHARGE_TIME_MASK        (7 << 4)
#define CDC_TOUCHCNT_SENSE_TIME_1US             (0)
#define CDC_TOUCHCNT_SENSE_TIME_2US             (1)
#define CDC_TOUCHCNT_SENSE_TIME_3US             (2)
#define CDC_TOUCHCNT_SENSE_TIME_10US            (3)
#define CDC_TOUCHCNT_SENSE_TIME_30US            (4)
#define CDC_TOUCHCNT_SENSE_TIME_100US           (5)
#define CDC_TOUCHCNT_SENSE_TIME_300US           (6)
#define CDC_TOUCHCNT_SENSE_TIME_1MS             (7)
#define CDC_TOUCHCNT_SENSE_TIME_MASK            (7)

// CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION register values
#define CDC_TOUCHCNT_SAR_COMPARATOR_BIAS_100                    (0)
#define CDC_TOUCHCNT_SAR_COMPARATOR_BIAS_125                    (1 << 6)
#define CDC_TOUCHCNT_SAR_COMPARATOR_BIAS_150                    (2 << 6)
#define CDC_TOUCHCNT_SAR_COMPARATOR_BIAS_200                    (3 << 6)
#define CDC_TOUCHCNT_SAR_COMPARATOR_BIAS_MASK                   (3 << 6)
#define CDC_TOUCHCNT_SAMPLE_DURATION_DOUBLE                     (1 << 5)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_0_25US    (0)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_1US       (1)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_3US       (2)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_10US      (3)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_30US      (4)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_100US     (5)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_300US     (6)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_1MS       (7)
#define CDC_TOUCHCNT_PANEL_VOLTAGE_STABILIZATION_TIME_MASK      (7)

// CDC_TOUCHCNT_STATUS register values
#define CDC_TOUCHCNT_STATUS_PEN_DETECT          (1 << 7)
#define CDC_TOUCHCNT_STATUS_ADC_BUSY            (1 << 6)

// CDC_TOUCHCNT_TWL_PEN_DOWN register values
#define CDC_TOUCHCNT_TWL_PEN_DOWN_ENABLE        (1 << 7)
// TODO: CDC_TOUCHCNT_TWL_PEN_DOWN bits 0-6

// CDC_TOUCHCNT_SCAN_MODE_TIMER_CLOCK and CDC_TOUCHCNT_SAR_ADC_CLOCK register values
#define CDC_TOUCHCNT_CLOCK_EXTERNAL             (1 << 7)
#define CDC_TOUCHCNT_CLOCK_EXTERNAL_DIV(n)      (n)
#define CDC_TOUCHCNT_CLOCK_EXTERNAL_DIV_128     (0)
#define CDC_TOUCHCNT_CLOCK_EXTERNAL_DIV_MASK    0x7F

// CDC_TOUCHCNT_DEBOUNCE_PENUP and CDC_TOUCHCNT_DEBOUNCE_PENDOWN register values
#define CDC_TOUCHCNT_DEBOUNCE_TIME_0US          (0)
#define CDC_TOUCHCNT_DEBOUNCE_TIME_8US          (1)
#define CDC_TOUCHCNT_DEBOUNCE_TIME_16US         (2)
#define CDC_TOUCHCNT_DEBOUNCE_TIME_32US         (3)
#define CDC_TOUCHCNT_DEBOUNCE_TIME_64US         (4)
#define CDC_TOUCHCNT_DEBOUNCE_TIME_128US        (5)
#define CDC_TOUCHCNT_DEBOUNCE_TIME_256US        (6)
#define CDC_TOUCHCNT_DEBOUNCE_TIME_512US        (7)
#define CDC_TOUCHCNT_DEBOUNCE_TIME_MASK         (7)

#ifdef __cplusplus
}
#endif

#endif // LIBNDS_NDS_ARM7_CODEC_H__
