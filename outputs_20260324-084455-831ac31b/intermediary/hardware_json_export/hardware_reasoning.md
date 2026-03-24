# AI Reasoning - Hardware Design

*Generated: 2026-03-24T08:51:36.104436*

---

## Phase Purpose
In Hardware Design, I produce a device-specific map that binds the migrated functions to concrete TC4D7 resources: eGTM ATOM channels, pins, clocks, and ADC triggers. This matters because every later phase (driver init and application code) depends on these exact assignments and timing values to be valid on KIT_A3G_TC4D7_LITE.

## Data Inventory
- user_requirement (free-form)
  - From: user input.
  - Content: “Use EGTM ATOM0 Cluster 0: CH0–2 for complementary, center-aligned PWM; deadtime 1.0 µs rise/fall; 20 kHz; initial duties 25/50/75%; CH3 for TMADC trigger.”
  - Use: Source of truth for channels, timing, and trigger topology.

- requirements_json (refiner node)
  - From: previous phase structuring.
  - Content: MIGRATION to TC4xx; peripheral = GTM_ATOM; driver_name = IfxEgtm_Pwm; adc module = TMADC0 with trigger “EGTM ATOM0 C0 CH3 falling edge”; device_macro DEVICE_TC4D7; pin_package IFX_PIN_PACKAGE_BGA292_COM.
  - Use: Formal constraints, driver names, and device intent.

- template_capabilities (template library analyzer)
  - From: board template.
  - Content: Board KIT_A3G_TC4D7_LITE, device_macro DEVICE_TC4DX, pin_package IFX_PIN_PACKAGE_BGA436_COM; clock_config: xtal 25 MHz, syspll 500 MHz, ppupll 450 MHz.
  - Use: Actual board/device macro, available clocks, and iLLD baseline.

- reference_analysis (TC3xx project)
  - From: reference project parser.
  - Content: Patterns include IfxGtm_Pwm/IfxGtm_Trig usage, EVADC; watchdog disable, 3-channel inverter macros.
  - Use: Migrate usage patterns to TC4xx equivalents (IfxEgtm_Pwm, TMADC, trigger wiring).

- header_documentation_context
  - From: iLLD headers.
  - Content: IfxEgtm_Pwm API overview for TC4xx.
  - Use: Valid struct fields (e.g., IfxEgtm_Pwm_Config) and capabilities (complementary pairs).

- extracted_config_values
  - From: header parsing.
  - Content: deadTime.rising/falling, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0; output[0].pin/complementaryPin fields; example config.frequency 500000.0f.
  - Use: Field names and enums for JSON schema (e.g., deadTime.*, output[i].pin, complementaryPin).

- pin_mapping_from_docs (pin validator)
  - From: doc-based validator.
  - Content: ATOM0 CH0 examples: IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT and complementary IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT.
  - Use: Concrete pin macros for CH0 pair; pattern reference for CH1/CH2 lookup.

- target_board
  - From: user selection.
  - Content: KIT_A3G_TC4D7_LITE.
  - Use: Constrains device macro, package, and available pins.

- detected_peripheral_type
  - From: analyzer.
  - Content: PWM.
  - Use: Confirms primary peripheral domain.

## Execution Plan
1. Reconcile device/package: prefer template_capabilities (DEVICE_TC4DX, IFX_PIN_PACKAGE_BGA436_COM) over requirements_json to match the actual board.
2. Select drivers: IfxEgtm_Pwm for PWM; IfxAdc_Tmadc for TMADC0; IfxGtm_Trig (if needed) for ATOM→ADC trigger line.
3. Clocking plan:
   - Enable eGTM and TMADC clocks.
   - Set ATOM time base to achieve 20 kHz center-aligned PWM (compute period ticks from CMU clock + prescaler).
   - Set DTM/CDTM clock source to IfxGtm_Dtm_ClockSource_cmuClock0; compute 1.0 µs ticks.
4. Channel assignment:
   - ATOM0 Cluster 0: CH0→Phase U (25%), CH1→Phase V (50%), CH2→Phase W (75%), CH3→ADC trigger (no output pin).
5. Complementary outputs and pins:
   - Map CH0 pins using validated macros:
     - output[0].pin = IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT
     - output[0].complementaryPin = IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT
   - Lookup and assign CH1/CH2 equivalent TOUT macros for P02.x (or board-valid alternatives) from IfxPort_PinMap; reserve them in JSON.
6. PWM mode and polarity:
   - Center-aligned mode; main polarity active-high; complementary inverted; enable deadTime.rising/falling = 1.0e-6.
7. ADC trigger:
   - Configure ATOM0 C0 CH3 compare to issue falling-edge trigger to TMADC0 per adc_configuration.
8. Resource validation:
   - Check for port conflicts on KIT_A3G_TC4D7_LITE; ensure shared prescaler supports all channels.
9. Emit hardware design JSON schema fields aligned to IfxEgtm_Pwm_Config (deadTime.*, output[i].pin/complementaryPin, dtmClockSource, frequency, alignment, trigger route).

## Key Decisions & Risks
- Device/package mismatch (DEVICE_TC4D7 vs DEVICE_TC4DX; BGA292 vs BGA436): I will use board template values and flag the discrepancy.
- Pin availability for ATOM0 CH1/CH2: only CH0 macros are validated; I’ll resolve exact macros from PinMap and flag if alternatives are needed.
- CMU/DTM clock frequencies not explicitly given: I’ll compute ticks based on configured CMU clock; risk of deadtime quantization error if CMU is low.
- ADC trigger routing: confirming TMADC0 accepts ATOM0 C0 CH3 falling-edge without cross-trigger fabric re-map; will fall back to IfxGtm_Trig configuration if required.