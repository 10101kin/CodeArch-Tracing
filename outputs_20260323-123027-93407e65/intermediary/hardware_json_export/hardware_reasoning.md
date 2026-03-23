# AI Reasoning - Hardware Design

*Generated: 2026-03-23T12:39:12.714812*

---

## Phase Purpose
In Hardware Design, I will translate the migration requirements into a concrete TC4D7 resource map: eGTM clocking, ATOM channel usage, complementary PWM pairing, deadtime setup, trigger routing, and pin selections for the KIT_A3G_TC4D7_LITE board. This output becomes the single source of truth that subsequent code generation uses to instantiate `IfxEgtm_Pwm` and related drivers correctly and safely.

## Data Inventory
- user_requirement (from user): TC387 TOM → TC4D7 eGTM/ATOM migration; Cluster 0, ATOM0 CH0–CH2 as complementary center‑aligned PWM at 20 kHz with 1 µs deadtime; HS active‑high / LS active‑low on P20.8/9, P20.10/11, P20.12/13; ATOM0 CH3 as 50% edge‑aligned trigger. I will anchor channel allocation, alignment, deadtime, and desired pins to this.
- target_board (from user): KIT_A3G_TC4D7_LITE. I’ll ensure pin/package/clocking align with this BSP.
- requirements_json (from refiner): Structured spec with `driver_name: IfxEgtm_Pwm`, `egtm.cluster: 0`, `atom: 0`, `pwm_channels: [0,1,2]`, `trigger_channel: 3`, `pwm_frequency_hz: 20000`, `deadtime_us: 1`, `target_device_macro: DEVICE_TC4D7`, `pin_package: IFX_PIN_PACKAGE_BGA292_COM`. I’ll use these as authoritative config values.
- template_capabilities (from template_library_analyzer): Template path/name for KIT_A3G_TC4D7_LITE, `device_macro: DEVICE_TC4DX`, clocks (`xtal: 25 MHz`, `syspll: 500 MHz`, `ppupll: 450 MHz`, `perpll1: 16 MHz`), pin package `IFX_PIN_PACKAGE_BGA436_COM`. I’ll base GTM CMU clock planning on these, and flag macro/pin‑package mismatches.
- reference_analysis (from reference_project_analyzer): TC3xx TOM reference patterns and includes (e.g., `IfxGtm_Pwm.h`, `IfxGtm_Trig.h`). I’ll use it to infer migration parallels (TOM→ATOM, `IfxGtm_Pwm`→`IfxEgtm_Pwm`) and trigger routing expectations.
- header_documentation_context (from intelligent_header_selector): `IfxEgtm_Pwm` API docs. I’ll map fields like `deadTime.rising/falling`, `output[i].pin/complementaryPin`, `polarity`, `complementaryPolarity`, `config.dtmClockSource`, `config.frequency`.
- extracted_config_values (from library_file_analyzer): Parsed struct defaults/enums such as `dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0`, `polarity = Ifx_ActiveState_high`, `complementaryPolarity = Ifx_ActiveState_low`. I’ll override values with our targets (20 kHz, 1 µs) and reuse enums.
- pin_mapping_from_docs (from pin_validator): Valid ATOM0 example macros like `IfxEgtm_ATOM0_0_TOUT0_P02_0_OUT` and complementary `_N_TOUT1_P02_1_OUT`. I’ll use this to validate availability of requested P20.* pins or pick closest legal alternates.
- detected_peripheral_type (from detector): PWM. Confirms driver/peripheral path.

## Execution Plan
1. Clock plan: Choose `CMU_CLK0` derived from `ppupll` 450 MHz to a convenient 100 MHz (÷4 or ÷5) for clean timing; document resulting `CMU_CLK0`. Reason: 20 kHz center‑aligned → period ticks ≈ CMU_CLK0/(2·20k) = 2500 ticks at 100 MHz.
2. ATOM resource map: Reserve Cluster 0 / ATOM0 channels 0,1,2 as complementary pairs; channel 3 as single‑ended trigger. Reason: Matches `requirements_json.signal_requirements`.
3. PWM timing: Set `config.frequency = 20000` Hz; `alignment = center-aligned`. Apply deadtime `deadTime.rising = deadTime.falling = 1e-6` seconds with `dtmClockSource = IfxGtm_Dtm_ClockSource_cmuClock0`.
4. Polarity: Set `polarity = Ifx_ActiveState_high` (HS), `complementaryPolarity = Ifx_ActiveState_low` (LS) per user.
5. Trigger channel: Configure ATOM0 CH3 as edge‑aligned with 50% duty; route via `IfxGtm_Trig` to EVADC (group/channel TBD). Reason: aligns with reference includes and requirement.
6. Pin assignment attempt: Map CH0 pair → P20.8/P20.9; CH1 → P20.10/P20.11; CH2 → P20.12/P20.13 using `IfxEgtm_ATOM0_x_TOUTy_P20_*_OUT` macros if valid. Validate against pin validator; if unavailable, propose nearest legal TOUTs on KIT_A3G_TC4D7_LITE (e.g., P02.* examples).
7. Consistency checks: Reconcile `DEVICE_TC4D7` vs template `DEVICE_TC4DX`, and `BGA292_COM` vs `BGA436_COM`. Lock to board’s actual device/package for final pin legality.
8. Produce the hardware design JSON (internally) capturing clocks, ATOM allocations, channel modes, deadtime, polarities, and pin macros.

## Key Decisions & Risks
- Driver mapping: Use `IfxEgtm_Pwm` (TC4xx) instead of legacy `IfxGtm_Pwm` (TC3xx).
- Pin feasibility: P20.8–P20.13 routing for ATOM0 must be confirmed; current docs sample only show P02.*. Risk of non‑routable pins on KIT_A3G_TC4D7_LITE.
- Device/package mismatch: `DEVICE_TC4D7` + `IFX_PIN_PACKAGE_BGA292_COM` vs template `DEVICE_TC4DX` + `BGA436_COM`. Must resolve to ensure correct pin map.
- CMU clock source/dividers: Selecting 100 MHz assumption; if GTM is tied to a different source, recompute period/deadtime ticks.
- EVADC trigger line: Exact TRIG path (group/source) not specified; I will leave a TODO with a sensible default and await confirmation.