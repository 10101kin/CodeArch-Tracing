# AI Reasoning - Detailed Design

*Generated: 2026-03-24T11:32:58.120486*

---

## Phase Purpose
In this Detailed Software Design phase, I will produce the concrete blueprint for code generation: exact C function signatures, module/driver structs, configuration constants, TOM1 channel/pin bindings, initialization and update sequences, and unit-test specifications. This matters because it locks down all driver/API choices and timing math so the generator can emit compilable code that meets the 20 kHz center-aligned, complementary PWM with deadtime and synchronous updates.

## Data Inventory
- user_requirement (from user): 3-phase PWM on KIT_A2G_TC387_5V_TFT, TOM1, pins P00.3/2, P00.5/4, P00.7/6, 20 kHz center-aligned, deadtime 0.5 µs, min pulse 1.0 µs, initial duties U=25%, V=50%, W=75%, +10% every 500 ms with wrap, synchronous updates; driver TBD. I will use this as the acceptance criteria and to drive driver selection (PwmHl vs Pwm).
- target_board (from user profile): KIT_A2G_TC387_5V_TFT. Confirms board template and pin package context.
- requirements_json (from refiner): Peripheral = GTM_TOM, TOM1, pins and timing as above, device_macro=DEVICE_TC38X, expected files GTM_TOM_3_Phase_Inverter_PWM.c/h. I’ll align file/function names and constants to this.
- hardware_json (from hardware phase): GTM TOM1 channels [5..10], driver IfxGtm_Tom_PwmHl, pin map:
  - U_H P00.3 TOM1_6 TOUT12; U_L P00.2 TOM1_5 TOUT11
  - V_H P00.5 TOM1_8 TOUT14; V_L P00.4 TOM1_7 TOUT13
  - W_H P00.7 TOM1_10 TOUT16; W_L P00.6 TOM1_9 TOUT15
  Clock: xtal 20 MHz, system 300 MHz. I’ll bind channels/pins exactly as listed and assume GTM clock derived per template.
- sw_architecture_json (from SW architecture): Suggested high-level driver IfxGtm_Pwm / IfxGtm_Tom_Pwm; init function name initGtmTom3phInv. I’ll keep names but override driver to PwmHl to meet deadtime/complementary requirement.
- template_capabilities (from template analyzer): TC3xx iLLD available, PinMap TC38x_516 present, PLL 300 MHz. I’ll reference available headers and ensure include set is valid.
- reference_analysis (from reference project): Prior usage of IfxGtm_Tom_PwmHl and TOM timer pattern; macros PWM_FREQ_HZ=20000; common watchdog disable. I’ll mirror init patterns and naming.
- header_documentation_context (from header selector): IfxGtm_Pwm notes about sync channels. I’ll enforce shadow-transfer synchronized updates.
- extracted_config_values (from library analyzer): Snippets of pin macros like IfxGtm_TOM1_5_TOUT11_P00_2_OUT and deadTime fields. I’ll use these exact pin macros and struct field names.
- detected_peripheral_type: PWM. Confirms scope.

## Execution Plan
1) Select driver: choose IfxGtm_Tom_PwmHl for complementary high/low with deadtime; keep TOM1 as time base.
2) Define module structs:
   - Runtime: Gtm3Ph_Context with IfxGtm_Tom_PwmHl pwm, IfxGtm_Tom_Timer timer, dutyPercent[3].
   - Config: Gtm3Ph_Config with frequencyHz=20000, deadtimeNs=500, minPulseNs=1000, channel map TOM1 (pairs 5/6, 7/8, 9/10) and pins P00.{3/2,5/4,7/6}.
3) Constants/macros: DEVICE_TC38X, IFX_PIN_PACKAGE_516, PWM_FREQ_HZ 20000, DEADTIME_NS 500, MIN_PULSE_NS 1000, UPDATE_INTERVAL_MS 500, initial duties {25,50,75}.
4) Function signatures:
   - void initGtmTom3phInv(void);
   - void gtm3ph_setDutiesSync(uint8 u, uint8 v, uint8 w);
   - void gtm3ph_stepDuties500ms(void); // +10% wrap, then sync apply
   - static inline uint16 gtm3ph_percentToTicks(uint8 pct);
5) Initialization sequence:
   - Enable GTM, configure CMU clocks (use template defaults), init TOM1 timer up-down for 20 kHz, compute period ticks.
   - Init IfxGtm_Tom_PwmHl with channel pairs (5/6,7/8,9/10), bind pins via IfxGtm_TOM1_x_TOUTy_P00_z_OUT.
   - Set deadtime = 0.5 µs, enable min pulse clamp = 1.0 µs.
   - Enable syncUpdate on all channels; load initial duties 25/50/75 using shadow registers; trigger a single global update.
6) Duty update logic:
   - In gtm3ph_stepDuties500ms: add +10 with modulo 100 per phase, clamp to respect MIN_PULSE_NS, write all three duties to shadows, then trigger one shadow transfer for synchronous edge update.
7) Unit-test specifications:
   - Init applies 25/50/75 within ±1 tick.
   - Updates at 500 ms change all three channels in the same PWM cycle (single shadow transfer event).
   - Wrap at 100% → 0% while honoring min pulse.
   - Deadtime of 0.5 µs enforced on both edges; no overlap.

## Key Decisions & Risks
- Driver choice: IfxGtm_Tom_PwmHl (overrides sw_architecture’s IfxGtm_Pwm suggestion) to meet complementary + deadtime.
- GTM clock source/ticks: template doesn’t state GTM CMU exact frequency; I will compute period/min-pulse/deadtime ticks from the active GTM clock obtained at runtime (e.g., IfxGtm_Cmu_getModuleFrequency). Risk: mismatch if CMU not set as expected.
- Synchronous update: ensure all three TOM1 channels participate in the same TRIG/Shadow transfer; risk if a base channel is mis-assigned.
- 500 ms scheduler: not specified; I’ll define API only and assume caller triggers every 500 ms (STM/BSP).