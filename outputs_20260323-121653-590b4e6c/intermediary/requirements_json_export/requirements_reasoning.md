# AI Reasoning - Requirements Analysis

*Generated: 2026-03-23T12:15:19.885032*

---

## Phase Purpose
In Requirements Analysis, I turn the natural-language migration request into a precise, machine-consumable requirements model. This model captures peripherals, timing, pin routing, triggers, and migration constraints so every downstream phase (HW mapping, architecture, design, code-gen) can proceed without reinterpreting intent.

## Data Inventory
- user_requirement (from user)
  - What: “Migrate the TC387 3-phase inverter to TC4D7 using eGTM ATOM and TMADC: use eGTM Cluster 0 with ATOM0 CH0–CH2 for center‑aligned complementary PWM at 20 kHz (1 µs dead-time), ATOM0 CH3 as a 50% center trigger routed internally to TMADC0 and externally to P33.0, and map six PWM outputs to P20.8–P20.13.”
  - How I’ll use: Core functional/timing specs, resource selections (eGTM C0/ATOM0/CH0–3), and pin targets.

- target_board (from user selection)
  - What: KIT_A3G_TC4D7_LITE.
  - How I’ll use: Constrains available pins, package (BGA436), power/clock defaults, and BSP assumptions.

- template_capabilities (from template_library_analyzer)
  - What: Template path /infineon_POC_MVP/ads_templates/KIT_A3G_TC4D7_LITE; MCU family TC4xx; device_macro DEVICE_TC4DX; clocks: xtal 25,000,000 Hz, syspll 500,000,000 Hz, ppupll 450,000,000 Hz, perpll1 16,000,000 Hz; iLLD inventory (168 drivers) including timers (e.g., IfxStm) and SPI; pin package IFX_PIN_PACKAGE_BGA436_COM.
  - How I’ll use: Verify eGTM/ATOM and ADC/TMADC driver presence, select feasible timebases, and check that P20.x/P33.x functions exist on this package.

- reference_analysis (from reference_project_analyzer on TC387 source)
  - What: Shows prior use of GTM TOM for 3-phase inverter (e.g., “IfxGtm_Pwm &g_gtmTom3phInv...”), GPIO via IfxPort, watchdog patterns, and coding conventions.
  - How I’ll use: Identify migration delta (TOM → eGTM ATOM, EVADC → ADC TMADC), reuse naming patterns, and confirm complementary center-aligned approach.

- clarifier_result (from chatbot_chain)
  - What: Understood intent confirms eGTM ATOM0 CH0–CH2, CH3 trigger to TMADC0 and P33.0; flags missing fields: exact ATOM↔pin mapping for P20.8–P20.13, and ADC0 TMADC channel list/timing.
  - How I’ll use: Record open items as TBD in requirements with validation hooks.

## Execution Plan
1. Normalize PWM spec
   - From user_requirement: encode center-aligned complementary PWM, fpwm = 20 kHz (Tpwm = 50 µs), dead-time = 1 µs, eGTM Cluster 0, ATOM0 CH0–CH2 as three HS/LS pairs.

2. Define trigger channel
   - Specify ATOM0 CH3 as a 50% duty, center-aligned timebase/compare that toggles at mid-period; outputs: internal route to TMADC0 trigger, external to P33.0.

3. Express time in physical units
   - Use time values (50 µs period, 1 µs dead-time) in the JSON; downstream will derive ticks from template_capabilities.clock_config (e.g., PPU PLL 450 MHz → CMU_x selection). Avoid hard-coding ticks without confirmed eGTM CMU clock.

4. Propose pin mapping constraints
   - Capture requirement: map six PWM outputs to P20.8–P20.13.
   - Tentative pairing policy (subject to mux validation): HS → P20.8/10/12, LS → P20.9/11/13. Mark as decision_pending until ATOM0.CHx→P20.y functions are verified for IFX_PIN_PACKAGE_BGA436_COM.

5. ADC/TMADC requirements
   - Record ADC0 TMADC usage, trigger source = eGTM C0 ATOM0 CH3.
   - Add placeholders for TMADC channel list, acquisition times, and post-trigger sampling offsets (missing per clarifier_result).

6. Migration constraints
   - From reference_analysis: state “Replace GTM TOM-based inverter (g_gtmTom3phInv) with eGTM ATOM-based inverter; maintain center alignment, complementary outputs, and ADC sync behavior.”

7. Validate template support
   - Note requirement for eGTM/ATOM and ADC/TMADC drivers; if template_capabilities.available_drivers lacks these, flag risk in requirements metadata.

## Key Decisions & Risks
- ATOM channel to pin routing: Need confirmation that ATOM0 CH0–CH2 provide complementary outputs to P20.8–P20.13 on KIT_A3G_TC4D7_LITE (BGA436). Risk: mux/function mismatch.
- Trigger export: Verify ATOM0 CH3 can drive P33.0 and TMADC0 simultaneously. Risk: signal gating or pad function not available.
- eGTM CMU clock selection: Must choose CMU_x source (likely from PPU PLL 450 MHz). Impacts tick calculations for 1 µs dead-time. Risk: jitter/quantization if clock too low.
- TMADC configuration: Missing exact ADC0 TMADC channel list and timing windows; required for deterministic sampling.
- Driver availability: Template must include eGTM/ATOM and ADC/TMADC iLLDs; otherwise, plan fallback or add drivers.