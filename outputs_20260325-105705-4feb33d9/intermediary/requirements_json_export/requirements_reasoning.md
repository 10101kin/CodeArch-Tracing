# AI Reasoning - Requirements Analysis

*Generated: 2026-03-25T10:55:40.958053*

---

## Phase Purpose
In Requirements Analysis, I turn the user’s natural-language ask into a precise, structured requirements spec that downstream phases will treat as the single source of truth. For this PWM inverter task, I’ll lock down peripherals, timing, interrupts, and migration deliverables so hardware mapping, architecture, and code generation can proceed unambiguously.

## Data Inventory
- user_requirement (from the user)
  - What: Review/patch C code for “iLLD_TC387_ADS_GTM_TOM_3_Phase_Inverter_PWM_2” using IfxGtm_Pwm for 3-phase complementary PWM with DTM deadtime and a period interrupt. Defaults: TOM0 CH0/1, CH2/3, CH4/5; complementary via DTM0; center-aligned; 10 kHz PWM; interrupt from TOM0 period; provide clean `initGtmTom3phInv()` and `updateGtmTom3phInvDuty()`.
  - How I’ll use it: Seed the functional, timing, and ISR requirements; define deliverables and review scope.

- target_board (from project selection)
  - What: KIT_A2G_TC387_5V_TFT.
  - How I’ll use it: Constrain pin/package and available TOUT routing on TC387; align to board-specific pin maps.

- template_capabilities (from template_library_analyzer)
  - What: TC3xx family; device macro `DEVICE_TC38X`; pin package IFX_PIN_PACKAGE_516; GTM supported; available headers include `Libraries\iLLD\TC3xx\Tricore\_PinMap\TC38x\IfxGtm_PinMap_TC38x_516.h`; clocks: xtal 20 MHz, PLL 300 MHz (plus PLL1 320 MHz, PLL2 200 MHz).
  - How I’ll use it: Choose correct pin-map variant (TC38x LFBGA516), note clock sources for later CMU tick derivation, and confirm iLLD driver availability (GTM/TOM/DTM).

- clarifier_result (from chatbot clarifier)
  - What: Intent confirmed; missing_fields: code_to_review, exact pin macros (PHASE_*), selected TOM0 output pins; peripheral_type: GTM_TOM; confidence: medium.
  - How I’ll use it: Record gaps as TBDs and explicitly tag items needing user/code input.

## Execution Plan
1. Extract core functional requirements from user_requirement: 3-phase complementary PWM on `TOM0` pairs (0/1, 2/3, 4/5), center-aligned, with `DTM0` deadtime, and a period interrupt; functions `initGtmTom3phInv()` and `updateGtmTom3phInvDuty()`.
2. Define timing constraints: PWM frequency = 10 kHz; capture center-aligned mode implications (use TOM CM0/CM1 with symmetric update). Mark deadtime value as TBD until code or user supplies ns/us.
3. Bind peripherals: Reserve `GTM.TOM0` and `GTM.DTM0`; plan to use iLLD modules such as `IfxGtm_Tom_PwmHl`, `IfxGtm_Tom_Timer`, and `IfxGtm_Dtm` even if the user referenced `IfxGtm_Pwm`, to reduce API mismatch risk.
4. Interrupts: Specify “TOM0 period event ISR” routed via SRC_GTMTOM0 service request; priority/core TBD; purpose: period-sync housekeeping.
5. IO/pins: Constrain to TC38x LFBGA516 pin map (`IfxGtm_PinMap_TC38x_516.h`). Record PHASE_U/V/W HS/LS TOUT pins as TBD pending board routing on KIT_A2G_TC387_5V_TFT.
6. Clocks: Capture board clocks (20 MHz xtal, 300 MHz PLL). Mark GTM CMU clock source/divider as TBD; period ticks to be derived once CMU is selected.
7. Migration/Review scope: Require the source files to identify exact compile/API errors (line-accurate), common issues (headers, config structs, shadow transfers), and provide cleaned `initGtmTom3phInv()` and `updateGtmTom3phInvDuty()` using consistent iLLD APIs.
8. Produce the structured Requirements JSON with explicit fields and “needs_user_input” tags for TBDs.

## Key Decisions & Risks
- Driver API choice: `IfxGtm_Pwm` vs `IfxGtm_Tom_PwmHl`/`IfxGtm_Tom_Timer`; mismatch can cause compile errors.
- GTM CMU clock selection affects PWM tick math; unknown in snapshot.
- Exact TOUT pins for TOM0 channels on KIT_A2G_TC387_5V_TFT are not provided; risk of unroutable or conflicting pins.
- DTM mapping/polarity per pair and deadtime value are unspecified; wrong settings can cause shoot-through.
- ISR source, SR number, priority, and target CPU core are TBD.
- Without the user code, I can’t enumerate exact line-level fixes yet; that deliverable is contingent on receiving the sources.