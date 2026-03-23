# AI Reasoning - Hardware Design

*Generated: 2026-03-23T14:47:52.513513*

---

## Phase Purpose
In this Hardware Design phase, I will translate the PWM requirements into a concrete TC4D7 resource plan: eGTM cluster/submodule selection, ATOM channel pairing, pin assignments, clocks, and interrupt routing. This mapping is what the code generator needs to emit correct IfxEgtm_Pwm configuration and board‑valid pin macros for KIT_A3G_TC4D7_LITE.

## Data Inventory
- user_requirement (from user prompt)
  - What: “TC4D7 (KIT_A3G_TC4D7_LITE) eGTM ATOM, Cluster 1 ATOM1; 3‑phase complementary PWM, center‑aligned 20 kHz, 1 µs deadtime, independent U/V/W duty, single period ISR on CPU0 (IfxSrc_Tos_cpu0, prio 20).”
  - How I use: Drives all hardware resource choices (cluster, ATOM instance, ISR mapping, timing).

- target_board (system context)
  - What: KIT_A3G_TC4D7_LITE.
  - How I use: Constrains pin availability and device macro; aligns to the template’s pin package and clock tree.

- requirements_json (refiner output)
  - What: Structured details: driver_name IfxEgtm_Atom_Pwm; cluster 1, submodule ATOM1; channels U/V/W; 20 kHz center‑aligned; deadtime_ns 1000; ISR {CPU0, IfxSrc_Tos_cpu0, priority 20}; device_macro DEVICE_TC4D7; pin_package IFX_PIN_PACKAGE_BGA292_COM; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/h.
  - How I use: Primary spec for JSON fields and constraints. Note the pin_package mismatch I will resolve.

- template_capabilities (template analyzer)
  - What: Template path for KIT_A3G_TC4D7_LITE; device_macro DEVICE_TC4DX; pin_package IFX_PIN_PACKAGE_BGA436_COM; clock_config: xtal 25 MHz, syspll 500 MHz, ppupll 450 MHz, perpll1 16 MHz; iLLD availability (EGTM supported).
  - How I use: Source of actual board/device macro, pin package, and clock sources for CMU planning.

- reference_analysis (TC3xx TOM reference)
  - What: Shows prior TOM‑based 3‑phase PWM patterns, watchdog disable, ISR usage, macros like PWM_FREQUENCY.
  - How I use: Migration cues (TOM→ATOM), ISR pattern, naming and file structure alignment.

- header_documentation_context (iLLD docs)
  - What: IfxEgtm_Pwm API docs and IfxEgtm_PinMap presence.
  - How I use: Valid struct fields (output[].pin, complementaryPin, deadTime.rising/falling), ISR source naming.

- extracted_config_values (from headers)
  - What: Fields like deadTime.rising/falling, polarity enums, dtmClockSource IfxGtm_Dtm_ClockSource_cmuClock0, example output[].pin/complementaryPin usage, IFXEGTM_PWM_NUM_CHANNELS=8.
  - How I use: Concrete struct members and enums to populate; confirms complementary pin model and DTM clock selection.

- pin_mapping_from_docs (pin validator)
  - What: Sample macros for ATOM0 (e.g., IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT and complementary IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT).
  - How I use: Pairing pattern (0/1, 2/3, 4/5) and naming; will search equivalent for Cluster1 ATOM1.

- detected_peripheral_type
  - What: PWM.
  - How I use: Confirms the flow.

## Execution Plan
1. Resolve device/board baseline
   - Use KIT_A3G_TC4D7_LITE template’s device/pin_package (DEVICE_TC4DX family entry, IFX_PIN_PACKAGE_BGA436_COM) while honoring target_device_macro DEVICE_TC4D7 in the JSON. I will flag the package mismatch and align pins to the board’s BGA436.

2. Clock planning for eGTM/ATOM
   - Target CMU clock ≈100 MHz (from SYSPLL/PPUPLL dividers). Compute:
     - Center‑aligned period ticks = f_cmu / (2 × 20 kHz) → 2500 ticks at 100 MHz (or 2000 at 80 MHz).
     - Deadtime ticks = f_cmu × 1 µs → 100 ticks at 100 MHz.
   - Set dtmClockSource = cmuClock0 (per extracted_config_values).

3. Channel and pairing strategy
   - eGTM Cluster 1, ATOM1.
   - Phase U: channels 0/1 (HS/LS), V: 2/3, W: 4/5 to match complementary pairs.
   - Configure center‑aligned mode with independent duty updates per phase; complementaryPolarity set appropriately and DTM deadtime = 1 µs rising/falling.

4. Pin selection
   - Query IfxEgtm_PinMap for Cluster1 ATOM1 TOUT macros on KIT_A3G_TC4D7_LITE (BGA436); prefer same port bank and contiguous pins; avoid conflicts with QSPI/CAN.
   - Map each pair (e.g., ATOM1_0 TOUTx and ATOM1_0N TOUTx+1) to inverter U/V/W headers.
   - Validate with pin_validator; if missing, fall back to closest alternative bank with available complementary pins.

5. Interrupt routing
   - Enable a single period ISR from ATOM1 CH0 (period event) only.
   - Route SRC to CPU0 with IfxSrc_Tos_cpu0 and priority 20; disable other channel SRNs.

6. Produce hardware design JSON
   - Emit device, clocks (f_cmu), eGTM {cluster:1, atom:1}, channel map {U/V/W}, pin macros, DTM settings, ISR {src, tos, prio}, and code_structure filenames.

7. Consistency checks
   - Verify driver availability IfxEgtm_Pwm/IfxEgtm_Atom_Pwm, channel count ≤ IFXEGTM_PWM_NUM_CHANNELS (6 used), and timing tick calculations align with actual f_cmu.

## Key Decisions & Risks
- Pin assignment risk: ATOM1 Cluster1 TOUT pin macros for TC4D7 aren’t listed; I must resolve from IfxEgtm_PinMap for KIT_A3G_TC4D7_LITE. Fallback criteria will be applied.
- Package mismatch: requirements_json says BGA292_COM, template uses BGA436_COM; I will standardize on BGA436 for this board and flag the discrepancy.
- Clock source selection: Exact CMU source/dividers aren’t specified; I’ll target 100 MHz and adapt tick counts to the realized clock.
- API naming: Use IfxEgtm_Pwm with complementaryPin fields; ensure compatibility with “IfxEgtm_Atom_Pwm” requirement label.
- ISR event source: Use CH0 period event only to meet “single period ISR” while allowing independent duty updates per phase.