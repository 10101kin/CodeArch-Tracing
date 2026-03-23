# AI Reasoning - Hardware Design

*Generated: 2026-03-23T16:21:28.993724*

---

## Phase Purpose
In Hardware Design, I will produce a concrete mapping from the PWM requirements to TC387 GTM resources: TOM instance/channels, TOUT-to-pin routing, timer/clock selection, and per-channel parameters. This matters because the downstream code generation and configuration structs rely on exact TOM/TOUT pin macros and a shared timer setup to guarantee the 20 kHz, center-aligned, synchronized updates.

## Data Inventory
- user_requirement (from user): Six single-output IfxGtm_Tom_Pwm on a shared IfxGtm_Tom_Timer at 20 kHz, center-aligned; PHASE_U P02.0/P02.7, PHASE_V P02.1/P02.4, PHASE_W P02.2/P02.5; highs init 25/50/75%; update +10% every 500 ms; low-sides = complementary (no dead-time); TOM instance/ch selection TBD. I’ll translate this into concrete TOMx.CHy and TOUT pin macros.
- target_board (from user): KIT_A2G_TC387_5V_TFT. I’ll use this to select device macros and the correct GTM pin-map header.
- requirements_json (from refiner): PWM at 20000 Hz, center-aligned; shared_timer: IfxGtm_Tom_Timer on TOM0 CH3; device_macro DEVICE_TC38X; pin_package IFX_PIN_PACKAGE_292. I’ll honor TOM0:3 for the timer unless routing forces a change; I’ll also reconcile the pin-package mismatch.
- template_capabilities (from template analyzer): Board template path, TC3xx family, clock_config: xtal 20 MHz, pll 300 MHz, pin_package IFX_PIN_PACKAGE_516, IfxGtm_PinMap_TC38x_LFBGA516.h available. I’ll base CMU clocking and pin-map lookups on this.
- reference_analysis (from reference project): Uses IfxGtm_Tom_PwmHl and defines PWM_FREQ_HZ 20000; shows direct init patterns for IfxGtm_Tom_Timer/PwmHl. I’ll reuse timer init patterns but adapt to IfxGtm_Tom_Pwm (single-output).
- header_documentation_context (from iLLD docs): API surface for IfxGtm_Tom_Pwm. I’ll confirm fields for center-aligned mode, polarity, and sync update.
- extracted_config_values (from headers): Examples like tomChannel = IfxGtm_Tom_Ch_0; sample pin IfxGtm_TOM0_0_TOUT106_P10_4_OUT. I’ll mirror this structure for P02.x pins with the correct macros.
- pin_mapping_from_docs (from pin validator): Sample TOM1_5_TOUT11_P00_2_OUT. Not our pins; I’ll perform a targeted lookup for P02.0/1/2/4/5/7.
- detected_peripheral_type: PWM. Confirms the path.

## Execution Plan
1) Resolve device/pin-package: Board indicates IFX_PIN_PACKAGE_516; requirements_json says 292. I will lock to IFX_PIN_PACKAGE_516 (KIT_A2G_TC387_5V_TFT) and note the migration delta.
2) Clocking: Enable GTM and select CMU_CLK0 derived from pll_frequency=300 MHz; configure IfxGtm_Tom_Timer on TOM0.CH3 for 20 kHz center-aligned (up/down), letting the driver compute prescaler/period for fine resolution.
3) Pin routing search: In IfxGtm_PinMap_TC38x_LFBGA516.h, find TOUT mappings for P02.0, P02.7, P02.1, P02.4, P02.2, P02.5. Extract their TOM instance and channel numbers/macros (e.g., IfxGtm_TOMx_y_TOUTnn_P02_m_OUT).
4) TOM instance selection: Prefer a single TOM (ideally TOM0) that supports all six P02.x routes. If multiple TOMs are unavoidable, ensure they can be driven by the same CMU clock and support synchronized trigger, else revise routing.
5) TGC grouping: Place all channels within the same TOM TGC (e.g., TOMx TGC0: CH0–7) to use sync-update and center-aligned shadow transfers across all six.
6) Channel assignment: Map highs to the first three selected channels; map lows to their paired pins. Set low-side polarity = inverted, dead-time = 0. Record initial duties: U/V/W highs 25%/50%/75%, lows complementary.
7) JSON emission: Define device macro DEVICE_TC38X, pin_package IFX_PIN_PACKAGE_516, GTM CMU clock, TOM timer (TOM0.CH3), and a channels array with exact pin macros, TOMx.CHy, TGC, polarity, alignment, and sync-update enabled.
8) Update behavior metadata: Note 500 ms +10% wrap for later scheduling (not a hardware resource).

## Key Decisions & Risks
- Low-side behavior: default complementary; await user confirmation if same-polarity is desired.
- Pin-package mismatch (292 vs 516): I will proceed with 516; using 292 would change available TOUT routes.
- Routing availability: All six P02.x pins must exist and preferably on one TOM/TGC; if not, we’ll propose alternate pins or TOM layout.
- Reference vs driver: Reference used IfxGtm_Tom_PwmHl; we will use IfxGtm_Tom_Pwm per requirement—API differences will be handled.
- CMU clock choice affects duty resolution; I’ll select CMU_CLK0 to balance resolution and range.