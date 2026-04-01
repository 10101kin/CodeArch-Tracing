#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include <string.h>

Ifx_EGTM EGTM_stub;
Ifx_P P00_stub;
Ifx_P P01_stub;
Ifx_P P02_stub;
Ifx_P P03_stub;
Ifx_P P04_stub;
Ifx_P P10_stub;
Ifx_P P13_stub;
Ifx_P P14_stub;
Ifx_P P15_stub;
Ifx_P P16_stub;
Ifx_P P20_stub;
Ifx_P P21_stub;
Ifx_P P22_stub;
Ifx_P P23_stub;
Ifx_P P25_stub;
Ifx_P P30_stub;
Ifx_P P31_stub;
Ifx_P P32_stub;
Ifx_P P33_stub;
Ifx_P P34_stub;
Ifx_P P35_stub;
Ifx_P P40_stub;

/* ── Spy counter definitions ──────────────────────────── */
unsigned int mock_IfxEgtm_Cmu_setClkFrequency_call_count = 0;
unsigned int mock_IfxEgtm_Cmu_getModuleFrequency_call_count = 0;
unsigned int mock_IfxEgtm_Cmu_setGclkFrequency_call_count = 0;
unsigned int mock_IfxEgtm_Cmu_enableClocks_call_count = 0;
unsigned int mock_IfxPort_setPinModeOutput_call_count = 0;
unsigned int mock_IfxPort_togglePin_call_count = 0;
unsigned int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_call_count = 0;
unsigned int mock_IfxEgtm_Pwm_initConfig_call_count = 0;
unsigned int mock_IfxEgtm_Pwm_init_call_count = 0;
unsigned int mock_IfxEgtm_enable_call_count = 0;
unsigned int mock_IfxEgtm_isEnabled_call_count = 0;

/* ── Spy capture definitions ──────────────────────────── */
Ifx_EGTM * mock_IfxEgtm_Cmu_setClkFrequency_last_egtm = NULL_PTR;
IfxEgtm_Cmu_Clk mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex = {0};
uint32 mock_IfxEgtm_Cmu_setClkFrequency_last_count = {0};
Ifx_EGTM * mock_IfxEgtm_Cmu_getModuleFrequency_last_egtm = NULL_PTR;
Ifx_EGTM * mock_IfxEgtm_Cmu_setGclkFrequency_last_egtm = NULL_PTR;
uint32 mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator = {0};
uint32 mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator = {0};
Ifx_EGTM * mock_IfxEgtm_Cmu_enableClocks_last_egtm = NULL_PTR;
uint32 mock_IfxEgtm_Cmu_enableClocks_last_clkMask = {0};
Ifx_P * mock_IfxPort_setPinModeOutput_last_port = NULL_PTR;
uint8 mock_IfxPort_setPinModeOutput_last_pinIndex = {0};
IfxPort_OutputMode mock_IfxPort_setPinModeOutput_last_mode = {0};
IfxPort_OutputIdx mock_IfxPort_setPinModeOutput_last_index = {0};
Ifx_P * mock_IfxPort_togglePin_last_port = NULL_PTR;
uint8 mock_IfxPort_togglePin_last_pinIndex = {0};
IfxEgtm_Pwm * mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_pwm = NULL_PTR;
float32 * mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_requestDuty = NULL_PTR;
IfxEgtm_Pwm_Config * mock_IfxEgtm_Pwm_initConfig_last_config = NULL_PTR;
Ifx_EGTM * mock_IfxEgtm_Pwm_initConfig_last_egtmSFR = NULL_PTR;
IfxEgtm_Pwm * mock_IfxEgtm_Pwm_init_last_pwm = NULL_PTR;
IfxEgtm_Pwm_Channel * mock_IfxEgtm_Pwm_init_last_channels = NULL_PTR;
IfxEgtm_Pwm_Config * mock_IfxEgtm_Pwm_init_last_config = NULL_PTR;
Ifx_EGTM * mock_IfxEgtm_enable_last_egtm = NULL_PTR;
Ifx_EGTM * mock_IfxEgtm_isEnabled_last_egtm = NULL_PTR;

