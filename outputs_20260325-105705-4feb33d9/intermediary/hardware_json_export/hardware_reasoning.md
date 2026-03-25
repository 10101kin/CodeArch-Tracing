# AI Reasoning - Hardware Design

*Generated: 2026-03-25T11:01:57.180531*

---

## Phase Purpose
In Hardware Design, I will generate a board- and device-specific hardware design that binds the PWM requirements to concrete AURIX TC3xx resources: TOM channels, DTM pairing, GTM clocks, pin mux, and interrupt routing. This matters because the next code-generation phase relies on these exact resource choices to configure IfxGtm_Pwm and IfxGtm_Dtm correctly and to avoid pin/clock conflicts on KIT_A2G_TC387_5V_TFT.

## Data Inventory
- user_requirement (from user prompt)
  - Contains: 3-phase inverter PWM using GTM TOM0 with pairs (CH0/1, CH2/3, CH4/5), complementary via DTM0, center-aligned, 10 kHz PWM, interrupt from TOM0 period; other fields TBD.
  - Use: Core functional intent and fixed channel topology; defines clock and interrupt expectations.

- target_board (from project setup)
  - Contains: KIT_A2G_TC387_5V_TFT.
  - Use: Drives pin package and pin map selection for TC38x 516 package.

- requirements_json (from refiner node)
  - Contains: peripheral_requirement type=GTM_TOM, driver IfxGtm_Pwm; signal_requirements for TOM0 pairs A/B/C; timing (alignment center-aligned (up/down), pwm_frequency_hz=10000, gtm_clocks.cmu_clk0_hz=100000000, fxclk_hz=100000000); device_configuration {DEVICE_TC38X, IFX_PIN_PACKAGE_516}.
  - Use: Authoritative constraints for clocks, module, channels, and driver.

- template_capabilities (from template_library_analyzer)
  - Contains: mcu_family=TC3xx, device_config.device_macro=DEVICE_TC38X, pin_package=IFX_PIN_PACKAGE_516; available pin maps: IfxGtm_PinMap_TC38x_516.h; clock_config (xtal=20 MHz, pll=300 MHz).
  - Use: Pick correct iLLD headers and ensure GTM clocks are feasible with board PLL.

- header_documentation_context (from intelligent_header_selector)
  - Contains: IfxGtm_Pwm API notes (e.g., IrqMode default, sync channel constraint).
  - Use: Validate API usage (sync channels only), choose irq mode.

- extracted_config_values (from library_file_analyzer)
  - Contains: examples of fields like deadTime.rising, config.syncUpdateEnabled, enum dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0; macro IFXGTM_CH0CCU0_INTR_PRIO=3.
  - Use: Seed default DTM clock source and interrupt priority.

- pin_mapping_from_docs (from pin_validator)
  - Contains: Example mapping IfxGtm_TOM1_5_TOUT11_P00_2_OUT.
  - Use: Confirms pin-map header availability; highlights gap for TOM0 CH0..5 mappings in snapshot.

- detected_peripheral_type
  - Contains: PWM.
  - Use: Confirms peripheral class.

## Execution Plan
1. Confirm device/board context
   - Use DEVICE_TC38X and IFX_PIN_PACKAGE_516 from requirements_json/template_capabilities to select IfxGtm_PinMap_TC38x_516.h and GTM on TC38x.

2. Clock design
   - Set GTM CMU CLK0 = 100 MHz and FXCLK = 100 MHz per timing_requirements.
   - Justification: 10 kHz center-aligned ⇒ period ticks ≈ (100e6 / (2*10e3)) = 5000; safe resolution for deadtime.

3. TOM channel allocation
   - Bind TOM0 channel pairs: Phase A (CH0/1), B (CH2/3), C (CH4/5) exactly as user_requirement.
   - Ensure channels are under TOM0 TGC0 for synchronous updates.

4. DTM pairing
   - Assign DTM0 to generate complementary outputs for each pair; source clock IfxGtm_Dtm_ClockSource_cmuClock0 (from extracted_config_values).
   - Choose initial deadtime placeholder (decision below), symmetric rising/falling.

5. PWM alignment and frequency
   - Configure center-aligned (up/down) mode in IfxGtm_Pwm_Config; target frequency=10000 Hz; clock source cmuClock0.

6. Interrupt routing
   - Use TOM0_CH0 CCU0 (period) interrupt as PWM period ISR; route to CPU0 with priority 3 (IFXGTM_CH0CCU0_INTR_PRIO) and provider IfxSrc_Tos_cpu0.

7. Pin mux selection
   - Query IfxGtm_PinMap_TC38x_516.h for TOM0_CH0..CH5 valid TOUT macros.
   - Choose a consistent port bank (prefer contiguous P00/P02) and assign: e.g., `IfxGtm_TOM0_0_TOUTxx_Pyy_z_OUT` ... `IfxGtm_TOM0_5_TOUTxx_Pyy_z_OUT`.
   - Set pad driver to IfxPort_PadDriver_cmosAutomotiveSpeed1 and push-pull.

8. Emit hardware design JSON
   - Include: tom_module="TOM0", channel_pairs=[(0,1),(2,3),(4,5)], dtm="DTM0", clocks {cmu_clk0=100e6, fxclk=100e6}, pwm {frequency=10k, centerAligned}, interrupt {source=TOM0_CH0_CCU0→CPU0, prio=3}, pins with exact TOUT macros, pad drivers, and DTM deadtime.

## Key Decisions & Risks
- Pin assignment: Snapshot lacks TOM0_CH0..5 pin options; I will resolve from IfxGtm_PinMap_TC38x_516.h. Risk: clashes with TFT or other board functions; mitigation: prefer unused P00/P02 and validate against board schematics.
- Deadtime value: Not specified. I will default to 500 ns (DTM0 via CMU_CLK0) unless provided; must match gate-driver needs.
- Interrupt source: Using CH0 CCU0 as “period” tick aligns with IfxGtm_Pwm pulseNotify; verify ISR load and sync update timing.
- Clock domain: Ensure GTM CMU setup doesn’t conflict with other GTM users; lock CMU CLK0 at 100 MHz as global.