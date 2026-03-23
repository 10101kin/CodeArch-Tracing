#include "IfxGtm_Tom_Timer.h"

/* Call counters */
static uint32 s_init_count = 0;
static uint32 s_disableUpdate_count = 0;
static uint32 s_applyUpdate_count = 0;
static uint32 s_initConfig_count = 0;

/* Returns */
static boolean s_init_ret = 0;

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config)
{
    (void)driver;
    (void)config;
    s_init_count++;
    return s_init_ret;
}

void IfxGtm_Tom_Timer_disableUpdate(IfxGtm_Tom_Timer *driver)
{
    (void)driver;
    s_disableUpdate_count++;
}

void IfxGtm_Tom_Timer_applyUpdate(IfxGtm_Tom_Timer *driver)
{
    (void)driver;
    s_applyUpdate_count++;
}

void IfxGtm_Tom_Timer_initConfig(IfxGtm_Tom_Timer_Config *config, Ifx_GTM *gtm)
{
    (void)config;
    (void)gtm;
    s_initConfig_count++;
}

/* Mock controls */
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_init(void)         { return s_init_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_disableUpdate(void){ return s_disableUpdate_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void)  { return s_applyUpdate_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void)   { return s_initConfig_count; }

void   IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value)   { s_init_ret = value; }

void IfxGtm_Tom_Timer_Mock_Reset(void)
{
    s_init_count = 0; s_disableUpdate_count = 0; s_applyUpdate_count = 0; s_initConfig_count = 0;
    s_init_ret = 0;
}
