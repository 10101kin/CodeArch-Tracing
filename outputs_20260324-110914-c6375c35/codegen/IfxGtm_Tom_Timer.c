#include "IfxGtm_Tom_Timer.h"

/* Call counters */
static uint32  s_applyUpdate_count = 0;
static uint32  s_initConfig_count  = 0;
static uint32  s_init_count        = 0;
static uint32  s_addMask_count     = 0;

/* Return controls */
static boolean s_init_ret          = 0; /* default FALSE */

/* Argument captures */
static uint32  s_addMask_lastChannel = 0;

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

boolean IfxGtm_Tom_Timer_init(IfxGtm_Tom_Timer *driver, const IfxGtm_Tom_Timer_Config *config)
{
    (void)driver;
    (void)config;
    s_init_count++;
    return s_init_ret;
}

void IfxGtm_Tom_Timer_addToChannelMask(IfxGtm_Tom_Timer *driver, IfxGtm_Tom_Ch channel)
{
    (void)driver;
    s_addMask_count++;
    s_addMask_lastChannel = (uint32)channel;
}

uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_applyUpdate(void) { return s_applyUpdate_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_initConfig(void)  { return s_initConfig_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_init(void)        { return s_init_count; }
uint32 IfxGtm_Tom_Timer_Mock_GetCallCount_addToChannelMask(void) { return s_addMask_count; }

void IfxGtm_Tom_Timer_Mock_SetReturn_init(boolean value) { s_init_ret = value; }

uint32 IfxGtm_Tom_Timer_Mock_GetLastArg_addToChannelMask_channel(void)
{
    return s_addMask_lastChannel;
}

void IfxGtm_Tom_Timer_Mock_Reset(void)
{
    s_applyUpdate_count = 0;
    s_initConfig_count  = 0;
    s_init_count        = 0;
    s_addMask_count     = 0;

    s_init_ret          = 0;
    s_addMask_lastChannel = 0;
}
