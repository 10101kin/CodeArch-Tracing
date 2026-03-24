# AI Reasoning - Hardware Design

*Generated: 2026-03-24T07:15:31.779087*

---

## Phase Purpose
In Hardware Design, I will produce a device-specific hardware design JSON that binds the migrated PWM requirements to concrete TC4D7 resources: eGTM clocks, ATOM channels, output pins, and driver parameters. This definition is the contract that the code-generation phase uses to emit correct iLLD initialization and ensures the migrated behavior matches the reference on the new board.

## Data Inventory
- user_requirement (from user): TC4D7 on KIT_A3G_TC4D7_LITE; eGTM Cluster 0, ATOM0 CH0–2 as three complementary, center‑aligned PWM pairs at 20 kHz with 1 µs deadtime mapped to U(P00.3/P00.2), V(P00.5/P00.4), W(P00.7/P00.6); CH3 is 20 kHz 50% falling‑edge ADC trigger on P33. I’ll treat this as the primary intent and pin wishes.
- target_board (from tool input): KIT_A3G_TC4D7_LITE. I’ll select board-specific pin/package and clock defaults.
- requirements_json (from refiner): Structured form using driver `IfxEgtm_Atom_Pwm` on TC4xx, eGTM cluster 0/ATOM0, frequency 20 kHz, center-aligned, deadtime_us 1.0, initial duty U=25%, V=50%, W=…; device macro `DEVICE_TC4D7`, pin package `IFX_PIN_PACKAGE_BGA292_COM`. I’ll source canonical constraints and expected files from here.
- template_capabilities (from template library): Template path for KIT_A3G_TC4D7_LITE, TC4xx support, clock hints (xtal 25 MHz, syspll 500 MHz), but device macro `DEVICE_TC4DX` and package `IFX_PIN_PACKAGE_BGA436_COM`. I’ll use clocks, but override device/package per requirements_json; I’ll flag the mismatch.
- reference_analysis (from reference project): Shows GTM PWM usage patterns and watchdog/CPU init. I’ll mirror init ordering and style but not copy code.
- header_documentation_context (iLLD docs): API for `IfxEgtm_Pwm`/`IfxEgtm_Atom_Pwm`. I’ll map fields like `deadTime.rising/falling`, `output[].pin`, `complementaryPin`, `dtmClockSource`.
- extracted_config_values (from headers): Defaults such as `deadTime.rising=0.0f`, `dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0`, example pin arrays. I’ll override defaults with our 1 µs and selected pins.
- pin_mapping_from_docs (from pin validator): Example macros for ATOM0 CH0 on P02 (e.g., `IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT` and `_0N_TOUT1_P02_1_OUT`). I’ll use these to sanity‑check the complementary mapping model; pin availability on P00/P33 still needs validation.
- detected_peripheral_type: PWM. Confirms pathway.

## Execution Plan
1. Select driver and instance
   - Use `IfxEgtm_Atom_Pwm` on eGTM Cluster 0, `ATOM0`, channels 0–3 per requirements_json.

2. Clock/Timing synthesis
   - Choose CMU clock source `IfxGtm_Dtm_ClockSource_cmuClock0`.
   - Set CMU_CLK0 to a feasible board value (target 100 MHz using template clocks).
   - Compute period ticks for 20 kHz center-aligned: ticks = f_clk / 20,000 (e.g., 100 MHz → 5000).
   - Deadtime ticks: 1 µs × f_clk → 100 ticks. Set `deadTime.rising = deadTime.falling = 1e-6`.

3. Channel pairing and polarity
   - Configure CH0/1/2 as complementary outputs using `output[i].pin` and `output[i].complementaryPin` with opposite polarity fields.

4. Pin assignment
   - Attempt to bind: U → CH0 to P00.3 (high)/P00.2 (low); V → CH1 to P00.5/P00.4; W → CH2 to P00.7/P00.6.
   - Validate against device/package pin map. If not available, propose nearest valid TOUT pairs (e.g., P02_x from `pin_mapping_from_docs`) and record as alternative.

5. ADC trigger output
   - Configure CH3 at 20 kHz, 50% duty, falling-edge.
   - Route to a P33 TOUT if available; otherwise mark as internal-only trigger with “observable” fallback pin candidate list.

6. Device/package and files
   - Set `DEVICE_TC4D7`, `IFX_PIN_PACKAGE_BGA292_COM`; note template discrepancy.
   - Record expected files `egtm_atom_3ph_pwm.c/.h`.

## Key Decisions & Risks
- Pin feasibility: P00.2/3/4/5/6/7 and a P33 TOUT must exist for ATOM0 on TC4D7 BGA292; current doc sample only shows P02_x. Risk: remap needed.
- Clock source value: I assume CMU_CLK0 ≈ 100 MHz; must confirm actual GTM CMU clock tree on KIT_A3G_TC4D7_LITE to keep deadtime ticks integer and within range.
- Device/package mismatch: template has `DEVICE_TC4DX` and BGA436; I will override but call out potential template gaps.
- Complementary polarity and deadtime application: ensure both rising and falling are 1 µs; verify `complementaryPolarity` settings per `IfxEgtm_Pwm` docs.