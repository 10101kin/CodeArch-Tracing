# AI Reasoning - Software Architecture

*Generated: 2026-03-25T11:03:58.094609*

---

## Phase Purpose
In this Software Architecture phase, I define the module decomposition, driver abstractions, include hierarchy, ISR wiring, and the initialization/update sequence for the 3‑phase inverter PWM on TC387. The output is a clear blueprint for GTM TOM0 with DTM0 complementary outputs and a TOM period interrupt, so that the subsequent implementation phase can generate clean C code with correct iLLD usage and resource mapping.

## Data Inventory
- user_requirement (user input)
  - Contains: Goals to review/patch iLLD_TC387_ADS_GTM_TOM_3_Phase_Inverter_PWM_2, using IfxGtm_Pwm on TOM0 CH0/1, CH2/3, CH4/5, complementary via DTM0, center-aligned 10 kHz, period interrupt from TOM0.
  - Use: Drives the module API names (`initGtmTom3phInv()`, `updateGtmTom3phInvDuty()`), PWM topology, and interrupt intent.

- target_board (project metadata)
  - Contains: KIT_A2G_TC387_5V_TFT.
  - Use: Determines template/bsp inclusion and pin map header set for TC38x LFBGA-516.

- requirements_json (refiner node)
  - Contains: Peripheral = GTM_TOM, driver = IfxGtm_Pwm, TOM0 channels [0..5], center-aligned, 10 kHz, cmu_clk0_hz = 100000000, device_macro DEVICE_TC38X, pin_package IFX_PIN_PACKAGE_516, expected files [GTM_TOM_3ph_Inv_Pwm.c/h].
  - Use: Locks driver choice, timing, device macros, file structure, and initial clock assumptions.

- hardware_json (hardware node)
  - Contains: MCU TC387 verified, TOM0 channels [0..5], xtal 20 MHz, system 300 MHz; pin_assignments = TBD with conflicts (TOUT selection unresolved).
  - Use: Confirms resource availability and flags pin mapping as an open decision I must plan for.

- template_capabilities (template analyzer)
  - Contains: Available drivers incl. IfxGtm_PinMap_TC38x_516, IfxGtm_Pwm, IfxGtm_Tom, IfxGtm_Dtm; clock_config: 20 MHz xtal, 300 MHz PLL; pin_package IFX_PIN_PACKAGE_516.
  - Use: Validates that the necessary iLLD components and pin map headers exist in this template.

- header_documentation_context (intelligent header selector)
  - Contains: IfxGtm_Pwm usage notes (use IfxGtm_IrqMode_pulseNotify; API for sync channels only).
  - Use: Influences ISR mode selection and channel role assignment (base vs sync).

- extracted_config_values (library analyzer)
  - Contains: Hints like deadTime.rising/falling fields, output[i].pin pattern, macro IFXGTM_CH0CCU0_INTR_PRIO=3.
  - Use: Guides struct field naming and ISR priority macros in the design.

- detected_peripheral_type
  - Contains: PWM.
  - Use: Confirms peripheral category for module naming and layering.

## Execution Plan
1. Define module APIs and files
   - Files: GTM_TOM_3ph_Inv_Pwm.h/.c exporting initGtmTom3phInv() and updateGtmTom3phInvDuty(phaseA, phaseB, phaseC).
   - Internal state: descriptors for three complementary pairs and a dead-time config (deadTime.rising/falling).

2. Map resources
   - TOM: Use TOM0 channels {0/1, 2/3, 4/5} under TGC0.
   - DTM: Use DTM0 to generate complementary LS from HS with deadtime.

3. Plan pin routing
   - Select TOUT pins from IfxGtm_PinMap_TC38x_516.h for TOM0_CH0..5; expose via config constants output[i].pin.
   - Mark as BoardConfig decision gate.

4. Clock and period calculation
   - Use CMU CLK0 = 100 MHz; center-aligned up/down => periodTicks = 100e6 / (2*10e3) = 5000.
   - Ensure GTM and TOM clocks enabled via IfxGtm and IfxGtm_Cmu APIs.

5. Driver configuration layers
   - Include: IfxGtm_Pwm.h, IfxGtm_Dtm.h, IfxGtm_Tom.h, IfxGtm_PinMap.h, IfxPort.h, IfxSrc.h, Bsp.h.
   - Configure IfxGtm_Pwm channels as sync channels; avoid using base channel per header note.

6. Complementary + deadtime
   - Bind HS channel to DTM0 output; configure deadTime.rising/falling in nanoseconds (TBD, default-safe placeholder).
   - Set LS polarity inverted relative to HS.

7. ISR registration
   - Period interrupt source: TOM0 CH0 period (SRC_GTMTOM0_0).
   - Priority via IFXGTM_CH0CCU0_INTR_PRIO and cpu affinity; ISR mode IfxGtm_IrqMode_pulseNotify.

8. Initialization sequence
   - Enable GTM, CMU clocks, TOM0; configure DTM0; configure TOM0 pairs; load shadow registers; enable TGC0 update; start PWM.

9. Duty update pathway
   - updateGtmTom3phInvDuty(): percent-to-ticks conversion, clamp/symmetry handling, atomic shadow update, trigger TGC0 shadow transfer.

## Key Decisions & Risks
- Pin assignment: Specific TOUT pins for TOM0_CH0..5 on TC38x LFBGA‑516 are TBD; wrong choice leads to routing conflicts.
- Dead-time value: Not provided; needs a system-level value (e.g., 300–800 ns); mis-set deadtime risks shoot-through or distortion.
- IfxGtm_Pwm “sync-only” constraint: Must ensure all configured channels are sync channels; base-channel misuse would break API guarantees.
- Interrupt source: TOM0 CH0 period vs TGC interrupt; I choose CH0 period for simplicity—verify it matches system timing needs.
- Clock assumption: cmu_clk0_hz=100 MHz assumed; if different on this project, periodTicks must be recomputed.