# AI Reasoning - Detailed Design

*Generated: 2026-03-24T09:11:23.935405*

---

## Phase Purpose
In this Detailed Software Design phase, I will produce the exact, implementation-ready design: function signatures, structs, constants, pin maps, initialization sequences, and unit-test specs. This is the blueprint the generator will turn into code, and it locks down ambiguous choices (TOM instance, channels, pins, clock) while validating feasibility against the device/package.

## Data Inventory
- user_requirement (user): Pin move to P02.x with U/V/W pairs, TOM1 time base, 20 kHz, 0.5 µs dead-time, 1.0 µs min pulse, cmuFxclk0, pushPull + cmosAutomotiveSpeed1, initial duties 25/50/75, use IfxGtm_Tom_PwmHl; replace PHASE_* macros; verify TOM1/TOUT mapping for TC387 LFBGA-292; if P02 not on TOM1, TOM instance TBD. Use: primary design targets and acceptance criteria.
- target_board (phase 0): KIT_A2G_TC387_5V_TFT. Use: pick correct PinMap header/package and BSP clock assumptions.
- requirements_json (phase 1): Confirms driver IfxGtm_Tom_PwmHl, frequency_hz=20000, deadtime_us=0.5, min_pulse_us=1.0, cmuFxclk0, device_macro=DEVICE_TC38X, pin_package=IFX_PIN_PACKAGE_292. Use: encode constants and device guards in JSON.
- hardware_json (phase 2): Suggests TOM0 with channels [0,1,2,4,5,7] and P02 mapping macros e.g., U_HS=P02.0 → `IfxGtm_TOM0_0_TOUT0_P02_0_OUT`, U_LS=P02.7 → `IfxGtm_TOM0_7_TOUT7_P02_7_OUT`; validation flags package conflict (board template 516 vs requested 292). Use: provisional TOM instance/channel and exact PinMap symbols; flag conflicts.
- sw_architecture_json (phase 3): Names and call graph: `GTM_TOM_3_Phase_Inverter_PWM_init`, `IfxGtm_Tom_Timer_initConfig`, `IfxGtm_Tom_PwmHl_initConfig`, etc. Use: finalize API surface and init ordering.
- template_capabilities (template analyzer): Available PinMap headers include `IfxGtm_PinMap_TC38x_LFBGA292.h` and 516; device_macro DEVICE_TC38X; clocks xtal=20 MHz, pll=300 MHz. Use: include selection and clock derivations.
- reference_analysis (reference project): Uses IfxGtm_Tom_PwmHl; macro `PWM_FREQ_HZ 20000`; pattern of timer+pwm handle. Use: align naming and struct layout for minimal churn.
- header_documentation_context (header selector): PinMap overview. Use: confirm include and symbol style.
- extracted_config_values (library analyzer): Fields like `deadTime.rising/falling`, `output[i].pin`, example PinMap symbol style. Use: precise struct field names for JSON.
- detected_peripheral_type: PWM. Use: sanity check.

## Execution Plan
1) Pin/TOM verification
   - Cross-check PinMap for P02.x on TC387 LFBGA-292. If TOM1 symbols exist for all six pins, set `tomInstance=TOM1`. Else set `tomInstance=TOM0` with channels: U(HS/LS)=(ch0/ch7), V=(ch1/ch4), W=(ch2/ch5); set DECISION_REQUIRED tag.
2) Define structs
   - Handle: `Gtm3Ph_Pwm { IfxGtm_Tom_Timer timer; IfxGtm_Tom_PwmHl pwm; float duty[3]; }`
   - Config: `Gtm3Ph_Pwm_Config { IfxGtm_Tom tom; IfxGtm_Tom_Ch timerChannel /*=ch0*/; IfxGtm_Tom_Ch pair[3][2]; const IfxGtm_Tom_ToutMap *pin[3][2]; IfxGtm_Tom_Ch_ClkSrc clk /*cmuFxclk0*/; float freqHz; float deadtimeUs; float minPulseUs; IfxPort_OutputMode outputMode; IfxPort_PadDriver padDriver; }`
3) Pin mapping
   - Replace PHASE_* macros with PinMap pointers:
     - U: `IfxGtm_TOM0_0_TOUT0_P02_0_OUT`, `IfxGtm_TOM0_7_TOUT7_P02_7_OUT`
     - V: `IfxGtm_TOM0_1_TOUT1_P02_1_OUT`, `IfxGtm_TOM0_4_TOUT4_P02_4_OUT`
     - W: `IfxGtm_TOM0_2_TOUT2_P02_2_OUT`, `IfxGtm_TOM0_5_TOUT5_P02_5_OUT`
     - If TOM1 variant verified, swap symbols accordingly.
4) Clock and ticks
   - Use `IfxGtm_Cmu_getFxClkFrequency(cmuFxclk0)` to compute ticksPerUs; derive periodTicks for 20 kHz center-aligned, deadtimeTicks for 0.5 µs, minPulseTicks for 1.0 µs; store in config.
5) Initialization sequence
   - Enable GTM/FXCLK0; `IfxGtm_Tom_Timer_initConfig` with `tomInstance`, `timerChannel=ch0`, `clock=cmuFxclk0`, `frequency=20kHz`, center-aligned.
   - `IfxGtm_Tom_PwmHl_initConfig`: assign three channel pairs and `output[i].pin` from PinMap, set `deadTime.{rising,falling}=0.5µs`, `minPulse=1.0µs`, `outputMode=pushPull`, `padDriver=cmosAutomotiveSpeed1`.
   - `..._init`, `..._setMode(centerAligned)`, set initial duties [0.25, 0.50, 0.75], `Timer_run`.
6) API surface
   - `GTM_TOM_3_Phase_Inverter_PWM_init(const Gtm3Ph_Pwm_Config*, Gtm3Ph_Pwm*)`
   - `..._setDuty(float u, float v, float w)`, `..._start()`, `..._stop()`, `..._setDeadtime(float us)`.
7) Unit tests
   - Verify PinMap symbols resolve for selected package.
   - Measure effective frequency ±1%.
   - Enforce min pulse and deadtime in ticks.
   - Complementary non-overlap across all pairs.
   - Duty initialization 25/50/75% applied correctly.

## Key Decisions & Risks
- TOM instance: User prefers TOM1; hardware_json indicates P02.* are on TOM0 (channels 0/1/2/4/5/7). Decision pending user confirmation; design will carry both mappings with a default to TOM0.
- Package mismatch: Board template shows IFX_PIN_PACKAGE_516, while requirement asks to verify LFBGA-292; wrong header selection will break PinMap symbols.
- Non-consecutive channel pairing (0–7, 1–4, 2–5): Supported by PwmHl, but must remain within the same TGC (TOM0 TGC0). I will validate TGC grouping.
- CMU FXCLK0 actual frequency: if not configured, tick calculations may be off; I will compute from runtime `IfxGtm_Cmu_getFxClkFrequency` and assert constraints.