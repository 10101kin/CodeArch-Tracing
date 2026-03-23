# AI Reasoning - Detailed Design

*Generated: 2026-03-23T16:43:33.709752*

---

## Phase Purpose
In Detailed Software Design, I will turn the validated requirements and hardware mapping into a precise blueprint for code generation: exact function prototypes, context structs, configuration constants, initialization sequences, and unit-test specs. This matters because it removes ambiguity (drivers, pins, timer base, update semantics) and locks down the iLLD API usage for the TC387 so the generator can emit correct, buildable code.

## Data Inventory
- user_requirement (from user): Six single-output IfxGtm_Tom_Pwm channels on a shared IfxGtm_Tom_Timer at 20 kHz, center-aligned; pins PHASE_U: P02.0/P02.7, PHASE_V: P02.1/P02.4, PHASE_W: P02.2/P02.5; initial HS duties 25/50/75%; update +10% every 500 ms with wrap; LS default = complementary, no dead-time; TOM instance/channels for P02.x TBD. I’ll lock TOM instance/ch mapping, duty behavior, and scheduling cadence here.
- target_board (from board DB): KIT_A2G_TC387_5V_TFT. I’ll select the correct TC38x pinmap header for this board/package.
- requirements_json (from refiner node): Confirms peripheral type GTM_TOM, driver_name IfxGtm_Tom_Pwm, shared_timer IfxGtm_Tom_Timer on TOM0 ch 3, 20 kHz center-aligned, expected files GTM_TOM_3_Phase_Inverter_PWM.c/.h. I’ll use TOM0.CH3 for the time-base and adhere to the file API names.
- hardware_json (from hardware node): MCU TC387 (DEVICE_TC38X), TOM0 channels [0,1,2,4,5,7], pin example PHASE_U_HS: P02.0 -> IfxGtm_TOM0_0_TOUT0_P02_0_OUT, clocks: xtal 20 MHz, system 300 MHz, all_verified true. I’ll bind channels to P02.[0,1,2,4,5,7], reserving CH3 for the timer, and compute period ticks from GTM CMU frequency.
- sw_architecture_json (from sw-architecture): Provides function names (GTM_TOM_3_Phase_Inverter_PWM_init, …), suggests IfxGtm_Pwm but keeps IfxGtm_Tom_Timer as primary. I’ll keep the function naming but deliberately use IfxGtm_Tom_Pwm to satisfy single-output TOM routing.
- template_capabilities (from template analyzer): Confirms iLLD availability for IfxGtm_Tom_Pwm/Timer and pinmap headers (TC38x LFBGA292/516), PLL 300 MHz. I’ll include the right pinmap header variant for KIT_A2G_TC387_5V_TFT.
- reference_analysis (from reference project): Prior art used IfxGtm_Tom_PwmHl; macros PWM_FREQ_HZ 20000; watchdog patterns. I’ll mirror naming/style and init patterns but swap to single-output API.
- header_documentation_context + extracted_config_values (from header selector/parser): Valid field/enum snippets, e.g., IfxGtm_TOM0_0_TOUT0_P02_0_OUT, tomChannel = IfxGtm_Tom_Ch_0. I’ll use these exact macros/enums in the design.

## Execution Plan
1) Finalize channel-to-pin map using hardware_json:
   - TOM0: CH0=P02.0 (PHASE_U_HS), CH7=P02.7 (PHASE_U_LS), CH1=P02.1 (PHASE_V_HS), CH4=P02.4 (PHASE_V_LS), CH2=P02.2 (PHASE_W_HS), CH5=P02.5 (PHASE_W_LS). Reserve CH3 for IfxGtm_Tom_Timer (per requirements_json).
2) Define configuration constants:
   - PWM_FREQ_HZ = 20000; UPDATE_INTERVAL_MS = 500; initial duties HS = {0.25f, 0.50f, 0.75f}; LS mode default = complementary; TOM instance = IfxGtm_Tom_0; timerCh = IfxGtm_Tom_Ch_3.
3) Derive timer period for center-aligned mode:
   - Obtain GTM CMU clock at runtime (e.g., IfxGtm_Cmu_getModuleFrequency or timer getter) and compute ticks = f_gtm / (2 * PWM_FREQ_HZ). Store in config and assert range.
4) Define context/structs and prototypes:
   - typedef struct GtmTom3PhaseCtx { IfxGtm_Tom_Timer timer; IfxGtm_Tom_Pwm u_hs, u_ls, v_hs, v_ls, w_hs, w_ls; float32 dutyU, dutyV, dutyW; boolean lsComplementary; }.
   - API: void GTM_TOM_3_Phase_Inverter_PWM_init(void); void GTM_TOM_3_Phase_Inverter_PWM_update(float32 dutyU, float32 dutyV, float32 dutyW, boolean lsComplementary); void GTM_TOM_3_Phase_Inverter_PWM_tick500ms(void).
5) Initialization sequence:
   - Enable GTM, configure IfxGtm_Tom_Timer on TOM0.CH3 for center-aligned 20 kHz; init six IfxGtm_Tom_Pwm channels with output pins (&IfxGtm_TOM0_0_TOUT0_P02_0_OUT, …), link to shared timer, set alignment to center, set initial duties, start timer and channels with sync update.
6) Duty update logic:
   - tick500ms(): increment HS duties by +0.10 with wrap, recompute LS as inverted if lsComplementary=true (no dead-time), else mirror HS.
7) Unit-test specifications:
   - Verify TOUT routing per pin macros; frequency within ±1%; symmetry (center-aligned) by reading CCU compare values; duty wrap behavior across 10 updates; CH3 not routed to a TOUT; toggle LS mode runtime.

## Key Decisions & Risks
- Driver choice: Use IfxGtm_Tom_Pwm (requirement) instead of suggested IfxGtm_Pwm; document rationale.
- Pin package header: Template shows IFX_PIN_PACKAGE_516 while some inputs mention 292; I’ll select the correct IfxGtm_PinMap_TC38x_LFBGA516 vs _LFBGA292 based on KIT_A2G_TC387_5V_TFT and flag mismatch risk.
- GTM CMU frequency source not explicit; I’ll compute ticks at runtime to avoid hardcoding.
- Complementary with zero dead-time risks shoot-through in real power stages; kept as per requirement but highlighted for HW safety review.