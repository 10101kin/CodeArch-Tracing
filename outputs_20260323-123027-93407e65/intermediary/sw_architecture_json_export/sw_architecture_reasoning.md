# AI Reasoning - Software Architecture

*Generated: 2026-03-23T12:46:01.236239*

---

## Phase Purpose
In this Software Architecture phase, I define the high-level module decomposition, driver abstractions, include hierarchy, ISR strategy, and the init sequence to realize the migrated 3‑phase inverter on TC4D7. This blueprint ensures the eGTM ATOM PWM and EVADC trigger are organized into clear modules that map cleanly onto TC4xx iLLD, de-risking detailed implementation in the next phase.

## Data Inventory
- user_requirement (user input)
  - What: Migrate TC387 TOM+EVADC to TC4D7 eGTM ATOM0, Cluster 0: CH0–CH2 complementary center-aligned PWM at 20 kHz, 1 µs deadtime; HS active-high, LS active-low on P20.8/9, P20.10/11, P20.12/13; CH3 50% edge-aligned trigger.
  - Use: Functional target, pin intent, timing, channel map.

- target_board (phase 0)
  - What: KIT_A3G_TC4D7_LITE.
  - Use: Board constraints and available TOUT pinout expectations.

- requirements_json (phase 1 refiner)
  - What: Structured constraints: driver IfxEgtm_Pwm, eGTM cluster 0 ATOM0 channels [0,1,2], trigger 3; 20 kHz, 1 µs; device_macro DEVICE_TC4D7; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h; includes IfxGtm_Pwm.h, IfxGtm_Trig.h, IfxEVadc_Adc.h.
  - Use: Driver selection, file naming, includes, device macro baseline.

- hardware_json (phase 2 hardware)
  - What: MCU TC4DX (device_macro DEVICE_TC4DX), eGTM.ATOM0 channels [0..3], example TOUT pins: ATOM0.CH0_HS -> P02.0 (TOUT0); clocks: 25 MHz XTAL, 500 MHz system; validation shows conflicts: requested P20.8/9/10/11/12/13 (and P33.0) not verified.
  - Use: Verify feasibility, flag pin conflicts, pick interim pin maps if needed.

- template_capabilities (template analyzer)
  - What: Template KIT_A3G_TC4D7_LITE, TC4xx iLLD available; device_macro DEVICE_TC4DX; clock config; drivers include IfxEgtm_Pwm (via header docs list) and timing (IfxStm).
  - Use: Confirm driver availability, clock assumptions, interrupt infra.

- reference_analysis (reference project)
  - What: Prior TOM-based structure; includes Ifx_Types.h, IfxCpu.h, IfxScuWdt.h, IfxGtm_Pwm.h, IfxGtm_Trig.h, IfxEVadc_Adc.h; watchdog disable/init patterns.
  - Use: Preserve include hierarchy and init style; migrate to eGTM ATOM API.

- header_documentation_context (iLLD docs)
  - What: IfxEgtm_Pwm usage, complementary outputs, deadtime config, pin macros.
  - Use: Define config structs, polarity, deadtime, channel binding.

- extracted_config_values (header analyzer)
  - What: Examples like deadTime.rising/falling, dtmClockSource IfxGtm_Dtm_ClockSource_cmuClock0; pin macros e.g., IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT; config.frequency placeholders.
  - Use: Populate init fields and valid TOUT mappings.

- detected_peripheral_type
  - What: PWM.
  - Use: Confirms driver stack focus.

## Execution Plan
1. Select driver stack: Use IfxEgtm_Pwm (TC4xx) per requirements_json; keep IfxGtm_Trig for EVADC trigger routing (from reference_context).
2. Define modules/files:
   - EGTM_ATOM_3_Phase_Inverter_PWM.h/.c: Pwm3Phase module owning IfxEgtm_Pwm handle, three complementary pairs on ATOM0 CH0–CH2, plus CH3 trigger.
   - PwmTrig sub-module: config for CH3 50% edge-aligned and EVADC trigger routing.
3. Include hierarchy: In .h include Ifx_Types.h, IfxGtm_Pwm.h; in .c include module .h, IfxCpu.h, IfxScuWdt.h, IfxPort.h, IfxGtm_Trig.h, IfxEVadc_Adc.h.
4. Clock/init sequence:
   - Disable watchdogs (IfxScuWdt_*), enable GTM/eGTM, set CMU clocks; ensure DTM clock = IfxGtm_Dtm_ClockSource_cmuClock0.
   - Compute 20 kHz center-aligned period using system 500 MHz and CMU divider; document chosen clock path.
5. PWM configuration:
   - For CH0–CH2: center-aligned mode, complementary enabled, deadTime.rising/falling = 1e-6 s, polarity HS high / complementaryPolarity LS low.
   - Duty update hooks with 500 ms period if needed via STM (optional placeholder).
6. Trigger configuration:
   - CH3: edge-aligned, 50% duty; route to EVADC via IfxGtm_Trig API to the specified ADC group/channel (placeholder to bind group once provided).
7. Pin routing:
   - Target P20.8/9, P20.10/11, P20.12/13 per user; if unavailable on KIT_A3G_TC4D7_LITE, provide fallback macros like IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT from extracted_config_values and hardware_json, behind a compile-time switch.
8. ISR plan:
   - No PWM ISR required; optional EVADC conversion ISR stub in ADC module (out-of-scope).
9. Build-time macros:
   - DEVICE_TC4D7 as target; guard for DEVICE_TC4DX if template enforces it; expose configuration via a cfg header.

## Key Decisions & Risks
- Pin mapping conflict: Requested P20.x not verified on KIT_A3G_TC4D7_LITE; may need to remap to P02.x TOUT0/1/etc. Decision: attempt P20.x first; provide fallback and clearly document.
- Device macro mismatch: requirements_json = DEVICE_TC4D7 vs template/device = DEVICE_TC4DX. Decision: align to TC4D7; risk: template startup headers may expect TC4DX.
- EVADC trigger endpoints not fully specified for TC4D7. Decision: leave routing hook using IfxGtm_Trig with TODO for ADC group/channel.
- Clock derivation: Need exact CMU FXCLK selection/divider to hit 20 kHz center-aligned with 1 µs deadtime margin; risk if DTM clock too slow/fast.
- Driver naming variance: Some templates reference IfxGtm_Atom_*; ensure IfxEgtm_Pwm API present; otherwise, adapt wrapper.