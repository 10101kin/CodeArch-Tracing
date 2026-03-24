# AI Reasoning - Hardware Design

*Generated: 2026-03-24T05:02:29.371420*

---

## Phase Purpose
In Hardware Design, I will translate the migration requirements into a concrete TC4xx eGTM resource plan: pins, ATOM channels, DTM pairing, CMU clock roots, and interrupt routing for KIT_A3G_TC4D7_LITE. This produces a hardware design JSON the next phases will use to auto-generate driver code and ensure the TC3xx TOM PWM is correctly realized on TC4xx eGTM ATOM.

## Data Inventory
- user_requirement (author input)
  - From: User.
  - Contains: TC387→TC4D7 migration to IfxEgtm_Pwm; Cluster_1 ATOM0 CH0/1/2 at 20 kHz, center-aligned, complementary via DTM, 1 µs dead-time, syncStart/syncUpdate; CPU0 ISR prio 20 toggling LED P13.0; pin map P20.8/9/10/11/12/13; ATOM clock=FXCLK_0, DTM clock=CLK0.
  - Use: Primary source for channels, timing, complementary scheme, clocks, pin map, and ISR intent.

- target_board
  - From: Board selector.
  - Contains: KIT_A3G_TC4D7_LITE.
  - Use: Constrain pin/package availability and default clocks.

- requirements_json (refiner output)
  - From: Previous phase.
  - Contains: Structured fields: device_macro=DEVICE_TC4DX, device_family=TC4xx, eg. eGTM Cluster_1/ATOM0/ch[0..2], 20 kHz, DTM enabled, rising/falling deadtime 1.0 µs, interrupt on CPU0 prio 20, files EGTM_ATOM_3_Phase_Inverter_PWM.c/.h.
  - Use: Canonical fields for the JSON (names, enums, flags).

- template_capabilities (template analyzer)
  - From: Template library.
  - Contains: mcu_family=TC4xx, device_macro=DEVICE_TC4DX, pin_package=IFX_PIN_PACKAGE_BGA436_COM, clock_config (xtal 25 MHz, syspll 500 MHz), available iLLDs.
  - Use: Validate driver availability (IfxEgtm_Pwm), resolve CMU sources and interrupt provider; reconcile package mismatch.

- reference_analysis (reference project analyzer)
  - From: TC3xx project parse.
  - Contains: Patterns for PWM init, watchdog handling, GPIO usage; confirms complementary PWM via TOM and period-event ISR.
  - Use: Guide how to set up period event and sync behavior on eGTM.

- header_documentation_context (iLLD docs)
  - From: Intelligent header selector.
  - Contains: IfxEgtm_Pwm API usage and struct fields.
  - Use: Correct enums/struct names (e.g., dtmClockSource, output[].pin/complementaryPin, deadTime.rising/falling).

- extracted_config_values (header parser)
  - From: iLLD headers.
  - Contains: Example fields: deadTime.rising/falling, output[i].pin/complementaryPin, enum dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0.
  - Use: Exact field paths and enum values for the JSON.

- pin_mapping_from_docs (pin validator)
  - From: Docs/db.
  - Contains: Sample macros like IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT and complementary IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT.
  - Use: Validate and synthesize correct macros for requested P20.x pins (or flag mismatch).

- detected_peripheral_type
  - From: Detector.
  - Contains: PWM.
  - Use: Confirms peripheral scope.

## Execution Plan
1. Resolve device/board context
   - Set device_macro=DEVICE_TC4DX, family=TC4xx, board=KIT_A3G_TC4D7_LITE; note package mismatch (requirements_json BGA292 vs template BGA436) and select BGA436_COM from template for pin validation.

2. Configure eGTM CMU clocks
   - Enable FXCLK_0 for ATOM and CLK0 for DTM.
   - Set dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0; capture/derive FXCLK_0 frequency from board template to compute 20 kHz center-aligned period ticks.

3. Define PWM topology
   - Cluster=Cluster_1, ATOM=ATOM0, primary channels=[0,1,2].
   - Mode=center-aligned; complementary via DTM with deadTime.rising=1e-6, deadTime.falling=1e-6.
   - Enable syncStart and syncUpdate; configure independent duty updates (per-channel shadow registers).

4. Map pins to channels
   - Assign PHASE_U: CH0 → HS=P20.8, LS=P20.9; PHASE_V: CH1 → HS=P20.10, LS=P20.11; PHASE_W: CH2 → HS=P20.12, LS=P20.13.
   - Look up corresponding IfxEgtm_ATOM0_{ch}_TOUTx_P20_y_OUT and complementary macros; validate availability; bind to output[i].pin and output[i].complementaryPin.

5. Configure DTM pairing
   - Pair each primary channel with its complementary output through DTM; ensure polarity and complementaryPolarity align with high-side/low-side conventions.

6. Set AGC and synchronization
   - Configure ATOM0 AGC in Cluster_1 for synchronized start across CH0–CH2 and synchronized shadow transfers on update trigger.

7. Interrupt routing
   - Route ATOM period event (center-aligned period match) to SR0 on Cluster_1/ATOM0; map to CPU0 with priority 20.
   - Document LED P13.0 as GPIO output to be toggled in ISR.

8. Emit hardware design JSON
   - Include device, clocks, ATOM/DTM config, channel-pin map, ISR line/provider, and driver=IfxEgtm_Pwm.

## Key Decisions & Risks
- Pin macros for P20.8–P20.13: Current doc sample shows P02.x; P20.x availability for ATOM0 must be verified. Risk: pins not routable from Cluster_1/ATOM0; mitigation: alternative TOUT pins or different ATOM submodule.
- Package mismatch: requirements_json says BGA292_COM; template is BGA436_COM. Risk: pin indices differ; I will lock to BGA436_COM for KIT_A3G_TC4D7_LITE and flag if any requested pin is invalid.
- FXCLK_0 frequency unknown in snapshot: I will compute ticks using runtime API or template defaults; wrong assumption could shift PWM frequency.
- Interrupt source selection: Exact ATOM SR line and period-event source differ by device; I will choose SR0 for AGC period and note to verify in iLLD for TC4D7.
- DTM resource coupling: Ensure DTM channels are free and correctly tied to ATOM0 CH0–CH2; else, adjust mapping.