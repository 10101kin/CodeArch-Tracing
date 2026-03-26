/* Spy state + stub bodies + MODULE_* definitions */
#include "mock_gtm_tom_3_phase_inverter_pwm.h"
#include "IfxPort.h"
#include "IfxGtm.h"
#include "IfxGtm_Cmu.h"
#include "IfxGtm_Tom_Timer.h"
#include "IfxGtm_Pwm.h"
#include <string.h>

/* MODULE_* instance definitions */
Ifx_GTM MODULE_GTM;
Ifx_ASCLIN0 MODULE_ASCLIN0;  Ifx_ASCLIN1 MODULE_ASCLIN1;  Ifx_ASCLIN2 MODULE_ASCLIN2;  Ifx_ASCLIN3 MODULE_ASCLIN3;
Ifx_ASCLIN4 MODULE_ASCLIN4;  Ifx_ASCLIN5 MODULE_ASCLIN5;  Ifx_ASCLIN6 MODULE_ASCLIN6;  Ifx_ASCLIN7 MODULE_ASCLIN7;
Ifx_ASCLIN8 MODULE_ASCLIN8;  Ifx_ASCLIN9 MODULE_ASCLIN9;  Ifx_ASCLIN10 MODULE_ASCLIN10; Ifx_ASCLIN11 MODULE_ASCLIN11;
Ifx_ASCLIN12 MODULE_ASCLIN12; Ifx_ASCLIN13 MODULE_ASCLIN13; Ifx_ASCLIN14 MODULE_ASCLIN14; Ifx_ASCLIN15 MODULE_ASCLIN15;
Ifx_ASCLIN16 MODULE_ASCLIN16; Ifx_ASCLIN17 MODULE_ASCLIN17; Ifx_ASCLIN18 MODULE_ASCLIN18; Ifx_ASCLIN19 MODULE_ASCLIN19;
Ifx_ASCLIN20 MODULE_ASCLIN20; Ifx_ASCLIN21 MODULE_ASCLIN21; Ifx_ASCLIN22 MODULE_ASCLIN22; Ifx_ASCLIN23 MODULE_ASCLIN23;

Ifx_CAN0 MODULE_CAN0; Ifx_CAN1 MODULE_CAN1; Ifx_CAN2 MODULE_CAN2;
Ifx_CBS MODULE_CBS; Ifx_CCU60 MODULE_CCU60; Ifx_CCU61 MODULE_CCU61; Ifx_CONVCTRL MODULE_CONVCTRL;
Ifx_CPU0 MODULE_CPU0; Ifx_CPU1 MODULE_CPU1; Ifx_CPU2 MODULE_CPU2; Ifx_CPU3 MODULE_CPU3;
Ifx_DAM0 MODULE_DAM0; Ifx_DMA MODULE_DMA; Ifx_DMU MODULE_DMU; Ifx_DOM0 MODULE_DOM0;
Ifx_EDSADC MODULE_EDSADC; Ifx_ERAY0 MODULE_ERAY0; Ifx_ERAY1 MODULE_ERAY1; Ifx_EVADC MODULE_EVADC;
Ifx_FCE MODULE_FCE; Ifx_FSI MODULE_FSI; Ifx_GETH MODULE_GETH; Ifx_GPT120 MODULE_GPT120;
Ifx_HSCT0 MODULE_HSCT0; Ifx_HSSL0 MODULE_HSSL0; Ifx_I2C0 MODULE_I2C0; Ifx_I2C1 MODULE_I2C1;
Ifx_INT MODULE_INT; Ifx_IOM MODULE_IOM; Ifx_LMU0 MODULE_LMU0; Ifx_MINIMCDS MODULE_MINIMCDS;
Ifx_MSC0 MODULE_MSC0; Ifx_MSC1 MODULE_MSC1; Ifx_MSC2 MODULE_MSC2; Ifx_MTU MODULE_MTU;
Ifx_PFI0 MODULE_PFI0; Ifx_PFI1 MODULE_PFI1; Ifx_PFI2 MODULE_PFI2; Ifx_PFI3 MODULE_PFI3;
Ifx_PMS MODULE_PMS; Ifx_PMU MODULE_PMU;
Ifx_P MODULE_P00; Ifx_P MODULE_P01; Ifx_P MODULE_P02; Ifx_P MODULE_P10; Ifx_P MODULE_P11; Ifx_P MODULE_P12; Ifx_P MODULE_P13; Ifx_P MODULE_P14; Ifx_P MODULE_P15;
Ifx_P MODULE_P20; Ifx_P MODULE_P21; Ifx_P MODULE_P22; Ifx_P MODULE_P23; Ifx_P MODULE_P24; Ifx_P MODULE_P25; Ifx_P MODULE_P26;
Ifx_P MODULE_P30; Ifx_P MODULE_P31; Ifx_P MODULE_P32; Ifx_P MODULE_P33; Ifx_P MODULE_P34; Ifx_P MODULE_P40; Ifx_P MODULE_P41;
Ifx_PSI5S MODULE_PSI5S; Ifx_PSI5 MODULE_PSI5; Ifx_QSPI0 MODULE_QSPI0; Ifx_QSPI1 MODULE_QSPI1; Ifx_QSPI2 MODULE_QSPI2; Ifx_QSPI3 MODULE_QSPI3; Ifx_QSPI4 MODULE_QSPI4;
Ifx_SBCU MODULE_SBCU; Ifx_SCU MODULE_SCU; Ifx_SENT MODULE_SENT; Ifx_SMU MODULE_SMU; Ifx_SRC MODULE_SRC;
Ifx_STM0 MODULE_STM0; Ifx_STM1 MODULE_STM1; Ifx_STM2 MODULE_STM2; Ifx_STM3 MODULE_STM3;

