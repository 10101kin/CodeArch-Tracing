# AI Reasoning - Hardware Design

*Generated: 2026-03-24T06:30:35.546532*

---

## Phase Purpose
In this Hardware Design phase, I will produce a hardware-design JSON that binds the PWM requirements to concrete AURIX TC3xx resources: TOM instances/channels, CMU clocking, and pin routes on KIT_A2G_TC387_5V_TFT. This mapping is the contract for code generation, ensuring the iLLD drivers can be instantiated correctly on the exact pins and clocks the application expects.

## Data Inventory
- user_requirement (from user prompt)
  - Contains: Switch to IfxGtm_Pwm (single-output), keep TOM1 with TOM1_CH0 as 20 kHz center-aligned timebase on CMU Fxclk0; create PWM on TOM1 CH2/4/6 to P02.0/1/2 via &IfxGtm_TOM1_2_TOUT12_P02_0_OUT, &IfxGtm_TOM1_4_TOUT14_P02_1_OUT, &IfxGtm_TOM1_6_TOUT16_P02_2_OUT; init duties U/V/W = 25/50/75%; step +10% every 500 ms; update Cpu0 WAIT_TIME to 500 ms.
  - Use: Primary source of truth for channels, pins, clocks, frequency, and duty-update behavior.

- target_board (from workflow input)
  - Contains: KIT_A2G_TC387_5V_TFT.
  - Use: Select device macros, pin maps, and board-specific capabilities.

- requirements_json (from refiner)
  - Contains: PWM with driver_name IfxGtm_Pwm; gtm_enable true; cmu_clock Fxclk0; tom_module TOM1; timebase TOM1_CH0 center; pwm_frequency_hz 20000; duty_update_period_ms 500; duty_step_percent 10; device_macro DEVICE_TC38X; pin_package IFX_PIN_PACKAGE_292; reference files.
  - Use: Validate constraints and fill JSON fields (device, clocks, timing).

- template_capabilities (from template analyzer)
  - Contains: mcu_family TC3xx; device_macro DEVICE_TC38X; clock_config (xtal 20 MHz, pll 300 MHz); available pin maps for TC38x LFBGA292 and LFBGA516; pin_package IFX_PIN_PACKAGE_516.
  - Use: Resolve pin-map headers and CMU source frequency selection; flag package mismatch (292 vs 516).

- reference_analysis (from reference project)
  - Contains: Current uses IfxGtm_Tom_PwmHl; macros WAIT_TIME=10; PWM_FREQ_HZ=20000; TOM-based 3-phase example.
  - Use: Migrate to IfxGtm_Pwm; update WAIT_TIME to 500; preserve 20 kHz/center-aligned pattern.

- header_documentation_context (from iLLD docs)
  - Contains: IfxGtm_Pwm notes—use only for sync channels, not base; default IRQ mode, pin array fields like output[i].pin.
  - Use: Ensure TOM1_CH0 is base via timer; use IfxGtm_Pwm only on sync channels 2/4/6.

- extracted_config_values (from header parser)
  - Contains: Fields like output[0].pin, config.syncUpdateEnabled, dtmClockSource IfxGtm_Dtm_ClockSource_cmuClock0, polarity Ifx_ActiveHigh.
  - Use: Populate driver-specific parameters in JSON.

- pin_mapping_from_docs (from pin validator)
  - Contains: Example TOM1 pin map entry (e.g., IfxGtm_TOM1_5_TOUT11_P00_2_OUT).
  - Use: Cross-check pin-map pattern; primary pin macros come from user_requirement.

- detected_peripheral_type
  - Contains: PWM.
  - Use: Confirms peripheral selection path.

## Execution Plan
1. Confirm device and board
   - Set device_macro to DEVICE_TC38X and board to KIT_A2G_TC387_5V_TFT from requirements_json/template_capabilities.

2. Resolve pin package and pin-map headers
   - Prefer IFX_PIN_PACKAGE_292 (requirements_json) and include TC38x LFBGA292 PinMap; flag mismatch with template’s 516.

3. Enable GTM and CMU Fxclk0
   - Set GTM enable true; select CMU Fxclk0 as clock source (dtmClockSource IfxGtm_Dtm_ClockSource_cmuClock0).

4. Configure TOM1 timebase (base channel)
   - Use TOM1_CH0 as center-aligned base at 20 kHz.
   - Compute CMU CLK0 divider and TOM1_CH0 period based on IfxGtm_Cmu_getModuleFrequency so f_pwm = 20 kHz with up/down counting.

5. Instantiate PWM channels with IfxGtm_Pwm
   - Create three sync channels on TOM1 CH2/CH4/CH6 (slaved to CH0); set polarity active high; enable syncUpdate.
   - Assign pins:
     - CH2 -> &IfxGtm_TOM1_2_TOUT12_P02_0_OUT
     - CH4 -> &IfxGtm_TOM1_4_TOUT14_P02_1_OUT
     - CH6 -> &IfxGtm_TOM1_6_TOUT16_P02_2_OUT
   - Ignore low-side pins.

6. Initialize duty cycles and update cadence
   - Set initial duties: U=25%, V=50%, W=75%.
   - Set duty_update_period_ms=500, duty_step_percent=10, wraparound at 100% to 0%.

7. CPU timing macro
   - Update Cpu0 WAIT_TIME to 500 ms in the hardware JSON metadata for downstream codegen.

8. Produce hardware-design JSON
   - Emit clocks, GTM->CMU, TOM1 base, three IfxGtm_Pwm channels, pin routes, and driver parameters.

## Key Decisions & Risks
- Base-channel strategy: Per IfxGtm_Pwm docs, I will keep TOM1_CH0 as the base using a TOM timer (e.g., IfxGtm_Tom_Timer), and apply IfxGtm_Pwm only to sync channels 2/4/6.
- Pin package conflict: requirements_json says IFX_PIN_PACKAGE_292 while template lists IFX_PIN_PACKAGE_516. I’ll target LFBGA292 (needed for P02.0/1/2 mappings) and flag this for confirmation.
- Clock derivation: GTM module frequency isn’t explicitly stated; I’ll derive from IfxGtm_Cmu_getModuleFrequency and compute CMU Fxclk0 divider accordingly.
- Pin availability: The exact macros &IfxGtm_TOM1_2_TOUT12_P02_0_OUT, etc., must exist in TC38x LFBGA292 PinMap; if not, I’ll need alternates on P02.x or same TOUTs.