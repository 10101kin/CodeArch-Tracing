# AI Reasoning - Software Architecture

*Generated: 2026-03-23T12:48:16.721391*

---

## Phase Purpose
In this Software Architecture phase, I define the high-level module structure, driver abstraction, include hierarchy, ISR wiring, and the initialization sequence for the 3‑phase inverter PWM on TC4Dx. The outcome is a clear architectural blueprint that the next phase will translate into code, ensuring requirements and hardware constraints are satisfied and migration from TC3xx TOM to TC4xx eGTM ATOM is consistent.

## Data Inventory
- user_requirement (user): 3‑phase PWM on EGTM0.ATOM2 CH0/1/2 using IfxEgtm_Pwm; complementary via CDTM to P20.8/9, P20.10/11, P20.12/13; 20 kHz center‑aligned, 1 μs deadtime; sync start/update; per‑phase independent duty; one period ISR on CPU0 prio 20; ATOM FXCLK0, DTM CLK0. I’ll treat this as the architectural contract.
- target_board (hardware selection): KIT_A3G_TC4D7_LITE. I’ll align pin/clock choices and file naming to this BSP.
- requirements_json (Phase 1): Confirms migration from TC3xx to TC4xx, driver_name IfxEgtm_Pwm, files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h, module EGTM0 ATOM2 CH[0..2]. I’ll base module/file decomposition and ISR policy on this.
- hardware_json (Phase 2): MCU TC4Dx, peripheral_instance EGTM0.ATOM2 CH[0,1,2], clock 25 MHz XTAL / 500 MHz system; pin_assignments list P20.8–P20.13; validation shows conflicts for those pins. I’ll architect for P20.* but flag validation risk and keep routing abstracted via TOUTSEL.
- template_capabilities (template analyzer): TC4xx iLLD available; interrupt_config present; device_macro DEVICE_TC4DX. I’ll reference available iLLD headers/APIs and note macro mismatch with DEVICE_TC4D7.
- reference_analysis (Phase 0): Prior TC3xx used IfxGtm_Pwm with TOM; includes like IfxPort, Ifx_Types; ISR/IRQ patterns via IfxCpu_enableInterrupts. I’ll mirror include/ISR patterns while migrating to IfxEgtm_Pwm and ATOM/CDTM.
- header_documentation_context (docs): IfxEgtm_Pwm usage, ISR and config fields. I’ll derive struct and API names (e.g., alignment, deadTime, clock sources).
- extracted_config_values (headers): Fields like deadTime.rising/falling, config.dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, polarity enums, example pin maps. I’ll map these to TC4xx eGTM variants (FXCLK0, CLK0) and set numeric values (1e‑6 s, 20 kHz).
- detected_peripheral_type: PWM. Confirms path.

## Execution Plan
1. Define module decomposition:
   - Core: `EgtmAtom3PhPwm` (driver wrapper around IfxEgtm_Pwm).
   - HAL: pin routing/CDTM/TOUTSEL helper.
   - ISR: single period tick for CPU0.
2. Driver selection:
   - Use `IfxEgtm_Pwm` (per requirements_json). Migrate away from `IfxGtm_Pwm` references.
3. Clock plan:
   - Configure EGTM FXCLK0 for ATOM; DTM from CMU CLK0; enable module-frequency settings. Calculate 20 kHz center‑aligned period from FXCLK0.
4. Channel topology:
   - EGTM0.ATOM2 channels 0=U,1=V,2=W; complementary via CDTM per channel; one “master” generates the period event for ISR (choose CH0).
5. Pin routing strategy:
   - Target P20.8/9 (U HS/LS), P20.10/11 (V), P20.12/13 (W) via TOUTSEL; abstract through a routing table to accommodate validation fixes later.
6. Config structures:
   - Set `alignment=center`, `frequency=20000.0f`, `deadTime.rising=deadTime.falling=1e-6`, `polarity=ActiveHigh`, `complementaryPolarity=ActiveLow`, `dtmClockSource=cmuClock0`, `atomClock=fxclk0`, `syncStart=true`, `syncUpdate=true`.
7. Include hierarchy:
   - Header: `EGTM_ATOM_3_Phase_Inverter_PWM.h` includes `Ifx_Types.h`, `IfxEgtm_Pwm.h`, `IfxPort.h`.
   - Source: includes header plus `IfxSrc.h`, `IfxCpu.h`, `IfxScuWdt.h`.
8. ISR registration:
   - Define `IFX_INTERRUPT(EgtmAtom2_PeriodIsr, 0, 20)`, bind to EGTM0 ATOM2 CH0 period SRC via `IfxSrc_init`/`IfxSrc_enable`.
9. Init sequence:
   - Enable eGTM clocks; configure CMU FXCLK0 and CLK0; init DTM/CDTM; configure ATOM2 CH0/1/2; map TOUTSEL to P20.*; arm sync start; enable ISR.
10. Runtime API:
   - Expose `SetDuty_U/V/W(float duty)` with independent buffered updates, applied on next sync.

## Key Decisions & Risks
- Pin validation: hardware_json flags P20.8–P20.13 conflicts; I’ll keep routing abstract and may need alternative TOUTSEL or pins if board/package differs.
- Device macro mismatch: DEVICE_TC4D7 vs DEVICE_TC4DX in template; ensure correct conditional compilation.
- ISR source: use CH0 period event as global tick; verify single interrupt suffices for sync updates.
- Deadtime/clock domains: confirm CDTM availability on ATOM2 and resolution at CLK0 for 1 μs; adjust FXCLK0/CLK0 if needed.