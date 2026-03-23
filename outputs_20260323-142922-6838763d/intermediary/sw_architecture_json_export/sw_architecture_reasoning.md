# AI Reasoning - Software Architecture

*Generated: 2026-03-23T14:50:44.175694*

---

## Phase Purpose
In this Software Architecture phase, I’ll define the module breakdown, driver stack, include hierarchy, ISR wiring, and the initialization/startup sequence to realize a 3‑phase inverter PWM on AURIX TC387. The outcome is a clear, reviewable architecture that maps requirements to concrete iLLD drivers (GTM TOM1 + PwmHl) and pins, ensuring the next phase can generate/build code with minimal ambiguity.

## Data Inventory
- user_requirement (from user input)
  - What: Drive 3‑phase inverter at 20 kHz, center‑aligned, using GTM TOM1 with CH0 as time base; complementary outputs via `IfxGtm_Tom_PwmHl`; pins on Port 02: U HS=P02.0, U LS=P02.7; V HS=P02.1, V LS=P02.4; W HS=P02.2, W LS=P02.5.
  - Use: Define PWM topology (timer + PWM HL), alignment, frequency, and exact pin mapping.

- target_board (from project context)
  - What: `KIT_A2G_TC387_5V_TFT`.
  - Use: Select board template, BSP includes, and default clock/SSW assumptions.

- requirements_json (from refiner node)
  - What: Structured spec: `GTM.TOM1`, time_base_channel=0, center_aligned=True, `pwm_frequency_hz=20000`, `driver_name=IfxGtm_Tom_PwmHl`, `gtm_clock_source=CMU_CLK0`, `gtm_clock_freq_mhz=100`, device_macro `DEVICE_TC38X`, pin_package `IFX_PIN_PACKAGE_292`, expected files `GTM_TOM_3_Phase_Inverter_PWM.c/.h`, reference includes list.
  - Use: Authoritative constraints for driver choice, clocking, file names, and include hierarchy.

- hardware_json (from hardware node)
  - What: MCU `TC387`, `GTM.TOM1` channels [0..5], pin assignments with macros (e.g., `IfxGtm_TOM1_0_TOUT104_P02_0_OUT` for U_HS), xtal=20 MHz, system=300 MHz, all_verified=True.
  - Use: Bind TOM1 channels to exact TOUT macros and validate availability; confirm channel count and clocks.

- template_capabilities (from template library analyzer)
  - What: Template path `/infineon_POC_MVP/ads_templates/KIT_A2G_TC387_5V_TFT`, family TC3xx, available pinmap headers (e.g., `IfxGtm_PinMap_TC38x_LFBGA292.h`, `..._516.h`), PLL/clock summary.
  - Use: Ensure iLLD headers exist for our package; resolve pinmap header selection.

- reference_analysis (from reference project analyzer)
  - What: Uses `IfxGtm_Tom_PwmHl` and `IfxGtm_Tom_Timer`; files include `GTM_TOM_3_Phase_Inverter_PWM.c/.h`; pattern: watchdog disable, `IfxCpu_enableInterrupts`, macro `PWM_FREQ_HZ=20000`.
  - Use: Mirror proven include order, API usage, and file structure.

- header_documentation_context + extracted_config_values
  - What: API docs and extracted fields/macros (e.g., `dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0`, `IFXGTM_CH0CCU0_INTR_PRIO=3`, `deadTime.rising=0.0f`).
  - Use: Define ISR priority, DTM clock source, and deadtime config placeholders.

- detected_peripheral_type
  - What: PWM.
  - Use: Confirms driver domain.

## Execution Plan
1. Select drivers and layering
   - Use `IfxGtm_Tom_Timer` as time base on `GTM.TOM1` CH0 (center-aligned), and `IfxGtm_Tom_PwmHl` for 3 complementary pairs (U,V,W). Rationale: matches requirements_json and reference_analysis.

2. Define module decomposition
   - `GTM_TOM_3_Phase_Inverter_PWM.h/.c`: encapsulate init/start/stop and `setDutyUVW(float u, float v, float w)`.
   - Integration in `Cpu0_Main.c`: call init, then periodic updates if needed.

3. Establish include hierarchy
   - Core: `Ifx_Types.h`, `IfxCpu.h`, `IfxScuWdt.h`, `Bsp.h`.
   - GTM: `IfxGtm_Tom_Timer.h`, `IfxGtm_Tom_PwmHl.h`, `IfxGtm_PinMap.h` (package-specific), optionally `IfxGtm_Cmu.h`.

4. Clock and GTM enable
   - Use CMU CLK0 (100 MHz per requirements_json). Call GTM enable and CMU clock setup; retrieve with `IfxGtm_Cmu_getModuleFrequency`.

5. Timer configuration
   - Configure `IfxGtm_Tom_Timer_Config` for TOM1 CH0, up‑down mode, frequency 20 kHz, sync update enabled. Install ISR with `IFXGTM_CH0CCU0_INTR_PRIO=3`.

6. PWM HL channel/pin mapping
   - Map pairs: (CH0,CH1)=U (HS/LS), (CH2,CH3)=V, (CH4,CH5)=W.
   - Assign pins using hardware_json macros (e.g., `IfxGtm_TOM1_0_TOUT104_P02_0_OUT` for U_HS; likewise for P02.7, P02.1, P02.4, P02.2, P02.5).
   - Set complementary polarity and deadtime; choose `dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0`.

7. ISR registration
   - Register timer ISR on CCU0 (and CCU1 if needed for shadow transfer) with CPU0 TOS; enable interrupts post-init.

8. Initialization sequence
   - Disable CPU/STM watchdogs, init GTM timer, init PwmHl, set 50% neutral duty, enable outputs, start timer.

9. File outputs
   - Produce `GTM_TOM_3_Phase_Inverter_PWM.c/.h` matching reference include/order and expose clean APIs.

## Key Decisions & Risks
- Pinmap header/package mismatch: requirements_json says `IFX_PIN_PACKAGE_292`, template_capabilities default shows `IFX_PIN_PACKAGE_516`. I will select `IfxGtm_PinMap_TC38x_LFBGA292.h`; if absent, we must switch template or adjust package.
- Deadtime value not specified: default is `0.0f` in extracted_config_values; I’ll add a TODO and make it configurable.
- Duty-update mechanism: no runtime control specified; I’ll provide `setDutyUVW()` and a placeholder 50% at init.
- Interrupt strategy: if only timer base CCU0 ISR is required vs. both CCU0/CCU1; I’ll start with CCU0 for period tick and enable TOM shadow transfer—adjust if modulation requires tighter timing.