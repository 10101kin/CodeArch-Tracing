# AI Reasoning - Hardware Design

*Generated: 2026-03-23T12:40:08.325249*

---

## Phase Purpose
In this Hardware Design phase, I will translate the PWM requirements into a concrete mapping on the TC4D7 MCU: specific eGTM0/ATOM2 channels, pin routes (with CDTM), module clocks, and an ISR description. This produces the hardware design JSON the later phases will use to auto-generate initialization code and ensure pin/clock consistency.

## Data Inventory
- user_requirement (from user)
  - What: eGTM0 ATOM2 CH0/1/2 for U/V/W with CDTM complementary outputs on P20.8/9, P20.10/11, P20.12/13; 20 kHz center-aligned, 1 µs deadtime; IfxEgtm_Pwm; FXCLK0 and DTM CLK0; period-event ISR CPU0 prio 20.
  - Use: Primary source of truth for channels, pins, timing, clocks, and ISR.

- target_board (from user)
  - What: KIT_A3G_TC4D7_LITE.
  - Use: Select device macro, package, and BSP defaults.

- requirements_json (refiner node)
  - What: Structured constraints for MIGRATION to TC4xx/TC4D7; EGTM0/ATOM2 CH[0..2]; pins P20.8–P20.13; 20 kHz, center, 1.0 µs; sync start/update; ISR CPU0 prio 20; clocks: ATOM FXCLK0, DTM CLK0.
  - Use: Formalize fields in the hardware JSON (no TBDs).

- template_capabilities (template_library_analyzer)
  - What: Template path for KIT_A3G_TC4D7_LITE; device_macro DEVICE_TC4DX; clocks (XTAL 25 MHz, SYSPLL 500 MHz); pin_package IFX_PIN_PACKAGE_BGA436_COM.
  - Use: Derive feasible CMU/FXCLK0/CLK0 settings and resolve device/package macros.

- reference_analysis (reference_project_analyzer)
  - What: TC3xx TOM-based 3-phase sample; iLLD patterns (IfxGtm_Pwm); common init/ISR styles.
  - Use: Guide migration semantics (sync set, master/slave, ISR on period event).

- header_documentation_context (iLLD docs)
  - What: IfxEgtm_Pwm API description and struct fields.
  - Use: Map required fields (syncStart/syncUpdate, deadTime, polarity, service provider).

- extracted_config_values (library_file_analyzer)
  - What: Examples for deadTime.rising/falling, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, polarity enums, example frequency hooks.
  - Use: Choose correct enums and units (deadtime in seconds; DTM clock source).

- pin_mapping_from_docs (pin_validator)
  - What: Sample macros for ATOM0 on P02.x; no direct entries for ATOM2 P20.8–P20.13 in the snapshot.
  - Use: Trigger validation step; flag if ATOM2→P20.x macros are missing.

- detected_peripheral_type
  - What: PWM.
  - Use: Confirms driver path.

## Execution Plan
1. Validate board/device
   - From target_board + template_capabilities: lock target to TC4D7 on KIT_A3G_TC4D7_LITE; choose device_macro DEVICE_TC4DX. Note package mismatch (req: BGA-292 vs template: BGA436) and resolve to template’s IFX_PIN_PACKAGE_BGA436_COM for this board.

2. Clock plan
   - Set eGTM CMU FXCLK0 to 100 MHz (derived from SYSPLL 500 MHz with CMU divider 5) to get 10 ns resolution.
   - Set DTM CLK0 to 100 MHz (IfxGtm_Dtm_ClockSource_cmuClock0) for 1 µs deadtime = 100 ticks.
   - Record “module-frequency settings enabled” per requirement.

3. PWM timing math
   - 20 kHz center-aligned on up/down: period_ticks = FXCLK0 / (2 * f_pwm) = 100e6 / (40e3) = 2500 → PR = 2499.
   - Store ticks and timebase for ATOM2.

4. Channel and CDTM grouping
   - Create one PWM set on EGTM0.ATOM2 with channels CH0=U, CH1=V, CH2=W.
   - Enable complementary outputs via CDTM for each channel; map HS to “pin” and LS to “complementaryPin”; polarity active-high; complementaryPolarity inverted.

5. Pin routing via TOUTSEL
   - Assign:
     - CH0 U: P20.8 (HS), P20.9 (LS)
     - CH1 V: P20.10 (HS), P20.11 (LS)
     - CH2 W: P20.12 (HS), P20.13 (LS)
   - Resolve exact macros IfxEgtm_ATOM2_x_TOUTy_P20_z_OUT; verify availability; select proper TOUTSEL index per pin function.

6. Synchronization model
   - Enable syncStart and syncUpdate across CH0–CH2; select CH0 as master period source; allow per-phase independent duty updates.

7. Interrupt
   - Configure a single period-event ISR on CH0 (or ATOM2 shared period) with priority 20, service provider CPU0 (IfxSrc_Tos_cpu0).

8. Emit hardware design JSON
   - Fill device, clocks, EGTM0/ATOM2 config, CDTM/deadtime, pins/TOUTSEL, ISR, and driver IfxEgtm_Pwm. No TBDs.

## Key Decisions & Risks
- FXCLK0/CLK0 selection: Proposing 100 MHz; adjust if board CMU constraints differ.
- Pin/package mismatch: requirements_json says BGA-292; board template uses BGA436. I will use BGA436 for KIT_A3G_TC4D7_LITE unless the user confirms otherwise.
- Pin macros: Need confirmation that ATOM2→P20.8–P20.13 TOUT mappings exist. If missing, we must choose alternative TOUT pins for ATOM2 or switch to ATOM module supporting P20.x.
- Master channel for ISR: Defaulting CH0; confirm period-event source selection in IfxEgtm_Pwm supports single ISR for the set.