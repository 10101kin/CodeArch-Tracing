#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* Spy state definitions */
int     mock_IfxEgtm_enable_callCount = 0;
int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;

int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

int     mock_IfxEgtm_Cmu_enable_callCount = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

int     mock_IfxEgtm_Pwm_init_callCount = 0;
int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;

uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_update_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_update_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_update_lastDtFalling[MOCK_MAX_CHANNELS] = {0};
uint32  mock_togglePin_callCount = 0;

/* Captured number of channels for bounded copy */
static uint32 _captured_numChannels = 0;

/* MODULE_* instances */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P MODULE_P00 = {0};
Ifx_P MODULE_P01 = {0};
Ifx_P MODULE_P02 = {0};
Ifx_P MODULE_P03 = {0};
Ifx_P MODULE_P04 = {0};
Ifx_P MODULE_P10 = {0};
Ifx_P MODULE_P13 = {0};
Ifx_P MODULE_P14 = {0};
Ifx_P MODULE_P15 = {0};
Ifx_P MODULE_P16 = {0};
Ifx_P MODULE_P20 = {0};
Ifx_P MODULE_P21 = {0};
Ifx_P MODULE_P22 = {0};
Ifx_P MODULE_P23 = {0};
Ifx_P MODULE_P25 = {0};
Ifx_P MODULE_P30 = {0};
Ifx_P MODULE_P31 = {0};
Ifx_P MODULE_P32 = {0};
Ifx_P MODULE_P33 = {0};
Ifx_P MODULE_P34 = {0};
Ifx_P MODULE_P35 = {0};
Ifx_P MODULE_P40 = {0};

/* Reset function */
void mock_egtm_atom_3_phase_inverter_pwm_reset(void)
{
    mock_IfxEgtm_enable_callCount = 0;
    mock_IfxEgtm_isEnabled_callCount = 0;
    mock_IfxEgtm_isEnabled_returnValue = FALSE;

    mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;

    mock_IfxEgtm_Cmu_enable_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
    mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount = 0;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_update_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_update_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_update_lastDtFalling[i] = 0.0f;
    }
    mock_togglePin_callCount = 0;
    _captured_numChannels = 0;
}

/* Getters */
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }
int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount; }
int mock_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }

/* ---------------- IfxEgtm stubs ---------------- */
void IfxEgtm_disable(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_enable(Ifx_EGTM *egtm) { (void)egtm; mock_IfxEgtm_enable_callCount++; }
boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm) { (void)egtm; mock_IfxEgtm_isEnabled_callCount++; return mock_IfxEgtm_isEnabled_returnValue; }
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm) { (void)egtm; return FALSE; }
void IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, IfxEgtm_SuspendMode mode) { (void)egtm; (void)mode; }
float32 IfxEgtm_getSysClkFrequency(void) { return 0.0f; }
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm, uint32 cluster) { (void)egtm; (void)cluster; return 0.0f; }
float32 IfxEgtm_tickToS(Ifx_EGTM *egtm, uint32 ticks) { (void)egtm; (void)ticks; return 0.0f; }
uint32 IfxEgtm_sToTick(Ifx_EGTM *egtm, float32 seconds) { (void)egtm; (void)seconds; return 0u; }
void IfxEgtm_initProtSe(Ifx_EGTM *egtm, void *protSeCfg) { (void)egtm; (void)protSeCfg; }
void IfxEgtm_initCtrlProt(Ifx_EGTM *egtm, void *protCfg) { (void)egtm; (void)protCfg; }
void IfxEgtm_initClApu(Ifx_EGTM *egtm, const IfxEgtm_ClApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_initCtrlApu(Ifx_EGTM *egtm, const IfxEgtm_CtrlApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_initWrapApu(Ifx_EGTM *egtm, const IfxEgtm_WrapApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster, IfxEgtm_ClusterClockDiv div) { (void)egtm; (void)cluster; (void)div; }
void IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster) { (void)egtm; (void)cluster; }
void IfxEgtm_setResetProtection(Ifx_EGTM *egtm, boolean enabled) { (void)egtm; (void)enabled; }
boolean IfxEgtm_getResetProtection(Ifx_EGTM *egtm) { (void)egtm; return FALSE; }
void IfxEgtm_setHrpwmEnable(Ifx_EGTM *egtm, boolean enabled) { (void)egtm; (void)enabled; }
void IfxEgtm_setHrpwmChannelEnable(Ifx_EGTM *egtm, uint32 channel, boolean enabled) { (void)egtm; (void)channel; (void)enabled; }
void IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_disableAeiBridgeWriteBuffer(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_setAeiBridgeOpMode(Ifx_EGTM *egtm, IfxEgtm_AeiBridgeOpMode mode) { (void)egtm; (void)mode; }
void IfxEgtm_ConnectToMsc(Ifx_EGTM *egtm, const IfxEgtm_MscOut *mscOut) { (void)egtm; (void)mscOut; }
void IfxEgtm_initApConfig(IfxEgtm_ApConfig *cfg) { (void)cfg; }
void IfxEgtm_initAp(Ifx_EGTM *egtm, const IfxEgtm_ApConfig *cfg) { (void)egtm; (void)cfg; }
void IfxEgtm_configureAccessToEgtms(boolean enable) { (void)enable; }
float32 IfxClock_geteGtmFrequency(void) { return 0.0f; }

/* ---------------- IfxEgtm_Cmu stubs ---------------- */
void IfxEgtm_Cmu_enable(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_enable_callCount++; }
boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_isEnabled_callCount++; return mock_IfxEgtm_Cmu_isEnabled_returnValue; }
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *module)
{
    (void)module;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    if (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f)
    {
        return mock_IfxEgtm_Cmu_getModuleFrequency_returnValue;
    }
    return 100000000.0f; /* default 100 MHz */
}
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *module, float32 frequency) { (void)module; (void)frequency; mock_IfxEgtm_Cmu_setGclkFrequency_callCount++; }
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *module, uint32 mask) { (void)module; (void)mask; mock_IfxEgtm_Cmu_enableClocks_callCount++; }
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *module) { (void)module; return 0.0f; }
boolean IfxEgtm_Cmu_isClkClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk) { (void)module; (void)clk; return FALSE; }
boolean IfxEgtm_Cmu_isEclkClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk) { (void)module; (void)eclk; return FALSE; }
boolean IfxEgtm_Cmu_isFxClockEnabled(Ifx_EGTM *module, IfxEgtm_Cmu_Fxclk fx) { (void)module; (void)fx; return FALSE; }
void IfxEgtm_Cmu_selectClkInput(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, uint32 input) { (void)module; (void)clk; (void)input; }
void IfxEgtm_Cmu_setClkCount(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, uint32 count) { (void)module; (void)clk; (void)count; }
void IfxEgtm_Cmu_setEclkDivider(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk, uint32 div) { (void)module; (void)eclk; (void)div; }
void IfxEgtm_Cmu_setGclkDivider(Ifx_EGTM *module, uint32 numerator, uint32 denominator) { (void)module; (void)numerator; (void)denominator; }
float32 IfxEgtm_Cmu_getClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk) { (void)module; (void)clk; return 0.0f; }
float32 IfxEgtm_Cmu_getEclkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk) { (void)module; (void)eclk; return 0.0f; }
float32 IfxEgtm_Cmu_getFxClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Fxclk fx) { (void)module; (void)fx; return 0.0f; }
void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, float32 frequency) { (void)module; (void)clk; (void)frequency; mock_IfxEgtm_Cmu_setClkFrequency_callCount++; }
void IfxEgtm_Cmu_setEclkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Eclk eclk, float32 frequency) { (void)module; (void)eclk; (void)frequency; }

/* ---------------- IfxEgtm_Pwm stubs ---------------- */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency = config->frequency;
    }
}

boolean IfxEgtm_Pwm_init(IfxEgtm_Pwm *handle, const IfxEgtm_Pwm_Config *config)
{
    (void)handle;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (config != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = config->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency = config->frequency;
        _captured_numChannels = config->numChannels;
    }
    return TRUE;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *handle, const float32 *duties)
{
    (void)handle;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS) ? _captured_numChannels : MOCK_MAX_CHANNELS;
    if (duties != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_update_lastDuties[i] = duties[i];
        }
    }
}

void IfxEgtm_Pwm_updateChannelsDeadTimeImmediate(IfxEgtm_Pwm *handle, const float32 *dtRising, const float32 *dtFalling)
{
    (void)handle;
    mock_IfxEgtm_Pwm_updateChannelsDeadTimeImmediate_callCount++;
    uint32 n = (_captured_numChannels > 0 && _captured_numChannels <= MOCK_MAX_CHANNELS) ? _captured_numChannels : MOCK_MAX_CHANNELS;
    if (dtRising != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_update_lastDtRising[i] = dtRising[i];
        }
    }
    if (dtFalling != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_update_lastDtFalling[i] = dtFalling[i];
        }
    }
}
