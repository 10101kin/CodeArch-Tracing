# AI Reasoning - Detailed Design

*Generated: 2026-03-23T12:54:22.821747*

---

## Phase Purpose
In Detailed Software Design, I will lock down exact APIs, data structures, constants, and init/update sequences for the TC4xx eGTM ATOM PWM. This becomes the blueprint that code generation follows and the contract that unit tests validate, ensuring the migrated 3‑phase inverter on TC4D7 behaves like the TC387 reference.

## Data Inventory
- user_requirement (from user input)
  - Content: Migrate TC387 3‑phase inverter (GTM TOM + EVADC) to TC4D7 using eGTM ATOM Cluster 0, ATOM0 CH0–CH2 complementary center‑aligned at 20 kHz, 1 µs deadtime, HS active‑high/LS active‑low on P20.8/9, P20.10/11, P20.12/13; ATOM0 CH3 50% edge‑aligned trigger.
  - Use: Define functional targets (frequency, alignment, deadtime, polarity, channel roles, desired pins).

- target_board (from user selection)
  - Content: KIT_A3G_TC4D7_LITE.
  - Use: Constrain available pins/peripherals and select template path.

- requirements_json (from refiner, Phase 1)
  - Content: MIGRATION mode; driver IfxEgtm_Pwm; eGTM Cluster 0 ATOM0 CH0–2 PWM + CH3 trigger; 20 kHz, 1 µs; device macro DEVICE_TC4D7; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/h; includes IfxEgtm_Pwm, IfxPort, Ifx_Types.
  - Use: Drive driver choice, file names, and API surface (initEgtmAtom3phInv, duty update, trigger setup).

- hardware_json (from hardware mapping, Phase 2)
  - Content: MCU TC4DX, device_macro DEVICE_TC4DX; eGTM ATOM0 CH[0..3]; pin suggestions like ATOM0.CH0_HS -> P02.0 (TOUT0); conflicts: user pins P20.x not available on KIT_A3G_TC4D7_LITE.
  - Use: Resolve actual pin macros (IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT, etc.) or flag pin mapping decision.

- sw_architecture_json (from Phase 3)
  - Content: Recommended driver IfxEgtm_Pwm; init APIs IfxEgtm_Pwm_initConfig/IfxEgtm_Pwm_init; function name initEgtmAtom3phInv; migration notes IfxGtm_* → IfxEgtm_*.
  - Use: Fix driver layer and init flow; confirm function names.

- template_capabilities (from template analyzer)
  - Content: Template path /infineon_POC_MVP/ads_templates/KIT_A3G_TC4D7_LITE; clocks 25 MHz XTAL, 500 MHz SYSPLL; device_macro DEVICE_TC4DX.
  - Use: Clock basis for PWM tick math and CMU configuration; reconcile device macro.

- reference_analysis (from reference project)
  - Content: Prior TOM usage; patterns: watchdog disable, include styles, macro NUM_OF_CHANNELS (3).
  - Use: Style and naming consistency; adopt 3‑channel grouping.

- header_documentation_context (IfxEgtm_Pwm API docs)
  - Content: Structs, enums, polarity/complementary controls, deadtime, CMU sources.
  - Use: Exact struct/field names and valid enum values.

- extracted_config_values (from headers)
  - Content: deadTime.rising/falling; polarity enums; example pin macros IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT; dtmClockSource IfxGtm_Dtm_ClockSource_cmuClock0.
  - Use: Concrete field names and example macros for config JSON.

- detected_peripheral_type
  - Content: PWM.
  - Use: Confirms scope.

## Execution Plan
1. Finalize driver and includes
   - Use sw_architecture_json and header_documentation_context to standardize on IfxEgtm_Pwm; include Ifx_Types.h, IfxCpu.h, IfxEgtm_Pwm.h, IfxPort.h.

2. Clock and tick derivation
   - From template_capabilities (25 MHz XTAL, 500 MHz SYSPLL), choose CMU clock (e.g., cmuClk0) and prescaler to yield a convenient base (e.g., 100 MHz). Compute periodTicks for 20 kHz (50 µs) and deadtimeTicks for 1 µs.

3. Pin mapping resolution
   - Prefer hardware_json verified TOUT macros (e.g., IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT …) for CH0–CH2 HS/LS; document deviation from P20.x. Map CH3 trigger to a valid TOUT (e.g., P33.0 equivalent if available; else board‑valid pin).

4. Define design structs and constants
   - Define EgtmAtom3phInv_Config (frequencyHz, deadtimeUs, polarity HS=ActiveHigh, LS=ActiveLow, pin macros, cluster=0, atom=0).
   - Define EgtmAtom3phInv_Handle holding IfxEgtm_Pwm driver handle and three complementary channel handles plus trigger.

5. Initialization sequence
   - Sequence: IfxEgtm_Cmu_enableClocks → IfxEgtm_Pwm_initConfig (set center‑aligned, complementary, deadtime, polarity, CMU clock) → channel pin binding (output[].pin to verified TOUT macros) → IfxEgtm_Pwm_init → start PWM and set initial 50% duty for CH0–2; configure CH3 as edge‑aligned 50%.

6. API design
   - initEgtmAtom3phInv(const EgtmAtom3phInv_Config*, EgtmAtom3phInv_Handle*)
   - egtmAtom3phInv_setDuty(float dutyA, float dutyB, float dutyC) with range checks and shadow transfer.
   - egtmAtom3phInv_setTrigger50() or part of init.

7. Unit-test specs
   - Compute tick math test (20 kHz ±1%).
   - Deadtime ticks == 1 µs within 1 tick.
   - Polarity verification by reading config fields.
   - CH3 duty fixed at 50% edge‑aligned.
   - Update latency within duty_update_period_ms (500 ms) using STM timestamps.

8. File layout
   - Emit EGTM_ATOM_3_Phase_Inverter_PWM.h/.c with documented function signatures and structs.

## Key Decisions & Risks
- Pin assignment: User P20.x vs board‑verified P02.x; I will default to hardware_json verified TOUT pins and note the deviation.
- Device macro mismatch: DEVICE_TC4D7 (requirements) vs DEVICE_TC4DX (template); I will target DEVICE_TC4DX per template unless user confirms TC4D7 variant.
- CMU clock selection affects resolution/deadtime accuracy; I’ll choose a clock that yields integer ticks for 1 µs.
- EVADC trigger linkage: CH3 is prepared as 50% PWM; wiring to EVADC (IfxEgtm_Trig/IfxGtm_Trig) may need an extra design step if ADC coupling is required.