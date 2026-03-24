# AI Reasoning - Software Architecture

*Generated: 2026-03-24T11:30:32.413720*

---

## Phase Purpose
In Software Architecture, I define the module boundaries, driver abstractions, include hierarchy, ISR wiring, and the startup/update sequence that will realize the 3‑phase PWM behavior on KIT_A2G_TC387_5V_TFT. The outcome is a clear blueprint for implementing center‑aligned 20 kHz complementary PWM on TOM1 with deadtime/min‑pulse constraints and a synchronous 500 ms duty stepping pattern.

## Data Inventory
- user_requirement (from user): Requests U/V/W start at 25/50/75%, step +10% every 500 ms with wrap, synchronous updates; keep TOM1, pins P00.3/2, P00.5/4, P00.7/6; 20 kHz up‑down; deadtime 0.5 µs; min pulse 1.0 µs; driver TBD. I’ll lock these as non‑negotiable constraints.
- target_board (from setup): KIT_A2G_TC387_5V_TFT. Confirms board context and BSP availability.
- requirements_json (from refiner): Peripheral = GTM_TOM, driver_name = IfxGtm_Tom_PwmHl, phases U/V/W, initial duties U:25 V:50 W:75, device_macro DEVICE_TC38X, expected files GTM_TOM_3_Phase_Inverter_PWM.c/h, includes used (Ifx_Types.h, IfxCpu.h, IfxScuWdt.h, Bsp.h, IfxGtm_Tom_PwmHl.h). I’ll adopt PwmHl and reuse the expected file pattern and includes.
- hardware_json (from hardware): TOM1 channels [5..10] mapped to pins:
  - U: high P00.3 (IfxGtm_TOM1_6_TOUT12_P00_3_OUT), low P00.2 (TOM1_5_TOUT11_P00_2)
  - V: high P00.5 (TOM1_8_TOUT14), low P00.4 (TOM1_7_TOUT13)
  - W: high P00.7 (TOM1_10_TOUT16), low P00.6 (TOM1_9_TOUT15)
  Clocks: xtal 20 MHz, sys 300 MHz. I’ll bind PwmHl channels to TOM1 {5..10} and pick TOM1_0 as the base timer (not pinned).
- template_capabilities (from template analyzer): TC3xx iLLD present, PinMap TC38x_516 headers available. Confirms driver/header availability.
- reference_analysis (from reference project): Uses IfxGtm_Tom_PwmHl and IfxGtm_Tom_Timer, files GTM_TOM_3_Phase_Inverter_PWM.c/h, pattern IfxGtm_Tom_Timer_init(...). I’ll mirror this structure and style.
- header_documentation_context (from header selector): IfxGtm_Pwm notes; default IrqMode = IfxGtm_IrqMode_pulseNotify; sync channels only. I’ll configure pulseNotify for safe sync updates.
- extracted_config_values (from library analyzer): Hints for deadTime.rising, config.syncUpdateEnabled, pin map macros, IFXGTM_CH0CCU0_INTR_PRIO=3. I’ll enable sync updates, set priorities, and use TC38x_516 pin macros.
- detected_peripheral_type: PWM. Confirms scope.

## Execution Plan
1) Driver selection: Use IfxGtm_Tom_PwmHl (requirements_json, hardware_json) to get complementary outputs, deadtime, min‑pulse.
2) Module decomposition:
   - GTM_TOM_3_Phase_Inverter_PWM.c/h: exports init(), setDuties(float U,V,W), requestSyncUpdate(), and an ISR notify hook.
   - app_duty_manager.c/h: owns 500 ms stepping and wrap logic.
3) Time base: Configure IfxGtm_Tom_Timer on TOM1 channel 0:
   - 20 kHz up‑down (center‑aligned), syncUpdateEnabled = true, IrqMode = pulseNotify.
   - Route ISR (SRC_GTMTOM1_0) to CPU0, priority 3 (IFXGTM_CH0CCU0_INTR_PRIO).
4) PWM HL channels: Map pairs to TOM1 {5/6}=U, {7/8}=V, {9/10}=W with pin macros from hardware_json; polarity for complementary mode; deadtime = 0.5 µs; minPulse = 1.0 µs.
5) Include hierarchy: Ifx_Types.h, IfxCpu.h, IfxScuWdt.h, Bsp.h, IfxGtm_Tom_Timer.h, IfxGtm_Tom_PwmHl.h, IfxGtm_PinMap_TC38x_516.h, GTM_TOM_3_Phase_Inverter_PWM.h.
6) Initialization sequence:
   - Disable watchdogs; init GTM CMU (use template defaults), init TOM1 timer ch0; init PwmHl with channels 5..10.
   - Set initial duties U=25,V=50,W=75 via shadow; call requestSyncUpdate() so all 6 channels latch at the next period boundary.
7) ISR and sync: In TOM1_0 ISR, pulseNotify sets a flag; main loop/app uses it to know the safe point passed.
8) 500 ms stepping: In app_duty_manager, use Bsp_waitTime(500ms) or STM tick to compute next (d+10)%100 and call setDuties() + requestSyncUpdate().

## Key Decisions & Risks
- Decision: Use IfxGtm_Tom_PwmHl (complementary, deadtime) vs IfxGtm_Tom_Pwm (single‑ended). Chosen PwmHl per requirements_json and deadtime need.
- Base channel: Assume TOM1_0 is free; if reserved, pick another non‑pinned TOM1 channel.
- Timing: Ensure CMU clock yields exact 20 kHz up‑down; verify minPulse + deadtime do not violate extreme duty ends.
- Update cadence: Prefer STM/BSP delay over counting 20 kHz interrupts to avoid ISR load.
- Pin constraints: Must keep P00.3/2, .5/4, .7/6; any remap is a hard failure.