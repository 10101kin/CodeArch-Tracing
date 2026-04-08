/*
 * gtm_tom_3_phase_inverter_pwm.h
 * Public API for GTM TOM 3-Phase Inverter PWM (IfxGtm_Pwm unified driver)
 */
#ifndef GTM_TOM_3_PHASE_INVERTER_PWM_H
#define GTM_TOM_3_PHASE_INVERTER_PWM_H

/* Public API */
void initGtmTom3phInv(void);
void updateGtmTom3phInvDuty(void);

/* ISR (exported symbol; typically not called directly by application code) */
void interruptGtmAtom(void);

/* Unit-test hooks (stubs) */
void UT_DEADTIME_FALLING_S(void);
void UT_DEADTIME_RISING_S(void);
void UT_FLOAT_EPSILON(void);
void UT_INIT_DUTY_U_PERCENT(void);
void UT_INIT_DUTY_V_PERCENT(void);
void UT_INIT_DUTY_W_PERCENT(void);
void UT_MIN_PULSE_S(void);
void UT_NUM_CHANNELS(void);
void UT_PWM_FREQ_HZ(void);
void UT_STEP_PERCENT(void);
void UT_WRAP_THRESHOLD_PERCENT(void);
void accumulate(void);
void behavior(void);
void call(void);
/* Note: A function named 'for' cannot be declared in C (reserved keyword). */
void logic(void);
void percent(void);
void update(void);

#endif /* GTM_TOM_3_PHASE_INVERTER_PWM_H */
