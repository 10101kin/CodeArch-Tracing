# AI Reasoning - Detailed Design

*Generated: 2026-03-25T11:13:40.041676*

---

## Phase Purpose
In this Detailed Software Design phase, I will define the exact function signatures, module structs, configuration constants, init/update sequences, and unit-test specifications needed to generate code. This is the contract between requirements and implementation: it locks the GTM TOM0 3‑phase PWM design (channels, clocks, deadtime, interrupts) so code generation and reviews are unambiguous.

## Data Inventory
- user_requirement (user input)
  - From: User brief.
  - Contains: Request to patch iLLD_TC387_ADS_GTM_TOM_3_Phase_Inverter_PWM_2, with decided defaults TOM0 CH0/1, CH2/3, CH4/5; DTM0 complementary; center‑aligned; 10 kHz; period interrupt from TOM0.
  - Use: Anchor function names (initGtmTom3phInv, updateGtmTom3phInvDuty), topology (3 pairs on TOM0), PWM mode, and interrupt source.

- target_board
  - From: Platform selection.
  - Contains: KIT_A2G_TC387_5V_TFT.
  - Use: Constrain pin map to TC38x LFBGA‑516 and check resource fit.

- requirements_json (Phase 1)
  - From: Refiner node.
  - Contains: peripheral_requirement: IfxGtm_Pwm on TOM0; signal_requirements with channel pairs; timing_requirements with alignment “center‑aligned (up/down)”, pwm_frequency_hz 10000, gtm_clocks.cmu_clk0_hz 100000000; device_configuration: DEVICE_TC38X, IFX_PIN_PACKAGE_516.
  - Use: Fix frequency and clock math (CMU CLK0 = 100 MHz), confirm driver choice, define constants.

- hardware_json (Phase 2)
  - From: Hardware node.
  - Contains: TOM0 channels [0..5]; pin_assignments = TBD with conflict note “Specific TOUT pins for TOM0_CH0..CH5 on TC38x LFBGA‑516 not selected”; system clock 300 MHz.
  - Use: Declare pin mux placeholders and selection criteria; ensure channels 0–5 same TGC for sync update; document risk.

- sw_architecture_json (Phase 3)
  - From: SW architecture node.
  - Contains: Driver IfxGtm_Pwm.h; init API calls; file names GTM_TOM_3ph_Inv_Pwm.c/.h; function mapping for init/update/ISR.
  - Use: Finalize API list, include set, and module decomposition.

- template_capabilities
  - From: Template analyzer.
  - Contains: iLLD headers including IfxGtm_PinMap_TC38x_516.h; device_macro DEVICE_TC38X; xtal 20 MHz, PLL 300 MHz.
  - Use: Validate include paths, pin map family, and clock source assumptions.

- header_documentation_context
  - From: Intelligent header selector.
  - Contains: IfxGtm_Pwm notes (use IfxGtm_IrqMode_pulseNotify; sync‑channel API caveat).
  - Use: Choose IRQ mode and avoid base‑channel misuse.

- extracted_config_values
  - From: Library file analyzer.
  - Contains: Fields like output[i].pin, deadTime.rising/falling, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, IFXGTM_CH0CCU0_INTR_PRIO, IFXGTM_PWM_NUM_CHANNELS.
  - Use: Shape config struct fields, enums, and defaults for JSON.

- detected_peripheral_type
  - From: Detector.
  - Contains: PWM.
  - Use: Confirms peripheral category.

## Execution Plan
1. Define module API
   - Functions: initGtmTom3phInv(void); updateGtmTom3phInvDuty(float dutyA, float dutyB, float dutyC).
   - Rationale: Matches user_requirement and sw_architecture_json mapping.

2. Declare module context structs
   - Types: GtmTom3phInv_Config with fields: pwm_frequency_hz=10000, cmu_clk0_hz=100000000, alignment=center‑aligned, IfxGtm_Dtm_ClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, deadtime_ns=TBD, output[6].pin=TBD.
   - GtmTom3phInv_Handle with computed periodTicks (100e6/(10k*2)=5000), dutyTicks[3], dtmTicks.
   - Rationale: extracted_config_values indicates output[i].pin and deadTime.* fields.

3. Clock and timing design
   - Set GTM CMU CLK0 to 100 MHz; select TOM0 source = CLK0; compute period = 5000 for center‑aligned.
   - Rationale: requirements_json.timing_requirements.

4. Channel and pairing plan
   - Use TOM0 CH0/1, CH2/3, CH4/5 as A/B/C (HS=even, LS=odd), same TGC for sync updates.
   - Rationale: user_requirement and hardware_json channels.

5. DTM configuration
   - Map all pairs to DTM0, set dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0; deadTime.rising/falling = deadtime_ns converted to ticks.
   - Rationale: extracted_config_values.enum_values and user DTM requirement.

6. Pin assignment placeholders
   - Specify pin map fields as TBD macros from IfxGtm_PinMap_TC38x_516.h (e.g., IfxGtm_TOM0_0_TOUTx_Pyy_z_OUT), and document constraints (same port bank not required, but routable on board).
   - Rationale: hardware_json.validation conflicts.

7. Interrupt design
   - Period interrupt from TOM0 (use CH0 CCU0 or TOM global period per IfxGtm_Pwm guidance), IRQ mode IfxGtm_IrqMode_pulseNotify, priority macro default IFXGTM_CH0CCU0_INTR_PRIO or define PWM_TOM0_PRIO.
   - Rationale: header_documentation_context and extracted_config_values.macros.

8. Initialization sequence
   - IfxGtm_enable; IfxGtm_Cmu_enableClocks(CLK0|FXCLK); configure TOM0 TGC, channels, pins, DTM; load shadow registers; trigger sync update.
   - Rationale: sw_architecture_json.initialization_apis.

9. Duty update sequence
   - Bound inputs [0.0..1.0]; compute compare for center‑aligned; write to shadow for 3 HS channels; derive LS via complementary + deadtime; trigger TGC update atomically.
   - Rationale: stability and complementary operation.

10. Unit-test specifications
    - Verify periodTicks==5000; duty 0/50/100% mapping; complementary inversion and deadtime ticks applied; ISR fires at 10 kHz; no glitches on synchronous update.

## Key Decisions & Risks
- Pin selection unresolved for TOM0_CH0..5 on TC38x LFBGA‑516; must pick valid TOUTx macros from IfxGtm_PinMap_TC38x_516.h that exist on KIT_A2G_TC387_5V_TFT.
- Deadtime value not provided; propose default (e.g., 500 ns) but mark TBD; conversion to ticks depends on DTM clock.
- IfxGtm_Pwm unified API vs. TOM PwmHl nuances: ensure we apply API only to sync channels as per header note; avoid configuring base channel improperly.
- Period interrupt source on TOM0 may differ per iLLD version; confirm whether CH0 CCU0 or TOM global event is available in this template.