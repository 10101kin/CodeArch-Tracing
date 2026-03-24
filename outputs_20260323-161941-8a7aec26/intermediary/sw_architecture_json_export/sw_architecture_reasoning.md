# AI Reasoning - Software Architecture

*Generated: 2026-03-24T00:27:27.780355*

---

## Phase Purpose
In this Software Architecture phase, I define the high-level module breakdown, driver abstractions, include hierarchy, ISR strategy, and initialization sequence for the migrated PWM. The goal is to move from TC387 TOM PwmHl to TC4D7 eGTM ATOM using the unified IfxEgtm_Pwm driver while preserving 20 kHz center-aligned complementary outputs, 0.5 µs deadtime, and 1.0 µs minimum pulse.

## Data Inventory
- user_requirement (from user input)
  - What: Migrate TC387 TOM to TC4D7 eGTM ATOM1, six outputs, center-aligned 20 kHz, 0.5 µs deadtime, 1.0 µs min pulse, FXCLK0 from GCLK, one ATOM instance (default ATOM1 C…).
  - How I’ll use: Architectural constraints (driver selection IfxEgtm_Pwm, ATOM1 grouping, timing features).

- target_board (from board selector)
  - What: KIT_A3G_TC4D7_LITE.
  - How I’ll use: Select template path and board-compatible pin macros.

- requirements_json (from refiner)
  - What: Structured requirements (type PWM, driver_name IfxEgtm_Pwm, alignment center-aligned, phases U/V/W with high/low, pwm_frequency_hz 20000, deadtime_us 0.5, min_pulse_us 1.0, clocking FXCLK0 from GCLK, device_macro DEVICE_TC4DX, pin_package IFX_PIN_PACKAGE_BGA436_COM, expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h, reference includes Ifx_Types.h, IfxGtm_Tom_PwmHl.h, IfxCpu_Irq.h, IfxScuWdt.h).
  - How I’ll use: Drive module design, includes, and config fields mapping (migrating from IfxGtm_Tom_PwmHl.h to IfxEgtm_Pwm.h).

- hardware_json (from hardware phase)
  - What: MCU TC4D7, EGTM.ATOM1 channels [1..6], pins e.g., U_H -> P00.3 (TOUT12, pin_macro TBD), clock xtal 25 MHz, system 500 MHz; validation all_verified: False.
  - How I’ll use: Bind ATOM1 channel pairs to U/V/W high/low and map to TOUT pins; flag unresolved pin_macros; validate interrupt routing.

- template_capabilities (from template analyzer)
  - What: Template KIT_A3G_TC4D7_LITE, family TC4xx, device_macro DEVICE_TC4DX, available drivers include IfxEgtm_Pwm, clock config syspll 500 MHz.
  - How I’ll use: Confirm driver availability and device macros; pull clock/interrupt defaults.

- reference_analysis (from reference project)
  - What: Former TOM PwmHl usage; includes like IfxGtm_Tom_PwmHl.h; macros PWM_FREQ_HZ 20000, PWM_DEAD_TIME.
  - How I’ll use: Preserve file pattern and coding style; translate TOM init patterns to eGTM.

- header_documentation_context (from iLLD docs)
  - What: IfxEgtm_Pwm API usage, up to 16 signals (8 complementary pairs).
  - How I’ll use: Select config structs, ISR APIs, dead-time and CMU setup.

- extracted_config_values (from headers)
  - What: deadTime.rising/falling, dtmClockSource IfxGtm_Dtm_ClockSource_cmuClock0, polarity enums, pin macro examples IfxEgtm_ATOMx_y_TOUTz_Pxx_y_OUT, config.frequency exemplar.
  - How I’ll use: Populate configuration fields; select DTM clock source to CMU FXCLK0.

- detected_peripheral_type
  - What: PWM.
  - How I’ll use: Confirms chosen stack.

## Execution Plan
1. Select driver and modules
   - Use IfxEgtm_Pwm (requirements_json, template_capabilities). Create EGTM_ATOM_3_Phase_Inverter_PWM.c/.h with public API init/start/update.

2. Include hierarchy
   - Replace legacy includes with IfxEgtm_Pwm.h; keep Ifx_Types.h, IfxCpu.h, IfxCpu_Irq.h, IfxScuWdt.h (requirements_json, reference_analysis).

3. Clock and CMU setup
   - Configure eGTM CMU FXCLK0 sourced from GCLK (user_requirement). Use dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0 (extracted_config_values). Base timing from template_capabilities clock_config.

4. ATOM topology
   - Allocate EGTM.ATOM1 channels [1..6] as three complementary pairs: U(V/W) high/low (hardware_json). Enable center-aligned mode, complementary polarity (requirements_json, header_documentation_context).

5. Dead-time and min pulse
   - Set deadTime.rising/falling = 0.5 µs (extracted_config_values). Enforce 1.0 µs minimum pulse via driver minPulse if available; else clamp duty in update routine before calling IfxEgtm_Pwm_setDuty (fallback software guard).

6. Pin mapping
   - Map signals to pins: e.g., U_H -> P00.3 (TOUT12) and remaining V/W per hardware_json pin_assignments. Resolve pin_macro to IfxEgtm_ATOM1_x_TOUTy_Pzz_w_OUT names; flag TBD entries for resolution.

7. ISR registration
   - Configure ATOM1 interrupt (SR0) to CPU0, register ISR via IFX_INTERRUPT macro in EGTM_ATOM_3_Phase_Inverter_PWM.c for update/diagnostics (header_documentation_context, template_capabilities.interrupt_config).

8. Initialization sequence
   - Disable watchdogs, enable GTM/eGTM, initialize IfxEgtm_Pwm_Config, assign channels/pins, set frequency 20 kHz center-aligned, apply deadtime/min pulse, start outputs (reference_analysis patterns adapted).

9. API facade
   - Provide functions: Pwm3Ph_Init(), Pwm3Ph_Start(), Pwm3Ph_UpdateDuty(float du,dv,dw), Pwm3Ph_Stop(); document constraints.

## Key Decisions & Risks
- Driver choice: IfxEgtm_Pwm vs IfxGtm_Atom_Pwm; choosing unified IfxEgtm_Pwm per requirements_json to support complementary pairs cleanly.
- Pin macros: Some pin_macro = TBD; risk of build-time errors or wrong TOUT mapping. Need final routing confirmation for KIT_A3G_TC4D7_LITE.
- FXCLK0 source: Ensure GCLK-to-FXCLK0 path is enabled; misconfiguration would break 20 kHz accuracy.
- Min pulse enforcement: If API lacks built-in minPulse, software clamping is required; verify with header_documentation_context.
- Interrupt line mapping: ATOM1 SR routing to CPU0 must match device TC4D7 service request nodes; confirm with device header.