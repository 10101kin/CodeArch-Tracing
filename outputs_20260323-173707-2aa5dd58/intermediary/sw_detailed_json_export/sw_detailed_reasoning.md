# AI Reasoning - Detailed Design

*Generated: 2026-03-23T18:14:29.890438*

---

## Phase Purpose
In this Detailed Software Design phase, I will define the exact C interfaces and configuration for the TC4xx eGTM ATOM unified PWM driver: function signatures, module structs, constants, pin/channel/clock bindings, init/start/stop/update sequences, and unit-test specs. This is the blueprint the generator will convert directly into compilable code, ensuring the TC387 TOM design is faithfully migrated to TC4D7 eGTM ATOM with the required PWM behavior.

## Data Inventory
- user_requirement (from user): TC387 TOM → TC4D7 eGTM ATOM migration on KIT_A3G_TC4D7_LITE; 20 kHz center-aligned, complementary, 1 µs dead-time; HS active-high, LS active-low; push-pull; AGC sync start; ATOM on FXCLK0, DTM on CMU CLK0. I’ll anchor timing/polarity and startup semantics on this.
- target_board (from user): KIT_A3G_TC4D7_LITE. I’ll align device/package and PinMap to this board.
- requirements_json (from refiner): Driver IfxEgtm_Pwm; pins: PHASE_U_HS=P20.8, U_LS=P20.9, V_HS=P20.10, V_LS=P20.11, W_* …; device DEVICE_TC4D7, package IFX_PIN_PACKAGE_BGA292_COM; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/h. I’ll treat this as the contract and file layout.
- hardware_json (from hardware): eGTM0.ATOM0 channels [0..5]; clock 25 MHz xtal, 500 MHz system; pin_assignments present but macros unresolved; validation shows device/package mismatch. I’ll use the ATOM instance/channels and resolve pin macros; I’ll flag the mismatch.
- sw_architecture_json (from sw-arch): Driver selection IfxEgtm_Pwm; functions initEgtmAtom3phInv and runtime update; required includes (Ifx_Types.h, IfxEgtm_Pwm.h, IfxPort.h, IfxPort_PinMap.h, IfxCpu.h, IfxScuWdt.h, Bsp.h). I’ll base function names and include set on this.
- template_capabilities (from template analyzer): Template KIT_A3G_TC4D7_LITE but device_macro DEVICE_TC4DX, package BGA436; clock: 25 MHz xtal, 500 MHz syspll. I’ll derive clock constants, but note the device/package discrepancy to fix.
- reference_analysis (from reference project): Prior TOM usage, macros (NUM_OF_CHANNELS=3, PWM_FREQUENCY=20000), watchdog/interrupt patterns. I’ll mirror coding style and init patterns.
- header_documentation_context (from header selector): IfxEgtm_Pwm API usage. I’ll extract correct struct/field names and options (center-aligned, complementary, AGC sync).
- extracted_config_values (from library analyzer): Fields like deadTime.rising/falling, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, polarity enums, example pin macros IfxEgtm_ATOM0_x_TOUTx_P.._OUT. I’ll map timing/polarity and DTM source from here.

## Execution Plan
1) Define module types and globals
   - Create EgtmAtom3phInv_Config and EgtmAtom3phInv_Handle wrapping IfxEgtm_Pwm_Config, IfxEgtm_Pwm, and six output descriptors; reason: encapsulate all eGTM/ATOM details behind a single handle.

2) Pin/channel mapping
   - Map EGTM0.ATOM0 channels [0/1]=U (HS/LS), [2/3]=V, [4/5]=W using hardware_json.channels and requirements_json.signal_requirements.
   - Resolve TOUT macros for P20.8/9/10/11/… via IfxEgtm_PinMap.h; assign to config.output[i].pin.
   
3) Clock and timing
   - Set PWM frequency to 20 kHz center-aligned; compute period ticks from CMU FXCLK0 (from template clock; if FXCLK0 not explicit, derive from default GTM CMU settings or add design constant).
   - Set DTM source to IfxGtm_Dtm_ClockSource_cmuClock0; convert 1.0 µs to deadTime.rising/falling ticks.

4) Polarity and complementary mode
   - Configure HS polarity Ifx_ActiveState_high and LS complementary Ifx_ActiveState_low; enable complementary pair generation and push-pull outputs per header_documentation_context.

5) AGC synchronized start
   - Enable AGC sync start (ATOM0 AGC0) so all three phases start simultaneously.

6) Initialization sequence
   - Provide initEgtmAtom3phInv(void): disable WDT, enable GTM/eGTM, IfxEgtm_Pwm_initConfig, fill pins/channels/clock/polarity/dead-time, IfxEgtm_Pwm_init, preload 50% duty, arm AGC without starting outputs.

7) Control APIs
   - startEgtmAtom3phInv(void), stopEgtmAtom3phInv(void).
   - setDuty3ph(float du, float dv, float dw): range-check and update duty for three complementary pairs atomically.

8) Unit-test specs
   - Validate period ticks for 20 kHz, dead-time ticks = 1 µs at DTM clock, polarity assignment (HS high/LS low), six pins routed, AGC starts all edges in same eGTM time-base tick, duty saturation handling.

9) File structure and includes
   - Generate EGTM_ATOM_3_Phase_Inverter_PWM.c/h with includes: Ifx_Types.h, IfxEgtm_Pwm.h, IfxEgtm_PinMap.h, IfxPort.h, IfxPort_PinMap.h, IfxCpu.h, IfxScuWdt.h, Bsp.h.

## Key Decisions & Risks
- Pin macro resolution: Need exact IfxEgtm_ATOM0_x_TOUTx_P20.y_OUT symbols for P20.8–P20.13; hardware_json shows None; I will fetch from PinMap; risk of unavailable TOUT on those pins.
- Device/package mismatch: requirements_json says DEVICE_TC4D7/BGA292; template has DEVICE_TC4DX/BGA436. I will target DEVICE_TC4D7 and flag template corrections.
- FXCLK0 value: Not explicitly given; I’ll parameterize and compute ticks; risk if CMU FXCLK0 differs from assumed default.
- AGC group: I will use ATOM0 AGC0 for channels 0–5; confirm no cross-AGC pairing needed.
- Complementary polarity: Confirm push-pull mapping matches external gate driver expectations; invert if board-level inverters exist (not indicated).