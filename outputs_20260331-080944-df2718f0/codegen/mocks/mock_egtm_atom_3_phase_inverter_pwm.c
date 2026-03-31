#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxEgtm_Pwm.h"
#include "IfxPort.h"

/* MODULE instances */
Ifx_EGTM MODULE_EGTM = {0};
Ifx_P    MODULE_P02  = {0};
Ifx_P    MODULE_P03  = {0};
Ifx_P    MODULE_P20  = {0};
Ifx_P    MODULE_P21  = {0};
Ifx_SRC  MODULE_SRC  = {0};
Ifx_WTU  MODULE_WTU  = {0};

/* Pin symbol objects (ToutMap) */
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT        = {{0}};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT7_P02_7_OUT       = {{0}};
IfxEgtm_Pwm_ToutMap IfxEgtm_ATOM0_0N_TOUT65_P20_9_OUT      = {{0}};

/* Spy counters and return-value controls - definitions */
int     mock_IfxEgtm_enable_callCount = 0;
int     mock_IfxEgtm_isEnabled_callCount = 0;
boolean mock_IfxEgtm_isEnabled_returnValue = FALSE;

int     mock_IfxEgtm_Cmu_setGclkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_getModuleFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getModuleFrequency_returnValue = 0.0f;
int     mock_IfxEgtm_Cmu_enable_callCount = 0;
int     mock_IfxEgtm_Cmu_isEnabled_callCount = 0;
boolean mock_IfxEgtm_Cmu_isEnabled_returnValue = FALSE;
int     mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
float32 mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
int     mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
int     mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

int     mock_IfxEgtm_Pwm_init_callCount = 0;
int     mock_IfxEgtm_Pwm_initConfig_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
int     mock_IfxEgtm_Pwm_updateChannelsDeadTime_callCount = 0;

uint32  mock_togglePin_callCount = 0;

uint32  mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
uint32  mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
float32 mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;
float32 mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtFalling[MOCK_MAX_CHANNELS] = {0};

/* Internal capture for safe bounded copy */
static uint32 _captured_numChannels = 0;

/* Getters */
int mock_IfxEgtm_enable_getCallCount(void) { return mock_IfxEgtm_enable_callCount; }
int mock_IfxEgtm_isEnabled_getCallCount(void) { return mock_IfxEgtm_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_setGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_getModuleFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getModuleFrequency_callCount; }
int mock_IfxEgtm_Cmu_enable_getCallCount(void) { return mock_IfxEgtm_Cmu_enable_callCount; }
int mock_IfxEgtm_Cmu_isEnabled_getCallCount(void) { return mock_IfxEgtm_Cmu_isEnabled_callCount; }
int mock_IfxEgtm_Cmu_getGclkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_getGclkFrequency_callCount; }
int mock_IfxEgtm_Cmu_setClkFrequency_getCallCount(void) { return mock_IfxEgtm_Cmu_setClkFrequency_callCount; }
int mock_IfxEgtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxEgtm_Cmu_enableClocks_callCount; }

int mock_IfxEgtm_Pwm_init_getCallCount(void) { return mock_IfxEgtm_Pwm_init_callCount; }
int mock_IfxEgtm_Pwm_initConfig_getCallCount(void) { return mock_IfxEgtm_Pwm_initConfig_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount; }
int mock_IfxEgtm_Pwm_updateChannelsDeadTime_getCallCount(void) { return mock_IfxEgtm_Pwm_updateChannelsDeadTime_callCount; }
int mock_IfxPort_togglePin_getCallCount(void) { return (int)mock_togglePin_callCount; }

/* Reset */
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
    mock_IfxEgtm_Cmu_getGclkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_getGclkFrequency_returnValue = 0.0f;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount = 0;
    mock_IfxEgtm_Cmu_enableClocks_callCount = 0;

    mock_IfxEgtm_Pwm_init_callCount = 0;
    mock_IfxEgtm_Pwm_initConfig_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount = 0;
    mock_IfxEgtm_Pwm_updateChannelsDeadTime_callCount = 0;

    mock_togglePin_callCount = 0;

    mock_IfxEgtm_Pwm_init_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_init_lastFrequency = 0.0f;
    mock_IfxEgtm_Pwm_initConfig_lastNumChannels = 0;
    mock_IfxEgtm_Pwm_initConfig_lastFrequency = 0.0f;

    for (uint32 i = 0; i < MOCK_MAX_CHANNELS; ++i)
    {
        mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtRising[i] = 0.0f;
        mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtFalling[i] = 0.0f;
    }

    _captured_numChannels = 0;
}