/* ── Return-value override definitions ────────────────── */
float32 mock_IfxEgtm_Cmu_getModuleFrequency_return_value = {0};
boolean mock_IfxEgtm_isEnabled_return_value = {0};

/* ── Stub implementations ─────────────────────────────── */
void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM * egtm, IfxEgtm_Cmu_Clk clkIndex, uint32 count) {
    mock_IfxEgtm_Cmu_setClkFrequency_call_count++;
    mock_IfxEgtm_Cmu_setClkFrequency_last_egtm = egtm;
    mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex = clkIndex;
    mock_IfxEgtm_Cmu_setClkFrequency_last_count = count;
}

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM * egtm) {
    mock_IfxEgtm_Cmu_getModuleFrequency_call_count++;
    mock_IfxEgtm_Cmu_getModuleFrequency_last_egtm = egtm;
    return mock_IfxEgtm_Cmu_getModuleFrequency_return_value;
}

void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM * egtm, uint32 numerator, uint32 denominator) {
    mock_IfxEgtm_Cmu_setGclkFrequency_call_count++;
    mock_IfxEgtm_Cmu_setGclkFrequency_last_egtm = egtm;
    mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator = numerator;
    mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator = denominator;
}

void IfxEgtm_Cmu_enableClocks(Ifx_EGTM * egtm, uint32 clkMask) {
    mock_IfxEgtm_Cmu_enableClocks_call_count++;
    mock_IfxEgtm_Cmu_enableClocks_last_egtm = egtm;
    mock_IfxEgtm_Cmu_enableClocks_last_clkMask = clkMask;
}

void IfxPort_setPinModeOutput(Ifx_P * port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index) {
    mock_IfxPort_setPinModeOutput_call_count++;
    mock_IfxPort_setPinModeOutput_last_port = port;
    mock_IfxPort_setPinModeOutput_last_pinIndex = pinIndex;
    mock_IfxPort_setPinModeOutput_last_mode = mode;
    mock_IfxPort_setPinModeOutput_last_index = index;
}

void IfxPort_togglePin(Ifx_P * port, uint8 pinIndex) {
    mock_IfxPort_togglePin_call_count++;
    mock_IfxPort_togglePin_last_port = port;
    mock_IfxPort_togglePin_last_pinIndex = pinIndex;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm * pwm, float32 * requestDuty) {
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_call_count++;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_pwm = pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_requestDuty = requestDuty;
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config * config, Ifx_EGTM * egtmSFR) {
    mock_IfxEgtm_Pwm_initConfig_call_count++;
    mock_IfxEgtm_Pwm_initConfig_last_config = config;
    mock_IfxEgtm_Pwm_initConfig_last_egtmSFR = egtmSFR;
}

void IfxEgtm_Pwm_init(IfxEgtm_Pwm * pwm, IfxEgtm_Pwm_Channel * channels, IfxEgtm_Pwm_Config * config) {
    mock_IfxEgtm_Pwm_init_call_count++;
    mock_IfxEgtm_Pwm_init_last_pwm = pwm;
    mock_IfxEgtm_Pwm_init_last_channels = channels;
    mock_IfxEgtm_Pwm_init_last_config = config;
}

void IfxEgtm_enable(Ifx_EGTM * egtm) {
    mock_IfxEgtm_enable_call_count++;
    mock_IfxEgtm_enable_last_egtm = egtm;
}

boolean IfxEgtm_isEnabled(Ifx_EGTM * egtm) {
    mock_IfxEgtm_isEnabled_call_count++;
    mock_IfxEgtm_isEnabled_last_egtm = egtm;
    return mock_IfxEgtm_isEnabled_return_value;
}

