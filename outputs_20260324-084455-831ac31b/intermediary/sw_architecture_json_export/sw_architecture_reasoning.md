# AI Reasoning - Software Architecture

*Generated: 2026-03-24T08:56:32.800113*

---

## Phase Purpose
In this Software Architecture phase, I will define the high-level module layout, driver abstractions, include hierarchy, ISR wiring, and initialization order for migrating the TC3xx TOM+EVADC 3‑phase inverter to TC4xx eGTM ATOM+TMADC. The output is a clear blueprint and file skeleton that the code generator will implement consistently across PWM and ADC paths.

## Data Inventory
- user_requirement (User input)
  - Contains: Migrate to TC4D7 using EGTM ATOM0 Cluster 0: CH0–2 complementary, center-aligned PWM, deadtime 1.0 µs (rise/fall), 20 kHz, initial duties 25/50/75%, CH3 as TMADC trigger.
  - Use: Drive the functional architecture, channel mapping, timing, and trigger topology.

- target_board (User input)
  - Contains: KIT_A3G_TC4D7_LITE.
  - Use: Select template and BSP assumptions.

- requirements_json (Refiner node)
  - Contains: Driver decisions: IfxEgtm_Pwm for PWM, IfxAdc_Tmadc for TMADC0 with trigger “EGTM ATOM0 C0 CH3 falling edge”; files: EGTM_ATOM_3_Phase_Inverter.c/.h; device macro DEVICE_TC4D7.
  - Use: Authoritative requirements; drive module names, drivers, and trigger edge.

- hardware_json (Hardware node)
  - Contains: EGTM.ATOM0.C0 channels [0..3]; example pin P02.0 as TOUT0 for PWM_U_HIGH; clocks 25 MHz XTAL, 500 MHz system; device_macro DEVICE_TC4DX; validation conflicts for user pins P20.8–P20.13; verified_count=2.
  - Use: Seed pin map, detect macro mismatch, and clock basis; flag pin risks.

- template_capabilities (Template analyzer)
  - Contains: Template path for KIT_A3G_TC4D7_LITE, TC4xx iLLD availability, device_macro DEVICE_TC4DX, 500 MHz sysPLL.
  - Use: Confirm driver availability and interrupt config support.

- reference_analysis (Reference project analyzer)
  - Contains: Prior TOM+EVADC structure, includes (IfxGtm_Pwm.h, IfxEVadc_Adc.h, IfxPort.h, IfxGtm_Trig.h, IfxScuWdt.h), watchdog pattern, macros NUM_OF_CHANNELS=3.
  - Use: Inform migration points from IfxGtm_Pwm→IfxEgtm_Pwm and EVADC→TMADC.

- header_documentation_context (Header selector)
  - Contains: IfxEgtm_Pwm usage docs.
  - Use: Select structs/APIs (`IfxEgtm_Pwm_Config`, `IfxEgtm_Pwm`, channel config, DTM/CDTM).

- extracted_config_values (Library analyzer)
  - Contains: Fields like `deadTime.rising/falling`, enums `IfxGtm_Dtm_ClockSource_cmuClock0`, pin macros `IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT`.
  - Use: Concretize deadtime, polarity, and pin macros.

- detected_peripheral_type
  - Contains: PWM.
  - Use: Confirms focus on PWM core.

## Execution Plan
1) Reconcile device macro and board
   - From requirements_json vs hardware_json/template_capabilities: decide on `DEVICE_TC4D7` for KIT_A3G_TC4D7_LITE; record mismatch risk.

2) Define modules and files
   - Create `EGTM_ATOM_3_Phase_Inverter.c/.h` exposing:
     - PWM API: `EgtmPwm3Ph_init`, `EgtmPwm3Ph_setDuty(phase, percent)`, `EgtmPwm3Ph_start/stop`.
     - ADC API: `TmadcCtrl_init`, ISR handler, result callback.
   - Separate `Board_Pins.h` for TOUT pin macros.

3) Include hierarchy
   - Use `IfxEgtm_Pwm.h`, `IfxAdc_Tmadc.h`, `IfxPort.h`, `IfxPort_PinMap.h`, `IfxGtm_Trig.h`, `IfxScuWdt.h`, `IfxStm.h` (timing).

4) Clock and GTM enable
   - From template_capabilities clock_config: enable GTM/eGTM, set CMU clocks; select `IfxGtm_Dtm_ClockSource_cmuClock0`.

5) PWM configuration
   - Map EGTM.ATOM0.C0 CH0–2 as complementary center-aligned pairs with CDTM/DTM deadtime = 1.0 µs (rise/fall).
   - Frequency 20 kHz; initial duties: U=25%, V=50%, W=75%.
   - Use `IfxEgtm_Pwm_Config` and `deadTime.{rising,falling}`.

6) Pin mapping
   - Use available macros like `IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT`; assign remaining TOUTs per hardware_json, flag unverified pins.

7) ADC trigger and TMADC
   - Configure ATOM0 C0 CH3 to emit trigger (falling edge) via `IfxGtm_Trig` to `TMADC0`.
   - Initialize `IfxAdc_Tmadc` channels; set IRQ priorities; register ISR.

8) Initialization sequence
   - Disable watchdogs; init GTM/eGTM clocks; configure CDTM/DTM; init PWM; route pins; init TMADC; link trigger; enable interrupts; start PWM.

## Key Decisions & Risks
- Device macro conflict: DEVICE_TC4D7 (req) vs DEVICE_TC4DX (hardware/template). I will proceed with TC4D7 and raise a gating check.
- Pin validation: Only 2 pins verified; user-specified P20.8–P20.13 invalid. I will default to P02.x TOUT macros where available and mark remaining for resolution.
- GTM CMU exact frequency not specified; deadtime tick calculation depends on CMU. I will parameterize deadtime in seconds and compute ticks at build-time.
- TMADC trigger line mapping names may vary by derivative; I’ll abstract via `IfxGtm_Trig` and verify vector names during integration.