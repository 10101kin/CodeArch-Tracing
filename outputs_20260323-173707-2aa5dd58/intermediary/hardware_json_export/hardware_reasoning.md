# AI Reasoning - Hardware Design

*Generated: 2026-03-23T17:48:06.339058*

---

## Phase Purpose
In Hardware Design, I will translate the migrated PWM requirements into concrete TC4xx eGTM/ATOM resources for KIT_A3G_TC4D7_LITE. The output is a hardware design spec (JSON in the next step) that locks down peripheral instances, channels, pins, clocks, and driver parameters so firmware generation is deterministic and buildable.

## Data Inventory
- user_requirement (from user input)
  - What: Migrate TC387 TOM 3-phase inverter to TC4D7 eGTM ATOM using IfxEgtm_Pwm, 20 kHz center-aligned, 1 µs dead-time, HS active-high / LS active-low, push-pull, AGC synced start, ATOM on CMU FXCLK0, DTM on CMU CLK0.
  - How I’ll use: As the ground truth for frequency, alignment, polarity, dead-time, and clock sources.

- target_board (from project selection)
  - What: KIT_A3G_TC4D7_LITE.
  - How I’ll use: Constrains available pins and device resources.

- requirements_json (from refiner)
  - What: Structured constraints: driver IfxEgtm_Pwm, device_macro DEVICE_TC4D7, pin_package IFX_PIN_PACKAGE_BGA292_COM, board pin intents (e.g., PHASE_U_HS=P20.8, PHASE_U_LS=P20.9, PHASE_V_HS=P20.10, PHASE_V_LS=P20.11, …), pwm_frequency_hz=20000, alignment=center-aligned, dead_time_us=1.0, cmu.atom_clock=FXCLK0, cmu.dtm_clock=CLK0.
  - How I’ll use: To populate driver config (polarity, deadTime, dtmClockSource) and pin mappings.

- template_capabilities (from template library)
  - What: TC4xx support set, device_macro DEVICE_TC4DX, pin_package IFX_PIN_PACKAGE_BGA436_COM, GTM/clock baselines.
  - How I’ll use: Validate driver availability and clocks; flag macro/pin-package mismatch versus requirements.

- reference_analysis (from TC3xx TOM project)
  - What: Prior TOM usage (GTM_TOM_3_Phase_Inverter_PWM.c), macros like PWM_FREQUENCY, complementary pairs, watchdog pattern.
  - How I’ll use: Migrate behaviorally equivalent timing/polarity to ATOM.

- header_documentation_context (IfxEgtm_Pwm API)
  - What: Field names and enums: `IfxEgtm_Pwm_Config`, `output[i].pin`, `output[i].complementaryPin`, `polarity`, `complementaryPolarity`, `deadTime.rising/falling`, `dtmClockSource`.
  - How I’ll use: Author correct config fields.

- extracted_config_values (from headers)
  - What: Examples: `dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0`, `polarity=Ifx_ActiveState_high`, `complementaryPolarity=Ifx_ActiveState_low`.
  - How I’ll use: Concrete enum values for clocks and polarity.

- pin_mapping_from_docs (validator)
  - What: Valid eGTM ATOM macros like `IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT` and `IfxEgtm_ATOM0_0N_TOUT1_P02_1_OUT`.
  - How I’ll use: Cross-check feasibility of requested P20.x mappings; propose fallbacks if needed.

- detected_peripheral_type
  - What: PWM.
  - How I’ll use: Confirms path.

## Execution Plan
1) Select eGTM/ATOM resource
   - Choose `ATOM0` channels 0–5 as three complementary pairs: (0/1)=Phase U, (2/3)=Phase V, (4/5)=Phase W, to share one AGC for sync start.

2) Clock plan
   - Set ATOM clock source to `CMU.FXCLK0` and DTM to `CMU.CLK0` (`dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0`).
   - Compute period ticks for 20 kHz center-aligned: period = FXCLK0 / (2*20k); compute dead-time ticks = CLK0 * 1 µs.

3) Polarity and dead-time
   - Configure `polarity=Ifx_ActiveState_high` for HS and `complementaryPolarity=Ifx_ActiveState_low` for LS.
   - Set `deadTime.rising` and `deadTime.falling` to 1 µs in ticks; enable complementary mode with DTM insertion.

4) Pin assignment
   - Attempt to map:
     - U: HS → P20.8, LS → P20.9
     - V: HS → P20.10, LS → P20.11
     - W: HS → P20.12, LS → P20.13 (if present)
   - Validate each against eGTM ATOM TOUT routing; if invalid, propose closest valid macros (e.g., `IfxEgtm_ATOM0_0_TOUTx_P02_y_OUT`) and document deviation.

5) Port mode
   - Set GPIO to push-pull outputs with appropriate pad driver.

6) AGC and synchronization
   - Group ATOM0 ch0–5 under one AGC; configure synchronized start and shadow transfer strategy for center-aligned updates.

7) Emit hardware design spec
   - Record device macro, clocks, ATOM instance/channels, pins/macros, polarities, dead-time ticks, AGC sync flags.

## Key Decisions & Risks
- Device macro/pin-package mismatch: requirements say `DEVICE_TC4D7` + `BGA292_COM`, template shows `DEVICE_TC4DX` + `BGA436_COM`. I’ll align to TC4D7; risk: template headers may differ.
- Pin feasibility: Requested P20.x must be verified for ATOM TOUT on KIT_A3G_TC4D7_LITE. If not routable, I’ll suggest nearest valid `IfxEgtm_ATOMx_y_TOUTz_P.._OUT` options.
- Clock actual frequencies: FXCLK0/CLK0 values aren’t explicit; dead-time/period ticks depend on them. I’ll compute using board’s CMU defaults or leave as symbolic if unresolved.
- ATOM resource conflicts: Ensure ATOM0 ch0–5 are free on this board; otherwise, pick ATOM1 equivalent and update pins accordingly.