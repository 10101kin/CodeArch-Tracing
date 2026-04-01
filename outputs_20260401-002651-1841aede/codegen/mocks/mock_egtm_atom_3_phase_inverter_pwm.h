#ifndef MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H
#define MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H

/* ── Base type aliases ─────────────────────────────────── */
#include <stdint.h>
#include <stddef.h>

typedef float  float32;
typedef double float64;
typedef uint32_t uint32;
typedef int32_t  sint32;
typedef uint16_t uint16;
typedef int16_t  sint16;
typedef uint8_t  uint8;
typedef int8_t   sint8;
typedef int      boolean;
typedef int      Ifx_Priority;
typedef uint32   Ifx_SizeT;
typedef void   (*Ifx_FuncPtr)(void);

#ifndef TRUE
#define TRUE  1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL_PTR
#define NULL_PTR ((void *)0)
#endif
#ifndef IFX_STATIC
#define IFX_STATIC static
#endif

/* ── MODULE stubs ─────────────────────────────────────── */
typedef struct { uint32 dummy; } Ifx_EGTM;
extern Ifx_EGTM EGTM_stub;
#define MODULE_EGTM (&EGTM_stub)

typedef struct { uint32 dummy; } Ifx_P;
extern Ifx_P P00_stub;
#define MODULE_P00 (&P00_stub)

extern Ifx_P P01_stub;
#define MODULE_P01 (&P01_stub)

extern Ifx_P P02_stub;
#define MODULE_P02 (&P02_stub)

extern Ifx_P P03_stub;
#define MODULE_P03 (&P03_stub)

extern Ifx_P P04_stub;
#define MODULE_P04 (&P04_stub)

extern Ifx_P P10_stub;
#define MODULE_P10 (&P10_stub)

extern Ifx_P P13_stub;
#define MODULE_P13 (&P13_stub)

extern Ifx_P P14_stub;
#define MODULE_P14 (&P14_stub)

extern Ifx_P P15_stub;
#define MODULE_P15 (&P15_stub)

extern Ifx_P P16_stub;
#define MODULE_P16 (&P16_stub)

extern Ifx_P P20_stub;
#define MODULE_P20 (&P20_stub)

extern Ifx_P P21_stub;
#define MODULE_P21 (&P21_stub)

extern Ifx_P P22_stub;
#define MODULE_P22 (&P22_stub)

extern Ifx_P P23_stub;
#define MODULE_P23 (&P23_stub)

extern Ifx_P P25_stub;
#define MODULE_P25 (&P25_stub)

extern Ifx_P P30_stub;
#define MODULE_P30 (&P30_stub)

extern Ifx_P P31_stub;
#define MODULE_P31 (&P31_stub)

extern Ifx_P P32_stub;
#define MODULE_P32 (&P32_stub)

extern Ifx_P P33_stub;
#define MODULE_P33 (&P33_stub)

extern Ifx_P P34_stub;
#define MODULE_P34 (&P34_stub)

extern Ifx_P P35_stub;
#define MODULE_P35 (&P35_stub)

extern Ifx_P P40_stub;
#define MODULE_P40 (&P40_stub)

#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* ── Spy counters ─────────────────────────────────────── */
extern unsigned int mock_IfxEgtm_Cmu_setClkFrequency_call_count;
extern unsigned int mock_IfxEgtm_Cmu_getModuleFrequency_call_count;
extern unsigned int mock_IfxEgtm_Cmu_setGclkFrequency_call_count;
extern unsigned int mock_IfxEgtm_Cmu_enableClocks_call_count;
extern unsigned int mock_IfxPort_setPinModeOutput_call_count;
extern unsigned int mock_IfxPort_togglePin_call_count;
extern unsigned int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_call_count;
extern unsigned int mock_IfxEgtm_Pwm_initConfig_call_count;
extern unsigned int mock_IfxEgtm_Pwm_init_call_count;
extern unsigned int mock_IfxEgtm_enable_call_count;
extern unsigned int mock_IfxEgtm_isEnabled_call_count;

/* ── Spy capture variables ────────────────────────────── */
extern Ifx_EGTM * mock_IfxEgtm_Cmu_setClkFrequency_last_egtm;
extern IfxEgtm_Cmu_Clk mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex;
extern uint32 mock_IfxEgtm_Cmu_setClkFrequency_last_count;
extern Ifx_EGTM * mock_IfxEgtm_Cmu_getModuleFrequency_last_egtm;
extern Ifx_EGTM * mock_IfxEgtm_Cmu_setGclkFrequency_last_egtm;
extern uint32 mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator;
extern uint32 mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator;
extern Ifx_EGTM * mock_IfxEgtm_Cmu_enableClocks_last_egtm;
extern uint32 mock_IfxEgtm_Cmu_enableClocks_last_clkMask;
extern Ifx_P * mock_IfxPort_setPinModeOutput_last_port;
extern uint8 mock_IfxPort_setPinModeOutput_last_pinIndex;
extern IfxPort_OutputMode mock_IfxPort_setPinModeOutput_last_mode;
extern IfxPort_OutputIdx mock_IfxPort_setPinModeOutput_last_index;
extern Ifx_P * mock_IfxPort_togglePin_last_port;
extern uint8 mock_IfxPort_togglePin_last_pinIndex;
extern IfxEgtm_Pwm * mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_pwm;
extern float32 * mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_requestDuty;
extern IfxEgtm_Pwm_Config * mock_IfxEgtm_Pwm_initConfig_last_config;
extern Ifx_EGTM * mock_IfxEgtm_Pwm_initConfig_last_egtmSFR;
extern IfxEgtm_Pwm * mock_IfxEgtm_Pwm_init_last_pwm;
extern IfxEgtm_Pwm_Channel * mock_IfxEgtm_Pwm_init_last_channels;
extern IfxEgtm_Pwm_Config * mock_IfxEgtm_Pwm_init_last_config;
extern Ifx_EGTM * mock_IfxEgtm_enable_last_egtm;
extern Ifx_EGTM * mock_IfxEgtm_isEnabled_last_egtm;

/* ── Return-value overrides ───────────────────────────── */
extern float32 mock_IfxEgtm_Cmu_getModuleFrequency_return_value;
extern boolean mock_IfxEgtm_isEnabled_return_value;

void mock_reset_all(void);

#endif /* MOCK_EGTM_ATOM_3_PHASE_INVERTER_PWM_H */
