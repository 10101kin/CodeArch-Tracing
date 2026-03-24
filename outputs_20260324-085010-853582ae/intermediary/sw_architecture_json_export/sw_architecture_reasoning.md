# AI Reasoning - Software Architecture

*Generated: 2026-03-24T09:00:54.568650*

---

## Phase Purpose
In this Software Architecture phase, I define the high-level module breakdown, driver abstractions, include hierarchy, ISR bindings, and the init/start sequence for the GTM TOM 3‑phase complementary center‑aligned PWM. The goal is to realize the user’s timing and pinout requirements on KIT_A2G_TC387_5V_TFT using iLLD (IfxGtm_Tom_PwmHl), while de-risking TOM instance/pin-map correctness for TC387 LFBGA‑292.

## Data Inventory
- user_requirement (from user)
  - What: Use P02.0/P02.7 (U), P02.1/P02.4 (V), P02.2/P02.5 (W), 20 kHz, 0.5 µs dead‑time, 1.0 µs min pulse, cmuFxclk0, pushPull + cmosAutomotiveSpeed1, TOM1 with CH0 as time base, initial duties 25/50/75, replace PHASE_* with IfxGtm_PinMap.h.
  - How: Drives pin mapping, timing, polarity, driver choice, and init order; triggers TOM1 vs TOM0 verification.

- target_board (from project setup)
  - What: KIT_A2G_TC387_5V_TFT.
  - How: Select correct TC387 family and board BSP assumptions.

- requirements_json (from refiner node)
  - What: Structured spec confirming GTM_TOM + IfxGtm_Tom_PwmHl, DEVICE_TC38X, LFBGA‑292 focus, cmuFxclk0, 20 kHz, deadtime/minPulse; expected files GTM_TOM_3_Phase_Inverter_PWM.[ch].
  - How: Source of truth for config fields and deliverable files.

- hardware_json (from hardware node)
  - What: MCU TC387 verified; peripheral_instance currently TOM0, channels [0,1,2,4,5,7]; pin macros suggest TOM0 on P02.x (e.g., IfxGtm_TOM0_0_TOUT0_P02_0_OUT); validation conflict on package (292 vs 516).
  - How: Confirms that P02.x pins are on TOM0, not TOM1; flags package mismatch risk.

- template_capabilities (from template analyzer)
  - What: Available pin maps include IfxGtm_PinMap_TC38x_LFBGA292; template pin_package IFX_PIN_PACKAGE_516; clock 20 MHz XTAL, 300 MHz PLL.
  - How: Choose correct pin-map header; note package mismatch to resolve.

- reference_analysis (from reference project analyzer)
  - What: Uses IfxGtm_Tom_PwmHl; includes Ifx_Types.h, IfxCpu.h, IfxScuWdt.h, IfxGtm_Tom_PwmHl.h, IfxGtm_PinMap.h, Bsp.h; timer direct init pattern.
  - How: Guides include hierarchy and init sequence.

- header_documentation_context (from header selector)
  - What: IfxGtm_PinMap and IfxGtm_Pwm docs stubs.
  - How: Pin-map symbol forms and API expectations.

- extracted_config_values (from library analyzer)
  - What: Examples of output[x].pin pointing to IfxGtm_TOM1_*; enums for dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0; macros like IFXGTM_CH0CCU0_INTR_PRIO.
  - How: Populate PwmHl config (pins, DTM clock), and ISR priority.

- detected_peripheral_type
  - What: PWM.
  - How: Confirms driver family.

## Execution Plan
1. Verify pin-map on TC387 LFBGA‑292 using IfxGtm_PinMap_TC38x_LFBGA292.h: resolve P02.0/1/2/4/5/7 mappings. Hardware_json indicates TOM0 TOUT0/1/2/4/5/7 on P02.x.
2. Decide provisional TOM instance: use TOM0 (channels 0,1,2,4,5,7) to honor the P02 pins; keep TOM1 as TBD pending user confirmation if TOM1 is mandatory.
3. Define module decomposition:
   - File: GTM_TOM_3_Phase_Inverter_PWM.c/.h.
   - API: Pwm3Phase_init(), Pwm3Phase_start(), Pwm3Phase_setDuties(25/50/75 init), optional Pwm3Phase_isr().
4. Include hierarchy: Ifx_Types.h, IfxCpu.h, IfxScuWdt.h, IfxGtm_Tom_PwmHl.h, IfxGtm_PinMap.h, Bsp.h, module header (per reference_analysis).
5. Clocking: enable GTM, select cmuFxclk0; ensure system 300 MHz/XTAL 20 MHz (template_capabilities) and compute 20 kHz period (50 µs).
6. Timer base: configure IfxGtm_Tom_Timer on TOMx CH0 as time base (center‑aligned), update ISR on CCU0 with IFXGTM_CH0CCU0_INTR_PRIO; bind SRC for TOMx_0 to CPU0.
7. PwmHl configuration:
   - Outputs:
     - U: HS=&IfxGtm_TOM0_0_TOUT0_P02_0_OUT, LS=&IfxGtm_TOM0_7_TOUT7_P02_7_OUT
     - V: HS=&IfxGtm_TOM0_1_TOUT1_P02_1_OUT, LS=&IfxGtm_TOM0_4_TOUT4_P02_4_OUT
     - W: HS=&IfxGtm_TOM0_2_TOUT2_P02_2_OUT, LS=&IfxGtm_TOM0_5_TOUT5_P02_5_OUT
   - Mode: complementary, center‑aligned, push‑pull, pad driver IfxPort_PadDriver_cmosAutomotiveSpeed1.
8. Dead-time/min-pulse: set deadtime=0.5 µs via PwmHl/DTM (dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0); set minPulse=1.0 µs.
9. Initialization sequence: disable WDT, IfxGtm_enable, init Timer, init PwmHl, apply initial duties 25/50/75, enable outputs, start timer run.
10. Replace legacy PHASE_* macros with the above IfxGtm_PinMap symbols; add static asserts to guard device/package.

## Key Decisions & Risks
- TOM instance selection: P02.* pins appear on TOM0; requirement asked for TOM1. I will proceed with TOM0 and await confirmation if TOM1 is strictly required.
- CH0 as time base vs U_HS on CH0: If CH0 is reserved for the timer, U_HS cannot also use CH0. Current mapping places U_HS on TOUT0 (CH0). We must either:
  - Use a different base channel (e.g., CH3) or
  - Move U_HS to a non-CH0 TOUT and keep CH0 as base.
- Package mismatch: Template defaults to IFX_PIN_PACKAGE_516, while requirement targets LFBGA‑292; I’ll select the TC38x_LFBGA292 pin-map and flag template config update.
- DTM availability/timing margins: Ensure DTM routing for TOM0 pairs and that 0.5 µs deadtime + 1.0 µs minPulse are achievable with cmuFxclk0 resolution.