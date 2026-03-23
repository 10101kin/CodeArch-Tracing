# AI Reasoning - Detailed Design

*Generated: 2026-03-24T00:30:35.110766*

---

## Phase Purpose
In this Detailed Software Design phase, I will translate the migration requirements into a concrete, code-ready design: exact function signatures, driver structs, configuration constants, initialization sequences, and unit-test specs. This becomes the unambiguous blueprint the code generator will follow to produce working eGTM ATOM PWM for the TC4D7 board while preserving the original timing behavior.

## Data Inventory
- user_requirement (user input)
  - What: “Migrate TC387 TOM to TC4D7 eGTM ATOM using unified eGTM PWM HL; 20 kHz center‑aligned complementary; 0.5 µs deadtime; 1.0 µs min pulse; FXCLK0 from GCLK; one ATOM instance (default ATOM1).”
  - Use: Drive timing math, clock source, and ATOM resource selection.

- target_board (discovery)
  - What: KIT_A3G_TC4D7_LITE.
  - Use: Bind design to this BSP/template and its pin/package.

- requirements_json (Phase 1 – refiner)
  - What: Peripheral PWM via IfxEgtm_Pwm; center‑aligned PwmHl for phases U/V/W; timing: 20 kHz, 0.5 µs DT, 1.0 µs min; device_macro DEVICE_TC4DX; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h.
  - Use: Populate constants, file names, and API expectations in the design JSON.

- hardware_json (Phase 2 – hardware)
  - What: MCU TC4D7; EGTM.ATOM1 channels [1..6]; example pin U_H=P00.3 (TOUT12); xtal 25 MHz; system 500 MHz; verified_count=6; reference_pins_valid=True.
  - Use: Lock module (ATOM1), channel pairing, and port/pin macros; clock basis for FXCLK0 derivation.

- sw_architecture_json (Phase 3 – sw_architecture)
  - What: Driver IfxEgtm_Pwm; init calls include IfxEgtm_enable, IfxEgtm_Cmu_selectClkInput, IfxEgtm_Pwm_initConfig; init function name initEgtmAtom3phInv; migration notes TOM→eGTM.
  - Use: Select APIs and top-level function names.

- template_capabilities (template_library_analyzer)
  - What: Template KIT_A3G_TC4D7_LITE; device_macro DEVICE_TC4DX; syspll 500 MHz; iLLD drivers list includes eGTM; pin package IFX_PIN_PACKAGE_BGA436_COM.
  - Use: Ensure headers/paths are valid and clocks align with design.

- reference_analysis (reference_project_analyzer)
  - What: Prior TOM design used IfxGtm_Tom_PwmHl; macros like PWM_FREQ_HZ=20000; watchdog patterns; coding style.
  - Use: Preserve naming/style and macro semantics while mapping to eGTM.

- header_documentation_context (intelligent_header_selector)
  - What: IfxEgtm_Pwm API/struct fields (deadTime.rising/falling, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, IFXEGTM_PWM_NUM_CHANNELS=8).
  - Use: Define exact struct layouts and enums in the design.

- extracted_config_values (library_file_analyzer)
  - What: Defaults like deadTime.rising=0.0f; example pin macros IfxEgtm_ATOMx_y_TOUTn_P..; clock configs.
  - Use: Replace defaults with computed design values (e.g., 0.5 µs).

- detected_peripheral_type
  - What: PWM.
  - Use: Sanity check.

## Execution Plan
1. Clocking and timing derivation
   - From hardware_json/template_capabilities, assume GCLK→CMU FXCLK0; propose divider to get fxclk0_hz ≈ 100 MHz from 500 MHz (decision). Compute:
     - period_ticks = fxclk0_hz / (2 * 20 kHz) for center-aligned.
     - deadtime_ticks = 0.5 µs * fxclk0_hz.
     - min_pulse_ticks = 1.0 µs * fxclk0_hz.
   - Record formulas and example values at 100 MHz: period=2500, DT=50, min=100.

2. Channel/pin mapping
   - Use hardware_json EGTM.ATOM1 channels [1..6] to form PwmHl pairs: (1,2)=U, (3,4)=V, (5,6)=W.
   - Bind pins: use provided P00.3/TOUT12 for U_H; resolve remaining TOUT macros via template pin database; mark any TBDs.

3. Driver configuration structures
   - Define IfxEgtm_Pwm_Config and IfxEgtm_PwmHl_Config with:
     - clock selection: FXCLK0 from GCLK; dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0.
     - deadTime.{rising,falling} in seconds (0.5e-6f).
     - complementary polarity per PwmHl; active high.
     - output[].pin = concrete IfxEgtm_ATOM1_n_TOUTm_Pxx_y_OUT macros.

4. Initialization sequence
   - Function `void initEgtmAtom3phInv(void);`
     - IfxEgtm_enable(); IfxEgtm_Cmu_selectClkInput(FXCLK0, GCLK); set divider.
     - IfxEgtm_Pwm_initConfig(); fill timing and pins; IfxEgtm_Pwm_init(); IfxEgtm_PwmHl_enableOutputs().

5. Control APIs
   - `void setDutyUVW(float du, float dv, float dw);` clamp to [min_pulse_ticks/period, 1−min_pulse_ticks/period], enforce center-aligned update.
   - `void setDeadtime_us(float dt_us);` update deadTime fields safely.
   - `void pwmBrake(bool enable);` force-safe state.

6. Constants and headers
   - Macros: PWM_FREQ_HZ 20000U, PWM_DEADTIME_US 0.5f, PWM_MIN_PULSE_US 1.0f, EGTM_ATOM_MODULE EGTM.ATOM1.
   - Includes from sw_architecture_json/header_documentation_context.

7. Unit-test specs
   - Frequency: measure period ticks equals computed target ±1 tick.
   - Deadtime: scope-measured edge separation equals 0.5 µs ±5%.
   - Min pulse: request 0%/100% clamps to ≥1.0 µs off/on time.
   - Complementarity/center alignment: verify pair timing and symmetry.

## Key Decisions & Risks
- FXCLK0 divider selection to hit integer period_ticks (risk: mismatch if GCLK tree constraints differ).
- Final pin macros for all six outputs (risk: some pin_macro are TBD; template must confirm availability on KIT_A3G_TC4D7_LITE).
- Use of DTM hardware vs software deadtime (decision: use DTM via IfxEgtm_Pwm deadTime fields).
- iLLD version differences on TC4xx unified API naming (risk: minor API/struct name drift; mitigated by header_documentation_context).