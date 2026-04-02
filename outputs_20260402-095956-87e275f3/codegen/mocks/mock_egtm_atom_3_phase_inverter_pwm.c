#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxEgtm_Trigger.h"
#include "IfxAdc.h"
#include "IfxPort.h"
#include "IfxPort_Pinmap.h"

/* ===================== MODULE_* instance definitions ===================== */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_ADC  MODULE_ADC  = {0};
Ifx_P    MODULE_P20  = {0};
Ifx_P    MODULE_P33  = {0};

/* ===================== Spy state definitions ===================== */
uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;

float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

int mock_IfxEgtm_Pwm_init_callCount = 0;
int mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int mock_IfxAdc_isTmadcResultAvailable_callCount = 0;
int mock_IfxAdc_clearTmadcResultFlag_callCount = 0;
int mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
int mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
int mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
int mock_IfxEgtm_isEnabled_callCount = 0;
int mock_IfxEgtm_enable_callCount = 0;
int mock_IfxPort_togglePin_callCount = 0;
int mock_IfxPort_setPinModeOutput_callCount = 0;

boolean mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;
boolean mock_IfxEgtm_Trigger_trigToAdc_returnValue     = FALSE;
boolean mock_IfxEgtm_isEnabled_returnValue             = FALSE;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f; /* default path returns 100 MHz */

/* Internal capture of numChannels for bounded copies */
static uint32 _captured_numChannels = 0;

/* ===================== Stub bodies ===================== */

/* IfxEgtm_Pwm */
void IfxEgtm_Pwm_init(IfxEgtm_Pwm *pwm, IfxEgtm_Pwm_Channel *channels, IfxEgtm_Pwm_Config *config)
{
    (void)pwm; (void)channels;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR) {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtmSFR)
{
    (void)egtmSFR;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR) {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = config->frequency;
        _captured_numChannels = config->numChannels;
    }
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *pwm, float32 *requestDuty)
{
    (void)pwm;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0U && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS)
             ? _captured_numChannels
             : (uint32)MOCK_MAX_CHANNELS;
    if (requestDuty != NULL_PTR) {
        for (uint32 i = 0U; i < n; ++i) {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = requestDuty[i];
        }
    }
}

/* IfxAdc */
boolean IfxAdc_isTmadcResultAvailable(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum)
{
    (void)tmadc; (void)resultRegNum;
    mock_IfxAdc_isTmadcResultAvailable_callCount++;
    return mock_IfxAdc_isTmadcResultAvailable_returnValue;
}

void IfxAdc_clearTmadcResultFlag(Ifx_ADC_TMADC *tmadc, IfxAdc_TmadcResultReg resultRegNum)
{
    (void)tmadc; (void)resultRegNum;
    mock_IfxAdc_clearTmadcResultFlag_callCount++;
}

/* IfxEgtm_Cmu */
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *egtm, uint32 clkMask)
{
    (void)egtm; (void)clkMask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}

float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f) {
        return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* default 100 MHz */
}

void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *egtm, float32 frequency)
{
    (void)egtm; (void)frequency;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount++;
}

void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *egtm, IfxEgtm_Cmu_Clk clkIndex, float32 frequency)
{
    (void)egtm; (void)clkIndex; (void)frequency;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount++;
}

/* IfxEgtm */
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_isEnabled_callCount++;
    return mock_IfxEgtm_isEnabled_returnValue;
}

void IfxEgtm_enable(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_enable_callCount++;
}

/* IfxEgtm_Trigger */
boolean IfxEgtm_Trigger_trigToAdc(IfxEgtm_Cluster egtmCluster, IfxEgtm_TrigSource egtmSource, IfxEgtm_TrigChannel Channel, IfxEgtm_Cfg_AdcTriggerSignal adcTrigSignal)
{
    (void)egtmCluster; (void)egtmSource; (void)Channel; (void)adcTrigSignal;
    mock_IfxEgtm_Trigger_trigToAdc_callCount++;
    return mock_IfxEgtm_Trigger_trigToAdc_returnValue;
}

/* IfxPort */
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex)
{
    (void)port; (void)pinIndex;
    mock_IfxPort_togglePin_callCount++;
}

void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_OutputIdx index)
{
    (void)port; (void)pinIndex; (void)mode; (void)index;
    mock_IfxPort_setPinModeOutput_callCount++;
}

/* ===================== Getters ===================== */
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxAdc_isTmadcResultAvailable_getCallCount(void) { return mock_IfxAdc_isTmadcResultAvailable_callCount; }
int mock_IfxAdc_clearTmadcResultFlag_getCallCount(void) { return mock_IfxAdc_clearTmadcResultFlag_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Trigger_trigToAdc_getCallCount(void) { return mock_IfxEgtm_Trigger_trigToAdc_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return mock_IfxPort_togglePin_callCount; }
int mock_IfxPort_setPinModeOutput_getCallCount(void) { return mock_IfxPort_setPinModeOutput_callCount; }

/* ===================== Reset ===================== */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_Pwm_init_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_init_lastFrequency   = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0U;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency   = 0.0f;
    for (uint32 i = 0U; i < (uint32)MOCK_MAX_CHANNELS; ++i) {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[i] = 0.0f;
    }
    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxAdc_isTmadcResultAvailable_callCount = 0;
    mock_IfxAdc_clearTmadcResultFlag_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Trigger_trigToAdc_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxPort_togglePin_callCount = 0;
    mock_IfxPort_setPinModeOutput_callCount = 0;
    mock_IfxAdc_isTmadcResultAvailable_returnValue = FALSE;
    mock_IfxEgtm_Trigger_trigToAdc_returnValue = FALSE;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
    _captured_numChannels = 0U;
}

/* ===================== Required Pin Symbol Definitions ===================== */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT64_P20_8_OUT  = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_1N_TOUT67_P20_11_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_1_TOUT66_P20_10_OUT  = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_2N_TOUT69_P20_13_OUT = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_2_TOUT68_P20_12_OUT  = {0};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_3_TOUT22_P33_0_OUT   = {0};
