# AI Reasoning - Software Architecture

*Generated: 2026-03-23T13:26:46.875767*

---

## Phase Purpose
In this Software Architecture phase, I define the high-level module structure, driver abstractions, include hierarchy, ISR strategy, and init sequence for the TC387 GTM TOM1 3‑phase PWM. This ensures the migrated design cleanly maps requirements (20 kHz center‑aligned complementary PWM with deadtime) onto the KIT_A2G_TC387_5V_TFT hardware and iLLD drivers, setting up a maintainable codebase for later code generation and integration.

## Data Inventory
- requirements_json (from refiner node, Phase 1)
  - What: TC3xx migration; GTM TOM1 at 20 kHz, center‑aligned, 3‑phase complementary via IfxGtm_Tom_PwmHl; pins P02.0/P02.7 (U), P02.1/P02.4 (V), P02.2/P02.5 (W); pad driver IfxPort_PadDriver_cmosAutomotiveSpeed1; 500 ms duty updates: 25/50/75% start, +10% steps, 0–100% inclusive; TBD TOM1/TOUT macros.
  - Use: Source of functional/performance constraints, pin set, pad driver, timing plan, file names.
- hardware_json (from hardware node, Phase 2)
  - What: MCU TC387 (DEVICE_TC38X), TOM1 channels [0..5], clock 20 MHz XTAL / 300 MHz system; pin_assignments for U/V/W on P02.[0,1,2,4,5,7]; pin_macro unresolved; validation conflicts: TOM1/TOUT macros not resolved.
  - Use: Bind to actual TOM1 instance and channels; highlight unresolved pin macros to be decided here.
- template_capabilities (from template_library_analyzer)
  - What: Available iLLD pin maps including IfxGtm_PinMap_TC38x_LFBGA292 and _516; GTM drivers including IfxGtm_Tom_PwmHl; device_macro DEVICE_TC38X.
  - Use: Choose correct pin-map header for LFBGA292; confirm driver availability and include paths.
- reference_analysis (from reference_project_analyzer)
  - What: Files GTM_TOM_3_Phase_Inverter_PWM.c/.h; includes Ifx_Types.h, IfxCpu.h, IfxScuWdt.h, Bsp.h, IfxGtm_Tom_PwmHl.h; macro PWM_FREQ_HZ 20000; init pattern with IfxGtm_Tom_Timer and PwmHl.
  - Use: Preserve file structure, include ordering, and coding style.
- header_documentation_context (IfxGtm_Tom_PwmHl API docs)
  - What: Structs and init sequences; notes on timer reset behavior.
  - Use: Correctly configure timer/PwmHl linkage and center-aligned mode.
- extracted_config_values (from library_file_analyzer)
  - What: Examples of `output[n].pin` mapping macros form, deadTime/minPulse fields, priority macros like IFXGTM_CH0CCU0_INTR_PRIO.
  - Use: Shape of config and ISR priority placeholders; reuse existing deadtime/minPulse defaults.

## Execution Plan
1. Select device/pin-map headers
   - Include `IfxGtm_PinMap_TC38x_LFBGA292.h` (requirements_json pin_package IFX_PIN_PACKAGE_292) and guard against template default 516.
2. Define module decomposition
   - Files: `GTM_TOM_3_Phase_Inverter_PWM.h/.c` (per reference).
   - Public API: `GtmPwm3Ph_init()`, `GtmPwm3Ph_setDutyPercents(float u, v, w)`, `GtmPwm3Ph_stepPattern500ms()`.
   - Internal state: `IfxGtm_Tom_Timer timer; IfxGtm_Tom_PwmHl pwm; IfxStdIf_PwmHl stdIf;`.
3. Clock and GTM enable
   - Use system 300 MHz, XTAL 20 MHz (hardware_json); call GTM init from iLLD BSP if not already enabled.
4. Configure TOM1 timer (center-aligned 20 kHz)
   - `IfxGtm_Tom_Timer_Config`: frequency=20000 Hz, center-aligned, link to TOM1; ensure counter reset as per header docs.
5. Map PWMHL outputs to pins
   - Assign complementary pairs to TOM1 channels [0..5] for phases U/V/W.
   - Resolve and set `output[n].pin` to the correct macros for P02.0, P02.7, P02.1, P02.4, P02.2, P02.5 from `IfxGtm_PinMap_TC38x_LFBGA292`.
6. Configure PwmHl
   - `IfxGtm_Tom_PwmHl_Config`: three channel pairs, complementary, deadtime/minPulse reused from reference/defaults, pad driver `IfxPort_PadDriver_cmosAutomotiveSpeed1`, push‑pull.
   - Bind to `timer`.
7. Include hierarchy
   - In .h: `Ifx_Types.h`, `IfxGtm_Tom_PwmHl.h`.
   - In .c: previous + `IfxGtm_Tom_Timer.h`, `IfxPort.h`, `IfxCpu.h`, `IfxScuWdt.h`, `Bsp.h`, pin‑map header.
8. ISR strategy
   - Register TOM1 timer ISR stubs with priorities (e.g., `IFXGTM_CH0CCU0_INTR_PRIO`) if required by center‑aligned mode; no 500 ms ISR needed.
9. Initialization sequence
   - Init timer, init PwmHl, start PWM with initial 25/50/75% (requirements_json).
10. Duty update mechanism
   - Implement 500 ms stepper using `Bsp`/STM ticks; cycle 0–100% inclusive with 10% steps and saturation at endpoints.

## Key Decisions & Risks
- Pin macro resolution: Exact `IfxGtm_TOM1_TOUTxx_P02.y_OUT` macros for P02.{0,1,2,4,5,7} on LFBGA292 are TBD; failure blocks build.
- Package mismatch: requirements_json = IFX_PIN_PACKAGE_292; template default = 516; must force 292 headers.
- Channel routing: Need a valid TOM1 channel/TOUT mapping that supports all six P02 pins; verify no TOMx/TOUT mux conflicts.
- Deadtime/minPulse reuse: Ensure values are safe across 0% and 100% duties with complementary outputs.
- ISR necessity: Center-aligned operation must guarantee counter reset via timer; if omitted incorrectly, risk of erratic outputs.