void mock_reset_all(void) {
    mock_IfxEgtm_Cmu_setClkFrequency_call_count = 0;
    memset(&mock_IfxEgtm_Cmu_setClkFrequency_last_egtm, 0, sizeof(mock_IfxEgtm_Cmu_setClkFrequency_last_egtm));
    memset(&mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex, 0, sizeof(mock_IfxEgtm_Cmu_setClkFrequency_last_clkIndex));
    memset(&mock_IfxEgtm_Cmu_setClkFrequency_last_count, 0, sizeof(mock_IfxEgtm_Cmu_setClkFrequency_last_count));
    mock_IfxEgtm_Cmu_getModuleFrequency_call_count = 0;
    memset(&mock_IfxEgtm_Cmu_getModuleFrequency_last_egtm, 0, sizeof(mock_IfxEgtm_Cmu_getModuleFrequency_last_egtm));
    memset(&mock_IfxEgtm_Cmu_getModuleFrequency_return_value, 0, sizeof(mock_IfxEgtm_Cmu_getModuleFrequency_return_value));
    mock_IfxEgtm_Cmu_setGclkFrequency_call_count = 0;
    memset(&mock_IfxEgtm_Cmu_setGclkFrequency_last_egtm, 0, sizeof(mock_IfxEgtm_Cmu_setGclkFrequency_last_egtm));
    memset(&mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator, 0, sizeof(mock_IfxEgtm_Cmu_setGclkFrequency_last_numerator));
    memset(&mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator, 0, sizeof(mock_IfxEgtm_Cmu_setGclkFrequency_last_denominator));
    mock_IfxEgtm_Cmu_enableClocks_call_count = 0;
    memset(&mock_IfxEgtm_Cmu_enableClocks_last_egtm, 0, sizeof(mock_IfxEgtm_Cmu_enableClocks_last_egtm));
    memset(&mock_IfxEgtm_Cmu_enableClocks_last_clkMask, 0, sizeof(mock_IfxEgtm_Cmu_enableClocks_last_clkMask));
    mock_IfxPort_setPinModeOutput_call_count = 0;
    memset(&mock_IfxPort_setPinModeOutput_last_port, 0, sizeof(mock_IfxPort_setPinModeOutput_last_port));
    memset(&mock_IfxPort_setPinModeOutput_last_pinIndex, 0, sizeof(mock_IfxPort_setPinModeOutput_last_pinIndex));
    memset(&mock_IfxPort_setPinModeOutput_last_mode, 0, sizeof(mock_IfxPort_setPinModeOutput_last_mode));
    memset(&mock_IfxPort_setPinModeOutput_last_index, 0, sizeof(mock_IfxPort_setPinModeOutput_last_index));
    mock_IfxPort_togglePin_call_count = 0;
    memset(&mock_IfxPort_togglePin_last_port, 0, sizeof(mock_IfxPort_togglePin_last_port));
    memset(&mock_IfxPort_togglePin_last_pinIndex, 0, sizeof(mock_IfxPort_togglePin_last_pinIndex));
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_call_count = 0;
    memset(&mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_pwm, 0, sizeof(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_pwm));
    memset(&mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_requestDuty, 0, sizeof(mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_last_requestDuty));
    mock_IfxEgtm_Pwm_initConfig_call_count = 0;
    memset(&mock_IfxEgtm_Pwm_initConfig_last_config, 0, sizeof(mock_IfxEgtm_Pwm_initConfig_last_config));
    memset(&mock_IfxEgtm_Pwm_initConfig_last_egtmSFR, 0, sizeof(mock_IfxEgtm_Pwm_initConfig_last_egtmSFR));
    mock_IfxEgtm_Pwm_init_call_count = 0;
    memset(&mock_IfxEgtm_Pwm_init_last_pwm, 0, sizeof(mock_IfxEgtm_Pwm_init_last_pwm));
    memset(&mock_IfxEgtm_Pwm_init_last_channels, 0, sizeof(mock_IfxEgtm_Pwm_init_last_channels));
    memset(&mock_IfxEgtm_Pwm_init_last_config, 0, sizeof(mock_IfxEgtm_Pwm_init_last_config));
    mock_IfxEgtm_enable_call_count = 0;
    memset(&mock_IfxEgtm_enable_last_egtm, 0, sizeof(mock_IfxEgtm_enable_last_egtm));
    mock_IfxEgtm_isEnabled_call_count = 0;
    memset(&mock_IfxEgtm_isEnabled_last_egtm, 0, sizeof(mock_IfxEgtm_isEnabled_last_egtm));
    memset(&mock_IfxEgtm_isEnabled_return_value, 0, sizeof(mock_IfxEgtm_isEnabled_return_value));
}

/* ── Pin symbol allocations ──── */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT = {0};
