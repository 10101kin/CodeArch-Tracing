# AI Reasoning - Detailed Design

*Generated: 2026-03-24T09:02:20.220814*

---

## Phase Purpose
In this Detailed Software Design phase, I will produce a precise, code-ready design blueprint: function signatures, struct layouts, constants, computed tick values, init sequences, and unit-test specs. This design is the direct input to code generation, ensuring the TC3xx (TOM+EVADC) inverter is cleanly migrated to TC4xx eGTM ATOM + TMADC on KIT_A3G_TC4D7_LITE with the exact timing and pin behavior required.

## Data Inventory
- user_requirement (from user): eGTM ATOM0 C0 CH0–2 complementary center-aligned PWM at 20 kHz, CDTM/DTM deadtime 1.0 µs (rise/fall), initial duties 25/50/75%, CH3 as ADC trigger. I’ll anchor all timing and channel roles on these values.
- target_board (from user): KIT_A3G_TC4D7_LITE. I’ll align device macro and pin package to this board.
- requirements_json (refiner node): Structured requirements for PWM and TMADC, driver hints: IfxEgtm_Pwm, IfxAdc_Tmadc; target macro DEVICE_TC4D7; trigger “EGTM ATOM0 C0 CH3 falling edge.” I’ll use this to choose drivers, names, files: EGTM_ATOM_3_Phase_Inverter.c/h.
- hardware_json (hardware node): MCU TC4DX, device_macro DEVICE_TC4DX, EGTM.ATOM0.C0 [CH0..3], pin candidates e.g., P02.0 TOUT0 (IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT), clocks: xtal 25 MHz, system 500 MHz; validation conflicts on user pins P20.8–P20.13. I’ll compute ticks from 500 MHz and resolve pin mapping using verified TOUT macros.
- sw_architecture_json (sw_architecture node): Driver selection recommends IfxEgtm_Atom_PwmHl/IfxEgtm_Pwm; function mapping: initEgtmAtom3phInv; includes list. I’ll base function signatures and init call sequence on this.
- template_capabilities (template analyzer): Template = KIT_A3G_TC4D7_LITE; device_macro DEVICE_TC4DX; clocks confirm 500 MHz. I’ll harmonize device macro and confirm driver presence; add fallback paths if needed.
- reference_analysis (reference project): Patterns for watchdog disable, GTM PWM init flow, coding style, macros. I’ll mirror init ordering and naming conventions.
- header_documentation_context (API docs): IfxEgtm_Pwm API and fields. I’ll extract exact struct field names and enums (e.g., complementaryPolarity).
- extracted_config_values (header analyzer): Examples for fields: deadTime.rising/falling, dtmClockSource IfxGtm_Dtm_ClockSource_cmuClock0, pin macros IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT. I’ll fill these with computed values (1.0 µs) and chosen pins.
- detected_peripheral_type: PWM. Confirms primary focus.

## Execution Plan
1) Resolve device macro and board
- Prefer DEVICE_TC4DX from hardware/template for KIT_A3G_TC4D7_LITE; record mapping note vs requirements_json DEVICE_TC4D7.

2) Select drivers and files
- PWM: IfxEgtm_Pwm (PWM HL/complementary); ADC: IfxAdc_Tmadc; Files: EGTM_ATOM_3_Phase_Inverter.c/.h, Cpu0_Main.c includes per sw_architecture_json.

3) Clock/tick computation
- CMU base = 500 MHz (template_capabilities.clock_config.syspll_frequency).
- Center-aligned: periodTicks = fclk / (2*fpwm) = 500e6 / (40e3) = 12,500.
- Deadtime: dtTicks = 1.0 µs * 500 MHz = 500 ticks.
- Initial duty ticks: U 25% = 3,125; V 50% = 6,250; W 75% = 9,375.

4) Pin/channel assignment
- Use EGTM.ATOM0.C0 CH0–2 for phases U/V/W; CH3 as TMADC trigger.
- Map output[i].pin to verified TOUTs (e.g., IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT, etc.); resolve complementaryPin via IfxEgtm_PinMap with CDTM routing on same cluster. Document any unresolved pairs.

5) Define design artifacts
- Structs: EGTM_ATOM_3PhInv_Config (freqHz, deadTimeUs, pins), EGTM_ATOM_3PhInv_Handle (IfxEgtm_Pwm handle, channels[3], TMADC trigger).
- Public APIs: initEgtmAtom3phInv, startEgtmAtom3phInv, stopEgtmAtom3phInv, setEgtmAtom3phDuties(float u,v,w), setDeadtimeUs(float rise,float fall).
- Constants/macros: F_PWM_HZ=20000u, DEADTIME_US=1.0f, PERIOD_TICKS=12500u, DEADTIME_TICKS=500u.

6) Initialization sequence
- Disable WDT; enable eGTM; enable CMU clocks; configure ATOM0 C0 in center-aligned, complementary mode with CDTM/DTM; set period/duty/deadtime; route CH3 to TMADC0 external trigger (falling edge); init TMADC0 group/channels and trigger.

7) Unit-test specs
- Verify PERIOD_TICKS and DEADTIME_TICKS math; duty clamp [0..PERIOD_TICKS]; frequency within ±1%; complementary polarity and deadtime applied; CH3 trigger occurs at configured edge relative to period match.

## Key Decisions & Risks
- Device macro mismatch (DEVICE_TC4D7 vs DEVICE_TC4DX): I will standardize on DEVICE_TC4DX for this board and note aliasing if needed.
- Pin conflicts: P20.x not validated; I’ll prioritize verified P02.x TOUT macros; risk: limited complementary pairs availability—may require alternate pins.
- CDTM/DTM selection and clock source: use IfxGtm_Dtm_ClockSource_cmuClock0; ensure 500-tick deadtime fits limits.
- Driver availability: If IfxEgtm_Pwm header is absent in template, fall back to lower-level IfxEgtm_Atom + IfxGtm_Dtm APIs with identical interfaces.
- ADC trigger path naming must match TMADC0’s external trigger mux; I’ll flag if the exact mux enum differs from “EGTM ATOM0 C0 CH3 falling edge.”