# AI Reasoning - Hardware Design

*Generated: 2026-03-24T01:53:15.751978*

---

## Phase Purpose
In Hardware Design, I will translate the high‑level PWM requirements into a concrete, device‑accurate mapping: eGTM clocks, ATOM channels, CDTM/DTM deadtime, pin assignments, and one IRQ. This JSON becomes the single source of truth the code generator uses to emit iLLD configuration and board‑specific init code for KIT_A3G_TC4D7_LITE.

## Data Inventory
- user_requirement (from you)
  - What: TC387→TC4D7 migration; eGTM Cluster 1, ATOM0 CH0/1/2; 20 kHz center‑aligned complementary PWM, 1 µs deadtime via DTM/CDTM; sync start/updates; period IRQ on CPU0 prio 20 toggling LED P13.0; pin map PHASE_U/V/W on P20.8/9/10/11/12/13; float32[3] duty; IfxEgtm_Pwm or unified TC4D7 PWM API.
  - Use: Core requirements and pin targets. Drives channel, timing, deadtime, sync, interrupt, and LED configuration.

- target_board (from project selection)
  - What: KIT_A3G_TC4D7_LITE.
  - Use: Select correct device macro, clock tree, and pin package for validation.

- requirements_json (from refiner)
  - What: Structured constraints: device_family TC4xx, target_device_macro DEVICE_TC4DX, eGTM Cluster=1, ATOM=0, channels_used=[0,1,2], complementary=CDTM, pwm_frequency_hz=20000, deadtime_us=1.0, sync start/updates, GCLK/FXCLK0 enable.
  - Use: Machine‑readable parameters for JSON fields and consistency checks.

- template_capabilities (from template library analyzer)
  - What: Template path/name, TC4xx family, device_macro DEVICE_TC4DX, clock_config (xtal 25 MHz, SYSPLL 500 MHz, PERPLL1 16 MHz), pin_package IFX_PIN_PACKAGE_BGA436_COM, available iLLD.
  - Use: Clock source selection, interrupt provider, verify IfxEgtm_Pwm availability, and detect pin‑package mismatch risk.

- reference_analysis (from reference project analyzer)
  - What: Prior TC3xx TOM 3‑phase patterns, includes (IfxGtm_Pwm.h, IfxPort.h), init/update function names, watchdog/CPU init idioms.
  - Use: Preserve naming patterns and ISR toggling style; map TOM→ATOM while keeping similar API flow.

- header_documentation_context (from intelligent header selector)
  - What: IfxEgtm_Pwm API usage, struct fields (deadTime.rising/falling, polarity, dtmClockSource, sync update).
  - Use: Accurate driver field names and valid enum values for JSON output.

- extracted_config_values (from library file analyzer)
  - What: Defaults like dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0; example config.frequency=500000.0f; polarity enums; example pin macros.
  - Use: Seed values and enums; override frequencies to meet 1 µs resolution and 20 kHz.

- pin_mapping_from_docs (from pin validator)
  - What: Example ATOM0_0/0N on P02.0/P02.1 pin macros.
  - Use: Validate ATOM0 channel→pin feasibility; highlight that P20.x mapping must be verified.

- detected_peripheral_type
  - What: PWM.
  - Use: Confirms the path and template selection.

## Execution Plan
1. Clock plan for eGTM
   - Enable GCLK and set CMU FXCLK0 to a high rate (target 100 MHz) derived from SYSPLL 500 MHz to get fine resolution.
   - Reason: Up‑down at 20 kHz needs periodTicks = FXCLK0/(2*20 kHz) ≈ 2500 ticks; deadtime 1 µs = 100 ticks in DTM at 100 MHz.

2. Select eGTM resources
   - Cluster=1, ATOM0, Channels CH0/CH1/CH2 as phase carriers; use complementary outputs via CDTM (hardware deadtime).
   - Reason: Meets requirement and ensures synchronous start via common AGC.

3. Configure PWM mode
   - Center‑aligned (up/down), complementary enabled, deadTime.rising=deadTime.falling=1e-6, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, synchronousStart=true, synchronizedDutyUpdate=true.
   - Reason: Matches timing and update semantics.

4. Assign pins
   - Map: PHASE_U HS=P20.8, LS=P20.9; PHASE_V HS=P20.10, LS=P20.11; PHASE_W HS=P20.12, LS=P20.13 to ATOM0 CH0/0N, CH1/1N, CH2/2N if supported.
   - Validate against IfxEgtm_PinMap macros for DEVICE_TC4DX + KIT_A3G_TC4D7_LITE; if unavailable, search alternate TOUT functions for those pins or propose nearest valid ATOM0 routes and flag deviation.

5. Interrupt
   - Configure single period event on one channel (e.g., CH0 SR0) routed to CPU0 with priority 20; ISR toggles LED P13.0 (configure as push‑pull output).
   - Reason: Minimal ISR load with global cadence.

6. Duty interface and status
   - Define float32 duty[3] for U/V/W (0..1), clamp/saturate; expose basic error/status flags (clock ready, pin map valid, sync ok).

7. Emit hardware design JSON
   - Fill device, clocks, eGTM cluster/ATOM/channels, CDTM, pins, interrupt, and API driver=IfxEgtm_Pwm.

## Key Decisions & Risks
- Pin feasibility: Need confirmation that P20.8–P20.13 support eGTM ATOM0 CH0/1/2 and their N outputs on TC4D7. If not, we must either change channels or pins. Risk: pin map from docs shows P02.x examples only.
- Pin package mismatch: Requirements say IFX_PIN_PACKAGE_BGA292_COM; template shows IFX_PIN_PACKAGE_BGA436_COM. This may invalidate the requested P20.x pins.
- FXCLK0 rate: Device limits may cap FXCLK0; if <100 MHz, recompute period/deadtime ticks to preserve 1 µs and 20 kHz.
- IRQ source selection: Use CH0 period SR0 vs AGC‑level period interrupt—confirm driver support on TC4D7.