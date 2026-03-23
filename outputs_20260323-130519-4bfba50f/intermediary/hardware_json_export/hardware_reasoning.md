# AI Reasoning - Hardware Design

*Generated: 2026-03-23T13:15:17.260701*

---

## Phase Purpose
In Hardware Design, I translate the high-level PWM requirements into a concrete AURIX GTM/TOM1 resource plan: exact TOM channels, TOUT pin macros, clock sources, pad drivers, and timer/deadtime parameters. This produces the hardware design JSON the next phases will use to generate correct iLLD initialization and safe complementary 3‑phase PWM on KIT_A2G_TC387_5V_TFT.

## Data Inventory
- user_requirement (from you)
  - What: Remap 3‑phase PWM to P02.0/P02.7 (U HS/LS), P02.1/P02.4 (V HS/LS), P02.2/P02.5 (W HS/LS); TOM1 at 20 kHz center‑aligned using IfxGtm_Tom_PwmHl; duty update every 500 ms cycling 0–100%; pad driver IfxPort_PadDriver_cmosAutomotiveSpeed1; TBD: exact TOM1/TOUT macros.
  - Use: Hard constraints for pins, driver, frequency, alignment, duty plan, and pad driver.

- target_board (from board selection)
  - What: KIT_A2G_TC387_5V_TFT.
  - Use: Select correct pin‑package and pin‑map headers for TC38x LFBGA‑516 and board clocks.

- requirements_json (from refiner)
  - What: Structured fields: peripheral_requirement.type=GTM_TOM, driver_name=IfxGtm_Tom_PwmHl; timing_requirements.pwm_frequency_hz=20000, alignment=center; signal_requirements for P02.[0,1,2,4,5,7]; device_configuration.target_device_macro=DEVICE_TC38X, pin_package=IFX_PIN_PACKAGE_292.
  - Use: Source of truth for parameters; flag pin‑package mismatch vs board.

- template_capabilities (from template analyzer)
  - What: Device TC3xx/DEVICE_TC38X; pin_package IFX_PIN_PACKAGE_516; clocks: xtal 20 MHz, PLL 300 MHz; available pinmaps IfxGtm_PinMap_TC38x_LFBGA516.h.
  - Use: Determine GTM clock source and resolve TOUT macros for P02.x on TOM1 for LFBGA‑516.

- reference_analysis (from project analyzer)
  - What: Confirms IfxGtm_Tom_PwmHl and IfxGtm_Tom_Timer usage; macro PWM_FREQ_HZ=20000; init pattern for timer and PwmHl.
  - Use: Reuse init style; attempt to reuse existing deadtime/minPulse values if found.

- header_documentation_context (from iLLD docs)
  - What: IfxGtm_Tom_PwmHl API/struct fields (deadTime, minPulse, syncUpdateEnabled, dtmClockSource).
  - Use: Populate config fields correctly (center‑aligned, DTM clock, complementary).

- extracted_config_values (from header parsing)
  - What: Examples: deadTime.rising, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0; sample pin map IfxGtm_TOM1_5_TOUT11_P00_2_OUT.
  - Use: Field names/defaults; pin map pattern reference.

- pin_mapping_from_docs (from pin validator)
  - What: Example TOM1_5 on P00.2; not our target pins.
  - Use: Confirms lookup method; we still must resolve P02.x for TOM1.

## Execution Plan
1. Reconcile device/package
   - Adopt DEVICE_TC38X and LFBGA‑516 per KIT_A2G_TC387_5V_TFT (template_capabilities); flag the IFX_PIN_PACKAGE_292 mismatch (requirements_json).

2. Resolve TOUT macros for requested pins
   - Search IfxGtm_PinMap_TC38x_LFBGA516.h for P02.0, P02.1, P02.2, P02.4, P02.5, P02.7 entries restricted to TOM1 (pattern IfxGtm_TOM1_x_TOUTy_P02_z_OUT).
   - Record available TOM1 channel indices and TOUT numbers for each pin.

3. Validate TOM1 grouping for PWMHL
   - Ensure each HS/LS pair (U, V, W) resides in the same TOM1 TGC (prefer TGC0: channels 0–7) to allow synchronous update and DTM deadtime.
   - If multiple candidates exist, choose contiguous pairs (e.g., ch0/ch1 for U, ch2/ch3 for V, ch4/ch5 for W) to simplify configuration.

4. Build PwmHl output mapping
   - Fill IfxGtm_Tom_PwmHl_Config output pairs with the resolved pin macros and set push‑pull + IfxPort_PadDriver_cmosAutomotiveSpeed1 for all six pins.
   - Set low‑side inverted as required for complementary outputs.

5. Configure timer and clocks
   - Use IfxGtm_Tom_Timer on TOM1 with frequency 20 kHz, center‑aligned; clock source CMU CLK0; set dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0.
   - Enable syncUpdateEnabled for atomic shadow transfers.

6. Deadtime and minPulse
   - Extract and reuse deadTime/minPulse from reference files; if absent, keep existing project values unchanged per requirement.

7. Duty update plan
   - Encode duty_update_period_ms=500 and duty_cycle_plan (start 25/50/75%, ±10% steps, clamp 0–100%) in the hardware design JSON metadata for the app layer.

8. Conflicts/sanity checks
   - Verify pins aren’t used by the TFT or other board functions; confirm no TOM1 channel conflicts; document any deviations.

## Key Decisions & Risks
- Pin‑map availability: P02.[0,1,2,4,5,7] may not all be TOM1‑capable in LFBGA‑516; if any pin lacks a TOM1 mapping, I must either propose alternate P02 pins with TOM1 or negotiate TOM module change (contradicts “keep TOM1”).
- TGC grouping: If HS/LS pairs fall across TGC0/TGC1, deadtime/sync may be constrained; may need to reshuffle channel selections.
- Package mismatch: requirements_json says IFX_PIN_PACKAGE_292, board is IFX_PIN_PACKAGE_516 — I will proceed with 516 and flag for confirmation.
- Unknown deadtime/minPulse: If not recoverable from reference, risk of unsafe switching; I will not alter existing values without explicit data.
- Board resource conflicts: P02.4/P02.5/P02.7 may be shared with TFT or other features on KIT_A2G_TC387_5V_TFT; requires schematic check.