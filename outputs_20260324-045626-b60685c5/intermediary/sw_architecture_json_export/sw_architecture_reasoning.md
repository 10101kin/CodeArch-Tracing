# AI Reasoning - Software Architecture

*Generated: 2026-03-24T05:08:11.279908*

---

## Phase Purpose
In Software Architecture, I define the high-level module layout, driver abstractions, include hierarchy, ISR mapping, and the init/start/update sequence. This produces the blueprint that lets us migrate from TC3xx GTM TOM to TC4xx eGTM ATOM while preserving behavior and meeting timing, pinout, and interrupt requirements. Getting this right ensures later code-gen is clean, consistent, and buildable on KIT_A3G_TC4D7_LITE.

## Data Inventory
- user_requirement (UI)
  - Contains: TC387→TC4D7 migration; IfxEgtm_Pwm on EGTM Cluster_1 ATOM0 CH0/1/2 at 20 kHz, center-aligned, complementary via DTM, 1 µs dead-time, syncStart/syncUpdate, independent duty updates, ISR on CPU0 prio 20 toggling P13.0; outputs P20.8/9/10/11/12/13; FXCLK_0 and CLK0 enabled.
  - Use: Source of truth for timing, channeling, pin map, clocks, and ISR intent.

- target_board (catalog)
  - Contains: KIT_A3G_TC4D7_LITE.
  - Use: Select board-specific BSP includes and pin macros.

- requirements_json (phase 1 refiner)
  - Contains: peripheral_requirement.driver_name=IfxEgtm_Pwm; signal_requirements: Cluster_1, ATOM0, channels [0,1,2], center-aligned, complementary, syncStart/syncUpdate; timing 20 kHz, DTM 1.0 µs; device_configuration device_macro=DEVICE_TC4DX; code_structure expected_files [EGTM_ATOM_3_Phase_Inverter_PWM.c/.h]; reference_context includes (GTM_TOM_3_Phase_Inverter_PWM.h, IfxGtm_Pwm.h, etc.).
  - Use: Lock driver/API choice and file naming; guide migration from IfxGtm_* to IfxEgtm_*.

- hardware_json (phase 2 hardware)
  - Contains: microcontroller TC4D7 verified; EGTM Cluster_1 ATOM0 channels [0,1,2]; pin_assignments PHASE_U/V/W HS/LS on P20.8/9/10/11/12/13; clock_configuration 25 MHz XTAL, 500 MHz system; validation.all_verified=False, conflicts include pins P20.8..P20.13 and P13.0 not fully verified.
  - Use: Concrete instance selection and pin map; flag risks from unresolved pin/board verification.

- template_capabilities (template analyzer)
  - Contains: template_path for KIT_A3G_TC4D7_LITE; TC4xx family; peripheral_support includes PWM/EGTM; device_macro=DEVICE_TC4DX; clock_config (25 MHz XTAL, 500 MHz SYSPLL).
  - Use: Confirm template supports IfxEgtm_Pwm and system clocks; pick correct headers.

- reference_analysis (reference project analyzer)
  - Contains: Original TOM module structure; includes (GTM_TOM_3_Phase_Inverter_PWM.h, Ifx_Types.h, IfxCpu.h, IfxScuWdt.h, Bsp.h, IfxGtm_Pwm.h); patterns: watchdog disable, IFX_INTERRUPT usage; symbol g_gtmTom3phInv; NUM_OF_CHANNELS=3.
  - Use: Mirror structure and init sequence; map old symbols/APIs to new eGTM ones.

- header_documentation_context (iLLD docs)
  - Contains: IfxEgtm_Pwm overview and usage notes.
  - Use: Define config structs (e.g., IfxEgtm_Pwm_Config), modes, sync, interrupts.

- extracted_config_values (header analyzer)
  - Contains: deadTime.rising/falling fields; enum dtmClockSource=IfxGtm_Dtm_ClockSource_cmuClock0; polarity enums; example TOUT pin macros; clock config hints.
  - Use: Concrete field names/enum constants for config; DTM/clock enumerations.

- detected_peripheral_type (detector)
  - Contains: PWM.
  - Use: Sanity check.

## Execution Plan
1. Module decomposition
   - Create EGTM_ATOM_3_Phase_Inverter_PWM.c/.h (from requirements_json.code_structure).
   - Expose APIs: EgtmAtom3ph_init(), EgtmAtom3ph_start(), EgtmAtom3ph_setDuty(float u,v,w), EgtmAtom3ph_triggerSyncUpdate(), and ISR prototype.

2. Include hierarchy
   - In header: Ifx_Types.h, IfxEgtm_Pwm.h, IfxPort.h.
   - In Cpu0_Main.c: EGTM_ATOM_3_Phase_Inverter_PWM.h, IfxCpu.h, IfxScuWdt.h, Bsp.h (per reference_analysis, migrated names).

3. Driver abstraction
   - Define driver handle struct (e.g., IfxEgtm_Pwm g_egtmAtom3ph and channel array) mirroring g_gtmTom3phInv.
   - Use IfxEgtm_Pwm for unified ATOM+DTM complementary outputs.

4. Clock + module enable
   - Enable eGTM, CMU FXCLK_0 for ATOM and CMU CLK0 for DTM (user_requirement; extracted_config_values.dtmClockSource).
   - Keep system clock refs from template_capabilities.clock_config.

5. PWM configuration
   - Cluster_1 ATOM0 channels [0,1,2]; center-aligned at 20 kHz.
   - Complementary via DTM with deadTime.rising=1e-6, deadTime.falling=1e-6.
   - Polarity set to active-high; independent duty updates; enable syncStart and syncUpdate.

6. Pin routing
   - Map outputs:
     - U: HS P20.8, LS P20.9
     - V: HS P20.10, LS P20.11
     - W: HS P20.12, LS P20.13
   - Use appropriate IfxEgtm_ATOM0_TOUT macros for P20.x (hardware_json).

7. ISR registration
   - Configure period-event interrupt on EGTM Cluster_1 ATOM0 CH0.
   - Route SRC_EGTM1_ATOM0_CH0 to CPU0 (TOS cpu0), priority 20; enable.
   - Implement ISR to toggle LED on P13.0 via IfxPort.

8. Init/start sequence
   - Disable watchdogs (reference_analysis pattern), init ports (P20.x as outputs, P13.0 as output), init clocks, init PWM config, apply, perform syncStart, then enable outputs.

## Key Decisions & Risks
- Exact SRC name/line for “EGTM1 ATOM0 CH0 period” may vary; I’ll choose SR0 to CPU0 and confirm during build.
- Pin verification is flagged (hardware_json.validation.all_verified=False) for P20.8–P20.13 and P13.0; risk that board routing or TOUT mux differs—may need alternate TOUT mappings.
- FXCLK_0 frequency isn’t explicitly stated; I’ll rely on IfxEgtm_Pwm frequency parameterization instead of hardcoding ticks.
- API deltas from IfxGtm_Pwm (TC3xx) to IfxEgtm_Pwm (TC4xx) may require adapting init fields; I’ll align with header_documentation_context/extracted_config_values to avoid legacy calls.