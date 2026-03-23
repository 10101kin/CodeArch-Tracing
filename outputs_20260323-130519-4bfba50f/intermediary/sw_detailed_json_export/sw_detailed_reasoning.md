# AI Reasoning - Detailed Design

*Generated: 2026-03-23T13:42:32.397968*

---

## Phase Purpose
In Detailed Software Design, I will define the exact C interfaces and static configuration needed to generate code: function prototypes, module structs, constants (frequency, pins, deadtime), initialization and update sequences, plus unit-test specs. This locks down the GTM TOM1 3-phase PWM design so code-gen can produce buildable, reviewable sources without guessing.

## Data Inventory
- user_requirement (user): Refactor TC387 3-phase PWM to P02.0/2.7 (U), P02.1/2.4 (V), P02.2/2.5 (W), TOM1 at 20 kHz center-aligned, complementary with deadtime via IfxGtm_Tom_PwmHl; 500 ms duty updates starting 25/50/75% with +10% steps; pad IfxPort_PadDriver_cmosAutomotiveSpeed1; TBD: exact TOM1/TOUT macros. I will drive all design choices from this.
- target_board (user): KIT_A2G_TC387_5V_TFT. I will select the correct pin-map header for this kit/package.
- requirements_json (refiner/phase 1): Confirms peripheral: GTM_TOM, TOM1, driver IfxGtm_Tom_PwmHl; pwm_frequency_hz=20000; alignment=center; duty update plan; device_macro=DEVICE_TC38X; pin_package=IFX_PIN_PACKAGE_292. I will parameterize constants and select TC38x LFBGA292 pin map.
- hardware_json (phase 2): MCU TC387 verified; TOM1 channels [0..5]; pins assigned to P02.0/1/2/4/5/7; validation flags unresolved TOM1/TOUT macros for these pins. I will map TOM1 ch0..5 to U/V/W HS/LS and add a resolution task for macros.
- sw_architecture_json (phase 3): Module files GTM_TOM_3_Phase_Inverter_PWM.c/.h; init function name GTM_TOM_3_Phase_Inverter_PWM_init; driver calls (IfxGtm_enable, IfxGtm_Tom_Timer_init, IfxGtm_Tom_PwmHl_init). I will adopt these names and call sequences.
- template_capabilities (template analyzer): Provides available pin-map headers: IfxGtm_PinMap_TC38x_LFBGA292 and _516; clocks: xtal=20 MHz, pll=300 MHz. I will include the 292 pin-map (unless kit uses 516) and configure CMU for 20 kHz center-aligned TOM1.
- reference_analysis (reference project): Shows prior use of IfxGtm_Tom_PwmHl and macros PWM_FREQ_HZ=20000; deadtime/minPulse not extracted. I will keep deadtime/minPulse “as reference” and expose placeholders if values are missing.
- header_documentation_context (iLLD API): API signatures and struct fields for IfxGtm_Tom_PwmHl and Timer. I will base struct definitions and field usage on this.
- extracted_config_values (library analyzer): Examples of pin macros (e.g., IfxGtm_TOM1_5_TOUT11_P00_2_OUT), enum defaults (dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0). I will mirror field names and typical clock selections.

## Execution Plan
1) Define public APIs:
   - void GTM_TOM_3_Phase_Inverter_PWM_init(void);
   - void GTM_TOM_3_Phase_Inverter_PWM_start(void);
   - void GTM_TOM_3_Phase_Inverter_PWM_stop(void);
   - void GTM_TOM_3_Phase_Inverter_PWM_setDutyPercents(float32 u, float32 v, float32 w);
   - void GTM_TOM_3_Phase_Inverter_PWM_stepDutyPattern(void); // 500 ms scheduler
2) Define module struct:
   - Holds IfxGtm_Tom_Timer, IfxGtm_Tom_PwmHl, pin maps for 3 HS/LS pairs, pad driver, duty[] = {U,V,W}, update period ticks, pattern index.
3) Constants:
   - PWM_FREQ_HZ=20000; ALIGNMENT=center; UPDATE_PERIOD_MS=500; PAD=IfxPort_PadDriver_cmosAutomotiveSpeed1; DEVICE_TC38X guards.
4) Channel mapping:
   - TOM1 ch0/ch1 -> U HS/LS (P02.0/P02.7)
   - TOM1 ch2/ch3 -> V HS/LS (P02.1/P02.4)
   - TOM1 ch4/ch5 -> W HS/LS (P02.2/P02.5)
   - Fill IfxGtm_Tom_ToutMap pointers with pin macros from IfxGtm_PinMap_TC38x_LFBGA292.h (TBD resolution).
5) Timer setup:
   - Enable GTM; select CMU clock; configure TOM1 for center-aligned 20 kHz; link timer to PwmHl (per header docs).
6) PwmHl setup:
   - Complementary outputs with deadtime; reuse existing deadtime/minPulse from reference (if found), else declare config hooks.
7) GPIO pads:
   - Configure P02.x to push-pull with IfxPort_PadDriver_cmosAutomotiveSpeed1.
8) Duty scheduler:
   - Pattern start: U=25%, V=50%, W=75%; every 500 ms increment +10% modulo 0–100% inclusive; saturate edges.
9) Safe updates:
   - Use sync update (IfxGtm_Tom_PwmHl_setOnTime/shift + trigger update) to avoid glitches.
10) Unit tests:
   - Validate period register vs 20 kHz; verify complementary and deadtime enabled; step-through duty pattern; assert pin macros compile; probe pad driver config.

## Key Decisions & Risks
- Pin-map resolution: Exact IfxGtm_TOM1_x_TOUTy_P02_z_OUT macros for P02.0/1/2/4/5/7 are TBD; requirements_json says IFX_PIN_PACKAGE_292 but template default shows 516. I will target IfxGtm_PinMap_TC38x_LFBGA292 and fall back to _516 only if the kit variant demands it.
- Deadtime/minPulse values are not present; I will preserve from the reference if available, else require confirmation to avoid shoot-through risk.
- Channel-to-pin feasibility: Must confirm TOM1 supports all six P02.x outputs concurrently; if any pin lacks TOM1, I will propose alternate P02 mappings or a different TOMx.
- 500 ms timing source: Prefer IfxStm (via Bsp.h) for tick generation; confirm CPU clock (300 MHz) and tick conversion.