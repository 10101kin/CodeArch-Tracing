# AI Reasoning - Detailed Design

*Generated: 2026-03-23T15:16:30.602103*

---

## Phase Purpose
In Detailed Software Design, I will produce a precise design spec (functions, structs, constants, init sequence, and unit-test plan) that code-generation can implement without ambiguity. This locks down driver APIs, pin bindings, clock math, ISR wiring, and data models, ensuring the migration from TOM (TC3xx) to eGTM/ATOM (TC4D7) preserves behavior and timing.

## Data Inventory
- user_requirement (user input)
  - From: User.
  - Content: TC4D7 on KIT_A3G_TC4D7_LITE; eGTM Cluster 1, ATOM1; 3-phase complementary center-aligned PWM at 20 kHz, 1 µs deadtime; independent U/V/W duty; single period ISR on CPU0 (IfxSrc_Tos_cpu0, prio 20).
  - Use: Source of truth for timing, topology, ISR placement, and module/cluster selection.

- requirements_json (refiner node)
  - From: Phase 1.
  - Content: MIGRATION; driver_name IfxEgtm_Atom_Pwm; peripheral_requirement 3-phase complementary; cluster=1, submodule=ATOM1, channels U/V/W; pwm_frequency_hz=20000; deadtime_ns=1000; isr: CPU0, TOS IfxSrc_Tos_cpu0, priority 20; device DEVICE_TC4D7, package IFX_PIN_PACKAGE_BGA292_COM; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h.
  - Use: Concrete parameterization and file/driver expectations.

- hardware_json (hardware node)
  - From: Phase 2.
  - Content: EGTM.ATOM1 channels [0,1,2]; example pins: ATOM1_CH0_HS=P20.8, ...; clock: 25 MHz xtal, 500 MHz system; validation has conflicts (device/package mismatch).
  - Use: Channel indices and preliminary pin map; clocks for tick math; flag mismatches.

- sw_architecture_json (sw_architecture node)
  - From: Phase 3.
  - Content: Driver selection IfxEgtm_Pwm; init function name initEgtmAtom3phInv; driver calls include IfxEgtm_enable, IfxEgtm_Cmu_enableClocks, IfxEgtm_Pwm_init...; file names EGTM_ATOM_3_Phase_Inverter_PWM.c/.h; migration notes IfxGtm_Pwm -> IfxEgtm_Pwm.
  - Use: API layer and function naming to standardize signatures.

- template_capabilities (template_library_analyzer)
  - From: Template scan.
  - Content: Template KIT_A3G_TC4D7_LITE; mcu_family TC4xx; device_macro DEVICE_TC4DX; pin_package IFX_PIN_PACKAGE_BGA436_COM; clocks: xtal 25 MHz, syspll 500 MHz.
  - Use: Clock baselines and inclusion feasibility; highlight device/package discrepancy.

- reference_analysis (reference_project_analyzer)
  - From: Legacy project.
  - Content: Used IfxGtm_Pwm; macros NUM_OF_CHANNELS=3, PWM_FREQUENCY=20000; watchdog disable pattern; period ISR style.
  - Use: Preserve behavior and coding style; migrate constants and ISR cadence.

- header_documentation_context + extracted_config_values
  - From: Header introspection.
  - Content: IfxEgtm_Pwm API docs; defaults like deadTime.rising=0.0f; enums e.g., IfxGtm_Dtm_ClockSource_cmuClock0; example pin macros (GTM-style).
  - Use: Field names/units (deadtime in seconds), ISR/service request patterns; adjust names to EGTM variants where needed.

## Execution Plan
1. Fix driver layer and files
   - Choose IfxEgtm_Pwm (from sw_architecture_json) over IfxEgtm_Atom_Pwm to use the unified API; output files: EGTM_ATOM_3_Phase_Inverter_PWM.c/.h (requirements_json).

2. Define configuration constants
   - PWM_FREQUENCY_HZ=20000; DEADTIME_S=1e-6f; CENTER_ALIGNED=true; ISR_PRIO=20; ISR_TOS=IfxSrc_Tos_cpu0; CLUSTER=1; MODULE=ATOM1; CHANNELS U/V/W -> [0,1,2] (requirements_json, hardware_json).

3. Clock and period computation
   - Use syspll 500 MHz and EGTM CMU clock source (template_capabilities, hardware_json). Compute time base and period ticks so IfxEgtm_Pwm achieves 20 kHz center-aligned; document prescaler choice and formula periodTicks = fxclk / (2*PWM_FREQUENCY_HZ) for center-aligned.

4. Data structures
   - Define context struct: holds IfxEgtm_Pwm handle, per-phase channel handles, period ticks, deadtime config.
   - Pin struct: HS/LS pins for U/V/W mapped to P20.x as in hardware_json; reference IfxEgtm_PinMap symbols if available.

5. Function signatures
   - initEgtmAtom3phInv(const Pins*, float initDutyU, float initDutyV, float initDutyW)
   - egtm3phInv_setDutyU/V/W(float duty01)
   - egtm3phInv_setAllDuties(float u, float v, float w)
   - egtm3phInv_start(), egtm3phInv_stop()
   - isrEgtmAtomPeriod(void)

6. Initialization sequence
   - IfxEgtm_enable for Cluster 1; IfxEgtm_Cmu_enableClocks and select dtmClockSource (extracted_config_values suggests *_cmuClock0).
   - Configure ATOM1 channels [0,1,2] as complementary pairs with 1 µs deadtime, center-aligned (user_requirement).
   - Bind pins (hardware_json) and set output polarities: HS active high, LS complementary with deadtime.

7. ISR wiring
   - Configure EGTM1.ATOM1 period service request to CPU0, TOS IfxSrc_Tos_cpu0, priority 20; enable SRC, clear flags (requirements_json).

8. Unit-test specs
   - Verify period matches 20 kHz ±1%; verify deadtime = 1 µs ± one tick; verify independent duty updates (no cross-coupling); ISR fires once per period; complementary exclusivity (no HS/LS overlap).

## Key Decisions & Risks
- Driver/API naming: IfxEgtm_Pwm vs IfxEgtm_Atom_Pwm and enum namespaces (IfxEgtm_* vs IfxGtm_*). I will prefer EGTM variants; risk of header mismatch.
- SRC line selection for ATOM1 period on EGTM Cluster 1 needs confirmation (exact SRC_xxx symbol).
- Pin/package conflict: requirements_json (IFX_PIN_PACKAGE_BGA292_COM) vs template_capabilities (BGA436_COM) and hardware_json validation errors. I will proceed with provided P20.x pins but flag this for fix.
- CMU clock source and prescaler: must select a feasible fxclk that yields integer ticks for 20 kHz center-aligned; I’ll document chosen clock to avoid jitter.
- Deadtime units: driver expects seconds (extracted_config_values). I’ll set 1e-6f and verify DTM clock mapping.