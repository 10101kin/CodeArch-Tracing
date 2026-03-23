# AI Reasoning - Detailed Design

*Generated: 2026-03-23T12:58:56.705717*

---

## Phase Purpose
In this Detailed Software Design phase, I will produce the concrete blueprint the generator will follow: exact function prototypes, driver config structures and fields, constants (frequencies, ticks), init/start/update sequences, ISR wiring, and unit-test specs. Getting these details right ensures the generated code for TC4Dx uses the correct eGTM0 ATOM2/CDTM resources, clocks, and pins — with no “TBDs” left.

## Data Inventory
- user_requirement (User input)
  - What: 3-phase inverter on eGTM0 ATOM2 CH0/1/2 with CDTM complementary outputs to P20.8/9, P20.10/11, P20.12/13; IfxEgtm_Pwm; 20 kHz center-aligned; 1 µs deadtime; sync start/update; period ISR on CPU0 prio 20; ATOM FXCLK0, DTM CLK0.
  - Use: Source of truth for timing, routing, driver layer, and ISR policy.

- target_board (Phase 0)
  - What: KIT_A3G_TC4D7_LITE.
  - Use: Board context and naming in headers and tests.

- requirements_json (Phase 1 – Refiner)
  - What: Structured requirements: peripheral EGTM0.ATOM2 channels [0,1,2], pins P20.8..P20.13, pwm_frequency_hz: 20000, alignment: center, deadtime_us: 1.0, CPU0 ISR prio 20, device_macro TC4D7.
  - Use: Populate design JSON fields one-to-one (no open fields).

- hardware_json (Phase 2 – Hardware)
  - What: MCU TC4Dx (DEVICE_TC4D7), clocks xtal 25 MHz, system 500 MHz; pin_assignments for U/V/W; validation: all_verified=false, conflicts on P20.x TOUTSEL mapping.
  - Use: Clock basis for tick math; highlight/resolve pin TOUTSEL mapping risk.

- sw_architecture_json (Phase 3 – SW Arch)
  - What: Driver selection IfxEgtm_Pwm [DOC_REF_REQUIRED]; function mapping initEgtmAtom3phInv, ISR routing via IfxSrc_*; migration notes IfxGtm_* → IfxEgtm_*.
  - Use: Define function names, include set, and call sequence.

- template_capabilities (Template analyzer)
  - What: TC4xx iLLD present; clock config (25 MHz xtal, 500 MHz syspll); large driver set.
  - Use: Confirm environment and clock assumptions; verify ISR API availability.

- reference_analysis (Reference project)
  - What: Patterns for PWM init, watchdog handling, ISR style, coding conventions (camelCase).
  - Use: Style and init flow guidance; migrate TOM→ATOM semantics.

- header_documentation_context (Header doc)
  - What: IfxEgtm_Pwm usage and fields.
  - Use: Exact struct fields and enums.

- extracted_config_values (Header field scrape)
  - What: Fields: deadTime.rising/falling, enum dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, polarity, complementaryPolarity, output[i].pin mapping array, config.frequency, config.dtmClockSource.
  - Use: Field names/enum constants and pin-map pattern (migrated to IfxEgtm_*).

- detected_peripheral_type
  - What: PWM.
  - Use: Sanity check.

## Execution Plan
1. Driver and files
   - Select IfxEgtm_Pwm per sw_architecture_json; define files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h and public APIs: initEgtmAtom3phInv, start/stop, setPhaseDuty(U/V/W), setAllDuty, and ISR prototype IFX_INTERRUPT(egtm0Atom2PeriodIsr, 0, 20).

2. Clock setup
   - Use hardware_json clocks (xtal 25 MHz, sys 500 MHz). In design, call IfxEgtm_Cmu_enableClocks, IfxEgtm_Cmu_selectFxClock(…FXCLK0), IfxEgtm_Cmu_selectClock(…CLK0). Store runtime queries: atomFxclkHz = IfxEgtm_Cmu_getFxclkFrequency(FXCLK0), dtmClkHz = IfxEgtm_Cmu_getClkxFrequency(CLK0).

3. Timing math
   - Compute periodTicks for center-aligned up–down: periodTicks = atomFxclkHz / (2 * 20000). Compute deadtimeTicks = (uint32)(dtmClkHz * 1e-6). Add asserts on ranges.

4. PWM config structures
   - Fill IfxEgtm_Pwm_Config: .frequency = 20000, .alignment = center, .deadTime.rising = .deadTime.falling = 1e-6, .dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0, .polarity/complementaryPolarity per requirement.
   - Define three channels (ATOM2 CH0/1/2) with complementary outputs enabled, independent duty control, grouped for sync update.

5. Pin/TOUTSEL routing
   - Map outputs: U→P20.8/P20.9, V→P20.10/P20.11, W→P20.12/P20.13 using IfxEgtm_PinMap and TOUTSEL. Reference pin fields as output[i].pin; include TOUTSEL selector in design. Flag [DOC_REF_REQUIRED] macros to be validated against TC4D7 pinmap.

6. Init sequence
   - In initEgtmAtom3phInv: enable clocks, configure eGTM, init IfxEgtm_Pwm with channel array, bind pins, program deadtime, center-aligned mode, shadow transfer, and sync start for channels 0–2 at 50% duty.

7. ISR wiring
   - Route ATOM2 CH0 period event to CPU0: IfxSrc_init(&SRC_EGTM0_ATOM2_CH0, IfxSrc_Tos_cpu0, 20); IfxSrc_enable(). Provide ISR egtm0Atom2PeriodIsr() that services period event and optionally triggers shadow updates.

8. Duty update APIs
   - Implement setPhaseDuty(U/V/W) writing shadow compare values; setAllDuty with optional syncUpdate parameter. Ensure no re-trigger until ISR window if sync_update=true.

9. Unit-test spec
   - Define tests: frequency tolerance (+/−1%), deadtime tick match, sync start check (all edges aligned), independent duty sweep per phase, ISR rate = 20 kHz on CPU0, pin routing verification (GPIO mux/TOUTSEL readback).

## Key Decisions & Risks
- IfxEgtm_Pwm availability: If template lacks unified driver, fallback to IfxGtm_Atom + IfxGtm_Dtm low-level APIs (migration note).
- Pin routing risk: hardware_json reports unresolved P20.x TOUTSEL; I will insert exact IfxEgtm_PinMap/TOUTSEL symbols with [DOC_REF_REQUIRED] and add a validation step.
- Clock source prescalers: actual FXCLK0/CLK0 divisors must be confirmed; wrong values skew ticks. I will compute from runtime CMU getters and assert against expected ranges.