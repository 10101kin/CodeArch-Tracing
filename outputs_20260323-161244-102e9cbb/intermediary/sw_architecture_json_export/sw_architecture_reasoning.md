# AI Reasoning - Software Architecture

*Generated: 2026-03-23T16:30:25.098401*

---

## Phase Purpose
In this Software Architecture phase, I will define the module breakdown, driver abstractions, include hierarchy, ISR strategy, and the init/update sequence for a 3-phase, six-channel PWM on AURIX TC387. The goal is a clear, maintainable structure that implements a shared GTM TOM timer at 20 kHz, center-aligned, driving six single-output `IfxGtm_Tom_Pwm` channels mapped to the specified P02.x pins.

## Data Inventory
- user_requirement (from user): Six single-output PWM channels on a shared `IfxGtm_Tom_Timer` at 20 kHz, center-aligned; pins PHASE_U: P02.0/P02.7, PHASE_V: P02.1/P02.4, PHASE_W: P02.2/P02.5; initial HS duties 25/50/75%, update +10% every 500 ms with wrap; LS default = complementary; TOM instance/ch selection TBD. I’ll drive architecture to enforce this behavior and leave LS polarity configurable.

- target_board (from user): `KIT_A2G_TC387_5V_TFT`. I’ll align PinMap headers and BSP assumptions to this kit.

- requirements_json (from refiner, phase 1): Confirms peripheral type `GTM_TOM`, driver `IfxGtm_Tom_Pwm`, 20 kHz center-aligned, update_interval_ms=500, shared timer: `IfxGtm_Tom_Timer` on `TOM0` channel 3 (route_assigned=false), device macro `DEVICE_TC38X`, pin package `IFX_PIN_PACKAGE_292`, expected files `GTM_TOM_3_Phase_Inverter_PWM.c/.h`. I’ll anchor module/API names and timer/channel choice to this.

- hardware_json (from hardware, phase 2): MCU `TC387`, TOM instance `TOM0`, available channels `[0,1,2,4,5,7]`, clock 20 MHz XTAL / 300 MHz system, and pin mappings like `P02.0 -> IfxGtm_TOM0_0_TOUT0_P02_0_OUT`. I’ll lock final channel-to-pin routing:
  - U: HS ch0 → P02.0, LS ch7 → P02.7
  - V: HS ch1 → P02.1, LS ch4 → P02.4
  - W: HS ch2 → P02.2, LS ch5 → P02.5
  - Shared timer: TOM0 ch3 (no TOUT route)

- template_capabilities (from template analyzer): Provides `IfxGtm_Tom_Pwm`, `IfxGtm_Tom_Timer`, and PinMap headers for TC38x; template pin_package lists 516 but also includes `IfxGtm_PinMap_TC38x_LFBGA292`. I’ll select the 292 PinMap header explicitly.

- reference_analysis (from reference project analyzer): Prior module `GTM_TOM_3_Phase_Inverter_PWM.c/.h`, includes `IfxGtm_cfg_TC38x.h`, watchdog pattern, and an initialization pattern using a timer handle. I’ll mirror file names and init flow, but use `IfxGtm_Tom_Pwm` (not PwmHl).

- header_documentation_context / extracted_config_values: Confirms presence of TOM channel enums (e.g., `IfxGtm_Tom_Ch_0`), pin macros, and interrupt priority patterns. I’ll apply these for config structs and ISR priorities.

- detected_peripheral_type: PWM. Confirms focus.

## Execution Plan
1) Select drivers and headers
   - Use `IfxGtm_Tom_Timer` (TOM0 ch3) and six `IfxGtm_Tom_Pwm` instances.
   - Includes: `Ifx_Types.h`, `IfxCpu.h`, `IfxScuWdt.h`, `IfxGtm_Tom_Pwm.h`, `IfxGtm_Tom_Timer.h`, `IfxGtm_cfg_TC38x.h`, `IfxGtm_PinMap_TC38x_LFBGA292.h`.

2) Define module API and files
   - Files: `GTM_TOM_3_Phase_Inverter_PWM.c/.h`.
   - Public API: `pwm3ph_init()`, `pwm3ph_setDuty(float hsU, hsV, hsW, bool complementary)`, `pwm3ph_task500ms()`; optional `pwm3ph_isr()`.

3) Configure GTM and shared timer
   - Enable GTM, set CMU clocks from 20 MHz XTAL.
   - Init `IfxGtm_Tom_Timer` on TOM0 ch3 for 20 kHz center-aligned (up-down), `syncUpdateEnabled = TRUE`, ISR enabled with provider CPU0 and defined priority.

4) Bind PWM channels to pins
   - Create 6 `IfxGtm_Tom_Pwm_Config` using TOM0 channels [0,1,2,4,5,7].
   - Assign pins via macros (e.g., `IfxGtm_TOM0_0_TOUT0_P02_0_OUT`), link to the timer time base, center-aligned mode, no dead-time.

5) Initialization sequence
   - Initialize timer first; then each PWM channel.
   - Set initial HS duties: U=25%, V=50%, W=75%.
   - Compute LS duties: complementary (1 - HS) by default; store a policy flag for same-polarity option.
   - Apply a synchronous shadow transfer so all six outputs update together.

6) Update mechanism (every 500 ms)
   - Use the timer ISR to increment a counter; every 10,000 periods (~500 ms at 20 kHz), call `pwm3ph_task500ms()` to add +10% with wrap and recompute LS based on policy; apply synchronized update.
   - If ISR load is a concern, allow alternative scheduling via STM/OS tick (architected as a weak hook).

7) Include hierarchy and build-time guards
   - Guard pin package selection (prefer 292; fall back warning if 516 header appears).
   - Define ISR priority macros (e.g., `TOM0_CH3_PRIO`) in the header.

## Key Decisions & Risks
- Low-side behavior: complementary vs same-polarity is TBD; I’ll implement runtime-selectable policy, default complementary, no dead-time.
- Pin package mismatch: requirements say 292; template default shows 516—will force `LFBGA292` PinMap and flag if inconsistent.
- ISR strategy: 20 kHz timer ISR is frequent; logic kept minimal (counter only). Alternative: use a slower tick; document trade-off.
- Driver choice: using `IfxGtm_Tom_Pwm` (single-output) instead of `PwmHl`; ensures independent LS channel control but requires careful sync updates.
- Safety: zero dead-time can risk shoot-through on real hardware; callout for HW validation before enabling same-polarity mode.