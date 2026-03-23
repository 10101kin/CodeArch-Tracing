# AI Reasoning - Software Architecture

*Generated: 2026-03-23T18:00:10.470957*

---

## Phase Purpose
In this Software Architecture phase, I define the high-level module breakdown, driver abstraction, include hierarchy, ISR strategy, and the initialization/startup sequence for the migrated PWM. The goal is to move from TC387 TOM to TC4D7 eGTM ATOM using the unified PWM driver while preserving 20 kHz center-aligned complementary PWM, 1 µs dead-time, correct polarities, synchronized start, and CMU clocking.

## Data Inventory
- user_requirement (author input)
  - From: User brief for this migration.
  - What: TC387 TOM → TC4D7 eGTM ATOM using unified PWM, 20 kHz center-aligned, 1 µs dead-time, HS active-high / LS active-low, push-pull, AGC sync start, ATOM clock = CMU FXCLK0, DTM clock = CMU CLK0, board KIT_A3G_TC4D7_LITE.
  - Use: Drives architectural targets (driver, timing, polarity, sync, clocks).

- requirements_json (refiner node)
  - From: Phase 1 structured requirements.
  - What: peripheral_requirement.driver_name = IfxEgtm_Pwm; signal_requirements pins: PHASE_U(V/W) HS/LS on P20.8/9, P20.10/11, P20.12/13; timing: 20 kHz, center-aligned, dead_time_us = 1.0; cmu.atom_clock = FXCLK0, cmu.dtm_clock = CLK0; device_macro = DEVICE_TC4D7; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h; reference includes list.
  - Use: Canonical source for module names, pins, clocks, and file structure.

- hardware_json (hardware node)
  - From: Phase 2 hardware merging/validation.
  - What: peripheral_instance EGTM0.ATOM0 channels [0..5]; clock: 25 MHz XTAL, 500 MHz SYS; pin_assignments for the six phase signals on P20.x; validation reports conflict: device/package mismatch (requirements: DEVICE_TC4D7 + IFX_PIN_PACKAGE_BGA292_COM vs template: DEVICE_TC4DX + IFX_PIN_PACKAGE_BGA436_COM).
  - Use: Choose ATOM instance/channel pairing, map pins to TOUT, and flag device/package risk.

- template_capabilities (template library analyzer)
  - From: Template library introspection.
  - What: Template path/name = KIT_A3G_TC4D7_LITE; device_macro in template = DEVICE_TC4DX; available drivers show PWM support (suggested legacy: IfxGtm_Atom_Pwm); clock config values.
  - Use: Verify IfxEgtm_Pwm availability; plan fallback to IfxGtm_Atom_Pwm if needed; align include paths.

- reference_analysis (reference project analyzer)
  - From: Original TC3xx TOM project.
  - What: Includes used: Ifx_Types.h, IfxGtm_Pwm.h, IfxPort.h, IfxPort_PinMap.h, IfxCpu.h, IfxScuWdt.h, Bsp.h; patterns for watchdog disable and interrupt enable; file naming style.
  - Use: Mirror include hierarchy and init patterns, adapt from IfxGtm_* to IfxEgtm_*.

- header_documentation_context (intelligent header selector)
  - From: iLLD API docs.
  - What: IfxEgtm_Pwm overview: up to 16 PWM signals (8 complementary pairs); fields for polarity, complementaryPolarity, deadTime, DTM clock selection, etc.
  - Use: Define driver abstraction, config struct fields, and ISR hooks.

- extracted_config_values (library analyzer)
  - From: Header extraction.
  - What: deadTime.rising/falling fields; enum values: polarity = Ifx_ActiveState_high, complementaryPolarity = Ifx_ActiveState_low; dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0; example TOUT pin macros in IfxEgtm_* namespace.
  - Use: Concrete config choices for polarity/dead-time/clock; shape arrays like output[x].pin.

## Execution Plan
1. Select driver abstraction
   - Use IfxEgtm_Pwm (requirements_json.peripheral_requirement.driver_name). If template lacks it, define a thin wrapper that maps to IfxGtm_Atom_Pwm as fallback (template_capabilities.suggested_drivers).

2. Define module decomposition
   - Files: EGTM_ATOM_3_Phase_Inverter_PWM.c/.h (requirements_json.code_structure).
   - Public API in .h: init(), setDuty_UVW(), enableOutput(bool), startSync(), optional ISR prototypes.
   - Internal structs: handle for IfxEgtm_Pwm, channel descriptors for 3 complementary pairs.

3. Include hierarchy
   - In .h: Ifx_Types.h, IfxEgtm_Pwm.h (or IfxGtm_Atom_Pwm.h fallback).
   - In .c: IfxPort.h, IfxPort_PinMap.h, IfxCpu.h, IfxScuWdt.h, Bsp.h (reference_analysis.reference_context).

4. ATOM/Channel and pin mapping
   - Instance: EGTM0.ATOM0 (hardware_json.peripheral_instance).
   - Pairing: U=(ch0,ch1), V=(ch2,ch3), W=(ch4,ch5).
   - Map outputs to pins: PHASE_U_HS→P20.8, U_LS→P20.9, V_HS→P20.10, V_LS→P20.11, W_HS→P20.12, W_LS→P20.13 (requirements_json.signal_requirements).
   - Resolve TOUT macros (e.g., IfxEgtm_ATOM0_0_TOUTx_P20_8_OUT) via IfxPort_PinMap.h; verify routability; adjust ATOM submodule if needed.

5. Clocking and timing
   - Enable eGTM and CMU; set ATOM clock = FXCLK0 and DTM clock = CLK0 (requirements_json.timing_requirements.cmu).
   - Compute period for 20 kHz center-aligned using FXCLK0; configure center-aligned mode via IfxEgtm_Pwm.
   - Configure deadTime.rising = deadTime.falling = 1e-6f (extracted_config_values).

6. Polarity and pad config
   - HS polarity = Ifx_ActiveState_high; LS complementaryPolarity = Ifx_ActiveState_low (extracted_config_values).
   - Set port mode to push-pull for all six pins (user_requirement).

7. AGC synchronized start
   - Group all six channels in a common AGC; preload duty = 0%; arm update; trigger global start to align phases.

8. ISR strategy (optional)
   - Register an ATOM SR0 ISR on one reference channel for period boundary to atomically apply duty updates; configure SRC and priority via CPU0.

9. Initialization sequence
   - Disable CPU/SMU watchdogs, init clocks (Bsp.h), init eGTM/CMU, configure pins, init driver and channels, set dead-time/polarity, bind pins, setup AGC, enable interrupts, start PWM.

## Key Decisions & Risks
- Driver availability: IfxEgtm_Pwm must exist in this template; fallback to IfxGtm_Atom_Pwm wrapper if missing.
- Pin routing: Need confirmation that EGTM0.ATOM0 CH0..5 can route to P20.8–P20.13; may require selecting different ATOMx or TOUT lines.
- Device/package mismatch: requirements use DEVICE_TC4D7 + IFX_PIN_PACKAGE_BGA292_COM; template shows DEVICE_TC4DX + IFX_PIN_PACKAGE_BGA436_COM. I will align macros to DEVICE_TC4D7 and flag template update need.
- Clock source values: Ensure FXCLK0 frequency is sufficient for 20 kHz center-aligned with required resolution; verify DTM CLK0 enables 1 µs granularity.
- Safety timing: Center-aligned/complementary with dead-time relies on correct DTM linkage; misconfiguration risks shoot-through—will double-check DTM pairing per channel.