/* Spy counters and return values */
int mock_IfxGtm_Tom_Timer_run_callCount = 0;
int mock_IfxGtm_Tom_Timer_addToChannelMask_callCount = 0;
int mock_IfxGtm_Tom_Timer_applyUpdate_callCount = 0;
int mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
int mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount = 0;
int mock_IfxGtm_Tom_Timer_disableUpdate_callCount = 0;
int mock_IfxGtm_Tom_Timer_init_callCount = 0;

int mock_IfxGtm_enable_callCount = 0;

int mock_IfxGtm_Pwm_updateChannelsDuty_callCount = 0;
int mock_IfxGtm_Pwm_initConfig_callCount = 0;
int mock_IfxGtm_Pwm_init_callCount = 0;

int mock_IfxGtm_Cmu_enableClocks_callCount = 0;

boolean mock_IfxGtm_Tom_Timer_init_returnValue = FALSE;

uint32  mock_IfxGtm_Pwm_init_lastNumChannels = 0U;
float32 mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
float32 mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising[MOCK_MAX_CHANNELS] = {0};
float32 mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling[MOCK_MAX_CHANNELS] = {0};
uint32  mock_togglePin_callCount = 0U;

/* Stub bodies (never access struct members) */
void IfxGtm_Tom_Timer_run(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_run_callCount++; }
void IfxGtm_Tom_Timer_addToChannelMask(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Ch channel) { (void)driver; (void)channel; mock_IfxGtm_Tom_Timer_addToChannelMask_callCount++; }
void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_applyUpdate_callCount++; }
void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm) { (void)config; (void)gtm; mock_IfxGtm_Tom_Timer_initConfig_callCount++; }
void IfxGtm_Tom_Timer_updateInputFrequency(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount++; }
void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver) { (void)driver; mock_IfxGtm_Tom_Timer_disableUpdate_callCount++; }
boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config) { (void)driver; (void)config; mock_IfxGtm_Tom_Timer_init_callCount++; return mock_IfxGtm_Tom_Timer_init_returnValue; }

void IfxGtm_enable(Ifx_GTM *gtm) { (void)gtm; mock_IfxGtm_enable_callCount++; }

void IfxGtm_Pwm_updateChannelsDuty(IfxGtm_Pwm *pwm, float32 *requestDuty) {
    (void)pwm;
    mock_IfxGtm_Pwm_updateChannelsDuty_callCount++;
    if (requestDuty != NULL_PTR) {
        /* Capture first 3 duties for 3-phase context, clamp to available buffer */
        mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[0] = requestDuty[0];
        mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[1] = requestDuty[1];
        mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties[2] = requestDuty[2];
    }
}