/* ======================== IfxEgtm APIs ======================== */
void IfxEgtm_enable(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_enable_callCount++;
}

boolean IfxEgtm_isEnabled(Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_isEnabled_callCount++;
    return mock_IfxEgtm_isEnabled_returnValue;
}

void IfxEgtm_disable(Ifx_EGTM *egtm) { (void)egtm; }
boolean IfxEgtm_isModuleSuspended(Ifx_EGTM *egtm) { (void)egtm; return FALSE; }
void IfxEgtm_setSuspendMode(Ifx_EGTM *egtm, int mode) { (void)egtm; (void)mode; }
float32 IfxEgtm_getSysClkFrequency(Ifx_EGTM *egtm) { (void)egtm; return 0.0f; }
float32 IfxEgtm_getClusterFrequency(Ifx_EGTM *egtm, uint32 cluster) { (void)egtm; (void)cluster; return 0.0f; }
float32 IfxEgtm_tickToS(float32 ticks, float32 freq) { (void)freq; return ticks; }
float32 IfxEgtm_sToTick(float32 seconds, float32 freq) { (void)freq; return seconds; }
void IfxEgtm_initProtSe(void *cfg) { (void)cfg; }
void IfxEgtm_initCtrlProt(void *cfg) { (void)cfg; }
void IfxEgtm_initClApu(void *cfg) { (void)cfg; }
void IfxEgtm_initCtrlApu(void *cfg) { (void)cfg; }
void IfxEgtm_initWrapApu(void *cfg) { (void)cfg; }
void IfxEgtm_setClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster, uint32 div) { (void)egtm; (void)cluster; (void)div; }
void IfxEgtm_clearClusterClockDiv(Ifx_EGTM *egtm, uint32 cluster) { (void)egtm; (void)cluster; }
void IfxEgtm_setResetProtection(Ifx_EGTM *egtm, boolean enable) { (void)egtm; (void)enable; }
boolean IfxEgtm_getResetProtection(Ifx_EGTM *egtm) { (void)egtm; return FALSE; }
void IfxEgtm_setHrpwmEnable(Ifx_EGTM *egtm, boolean enable) { (void)egtm; (void)enable; }
void IfxEgtm_setHrpwmChannelEnable(Ifx_EGTM *egtm, uint32 channel, boolean enable) { (void)egtm; (void)channel; (void)enable; }
void IfxEgtm_enableAeiBridgeWriteBuffer(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_disableAeiBridgeWriteBuffer(Ifx_EGTM *egtm) { (void)egtm; }
void IfxEgtm_setAeiBridgeOpMode(Ifx_EGTM *egtm, uint32 mode) { (void)egtm; (void)mode; }
void IfxEgtm_ConnectToMsc(Ifx_EGTM *egtm, void *mscCfg) { (void)egtm; (void)mscCfg; }
void IfxEgtm_initApConfig(void *cfg) { (void)cfg; }
void IfxEgtm_initAp(void *cfg) { (void)cfg; }
void IfxEgtm_configureAccessToEgtms(void) { }
float32 IfxClock_geteGtmFrequency(void) { return 0.0f; }

/* ======================== IfxEgtm_Cmu APIs ======================== */
void IfxEgtm_Cmu_enable(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_enable_callCount++; }
boolean IfxEgtm_Cmu_isEnabled(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_isEnabled_callCount++; return mock_IfxEgtm_Cmu_isEnabled_returnValue; }
float32 IfxEgtm_Cmu_getModuleFrequency(Ifx_EGTM *module)
{
    (void)module;
    mock_IfxEgtm_Cmu_getModuleFrequency_callCount++;
    return (mock_IfxEgtm_Cmu_getModuleFrequency_returnValue != 0.0f) ? mock_IfxEgtm_Cmu_getModuleFrequency_returnValue : 100000000.0f; /* 100 MHz default */
}
void IfxEgtm_Cmu_setGclkFrequency(Ifx_EGTM *module, float32 frequency)
{
    (void)module; (void)frequency;
    mock_IfxEgtm_Cmu_setGclkFrequency_callCount++;
}
void IfxEgtm_Cmu_setClkFrequency(Ifx_EGTM *module, IfxEgtm_Cmu_Clk clk, float32 frequency)
{
    (void)module; (void)clk; (void)frequency;
    mock_IfxEgtm_Cmu_setClkFrequency_callCount++;
}
void IfxEgtm_Cmu_enableClocks(Ifx_EGTM *module, uint32 mask)
{
    (void)module; (void)mask;
    mock_IfxEgtm_Cmu_enableClocks_callCount++;
}
float32 IfxEgtm_Cmu_getGclkFrequency(Ifx_EGTM *module) { (void)module; mock_IfxEgtm_Cmu_getGclkFrequency_callCount++; return mock_IfxEgtm_Cmu_getGclkFrequency_returnValue; }

/* ======================== IfxEgtm_Pwm APIs ======================== */
void IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *cfg, Ifx_EGTM *egtm)
{
    (void)egtm;
    mock_IfxEgtm_Pwm_initConfig_callCount++;
    if (cfg != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_initConfig_lastNumChannels = cfg->numChannels;
        mock_IfxEgtm_Pwm_initConfig_lastFrequency   = cfg->frequency;
        _captured_numChannels = cfg->numChannels;
    }
}

