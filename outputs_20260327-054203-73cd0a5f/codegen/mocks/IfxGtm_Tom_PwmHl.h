#ifndef IFXGTM_TOM_PWMHL_H
#define IFXGTM_TOM_PWMHL_H

#include "mock_gtm_tom_3_phase_inverter_pwm.h"

/* Forward declaration for timer type to avoid cyclic include */
typedef struct IfxGtm_Tom_Timer IfxGtm_Tom_Timer;

/* TOM/DTM SFR stub types used by both drivers */
typedef struct { uint32 reserved; } Ifx_GTM_TOM_TGC;
typedef struct { uint32 reserved; } Ifx_GTM_CDTM_DTM;

/* Basic GTM TOM enums used in config */
typedef enum {
    IfxGtm_Tom_0 = 0,
    IfxGtm_Tom_1,
    IfxGtm_Tom_2,
    IfxGtm_Tom_3,
    IfxGtm_Tom_4,
    IfxGtm_Tom_5,
    IfxGtm_Tom_6,
    IfxGtm_Tom_7
} IfxGtm_Tom;

typedef enum {
    IfxGtm_Tom_Ch_0 = 0,
    IfxGtm_Tom_Ch_1,
    IfxGtm_Tom_Ch_2,
    IfxGtm_Tom_Ch_3,
    IfxGtm_Tom_Ch_4,
    IfxGtm_Tom_Ch_5,
    IfxGtm_Tom_Ch_6,
    IfxGtm_Tom_Ch_7,
    IfxGtm_Tom_Ch_8,
    IfxGtm_Tom_Ch_9,
    IfxGtm_Tom_Ch_10,
    IfxGtm_Tom_Ch_11,
    IfxGtm_Tom_Ch_12,
    IfxGtm_Tom_Ch_13,
    IfxGtm_Tom_Ch_14,
    IfxGtm_Tom_Ch_15
} IfxGtm_Tom_Ch;

typedef enum {
    IfxGtm_Tom_Ch_ClkSrc_cmuClk0 = 0,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk1,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk2,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk3,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk4,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk5,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk6,
    IfxGtm_Tom_Ch_ClkSrc_cmuClk7,
    IfxGtm_Tom_Ch_ClkSrc_fxclk
} IfxGtm_Tom_Ch_ClkSrc;

typedef enum {
    IfxGtm_Dtm_Ch_0 = 0,
    IfxGtm_Dtm_Ch_1,
    IfxGtm_Dtm_Ch_2,
    IfxGtm_Dtm_Ch_3
} IfxGtm_Dtm_Ch;

/* PWM HL specific basic enums/structs */
typedef enum {
    Ifx_Pwm_Mode_leftAligned = 0,
    Ifx_Pwm_Mode_rightAligned,
    Ifx_Pwm_Mode_centerAligned
} Ifx_Pwm_Mode;

/* StdIf PwmHl config used inside driver config */
typedef struct {
    uint8   channelCount; /* number of channels (pairs) */
    float32 frequency;    /* timer base frequency Hz */
} IfxStdIf_PwmHl_Config;

/* Pin map type for TOM (simplified) */
typedef struct {
    void   *port; /* Ifx_P* typically */
    uint8   pin;  /* pin index */
} IfxGtm_Tom_ToutMap;
typedef const IfxGtm_Tom_ToutMap* IfxGtm_Tom_ToutMapP;

/* IfxGtm_Tom_PwmHl_Base */
typedef struct
{
    Ifx_TimerValue  deadtime;
    Ifx_TimerValue  minPulse;
    Ifx_TimerValue  maxPulse;
    Ifx_Pwm_Mode    mode;
    sint8           setMode;
    Ifx_ActiveState ccxActiveState;
    Ifx_ActiveState coutxActiveState;
    boolean         inverted;
    uint8           channelCount;
} IfxGtm_Tom_PwmHl_Base;

/* IfxGtm_Tom_PwmHl_Config */
typedef struct
{
    IfxStdIf_PwmHl_Config          base;
    IfxGtm_Tom_Timer              *timer;
    IfxGtm_Tom                     tom;
    IfxGtm_Tom_ToutMapP            ccx;   /* array base.channelCount/2 */
    IfxGtm_Tom_ToutMapP            coutx; /* array base.channelCount/2 */
    boolean                        initPins;
} IfxGtm_Tom_PwmHl_Config;

/* IfxGtm_Tom_PwmHl_Mode */
typedef struct
{
    Ifx_Pwm_Mode                 mode;
    boolean                      inverted;
    void                        *update;        /* callback placeholders */
    void                        *updateAndShift;
    void                        *updatePulse;
} IfxGtm_Tom_PwmHl_Mode;

/* Driver object (minimal) */
typedef struct IfxGtm_Tom_PwmHl
{
    IfxGtm_Tom_PwmHl_Base base;
    IfxGtm_Tom_Timer     *timer;
    Ifx_GTM_TOM          *tom;
} IfxGtm_Tom_PwmHl;

/* API (DRIVERS TO MOCK set) */
void    IfxGtm_Tom_PwmHl_setOnTime(IfxGtm_Tom_PwmHl *driver, Ifx_TimerValue *tOn);
boolean IfxGtm_Tom_PwmHl_init(IfxGtm_Tom_PwmHl *driver, const IfxGtm_Tom_PwmHl_Config *config);
void    IfxGtm_Tom_PwmHl_initConfig(IfxGtm_Tom_PwmHl_Config *config);
boolean IfxGtm_Tom_PwmHl_setMode(IfxGtm_Tom_PwmHl *driver, Ifx_Pwm_Mode mode);

/* StdIf helper for PwmHl */
void IfxStdIf_PwmHl_setOnTime(void *driver, Ifx_TimerValue *tOn);

#endif /* IFXGTM_TOM_PWMHL_H */