void IfxGtm_Pwm_initConfig(IfxGtm_Pwm_Config *config, Ifx_GTM *gtmSFR) { (void)config; (void)gtmSFR; mock_IfxGtm_Pwm_initConfig_callCount++; }
void IfxGtm_Pwm_init(IfxGtm_Pwm *pwm, IfxGtm_Pwm_Channel *channels, IfxGtm_Pwm_Config *config) {
    (void)pwm; (void)channels; (void)config; mock_IfxGtm_Pwm_init_callCount++;
    /* Set a conventional 3-phase channel count hint */
    mock_IfxGtm_Pwm_init_lastNumChannels = 3U;
}

void IfxGtm_Cmu_enableClocks(Ifx_GTM *gtm, uint32 clkMask) { (void)gtm; (void)clkMask; mock_IfxGtm_Cmu_enableClocks_callCount++; }

/* Mock control */
void mock_gtm_tom_3_phase_inverter_pwm_reset(void) {
    mock_IfxGtm_Tom_Timer_run_callCount = 0;
    mock_IfxGtm_Tom_Timer_addToChannelMask_callCount = 0;
    mock_IfxGtm_Tom_Timer_applyUpdate_callCount = 0;
    mock_IfxGtm_Tom_Timer_initConfig_callCount = 0;
    mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount = 0;
    mock_IfxGtm_Tom_Timer_disableUpdate_callCount = 0;
    mock_IfxGtm_Tom_Timer_init_callCount = 0;

    mock_IfxGtm_enable_callCount = 0;

    mock_IfxGtm_Pwm_updateChannelsDuty_callCount = 0;
    mock_IfxGtm_Pwm_initConfig_callCount = 0;
    mock_IfxGtm_Pwm_init_callCount = 0;

    mock_IfxGtm_Cmu_enableClocks_callCount = 0;

    mock_IfxGtm_Tom_Timer_init_returnValue = FALSE;

    mock_IfxGtm_Pwm_init_lastNumChannels = 0U;
    mock_IfxGtm_Pwm_init_lastFrequency = 0.0f;
    memset(mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties, 0, sizeof(mock_IfxGtm_Pwm_updateChannelsDuty_lastDuties));
    memset(mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising, 0, sizeof(mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtRising));
    memset(mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling, 0, sizeof(mock_IfxGtm_Pwm_updateChannelDeadTimeImmediate_lastDtFalling));
    mock_togglePin_callCount = 0U;
}

/* Getters */
int mock_IfxGtm_Tom_Timer_run_getCallCount(void) { return mock_IfxGtm_Tom_Timer_run_callCount; }
int mock_IfxGtm_Tom_Timer_addToChannelMask_getCallCount(void) { return mock_IfxGtm_Tom_Timer_addToChannelMask_callCount; }
int mock_IfxGtm_Tom_Timer_applyUpdate_getCallCount(void) { return mock_IfxGtm_Tom_Timer_applyUpdate_callCount; }
int mock_IfxGtm_Tom_Timer_initConfig_getCallCount(void) { return mock_IfxGtm_Tom_Timer_initConfig_callCount; }
int mock_IfxGtm_Tom_Timer_updateInputFrequency_getCallCount(void) { return mock_IfxGtm_Tom_Timer_updateInputFrequency_callCount; }
int mock_IfxGtm_Tom_Timer_disableUpdate_getCallCount(void) { return mock_IfxGtm_Tom_Timer_disableUpdate_callCount; }
int mock_IfxGtm_Tom_Timer_init_getCallCount(void) { return mock_IfxGtm_Tom_Timer_init_callCount; }

int mock_IfxGtm_enable_getCallCount(void) { return mock_IfxGtm_enable_callCount; }

int mock_IfxGtm_Pwm_updateChannelsDuty_getCallCount(void) { return mock_IfxGtm_Pwm_updateChannelsDuty_callCount; }
int mock_IfxGtm_Pwm_initConfig_getCallCount(void) { return mock_IfxGtm_Pwm_initConfig_callCount; }
int mock_IfxGtm_Pwm_init_getCallCount(void) { return mock_IfxGtm_Pwm_init_callCount; }

int mock_IfxGtm_Cmu_enableClocks_getCallCount(void) { return mock_IfxGtm_Cmu_enableClocks_callCount; }
