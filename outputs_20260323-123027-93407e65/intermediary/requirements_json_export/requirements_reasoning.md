# AI Reasoning - Requirements Analysis

*Generated: 2026-03-23T12:29:22.051652*

---

## Phase Purpose
In Requirements Analysis, I turn the natural-language migration request into a single, structured requirements specification that captures behavior, timing, pins, and migration constraints. This matters because every downstream step (pin muxing, driver selection, clocking, code generation, and tests) consumes this spec as the authoritative source of truth.

## Data Inventory
- user_requirement (source: user input)
  - Contains: “Migrate the TC387 3-phase inverter (GTM TOM + EVADC) to TC4D7 using eGTM ATOM: Cluster 0 with ATOM0 CH0–CH2 as complementary center-aligned PWM at 20 kHz with 1 µs deadtime (HS active-high, LS active-low on P20.8/P20.9, P20.10/P20.11, P20.12/P20.13), and ATOM0 CH3 as a 50% edge-aligned trigger …”
  - Use: Primary functional/timing/pin requirements and migration intent (TOM+EVADC → ATOM+TMADC).

- target_board (source: board selection)
  - Contains: KIT_A3G_TC4D7_LITE
  - Use: Constrain pin availability, package (IFX_PIN_PACKAGE_BGA436_COM), and ensure features exist on TC4D7.

- template_capabilities (source: template_library_analyzer)
  - Contains: mcu_family TC4xx, device_macro DEVICE_TC4DX, clock_config (xtal 25 MHz, syspll 500 MHz, ppupll 450 MHz, perpll1 16 MHz), iLLD presence (168 drivers).
  - Use: Validate eGTM/ATOM support, pick clock domains for timing calc, confirm driver availability; note exact TC4xx iLLD naming.

- reference_analysis (source: reference_project_analyzer)
  - Contains: Prior TOM-based 3-phase inverter patterns (e.g., “IfxGtm_Pwm … g_gtmTom3phInv…”, NUM_OF_CHANNELS (3), NUM_OF_ADC_TRIG_CHANNELS (1)).
  - Use: Preserve update semantics and macros; guide migration from TOM to eGTM ATOM.

- conversation_history (source: chat log)
  - Contains: Commitment to migrate as requested.
  - Use: Context continuity; no new technical constraints.

- clarifier_result (source: clarifier)
  - Contains: Understood intent confirmed; missing_fields: TMADC0 5-channel analog input mapping; user_value_to_validate: ADC_TRIG_OUT pin P33.0 (proposed).
  - Use: Record unresolved TMADC mapping; tentatively include P33.0 as trigger-out for observability.

## Execution Plan
1. Extract core PWM spec from user_requirement: eGTM ATOM Cluster 0, ATOM0 CH0–CH2, complementary center-aligned 20 kHz, 1 µs deadtime, polarities (HS active-high, LS active-low), pins P20.8/9, P20.10/11, P20.12/13.
2. Capture trigger spec: ATOM0 CH3 edge-aligned 50% duty, used as ADC trigger; mirror to pin P33.0 for debug (from clarifier_result).
3. Record migration context: Source TC387 TOM+EVADC → Target TC4D7 eGTM ATOM+TMADC; keep three-phase API patterns (NUM_OF_CHANNELS, duty update timing) from reference_analysis.
4. Define timing fields as physical targets (20 kHz, 1 µs) and add clock derivation notes using template_capabilities (favor eGTM CMU clock derived from PPU PLL 450 MHz); downstream will translate to ticks.
5. Specify complementary generation method: ATOM AGC/complementary mode with synchronous shadow transfers; include deadtime insertion and output inversion to meet HS/LS polarity.
6. Create pin/channel map objects: Phase U/V/W mapping to P20.8/9, P20.10/11, P20.12/13 with explicit HS/LS polarity; CH3 mapped to a valid TOUT on P33.0 (to be validated).
7. Define TMADC trigger routing: Source = eGTM.ATOM0.CH3; Consumer = TMADC0 (group/channel TBD); add requirement for deterministic mid-carrier sampling alignment.
8. Add TBDs: TMADC0 5-channel analog input pin/channel mapping; confirm ATOM0→TOUT mapping indices for all P20.x and P33.0 on KIT_A3G_TC4D7_LITE.

## Key Decisions & Risks
- Driver/API choice on TC4xx: IfxEgtm_Atom vs IfxGtm_Atom_Pwm naming (must match iLLD in template_capabilities).
- eGTM CMU clock selection (FXCLK) impacts tick counts and deadtime resolution; must align with 20 kHz/1 µs.
- Complementary implementation: AGC pairing/inversion vs port polarity; ensure true deadtime on both HS/LS.
- TOUT mapping: Verify ATOM0 CH0–CH3 routability to P20.8–P20.13 and P33.0 on BGA436; alt-function indices may force channel reassignments.
- TMADC trigger path: Confirm internal eGTM→TMADC trigger mux; avoid jitter; decide on sync to PWM center.
- Missing TMADC0 5-channel mapping is a blocker for ADC part; mark as required input.