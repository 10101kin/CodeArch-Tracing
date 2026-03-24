# AI Reasoning - Hardware Design

*Generated: 2026-03-24T08:55:10.374301*

---

## Phase Purpose
In Hardware Design, I will translate the PWM requirements into a concrete GTM/TOM configuration for KIT_A2G_TC387_5V_TFT: TOM instance, time-base channel, exact TOM-to-TOUT-to-pin mappings, GTM clock source, and device/package-specific parameters. This output feeds the code-gen phase so the iLLD driver (IfxGtm_Tom_PwmHl) can be instantiated with correct pins, timing, and pad settings on the TC3xx device.

## Data Inventory
- user_requirement (from user): Requests 3‑phase complementary center‑aligned PWM on TOM1 with CH0 as time base, 20 kHz, 0.5 µs dead‑time, 1.0 µs min pulse, cmuFxclk0, pushPull+cmosAutomotiveSpeed1; pins: U=P02.0/P02.7, V=P02.1/P02.4, W=P02.2/P02.5; use IfxGtm_Tom_PwmHl; verify IfxGtm_PinMap.h for TC387 LFBGA‑292; TOM instance TBD if P02.* not on TOM1. I will treat these as hard constraints and perform pin/TOM validation first.
- target_board (from selection): KIT_A2G_TC387_5V_TFT. I’ll align pin/package and check for board-level conflicts.
- requirements_json (from refiner): MIGRATION mode on TC3xx; peripheral_requirement.type=GTM_TOM, driver_name=IfxGtm_Tom_PwmHl; device_configuration: DEVICE_TC38X, pin_package=IFX_PIN_PACKAGE_292; timing: frequency_hz=20000, deadtime_us=0.5, min_pulse_us=1.0, gtm_clock.cmu_clock=cmuFxclk0. I’ll use these to populate the hardware JSON fields.
- template_capabilities (from template analyzer): Template path for KIT_A2G_TC387_5V_TFT; available pinmaps include IfxGtm_PinMap_TC38x_LFBGA292 and _516; clock_config lists 20 MHz XTAL, 300 MHz PLL; template pin_package=IFX_PIN_PACKAGE_516 (mismatch). I’ll select the LFBGA292 pinmap header and flag the package mismatch risk.
- reference_analysis (from reference project): Confirms IfxGtm_Tom_PwmHl usage, PWM_FREQ_HZ=20000, and init pattern via IfxGtm_Tom_Timer + PwmHl. I’ll mirror these patterns in the design assumptions.
- header_documentation_context (from docs): IfxGtm_PinMap and IfxGtm_Pwm API references. I’ll consult struct/field names (e.g., IfxGtm_Tom_PwmHl_Config, deadTime.rising/falling).
- extracted_config_values (from headers): Examples of fields like deadTime.rising/falling, output[i].pin pointers (e.g., IfxGtm_TOM1_5_TOUT11_P00_2_OUT), and clock enums. I’ll map output[i].pin to the P02.* macros.
- pin_mapping_from_docs (from pin validator): Example TOM1_5 on P00.2. I’ll use this to confirm naming patterns and then search for P02.* equivalents.
- detected_peripheral_type: PWM. Confirms we’re in the right peripheral domain.

## Execution Plan
1) Validate device/package context
   - Use requirements_json.device_configuration (DEVICE_TC38X, IFX_PIN_PACKAGE_292) and select IfxGtm_PinMap_TC38x_LFBGA292.h from template_capabilities; flag the 516 vs 292 mismatch.

2) Resolve P02.* TOM1 mappings
   - Search IfxGtm_PinMap_TC38x_LFBGA292.h for macros:
     - IfxGtm_TOM1_?_TOUT?_P02_0_OUT, P02_7_OUT, P02_1_OUT, P02_4_OUT, P02_2_OUT, P02_5_OUT.
   - Record exact TOM1 channel (CHx) and TOUT indices for each requested pin.

3) Verify pairing and TGC grouping
   - Ensure each HS/LS pair (U/V/W) belongs to the same TOM1 TGC (TGC0 or TGC1) to support complementary/dead-time via IfxGtm_Tom_PwmHl.
   - Confirm CH0 is free to serve as the time base and shares the same TOM module.

4) Define channel allocation
   - Assign three complementary pairs to available TOM1 channels based on the resolved macros; keep CH0 as the time-base for IfxGtm_Tom_Timer.

5) Clock and timing
   - Configure gtm_clock=cmuFxclk0; compute period ticks for 20 kHz (50 µs) using IfxGtm_Cmu_getModuleFrequency() and center-aligned mode.
   - Set deadTime.rising/falling=0.5 µs; minPulse=1.0 µs.

6) Output pin/pad configuration
   - Set each output[i].pin to the exact IfxGtm_TOM1_*_TOUT*_P02_*_OUT macro.
   - Set pad driver to IfxPort_PadDriver_cmosAutomotiveSpeed1 and pad mode to pushPull.

7) Initialize duty cycle presets
   - U/V/W initial duties: 25% / 50% / 75% via IfxGtm_Tom_PwmHl_setDuty() or config fields.

8) Produce hardware design JSON
   - Emit TOM instance, time-base, channel-to-pin map, TOUT indices, clock source, timing, pad driver, and notes on validation results/TBDs.

## Key Decisions & Risks
- TOM1 availability on P02.*: If any P02.* does not have a TOM1 mapping in LFBGA‑292, TOM instance selection is TBD pending user confirmation (alternatives: TOM0/TOM2 or different pins).
- TGC grouping: All HS/LS pairs must align within TOM1 TGCs; otherwise dead-time/complementary control breaks.
- Package/library mismatch: Template shows IFX_PIN_PACKAGE_516 while requirements request 292; must confirm the board’s actual package to pick the correct pinmap header.
- Board conflicts: P02.* may be used by TFT or other peripherals on KIT_A2G_TC387_5V_TFT; needs schematic cross-check.
- CMU FXCLK0 frequency: Must confirm actual GTM FXCLK0 to compute accurate ticks and honor minPulse/deadTime constraints.