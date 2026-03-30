#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Tom_PwmHl.h"
#include "IfxGtm_PinMap.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* Spy counters and return values */
int mock_IfxGtm_Tom_Timer_applyUpdate_callCount = 0;
int mock_IfxGtm_Tom_Timer_getPeriod_callCount = 0;
int mock_IfxGtm_Tom_Timer_init_callCount = 0;
int mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
int mock_IfxGtm_Tom_Timer_disableUpdate_callCount = 0;

int mock_IfxGtm_isEnabled_callCount = 0;
int mock_IfxGtm_enable_callCount = 0;

int mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
int mock_IfxGtm_Cmu_enableClocks_callCount = 0;
int mock_IfxGtm_Cmu_enable_callCount = 0;
int mock_IfxGtm_Cmu_isEnabled_callCount = 0;
int mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
int mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;

int mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_init_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_setOnTime_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_setMode_callCount = 0;
int mock_IfxGtm_Tom_PwmHl_initConfig_callCount = 0;

int mock_IfxGtm_PinMap_setTomTout_callCount = 0;

int mock_IfxPort_togglePin_callCount = 0;
int mock_IfxPort_setPinModeOutput_callCount = 0;

uint32 mock_togglePin_callCount = 0;

boolean mock_IfxGtm_Tom_Timer_init_returnValue = FALSE;
uint32  mock_IfxGtm_Tom_Timer_getPeriod_returnValue = 0u;
boolean mock_IfxGtm_isEnabled_returnValue = FALSE;
float32 mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
boolean mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
boolean mock_IfxGtm_Tom_PwmHl_setDeadtime_returnValue = FALSE;
boolean mock_IfxGtm_Tom_PwmHl_init_returnValue = FALSE;
boolean mock_IfxGtm_Tom_PwmHl_setMode_returnValue = FALSE;

uint32  mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = 0u;
float32 mock_IfxGtm_Tom_PwmHl_init_lastFrequency = 0.0f;
uint32  mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = 0u;
float32 mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency = 0.0f;
float32 mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/* Captured number of channels for bounded copies */
static uint32 _captured_numChannels = 0u;

/* MODULE instances actually defined here */
Ifx_GTM MODULE_GTM = {0};
Ifx_GTM_TOM MODULE_GTM_TOM = {0};
Ifx_P MODULE_P02 = {0};

/* Required pin symbol allocations */
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_0N_TOUT7_P02_7_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_0_TOUT0_P02_0_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_10_TOUT2_P02_2_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_12_TOUT4_P02_4_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_13_TOUT5_P02_5_OUT = {0};
IfxGtm_Pwm_ToutMap IfxGtm_TOM1_1_TOUT1_P02_1_OUT = {0};

/* Stubs */
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver)
{
    (void)driver;
    mock_IfxGtm_Tom_Timer_applyUpdate_callCount++;
}

uint32 IfxGtm_Tom_Timer_getPeriod(IfxGtm_Tom_Timer *driver)
{
    (void)driver;
    mock_IfxGtm_Tom_Timer_getPeriod_callCount++;
    return mock_IfxGtm_Tom_Timer_getPeriod_returnValue;
}

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config)
{
    (void)driver;
    (void)config;
    mock_IfxGtm_Tom_Timer_init_callCount++;
    return mock_IfxGtm_Tom_Timer_init_returnValue;
}

void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm)
{
    (void)config;
    (void)gtm;
    mock_IfxGtm_Tom_Timer_initConfig_callCount++;
}

void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver)
{
    (void)driver;
    mock_IfxGtm_Tom_Timer_disableUpdate_callCount++;
}

boolean IfxGtm_isEnabled(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_isEnabled_callCount++;
    return mock_IfxGtm_isEnabled_returnValue;
}

void IfxGtm_enable(Ifx_GTM *gtm)
{
    (void)gtm;
    mock_IfxGtm_enable_callCount++;
}

/* CMU mandatory and mocked APIs */
void IfxGtm_Cmu_enable(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_enable_callCount++;
}

boolean IfxGtm_Cmu_isEnabled(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_isEnabled_callCount++;
    return mock_IfxGtm_Cmu_isEnabled_returnValue;
}

float32 IfxGtm_Cmu_getModuleFrequency(Ifx_GTM *module)
{
    (void)module;
    mock_IfxGtm_Cmu_getModuleFrequency_callCount++;
    return mock_IfxGtm_Cmu_getModuleFrequency_returnValue;
}

void IfxGtm_Cmu_setGclkFrequency(Ifx_GTM *module, float32 frequency)
{
    (void)module;
    (void)frequency;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount++;
}

void IfxGtm_Cmu_setClkFrequency(Ifx_GTM *module, IfxGtm_Cmu_Clk clk, float32 frequency)
{
    (void)module;
    (void)clk;
    (void)frequency;
    mock_IfxGtm_Cmu_setClkFrequency_callCount++;
}

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask)
{
    (void)gtm;
    (void)clkMask;
    mock_IfxGtm_Cmu_enableClocks_callCount++;
}

/* PWM HL APIs */
boolean IfxGtm_Tom_PwmHl_setDeadtime(IfxGtm_Tom_PwmHl *driver, float32 deadtime)
{
    (void)driver;
    mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount++;
    /* Capture into spy arrays using bounded count */
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    for (uint32 i = 0; i < n; ++i)
    {
        mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[i] = deadtime;
        mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[i] = deadtime;
    }
    return mock_IfxGtm_Tom_PwmHl_setDeadtime_returnValue;
}

boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config)
{
    (void)driver;
    mock_IfxGtm_Tom_PwmHl_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = config->numChannels;
        mock_IfxGtm_Tom_PwmHl_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
    else
    {
        mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = 0u;
        mock_IfxGtm_Tom_PwmHl_init_lastFrequency   = 0.0f;
        _captured_numChannels = 0u;
    }
    return mock_IfxGtm_Tom_PwmHl_init_returnValue;
}

void IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn)
{
    (void)driver;
    mock_IfxGtm_Tom_PwmHl_setOnTime_callCount++;
    uint32 n = (_captured_numChannels > 0u && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    for (uint32 i = 0; i < n; ++i)
    {
        /* Cast TimerValue to float32 for spy storage */
        mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[i] = (float32)tOn[i];
    }
}

boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode)
{
    (void)driver;
    (void)mode;
    mock_IfxGtm_Tom_PwmHl_setMode_callCount++;
    return mock_IfxGtm_Tom_PwmHl_setMode_returnValue;
}

void IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config)
{
    mock_IfxGtm_Tom_PwmHl_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = config->numChannels;
        mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
    else
    {
        mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = 0u;
        mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency   = 0.0f;
        _captured_numChannels = 0u;
    }
}

/* PinMap */
void IfxGtm_PinMap_setTomTout(IfxGtm_Tom_ToutMap *config, IfxPort_OutputMode outputMode, IfxPort_PadDriver padDriver)
{
    (void)config;
    (void)outputMode;
    (void)padDriver;
    mock_IfxGtm_PinMap_setTomTout_callCount++;
}

/* IfxPort */
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port;
    (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
    mock_togglePin_callCount++;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port;
    (void)pinIndex;
    (void)mode;
    (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

/* Getters */
int mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(void) { return mock_IfxGtm_Tom_Timer_applyUpdate_callCount; }
int mock_IfxGtm_Tom_Timer_getPeriod_getCallCount(void) { return mock_IfxGtm_Tom_Timer_getPeriod_callCount; }
int mock_IfxGtm_Tom_Timer_init_getCallCount(void) { return mock_IfxGtm_Tom_Timer_init_callCount; }
int mock_IfxGtm_Tom_Timer_initConfig_getCallCount(void) { return mock_IfxGtm_Tom_Timer_initConfig_callCount; }
int mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(void) { return mock_IfxGtm_Tom_Timer_disableUpdate_callCount; }

int mock_IfxGtm_isEnabled_getCallCount(void) { return mock_IfxGtm_isEnabled_callCount; }
int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }

int mock_IfxGtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
int mock_IfxGtm_Cmu_enable_getCallCount(void) { return mock_IfxGtm_Cmu_enable_callCount; }
int mock_IfxGtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxGtm_Cmu_isEnabled_callCount; }
int mock_IfxGtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxGtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxGtm_Cmu_setClkFrequency_callCount; }

int mock_IfxGtm_Tom_PwmHl_setDeadtime_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount; }
int mock_IfxGtm_Tom_PwmHl_init_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_init_callCount; }
int mock_IfxGtm_Tom_PwmHl_setOnTime_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_setOnTime_callCount; }
int mock_IfxGtm_Tom_PwmHl_setMode_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_setMode_callCount; }
int mock_IfxGtm_Tom_PwmHl_initConfig_getCallCount(void) { return mock_IfxGtm_Tom_PwmHl_initConfig_callCount; }

int mock_IfxGtm_PinMap_setTomTout_getCallCount(void) { return mock_IfxGtm_PinMap_setTomTout_callCount; }

int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }

/* Reset */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxGtm_Tom_Timer_applyUpdate_callCount = 0;
    mock_IfxGtm_Tom_Timer_getPeriod_callCount = 0;
    mock_IfxGtm_Tom_Timer_init_callCount = 0;
    mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
    mock_IfxGtm_Tom_Timer_disableUpdate_callCount = 0;

    mock_IfxGtm_isEnabled_callCount = 0;
    mock_IfxGtm_enable_callCount = 0;

    mock_IfxGtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxGtm_Cmu_enableClocks_callCount = 0;
    mock_IfxGtm_Cmu_enable_callCount = 0;
    mock_IfxGtm_Cmu_isEnabled_callCount = 0;
    mock_IfxGtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxGtm_Cmu_setClkFrequency_callCount = 0;

    mock_IfxGtm_Tom_PwmHl_setDeadtime_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_init_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_setOnTime_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_setMode_callCount = 0;
    mock_IfxGtm_Tom_PwmHl_initConfig_callCount = 0;

    mock_IfxGtm_PinMap_setTomTout_callCount = 0;

    mock_IfxPort_togglePin_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;

    mock_togglePin_callCount = 0u;

    mock_IfxGtm_Tom_Timer_init_returnValue = FALSE;
    mock_IfxGtm_Tom_Timer_getPeriod_returnValue = 0u;
    mock_IfxGtm_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    mock_IfxGtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxGtm_Tom_PwmHl_setDeadtime_returnValue = FALSE;
    mock_IfxGtm_Tom_PwmHl_init_returnValue = FALSE;
    mock_IfxGtm_Tom_PwmHl_setMode_returnValue = FALSE;

    mock_IfxGtm_Tom_PwmHl_init_lastNumChannels = 0u;
    mock_IfxGtm_Tom_PwmHl_init_lastFrequency = 0.0f;
    mock_IfxGtm_Tom_PwmHl_initConfig_lastNumChannels = 0u;
    mock_IfxGtm_Tom_PwmHl_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < (uint32)MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxGtm_Tom_PwmHl_setOnTime_lastDuties[i] = 0.0f;
        mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtRising[i] = 0.0f;
        mock_IfxGtm_Tom_PwmHl_setDeadtime_lastDtFalling[i] = 0.0f;
    }

    _captured_numChannels = 0u;
}
