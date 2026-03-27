#include "unity.h"
#include "mock_qspi.h"
#include "qspi.h"

/* Duplicate config values as UT_ macros for self-documenting tests */
#define UT_FLOAT_EPSILON                 (1e-4f)
#define UT_MAX_BAUDRATE_HZ               (50000000U)
#define UT_SPI_MODE                      (0U)
#define UT_FRAME_LENGTH_BITS             (8U)
#define UT_ISR_PRIO_TX                   (40U)
#define UT_ISR_PRIO_RX                   (41U)
#define UT_ISR_PRIO_ERR                  (42U)
#define UT_TLE9180_BUFFER_SIZE_BYTES     (8192U)

void setUp(void)   { mock_qspi_reset(); }
void tearDown(void) {}

/* Group 1 - Qspi_initQspi: initialization / enable guard */
void test_TC_G1_001_init_calls_xspi_and_irqs_once(void) {
    QspiCommunication ctx;

    Ifx_Status st = Qspi_initQspi(&ctx);
    TEST_ASSERT_EQUAL_INT_MESSAGE(Ifx_Status_ok, st, "Qspi_initQspi should return Ifx_Status_ok on successful init");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxXspi_Spi_initModuleConfig_getCallCount(), "IfxXspi_Spi_initModuleConfig must be called exactly once (TC4xx API)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxXspi_Spi_setXspiGpioPins_getCallCount(), "IfxXspi_Spi_setXspiGpioPins must be called exactly once (XSPI pin mapping)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxXspi_Spi_transferInit_getCallCount(), "IfxXspi_Spi_transferInit must be called exactly once (SPI timing init)");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxXspi_Spi_initModule_getCallCount(), "IfxXspi_Spi_initModule must be called exactly once (module bring-up)");

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_IfxCpu_Irq_installInterruptHandler_getCallCount(), "Three IRQ handlers (TX/RX/ERR) must be installed");

    /* TOS retrieval may be used when setting up interrupts; at least once expected */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxCpu_Irq_getTos_getCallCount() >= 1U, "Type-of-service (TOS) should be queried at least once during IRQ setup");
}

void test_TC_G1_002_gpio_direction_and_initial_state_configured(void) {
    QspiCommunication ctx;

    (void)Qspi_initQspi(&ctx);

    /* Device control lines: enable (output) and nINT (input) must be configured */
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_setPinModeOutput_getCallCount() >= 1U, "At least one output pin (enable) must be configured via IfxPort_setPinModeOutput");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxPort_setPinModeInput_getCallCount() >= 1U, "At least one input pin (nINT) must be configured via IfxPort_setPinModeInput");

    /* Initial state should be driven: allow any of setPinHigh/setPinLow/setPinState */
    uint32_t droveHigh = mock_IfxPort_setPinHigh_getCallCount();
    uint32_t droveLow  = mock_IfxPort_setPinLow_getCallCount();
    uint32_t setState  = mock_IfxPort_setPinState_getCallCount();
    TEST_ASSERT_TRUE_MESSAGE((droveHigh + droveLow + setState) >= 1U, "An initial pin state must be driven on the device enable line");
}

/* Group 2 - Qspi_initQspi: configuration values */
void test_TC_G2_001_baudrate_configured_to_50MHz(void) {
    QspiCommunication ctx;

    (void)Qspi_initQspi(&ctx);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(UT_MAX_BAUDRATE_HZ, mock_IfxXspi_Spi_initModule_lastFrequency, "XSPI module frequency must be set to 50 MHz");
}

void test_TC_G2_002_number_of_channels_is_one(void) {
    QspiCommunication ctx;

    (void)Qspi_initQspi(&ctx);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1U, mock_IfxXspi_Spi_initModule_lastNumChannels, "XSPI must initialize a single channel for CS0 (channel-based CS)");
}

/* Group 3 - Qspi_initQspi: runtime update logic (sanity around init-time behavior) */
void test_TC_G3_001_exchange_not_called_during_init(void) {
    QspiCommunication ctx;

    (void)Qspi_initQspi(&ctx);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxXspi_Spi_exchange_getCallCount(), "IfxXspi_Spi_exchange must NOT be called during initialization; only linked as callback");
}

void test_TC_G3_002_transfer_init_dead_times_zero_for_mode0(void) {
    QspiCommunication ctx;

    (void)Qspi_initQspi(&ctx);

    /* For SPI mode 0 and no extra delays, risen/fallen edge delays expected zero */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxXspi_Spi_transferInit_lastDtRising[0], "Rising-edge delay (DT rising) must be 0 for mode 0");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxXspi_Spi_transferInit_lastDtFalling[0], "Falling-edge delay (DT falling) must be 0 for mode 0");
}

/* Group 4 - Qspi_initQspi: ISR / interrupt behavior */
void test_TC_G4_001_interrupt_handlers_installed_tx_rx_err(void) {
    QspiCommunication ctx;

    (void)Qspi_initQspi(&ctx);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(3U, mock_IfxCpu_Irq_installInterruptHandler_getCallCount(), "Three interrupt handlers (TX/RX/ERR) must be installed on init");
    TEST_ASSERT_TRUE_MESSAGE(mock_IfxCpu_Irq_getTos_getCallCount() >= 1U, "TOS retrieval should occur during ISR configuration (IfxSrc_Tos_cpu0)");
}

void test_TC_G4_002_isr_driver_handlers_not_invoked_during_init(void) {
    QspiCommunication ctx;

    (void)Qspi_initQspi(&ctx);

    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxXspi_Spi_isrTransmit_getCallCount(), "Transmit ISR handler must not be invoked during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxXspi_Spi_isrReceive_getCallCount(), "Receive ISR handler must not be invoked during init");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0U, mock_IfxXspi_Spi_isrDmaReceive_getCallCount(), "DMA Receive ISR must not be used (DMA disabled)");
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_TC_G1_001_init_calls_xspi_and_irqs_once);
    RUN_TEST(test_TC_G1_002_gpio_direction_and_initial_state_configured);

    RUN_TEST(test_TC_G2_001_baudrate_configured_to_50MHz);
    RUN_TEST(test_TC_G2_002_number_of_channels_is_one);

    RUN_TEST(test_TC_G3_001_exchange_not_called_during_init);
    RUN_TEST(test_TC_G3_002_transfer_init_dead_times_zero_for_mode0);

    RUN_TEST(test_TC_G4_001_interrupt_handlers_installed_tx_rx_err);
    RUN_TEST(test_TC_G4_002_isr_driver_handlers_not_invoked_during_init);

    return UNITY_END();
}
