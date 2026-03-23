# AI Reasoning - Software Architecture

*Generated: 2026-03-23T15:04:51.110422*

---

## Phase Purpose
In this Software Architecture phase, I define the high-level module layout, driver abstractions, include hierarchy, ISR strategy, and the init sequence for the eGTM ATOM-based 3‑phase inverter PWM. The goal is to produce a clean, migration-ready structure (from TOM to ATOM) that satisfies timing, ISR, and pinout constraints on KIT_A3G_TC4D7_LITE and the TC4xx family.

## Data Inventory
- user_requirement (UI input)
  - From: User entry in this phase kickoff.
  - Content: “TC4D7 eGTM ATOM1 (Cluster 1) center‑aligned 20 kHz complementary PWM with 1 µs deadtime, independent U/V/W duty updates, single period ISR on CPU0 (IfxSrc_Tos_cpu0, priority 20).”
  - Use: Drives the core architecture choices: eGTM/ATOM1, center-aligned, complementary pairs, deadtime, and ISR mapping.

- requirements_json (refiner node)
  - From: Phase 1 structured synthesis.
  - Content: PWM peripheral using IfxEgtm_Atom_Pwm; cluster=1, submodule=ATOM1, channels U/V/W; 20 kHz; center_aligned=true; deadtime_ns=1000; ISR cpu=CPU0 tos=IfxSrc_Tos_cpu0 priority=20; device macro DEVICE_TC4D7; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h; reference includes (IfxGtm_Pwm.h, IfxPort.h, IfxPort_Pinmap.h, IfxScuWdt.h, IfxCpu.h, Bsp.h).
  - Use: Source of truth for module/file naming, ISR config, and driver selection.

- hardware_json (hardware node)
  - From: Phase 2 HW mapping.
  - Content: microcontroller TC4Dx, device_macro DEVICE_TC4D7, package IFX_PIN_PACKAGE_BGA292_COM; peripheral EGTM.ATOM1 channels [0,1,2]; pin_assignments include ATOM1_CH0_HS -> P20.8 (function ATOM1_CH0); clock: xtal 25 MHz, system 500 MHz; validation conflicts on device/package mismatch.
  - Use: Bind channels U/V/W to ATOM1.CH[0..2]; derive pinmux from pin_assignments; note device/package conflict as a risk.

- template_capabilities (template analyzer)
  - From: Template library.
  - Content: Path /infineon_POC_MVP/ads_templates/KIT_A3G_TC4D7_LITE; mcu_family TC4xx; device_macro DEVICE_TC4DX; supported EGTM/PWM/INTERRUPT; clock: syspll 500 MHz; pin_package IFX_PIN_PACKAGE_BGA436_COM.
  - Use: Confirm iLLD availability; flag macro/package divergence; adopt template include & startup constraints.

- reference_analysis (reference project)
  - From: Analyzer of TOM reference.
  - Content: Patterns for watchdog disable, CPU0 init, ISR enabling; includes like IfxPort, IfxScuWdt, IfxCpu; file naming and function patterns.
  - Use: Preserve coding style and init flow in the new ATOM module.

- header_documentation_context + extracted_config_values
  - From: iLLD headers.
  - Content: IfxEgtm_PinMap/IfxEgtm_Pwm docs; fields such as deadTime.rising/falling (default 0.0f), dtmClockSource IfxGtm_Dtm_ClockSource_cmuClock0, polarity/complementaryPolarity enums, example pin macros.
  - Use: Define struct fields in IfxEgtm_Pwm_Config (deadtime=1e-6 s both edges, complementary polarity), DTM clock source, and pinmap usage.

## Execution Plan
1. Define module decomposition
   - Files: EGTM_ATOM_3_Phase_Inverter_PWM.c/.h (as required).
   - Public API: init(), setDuty_U/V/W(float duty01), enableOutputs(bool), ISR handler prototype.

2. Include hierarchy
   - Use: IfxEgtm_Pwm.h, IfxEgtm_PinMap.h, IfxPort.h, IfxScuWdt.h, IfxCpu.h, Bsp.h.
   - Rationale: Minimal iLLD set for PWM, pinmux, watchdog/CPU, BSP ticks.

3. Clock and GTM enable
   - Use hardware_json clock (25 MHz xtal, 500 MHz sys). Initialize eGTM CMU to a suitable clock (e.g., CMU_CLK0) to achieve 20 kHz center-aligned with fine deadtime resolution.

4. Pin mapping for ATOM1 channels
   - Bind U/V/W to EGTM Cluster 1, ATOM1 CH0/1/2.
   - Map HS/LS pins per pin_assignments (e.g., ATOM1_CH0_HS -> P20.8); select corresponding complementary LS pins from the same list.
   - Validate 6 complementary outputs total.

5. IfxEgtm_Pwm configuration
   - Create IfxEgtm_Pwm_Config and IfxEgtm_Pwm handle.
   - Set center-aligned mode, complementary outputs enabled, deadTime.rising/falling = 1e-6, complementaryPolarity per inverter needs, dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0.
   - Assign three channels to ATOM1.CH[0..2] with independent duty update.

6. ISR registration
   - Configure single period ISR for ATOM1 (Cluster 1) on CPU0 with IfxSrc_Tos_cpu0 and priority 20; enable SRC and global interrupts.
   - Handler: update housekeeping or trigger ADC sync hook (placeholder).

7. Initialization sequence
   - Disable watchdogs (IfxScuWdt) per reference pattern; init clocks; init pins; init PWM; register ISR; start outputs disabled, then enable after first valid duty set.

## Key Decisions & Risks
- Device/package mismatch: requirements request DEVICE_TC4D7 with IFX_PIN_PACKAGE_BGA292_COM, template uses DEVICE_TC4DX and BGA436_COM. I must align device_macro and pin package or select a matching template to avoid wrong pin macros.
- Pin completeness: Only ATOM1_CH0_HS (P20.8) is explicitly shown; I will verify all 3 HS/LS pairs exist and are routable on KIT_A3G_TC4D7_LITE.
- ISR source line: ATOM1 period event routing (SR index) must be confirmed for EGTM1/ATOM1 on TC4D7 to ensure a single shared period ISR.
- Timing base: Choose CMU frequency/prescaler to meet 20 kHz center-aligned with 1 µs deadtime resolution; confirm DTM clock domain and synchronization with AGC.
- Driver availability: Ensure IfxEgtm_Pwm headers/libraries are present for TC4xx; otherwise, fall back to lower-level IfxEgtm Atom APIs.