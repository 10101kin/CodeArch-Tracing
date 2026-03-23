# AI Reasoning - Requirements Analysis

*Generated: 2026-03-24T00:18:02.824479*

---

## Phase Purpose
In this Requirements Analysis phase, I will convert the user’s migration request into a precise, structured requirements specification that downstream tools can trust. It will capture timing (20 kHz, deadtime, min pulse), peripheral choices (eGTM/ATOM), clocks (FXCLK0 from GCLK), resource allocations (one ATOM for six outputs), and migration constraints from TC387 TOM to TC4D7 eGTM ATOM.

## Data Inventory
- user_requirement
  - Source: User input.
  - What: “Migrate the TC387 TOM-based 3‑phase inverter PWM to TC4D7 using eGTM ATOM and the unified eGTM PWM high/low driver, preserving 20 kHz center‑aligned complementary outputs, 0.5 µs deadtime, and 1.0 µs minimum pulse. Use eGTM CMU FXCLK0 from GCLK, one ATOM instance for all six outputs (default ATOM1 …).”
  - How I’ll use it: Set functional/timing constraints, clock source, and resource preference (ATOM1, 6 channels, center-aligned complementary, deadtime/min-pulse).

- target_board
  - Source: Project selection.
  - What: KIT_A3G_TC4D7_LITE.
  - How I’ll use it: Constrain pin routing and peripheral availability to this board.

- template_capabilities (template_library_analyzer)
  - Source: Libraries scan of the KIT_A3G_TC4D7_LITE template.
  - What: TC4xx family (device_macro DEVICE_TC4DX), xtal 25 MHz, syspll 500 MHz, perpll1 16 MHz, pin package IFX_PIN_PACKAGE_BGA436_COM, 168 iLLD drivers found (e.g., IfxStm, IfxQspi...), clock/interrupt scaffolding.
  - How I’ll use it: Validate eGTM/ATOM and CMU clock feasibility, derive tick calculations from available clocks, and note driver/library environment.

- reference_analysis (reference_project_analyzer)
  - Source: Parsed TC387 TOM 3-phase PWM reference.
  - What: Uses IfxGtm_Tom_PwmHl and IfxGtm_Tom_Timer; macros like PWM_FREQ_HZ=20000; patterns show complementary HL with timer + PwmHl object (e.g., g_pwm3PhaseOutput.timer).
  - How I’ll use it: Capture behavioral parity and migration deltas (TOM→ATOM), reuse semantics (center-aligned, HL pairing, deadtime handling).

- clarifier_result (chatbot_chain)
  - Source: Clarification Q&A.
  - What: Intent confirmed; missing fields: eGTM instance and ATOM module/channels for 6 outputs, exact pin mapping for U/V/W HS/LS; peripheral_type TIMER; confidence medium.
  - How I’ll use it: Flag TBDs that must be explicit in the requirements JSON.

## Execution Plan
1. Extract and normalize timing requirements from user_requirement: 20 kHz center-aligned, complementary HL, 0.5 µs deadtime, 1.0 µs minimum pulse.
2. Define clocking: set CMU FXCLK0 source = GCLK (per user). Using template_capabilities clock_config (xtal 25 MHz, syspll 500 MHz), compute nominal FXCLK0 prescaler options and required ticks for:
   - Period ticks @20 kHz.
   - Deadtime ticks for 0.5 µs.
   - Min pulse ticks for 1.0 µs.
   Record acceptable ranges and rounding policy.
3. Allocate resources: prefer eGTM0.ATOM1, channels 0–5 (single ATOM instance for all six outputs) to ensure shared AGC/TGC for synchronized updates. If ATOM1 cannot expose six routable pins on this board, mark fallback ATOMx/channels as alternatives (TBD).
4. Specify driver/API: select the unified eGTM PWM High/Low driver (ATOM backend). Define config fields: center-aligned mode, complementary outputs, deadtime_us=0.5, minPulse_us=1.0, frequency_hz=20000, sync updates at center, one timer base shared across six channels.
5. Map signals: declare six PWM outputs (U_HS, U_LS, V_HS, V_LS, W_HS, W_LS) and constrain them to pins routable from chosen ATOM channels on KIT_A3G_TC4D7_LITE. Mark exact pin symbols as TBD with requirement to keep HS/LS on same TOM/ATOM pair polarity conventions.
6. Capture migration constraints: parity with IfxGtm_Tom_PwmHl semantics (duty clamp, deadtime insertion, complementary polarity), atomic shadow transfers, emergency shutdown line support (TBD), and behavior at 0%/100% duty (respect minPulse).
7. Produce the requirements JSON with resolved values and explicit TBDs for instance/channels and pin mapping.

## Key Decisions & Risks
- ATOM instance/channel selection: Need confirmation of eGTM instance and ATOM1 ch[0..5] availability; risk of pin routing conflicts on KIT_A3G_TC4D7_LITE (BGA436).
- Clock feasibility: FXCLK0 from GCLK exact frequency not listed; must ensure tick resolution supports 0.5 µs deadtime and 1.0 µs min pulse without excessive quantization.
- Driver availability: Unified eGTM PWM HL driver presence in template is not explicitly listed; may require adaptation from TOM PwmHl semantics.
- Safety/fault handling: Emergency shutdown inputs, brake logic, and safe states are unspecified (TBD).
- Update/synchronization: Ensuring truly center-aligned complementary behavior using ATOM compare/shadow and AGC/TGC grouping; polarity misconfiguration at extremes is a risk.