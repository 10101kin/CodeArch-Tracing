# AI Reasoning - Requirements Analysis

*Generated: 2026-03-24T01:40:28.889536*

---

## Phase Purpose
In this Requirements Analysis phase, I will convert the user’s natural-language migration request into a precise, structured requirements model that downstream tools will treat as the single source of truth. Getting this right ensures hardware mapping, timing, interrupts, and API choices are unambiguous for architecture, design, and code generation.

## Data Inventory
- user_requirement (from user input)
  - Contains: Migrate to TC4D7 (KIT_A3G_TC4D7_LITE) using eGTM Cluster 1, ATOM0 CH0/CH1/CH2, 20 kHz center-aligned complementary PWM, 1 µs deadtime via DTM/CDTM, synchronized start/updates, single period IRQ on CPU0 priority 20 toggling P13.0; enable eGTM GCLK + FXCLK0; map U/V/W: P20.8/9, P20.10/11, P20.12/13; independent float32[3] duties; use IfxEgtm_Pwm or unified TC4D7 PWM API.
  - Use: Primary behavioral, timing, pinout, and API intent.

- target_board (from user/project context)
  - Contains: KIT_A3G_TC4D7_LITE
  - Use: Bind MCU family, package, and BSP defaults.

- template_capabilities (from template_library_analyzer)
  - Contains: mcu_family=TC4xx, device_macro=DEVICE_TC4DX, clocks: xtal=25 MHz, syspll=500 MHz, perpll1=16 MHz; available_drivers list; pin package IFX_PIN_PACKAGE_BGA436_COM.
  - Use: Validate driver/API availability (IfxEgtm_*), derive clock sources (FXCLK0 planning), interrupt/provider macros.

- reference_analysis (from reference_project_analyzer)
  - Contains: Prior TC387 TOM-based scheme; includes IfxCpu, IfxPort; init pattern shows IfxGtm_Pwm and g_gtmTom3phInv.* usage.
  - Use: Map TOM concepts to ATOM/eGTM equivalents (shadow transfer, AGC sync, complementary pairs), preserve coding patterns and ISR style.

- conversation_history (from chatbot_chain)
  - Contains: Confirmation of migration path to eGTM ATOM.
  - Use: Confidence that ATOM (not TOM) is the target.

- clarifier_result (from clarifier)
  - Contains: Understood intent confirmed; validated new pins U_HS=P20.8, U_LS=P20.9, V_HS=P20.10, V_LS=P20.11, W_HS=P20.12, W_LS=P20.13; CPU0 IRQ prio 20; peripheral_type noted as GTM_TOM (legacy).
  - Use: Lock pin mapping; note legacy label (GTM_TOM) to be corrected to eGTM ATOM in requirements.

## Execution Plan
1. Normalize target platform
   - Set board=KIT_A3G_TC4D7_LITE, mcu_family=TC4xx, device_macro=DEVICE_TC4DX from template_capabilities.

2. Select PWM peripheral and channels
   - Record eGTM Cluster 1, ATOM0 channels CH0/CH1/CH2; define complementary outputs per phase (HS/LS) and pair them to DTM/CDTM resources.

3. Define timing
   - Encode 20 kHz center-aligned (up/down) PWM; specify 1 µs deadtime; note dependency on FXCLK0 frequency to compute ticks.

4. Clocking requirements
   - Require eGTM GCLK enable and FXCLK0 enable; add constraint to choose FXCLK0 so period/deadtime are representable without overflow.

5. Synchronization behavior
   - Specify synchronized start across CH0/1/2 and shadow-based synchronized duty updates via AGC/shadow transfer.

6. Interrupts
   - Define single period interrupt source per PWM period on CPU0, priority 20, provider IfxSrc_Tos_cpu0; ISR toggles LED P13.0.

7. Pin mapping
   - Bind ATOM outputs to pins: U HS=P20.8, LS=P20.9; V HS=P20.10, LS=P20.11; W HS=P20.12, LS=P20.13; output mode push-pull alt-func for eGTM.

8. API and data model
   - Prefer IfxEgtm_Pwm (or unified TC4D7 PWM API) handle; expose duty input float32 duty[3] (0..1); define basic status/error flags (clock_ok, sync_ok, duty_range_err).

9. Compatibility notes
   - Map legacy TOM patterns from reference_analysis (g_gtmTom3phInv) to eGTM ATOM equivalents; preserve ISR coding style and watchdog/CPU init patterns.

## Key Decisions & Risks
- API selection: IfxEgtm_Pwm vs unified TC4D7 PWM API; confirm availability in available_drivers (template list is truncated).
- FXCLK0 frequency: Not specified; must pick (e.g., 100 MHz) to meet 20 kHz and 1 µs deadtime granularity. Risk: wrong choice breaks timing.
- Pin-function mux: Need to confirm ATOM0 CH0–CH2 HS/LS alternate-function mapping to P20.8–P20.13 on BGA436; risk of mismatched output selection.
- DTM/CDTM routing: Exact DTM block/channel pairing for HS/LS needs confirmation to guarantee complementary with 1 µs deadtime.
- Interrupt source: Choose correct ATOM compare event for “period IRQ”; must align with center-aligned mode to ensure one IRQ per cycle.