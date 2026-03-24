# AI Reasoning - Detailed Design

*Generated: 2026-03-24T02:12:04.500367*

---

## Phase Purpose
In this Detailed Software Design phase, I will produce a complete, code-ready design spec: exact function signatures, data structures, constants, initialization sequencing, interrupt wiring, and unit-test specs. This is the blueprint the generator will implement to realize the migrated 3‑phase eGTM ATOM PWM on TC4D7 while preserving timing, pinout intent, and API behavior from the TC3xx reference.

## Data Inventory
- user_requirement (from user): TC387→TC4D7 migration using eGTM Cluster 1 ATOM0 CH0/1/2, 20 kHz center‑aligned complementary PWM, 1 µs deadtime via DTM/CDTM, synchronous start/updates, period IRQ on CPU0 prio 20 toggling LED P13.0; pins PHASE_U/V/W HS/LS to P20.8/9/10/11/12/13; float32[3] duty and status flags, driver IfxEgtm_Pwm. I’ll anchor timing, channeling, ISR, and public API to these.
- target_board (from project setup): KIT_A3G_TC4D7_LITE. I’ll ensure pins/LED exist on this board and reference its BSP defaults.
- requirements_json (from refiner node): Confirms MIGRATION to TC4xx, driver IfxEgtm_Pwm, eGTM Cluster 1 ATOM0 CH[0..2], 20 kHz center, 1.0 µs deadtime, CPU0 IRQ prio 20; expected files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h. I’ll map these into struct fields and file layout.
- hardware_json (from hardware node): MCU TC4D7 (DEVICE_TC4DX), EGTM.ATOM0 CH[0..2], tentative pins like P02.0 TOUT0 macros; xtal 25 MHz, system 500 MHz; validation all_verified: False; reference_pins_valid: False. I’ll detect/resolve the P20.x vs P02.x pin conflict and clock derivations (GCLK/FXCLK0).
- sw_architecture_json (from sw_architecture): Selected driver IfxEgtm_Pwm; API calls IfxEgtm_Pwm_initConfig/init; function mapping initEgtmAtom3phInv, update, ISR linkage. I’ll conform to this naming and layering.
- template_capabilities (from template analyzer): TC4xx iLLD available; clock config 25 MHz xtal, 500 MHz SYSPLL; interrupt infra present. I’ll reference standard clock/interrupt helpers (IfxSrc, IfxPort).
- reference_analysis (from reference project analyzer): Prior TOM pattern, watchdog disable, macros NUM_OF_CHANNELS=3, ISR toggling. I’ll mirror coding style and public function patterns.
- header_documentation_context (from intelligent header): IfxEgtm_Pwm API/fields. I’ll pull exact types/enums for config (e.g., dtmClockSource, polarity).
- extracted_config_values (from library analyzer): Examples for deadTime.rising/falling, dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0, pin macros for ATOM TOUTs. I’ll set deadtime=1e-6, FXCLK source, and complementary polarity.
- detected_peripheral_type: PWM. Confirms scope.

## Execution Plan
1. Define module files and public API
   - Files: EGTM_ATOM_3_Phase_Inverter_PWM.c/.h per requirements_json.
   - Functions: initEgtmAtom3phInv(void), egtmAtom3phInv_updateDuty(const float32 dutyUVW[3]), egtmAtom3phInv_getStatus(void), egtmAtom3phInv_isr(void).
   - Rationale: Matches sw_architecture_json function_mapping and user float32[3] interface.

2. Choose clocks and time base
   - Use system 500 MHz; set FXCLK0 to 100 MHz for 10 ns resolution (deadtime=1 µs → 100 ticks).
   - Configure GCLK, FXCLK0 in IfxEgtm_Cmu_enableClocks; note exact register fields in JSON.

3. Channel grouping and synchronization
   - Use eGTM Cluster 1, ATOM0 channels 0/1/2 with a single TGC for shadow transfer.
   - Enable synchronous start and synchronized duty updates via TGC shadow transfer triggers.

4. Complementary and dead-time
   - Use CDTM for complementary HS/LS pairs with 1.0 µs rising/falling deadtime.
   - Configure complementaryPolarity and deadTime.{rising,falling}=1e-6 per extracted_config_values schema.

5. Pin mapping
   - Target pins from user_requirement: P20.8/9/10/11/12/13 with specific IfxEgtm_ATOM0_x_TOUTy_P20_z_OUT macros.
   - If hardware_json P02.x macros don’t match, flag and keep placeholders with required P20.x macros in the design.

6. Interrupt
   - Configure ATOM0 period/compare match to raise a single period IRQ routed to CPU0, priority 20.
   - ISR toggles LED P13.0 via IfxPort; add pin init (output, push‑pull, default low).

7. Data structures and constants
   - Define config struct (wrapping IfxEgtm_Pwm_Config) and handle struct (IfxEgtm_Pwm plus channel handles).
   - Constants: PWM_FREQ_HZ=20000, FXCLK0_HZ=100000000, DEADTIME_S=1e-6, IRQ_PRIO=20.
   - Status/error flags: pinMapValid, clockOk, syncOk, overDuty, initDone.

8. Unit-test specifications
   - Tests for: frequency ±1%, duty saturation/clamping [0..0.95] per leg, synchronous update (edge-aligned timestamps within 1 tick), deadtime tick count=100±1, ISR rate=20 kHz toggling P13.0, pin mux verification.

## Key Decisions & Risks
- Pin assignment conflict: user P20.8–P20.13 vs hardware_json P02.x; I must resolve correct TOUT macros for KIT_A3G_TC4D7_LITE or keep TODOs.
- Dead-time engine: Prefer CDTM on ATOM for paired outputs; fall back to DTM if CDTM not available on ATOM0 channels.
- Clock choice: FXCLK0=100 MHz balances resolution and jitter; verify CMU limits and shared use.
- IRQ source wiring: Confirm correct ATOM0 SR line and SRC group for period event on TC4D7; ensure single interrupt, not per channel.
- LED P13.0 availability: Validate on this board; if absent, substitute a verified LED pin and document.