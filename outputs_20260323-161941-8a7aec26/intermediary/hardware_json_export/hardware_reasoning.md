# AI Reasoning - Hardware Design

*Generated: 2026-03-24T00:21:45.609895*

---

## Phase Purpose
In Hardware Design, I produce a concrete eGTM/ATOM resource plan for KIT_A3G_TC4D7_LITE: clocking, ATOM instance/channel usage, complementary PWM pairing, and TOUT pin assignments. This matters because the subsequent driver config/code-gen must match real TC4xx resources to preserve the 20 kHz center-aligned complementary PWM with the specified deadtime and minimum pulse.

## Data Inventory
- user_requirement (from user): Migrate TC387 TOM 3‑phase inverter PWM to TC4D7 using eGTM ATOM unified PWM HL; keep 20 kHz center‑aligned, 0.5 µs deadtime, 1.0 µs min pulse; use eGTM CMU FXCLK0 from GCLK; one ATOM instance for all six outputs (prefer ATOM1). I’ll anchor timing and instance selection from this.
- target_board (from user): KIT_A3G_TC4D7_LITE. I’ll target board-level pin availability.
- requirements_json (from refiner): Structured constraints: driver IfxEgtm_Pwm, phases U/V/W with high/low pairs, device_macro DEVICE_TC4DX, pin_package IFX_PIN_PACKAGE_BGA436_COM, expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h. I’ll map these into the hardware JSON fields.
- template_capabilities (from template analyzer): TC4xx family, device_macro DEVICE_TC4DX, xtal 25 MHz, syspll 500 MHz, pin package BGA436_COM. I’ll derive GCLK→FXCLK0 (e.g., 500 MHz / 5 = 100 MHz) and ensure device macros align.
- reference_analysis (from reference project analyzer): Prior TC38x used IfxGtm_Tom_PwmHl with PWM_FREQ_HZ 20000 and defined deadtime. I’ll preserve timing/alignment while switching to IfxEgtm_Pwm on TC4xx.
- header_documentation_context (from header selector): IfxEgtm_Pwm usage and fields. I’ll use struct names/fields (e.g., deadTime.rising/falling, complementaryPolarity, dtmClockSource).
- extracted_config_values (from library analyzer): Examples show deadTime.rising/falling fields, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, IFXEGTM_PWM_NUM_CHANNELS=8. I’ll set DTM source and encode deadtime/min-pulse in ticks.
- pin_mapping_from_docs (from pin validator): Concrete examples for ATOM0 ch0: IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT and complementary IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT. I’ll use this pattern and resolve full channel list for the chosen ATOM instance.
- detected_peripheral_type (system): PWM. Confirms path.

## Execution Plan
1) Select driver and mode: Use IfxEgtm_Pwm in complementary PwmHl, center-aligned per requirements_json.
2) Clocking: Set eGTM CMU FXCLK0 source = GCLK; choose FXCLK0 = 100 MHz from SYSPLL 500 MHz via divider 5 (template_capabilities). Rationale: 10 ns resolution easily satisfies 0.5 µs deadtime and 1.0 µs min pulse.
3) Compute timing ticks:
   - Period ticks = 100 MHz / 20 kHz = 5000 (driver handles center-aligned internally).
   - Deadtime ticks = 0.5 µs × 100 MHz = 50.
   - Min pulse ticks = 1.0 µs × 100 MHz = 100.
4) Choose ATOM instance and channels: Prefer ATOM1, channels 0–5 as three complementary pairs via a single TGC for synchronous updates.
5) Resolve TOUT pins:
   - Query pin validator for ATOM1 channel macros like IfxEgtm_ATOM1_0_TOUTx / ATOM1_0N_TOUTy matching KIT_A3G_TC4D7_LITE.
   - If ATOM1 pairing on the board is constrained, fall back to ATOM0 channels 0–5; known example: U-phase could use IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT (high) and IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT (low).
6) Configure polarity and safety: polarity = Ifx_ActiveState_high, complementaryPolarity = Ifx_ActiveState_low; set DTM clock source dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0.
7) Synchronization: Group all six channels under the same TGC; enable center-aligned mode; ensure shadow transfer on period boundary.
8) Emit hardware_design JSON: device_macro DEVICE_TC4DX, pin_package BGA436_COM, clocks (GCLK, FXCLK0=100 MHz), eGTM→ATOM instance, channels 0–5 to phases U/V/W, TOUT pins, deadtime/min-pulse ticks, and file naming per code_structure.

## Key Decisions & Risks
- ATOM instance/pin availability: Requirement prefers ATOM1, but current pin_mapping_from_docs only shows ATOM0 ch0 examples. Risk: ATOM1 TOUT pins on KIT_A3G_TC4D7_LITE may constrain routing; mitigation: validate full ATOM1 pin list, otherwise use ATOM0 ch0–5 on P02 bank if available.
- FXCLK0 rate: I propose 100 MHz; if GCLK divisors differ on this board, I’ll adjust to maintain ≥10 ns resolution while meeting 20 kHz without overflow.
- Deadtime engine: Use DTM with cmuClock0; if DTM not routable to chosen outputs, the driver must emulate deadtime—flag as a fallback.
- Min pulse enforcement: Ensure 100-tick floor; risk of saturation at very small duty cycles; document clamp behavior.