#include "adc.h"
#include "nrf.h" // Assuming this provides NRF_SAADC base pointer and register definitions
#include <stddef.h> 

#include "uart.h"

//=========================== defines =========================================

// Use channel 0 for simplicity in this basic driver
#define ADC_CHANNEL 0

//=========================== variables =======================================

static bool adc_initialized = false;
static uint16_t adc_result_buffer; // Single buffer for the result

//=========================== private functions ===============================


//Map the logical XIAO pin enum to the nRF52 SAADC AIN input number.

static uint32_t map_xiao_pin_to_ain(adc_analog_input_pin_t pin) {
    switch (pin) {
        case ADC_PIN_A0: return SAADC_CH_PSELP_PSELP_AnalogInput0; // AIN0 -> P0.02
        case ADC_PIN_A1: return SAADC_CH_PSELP_PSELP_AnalogInput1; // AIN1 -> P0.03
        case ADC_PIN_A2: return SAADC_CH_PSELP_PSELP_AnalogInput4; // AIN4 -> P0.28
        case ADC_PIN_A3: return SAADC_CH_PSELP_PSELP_AnalogInput5; // AIN5 -> P0.29
        case ADC_PIN_A4: return SAADC_CH_PSELP_PSELP_AnalogInput2; // AIN2 -> P0.04
        case ADC_PIN_A5: return SAADC_CH_PSELP_PSELP_AnalogInput3; // AIN3 -> P0.05
        default:         return SAADC_CH_PSELP_PSELP_NC; // Not Connected / Disabled
    }
}

//=========================== public functions ================================

void adc_init(const adc_config_t* config) {
    if (adc_initialized || config == NULL) {
        return; // Already initialized or invalid config
    }

    // --- Peripheral Configuration ---

    // 1. Set resolution
    NRF_SAADC->RESOLUTION = (uint32_t)config->resolution;

    // 2. Set oversampling
    // Note: Oversampling requires the sample buffer size (MAXCNT) to be 1.
    NRF_SAADC->OVERSAMPLE = (uint32_t)config->oversample;

    // 3. Configure Result Buffer
    // Point the SAADC result buffer to our variable.
    // MAXCNT must be 1 when oversampling is enabled or for simple blocking mode.
    NRF_SAADC->RESULT.PTR    = (uint32_t)&adc_result_buffer;
    NRF_SAADC->RESULT.MAXCNT = 1; // Store one sample

    // Disable channel first to configure safely
    NRF_SAADC->CH[ADC_CHANNEL].CONFIG = 0;
    NRF_SAADC->CH[ADC_CHANNEL].PSELP  = 0;
    NRF_SAADC->CH[ADC_CHANNEL].PSELN  = 0;

    // 4. Configure Channel 0 Settings
    uint32_t ch_config = 0;
    // Reference Selection
    ch_config |= ((uint32_t)config->reference << SAADC_CH_CONFIG_REFSEL_Pos) & SAADC_CH_CONFIG_REFSEL_Msk;
    // Gain Selection
    ch_config |= ((uint32_t)config->gain << SAADC_CH_CONFIG_GAIN_Pos) & SAADC_CH_CONFIG_GAIN_Msk;
    // Acquisition Time
    ch_config |= ((uint32_t)config->acq_time << SAADC_CH_CONFIG_TACQ_Pos) & SAADC_CH_CONFIG_TACQ_Msk;
    // Mode: Single-ended
    ch_config |= (SAADC_CH_CONFIG_MODE_SE << SAADC_CH_CONFIG_MODE_Pos) & SAADC_CH_CONFIG_MODE_Msk;
    // Resistor Ladder: No pull resistors needed for typical analog input
    ch_config |= (SAADC_CH_CONFIG_RESP_Bypass << SAADC_CH_CONFIG_RESP_Pos) & SAADC_CH_CONFIG_RESP_Msk;
    ch_config |= (SAADC_CH_CONFIG_RESN_Bypass << SAADC_CH_CONFIG_RESN_Pos) & SAADC_CH_CONFIG_RESN_Msk;
    // Burst Mode: Disabled for single sample
    ch_config |= (SAADC_CH_CONFIG_BURST_Disabled << SAADC_CH_CONFIG_BURST_Pos) & SAADC_CH_CONFIG_BURST_Msk;

    NRF_SAADC->CH[ADC_CHANNEL].CONFIG = ch_config;

    // 5. Select Positive Input Pin (Negative input is not used in single-ended mode)
    uint32_t ain_pin = map_xiao_pin_to_ain(config->pin);
    if (ain_pin == SAADC_CH_PSELP_PSELP_NC) {
         // Handle invalid pin selection error if desired
         return; // Cannot initialize with invalid pin
    }
    NRF_SAADC->CH[ADC_CHANNEL].PSELP = (ain_pin << SAADC_CH_PSELP_PSELP_Pos) & SAADC_CH_PSELP_PSELP_Msk;
    NRF_SAADC->CH[ADC_CHANNEL].PSELN = (SAADC_CH_PSELN_PSELN_NC << SAADC_CH_PSELN_PSELN_Pos) & SAADC_CH_PSELN_PSELN_Msk; // Not connected for SE

    // --- Enable ADC ---
    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos) & SAADC_ENABLE_ENABLE_Msk;

    // --- Perform Calibration (Optional but Recommended) ---
    // Clear previous calibration done event
    NRF_SAADC->EVENTS_CALIBRATEDONE = 0;
    // Start calibration task
    NRF_SAADC->TASKS_CALIBRATEOFFSET = 1;
    // Wait for calibration to complete
    while (NRF_SAADC->EVENTS_CALIBRATEDONE == 0);
    NRF_SAADC->EVENTS_CALIBRATEDONE = 0; // Clear event again


    adc_initialized = true;
}

