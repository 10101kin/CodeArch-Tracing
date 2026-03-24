# AI Reasoning - Software Architecture

*Generated: 2026-03-24T02:03:16.615188*

---

## Phase Purpose
In Software Architecture, I define the high-level module layout, driver abstractions, include tree, ISR wiring, and the init/update sequence for the migrated PWM. The goal is a TC4D7 eGTM ATOM-based, center‑aligned complementary 3‑phase inverter PWM with hardware deadtime, synchronized start/updates, and a single period IRQ that toggles LED P13.0.

## Data Inventory
- user_requirement (from the user)
  - Content: TC387 TOM → TC4D7 eGTM Cluster1 ATOM0 CH0/1/2, 20 kHz center‑aligned complementary PWM with 1 µs deadtime via DTM/CDTM; pins PHASE_U/V/W mapped to P20.8/9/10/11/12/13; IRQ on CPU0 prio 20 toggling P13.0; float32[3] duty API using IfxEgtm_Pwm.
  - Use: Source of truth for behavior, pin intent, timing, API expectations.

- target_board (from platform selection)
  - Content: KIT_A3G_TC4D7_LITE.
  - Use: Constrains pin map and available eGTM instances/clocking.

- requirements_json (from refiner node)
  - Content: PWM on EGTM.ATOM0 channels [0,1,2], complementary via CDTM, 20 kHz, 1.0 µs, synchronous start/updates, files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h, includes IfxEgtm_Pwm.h.
  - Use: Concrete driver choice (IfxEgtm_Pwm), file names, and alignment/deadtime semantics.

- hardware_json (from hardware node)
  - Content: MCU TC4D7 verified; peripheral EGTM.ATOM0 CH[0..2]; example TOUT pins P02.0/P02.1/...; clocks 25 MHz XTAL, 500 MHz system; validation flags show reference_pins_valid=False.
  - Use: Confirms ATOM0 availability/clocks; highlights a pin map mismatch vs requested P20.x to resolve.

- template_capabilities (from template analyzer)
  - Content: TC4xx template path; interrupt config present; iLLD set includes IfxEgtm_Pwm support.
  - Use: Guides include hierarchy and startup glue.

- reference_analysis (from reference project)
  - Content: Legacy TOM example included IfxGtm_Pwm.h; patterns like initGtmTom3phInv(), update duty API, watchdog disable, CPU0 main ISR style.
  - Use: Inform naming and include migration, but replace TOM with eGTM ATOM.

- header_documentation_context (iLLD API docs)
  - Content: IfxEgtm_Pwm usage, complementary pairs, deadtime config, clock sources, ISR hooks.
  - Use: Field/ISR API mapping for architecture.

- extracted_config_values (from headers)
  - Content: deadTime.rising/falling, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, polarity fields, example pin macros IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT.
  - Use: Structure field names and sensible defaults; clock source for DTM/CDTM.

- detected_peripheral_type
  - Content: PWM.
  - Use: Confirms path.

## Execution Plan
1. Choose driver and files
   - Use IfxEgtm_Pwm with files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h per requirements_json.

2. Define module API and data
   - Functions: initEgtmAtom3phInv(), updateEgtmAtom3phInvDutyF32(float32 duty[3]).
   - Objects: IfxEgtm_Pwm handle, three channel descriptors, status/error flags.

3. Clock and module enable
   - Enable eGTM, GCLK, and FXCLK0; set FXCLK0 to meet 20 kHz and 1 µs CDTM resolution (target ≥ 50–100 MHz). Use template_capabilities clock config; select dtmClockSource=cmuClock0.

4. ATOM timing config
   - Center-aligned mode with up/down counting; compute period ticks = FXCLK0/(2*20 kHz). Configure synchronized start for channels 0/1/2 via AGC/Shadow transfer.

5. Complementary and deadtime
   - Configure CDTM to generate LS from HS with deadTime.rising=deadTime.falling=1.0e-6; set complementaryPolarity and fastShutDown per header docs.

6. Pin mapping
   - Map outputs to requested P20.8/9/10/11/12/13 (PHASE_U/V/W HS/LS). IfxEgtm_ATOM0_x_TOUTy_P20_z_OUT macros must be resolved; fallback to hardware_json P02.x only if P20.x unavailable. Configure pad drivers.

7. Interrupt
   - Route a single period event from one ATOM channel to an IRQ on CPU0 priority 20; define ISR IFX_INTERRUPT(EgtmAtomPeriodIsr, 0, 20) toggling LED P13.0 via IfxPort.

8. Include hierarchy
   - In .c/.h include: IfxEgtm_Pwm.h, IfxPort.h, IfxPort_PinMap.h, IfxCpu_Irq.h; expose header with API prototypes and status flags.

9. Duty update path
   - Implement atomic, synchronized shadow updates of all three duties; clamp 0.0f–0.95f; trigger AGC update at safe point.

## Key Decisions & Risks
- Pin map conflict: user wants P20.8–P20.13; hardware_json proposes P02.x; I must verify TOUT availability on P20.x for EGTM1.ATOM0 and adjust routing or document fallback.
- Cluster/instance clarity: requirement says Cluster 1; hardware lists EGTM.ATOM0 (cluster unspecified). I must ensure ATOM0 is on Cluster 1 (eGTM1) or rebind per device datasheet.
- FXCLK0 rate: deadtime resolution and 20 kHz center-aligned period depend on FXCLK0; I’ll calculate ticks and validate CDTM granularity.
- IRQ source: Choose consistent period event (e.g., ATOM CH0 CM0/period) and ensure single IRQ per cycle.
- Driver API diffs: Replace TC3xx IfxGtm_Pwm patterns with TC4xx IfxEgtm_Pwm equivalents; confirm shadow-transfer semantics for sync updates.