#ifndef IFXEGTM_PWM_H
#define IFXEGTM_PWM_H

#include "mock_egtm_atom_3_phase_inverter_pwm.h"
#include "IfxEgtm.h"
#include "IfxEgtm_Cmu.h"
#include "IfxPort.h"

/* Clock source union (uint32 fields to avoid enum-conversion issues) */
typedef union {
    uint32 atom;
    uint32 tom;
} IfxEgtm_Pwm_ClockSource;

/* Tout mapping types (single-pin record + map) */
typedef struct {
    Ifx_P *port;
    uint8  pinIndex;
} IfxEgtm_Pwm_Tout;

typedef struct {
    IfxEgtm_Pwm_Tout pin;
} IfxEgtm_Pwm_ToutMap;

/* DTM config / Deadtime */
typedef struct {
    float32 dtRising;
    float32 dtFalling;
    uint32  clockSource; /* abstract clock source */
} IfxEgtm_Pwm_DtmConfig;

typedef IfxEgtm_Pwm_DtmConfig IfxEgtm_Pwm_DeadTime;

/* Interrupt config */
typedef struct {
    IfxEgtm_IrqMode mode;
    Ifx_Priority    priority;
    IfxSrc_Tos      typeOfService;
    uint8           interruptLine;
} IfxEgtm_Pwm_InterruptConfig;

/* Output (pin + polarity) */
typedef struct {
    const IfxEgtm_Pwm_ToutMap *pin;
    const IfxEgtm_Pwm_ToutMap *complementaryPin;
    Ifx_ActiveState            polarity;
    Ifx_ActiveState            complementaryPolarity;
} IfxEgtm_Pwm_OutputConfig;

/* Channel config */
typedef struct {
    uint32                         timerCh;
    float32                        phase;
    float32                        duty;
    IfxEgtm_Pwm_DtmConfig         *dtm;
    IfxEgtm_Pwm_OutputConfig      *output;
    void                          *mscOut;     /* MSC config pointer */
    IfxEgtm_Pwm_InterruptConfig   *interrupt;
} IfxEgtm_Pwm_Channel;

typedef IfxEgtm_Pwm_Channel IfxEgtm_Pwm_ChannelConfig;

/* Driver config */
typedef struct {
    Ifx_EGTM                 *egtmSfr;
    uint32                    subModule;
    uint32                    cluster;
    IfxEgtm_Pwm_ClockSource   clockSource;
    float32                   frequency;
    uint32                    numChannels;
    IfxEgtm_Pwm_Channel      *channels;   /* array of numChannels */
    boolean                   syncStart;
    boolean                   syncUpdateEnabled;
    uint32                    dtmClockSource;
} IfxEgtm_Pwm_Config;

/* Driver handle */
typedef struct {
    Ifx_EGTM *egtmSfr;
} IfxEgtm_Pwm;

/* API */
void    IfxEgtm_Pwm_initConfig(IfxEgtm_Pwm_Config *config, Ifx_EGTM *egtm);
boolean IfxEgtm_Pwm_init(IfxEgtm_Pwm *driver, IfxEgtm_Pwm_Config *config);
void    IfxEgtm_Pwm_updateChannelsDutyImmediate(IfxEgtm_Pwm *driver, const float32 *duties);
void    IfxEgtm_Pwm_updateDeadtime(IfxEgtm_Pwm *driver, const float32 *dtRising, const float32 *dtFalling);

#endif /* IFXEGTM_PWM_H */