bool adc_sample(uint16_t* result) {
    if (!adc_initialized || result == NULL) {
        return false; // Not initialized or result pointer invalid
    }

    // Clear the END event flag before starting
    NRF_SAADC->EVENTS_END = 0;
    NRF_SAADC->EVENTS_STARTED = 0; // Clear STARTED event as well

    // Start the ADC conversion
    NRF_SAADC->TASKS_START = 1;

    // Wait for the STARTED event
    while (NRF_SAADC->EVENTS_STARTED == 0);
    NRF_SAADC->EVENTS_STARTED = 0; // Clear the event

    // Trigger the sample task
    NRF_SAADC->TASKS_SAMPLE = 1;

    // Wait for the conversion to complete (END event)
    while (NRF_SAADC->EVENTS_END == 0);
    NRF_SAADC->EVENTS_END = 0; // Clear the event
    *result = adc_result_buffer;

    // Optional: Stop the ADC if not taking continuous samples to save power
     NRF_SAADC->TASKS_STOP = 1;
     while (NRF_SAADC->EVENTS_STOPPED == 0); // Wait if needed
     NRF_SAADC->EVENTS_STOPPED = 0;

    return true;
}

void adc_uninit(void) {
    if (!adc_initialized) {
        return;
    }

    // Stop ongoing conversions if any
    NRF_SAADC->TASKS_STOP = 1;
    // Wait for ADC to be stopped
    while (NRF_SAADC->STATUS == (SAADC_STATUS_STATUS_Busy << SAADC_STATUS_STATUS_Pos));

    // Disable the peripheral
    NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos) & SAADC_ENABLE_ENABLE_Msk;

    // Clear channel configurations (optional, but good practice)
    NRF_SAADC->CH[ADC_CHANNEL].CONFIG = 0;
    NRF_SAADC->CH[ADC_CHANNEL].PSELP  = 0;
    NRF_SAADC->CH[ADC_CHANNEL].PSELN  = 0;

    // Reset result pointer and count
    NRF_SAADC->RESULT.PTR = 0;
    NRF_SAADC->RESULT.MAXCNT = 0;

    adc_initialized = false;
}