boolean IfxEgtm_Pwm_init(IfxEgtm_Pwm *driver, IfxEgtm_Pwm_Config *cfg)
{
    (void)driver;
    mock_IfxEgtm_Pwm_init_callCount++;
    if (cfg != NULL_PTR)
    {
        mock_IfxEgtm_Pwm_init_lastNumChannels = cfg->numChannels;
        mock_IfxEgtm_Pwm_init_lastFrequency   = cfg->frequency;
        _captured_numChannels = cfg->numChannels;
    }
    return TRUE;
}

void IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *driver, const float32 *duties, uint32 numChannels)
{
    (void)driver;
    mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_callCount++;
    uint32 nInit = (_captured_numChannels > 0 && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    uint32 n     = (numChannels < nInit) ? numChannels : nInit;
    if (duties != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[i] = duties[i];
        }
        for (uint32 j = n; j < (uint32)MOCK_MAX_CHANNELS; ++j)
        {
            mock_IfxEgtm_Pwm_updateChannelsDutyImmediate_lastDuties[j] = 0.0f;
        }
    }
}

void IfxEgtm_Pwm_updateChannelsDeadTime(IfxEgtm_Pwm *driver, const IfxEgtm_Pwm_DeadTime *dt, uint32 numChannels)
{
    (void)driver;
    mock_IfxEgtm_Pwm_updateChannelsDeadTime_callCount++;
    uint32 nInit = (_captured_numChannels > 0 && _captured_numChannels <= (uint32)MOCK_MAX_CHANNELS) ? _captured_numChannels : (uint32)MOCK_MAX_CHANNELS;
    uint32 n     = (numChannels < nInit) ? numChannels : nInit;
    if (dt != NULL_PTR)
    {
        for (uint32 i = 0; i < n; ++i)
        {
            mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtRising[i]  = dt[i].dtRising;
            mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtFalling[i] = dt[i].dtFalling;
        }
        for (uint32 j = n; j < (uint32)MOCK_MAX_CHANNELS; ++j)
        {
            mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtRising[j]  = 0.0f;
            mock_IfxEgtm_Pwm_updateChannelsDeadTime_lastDtFalling[j] = 0.0f;
        }
    }
}

void IfxEgtm_Pwm_start(IfxEgtm_Pwm *driver) { (void)driver; }
void IfxEgtm_Pwm_stop(IfxEgtm_Pwm *driver) { (void)driver; }

/* ======================== IfxPort APIs ======================== */
void IfxPort_setPinModeOutput(Ifx_P *port, uint8 pinIndex, IfxPort_OutputMode mode, IfxPort_PadDriver pad)
{ (void)port; (void)pinIndex; (void)mode; (void)pad; }
void IfxPort_setPinHigh(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_setPinLow(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; }
void IfxPort_togglePin(Ifx_P *port, uint8 pinIndex) { (void)port; (void)pinIndex; mock_togglePin_callCount++; }
void IfxPort_setPinState(Ifx_P *port, uint8 pinIndex, IfxPort_State state) { (void)port; (void)pinIndex; (void)state; }
void IfxPort_setPinPadDriver(Ifx_P *port, uint8 pinIndex, IfxPort_PadDriver pad) { (void)port; (void)pinIndex; (void)pad; }
