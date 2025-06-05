#ifndef __ADC_H__
#define __ADC_H__

#include <stdint.h>
#include <stdbool.h>

//=========================== defines =========================================

// Define the available Analog Input Pins on the XIAO nRF52840
// Mapping based on XIAO nRF52840 Schematic/Pinout:
// A0 -> P0.02 -> AIN0
// A1 -> P0.03 -> AIN1
// A2 -> P0.28 -> AIN4
// A3 -> P0.29 -> AIN5
// A4 -> P0.04 -> AIN2
// A5 -> P0.05 -> AIN3
typedef enum {
    ADC_PIN_A0 = 0, // P0.02 / AIN0
    ADC_PIN_A1 = 1, // P0.03 / AIN1
    ADC_PIN_A2 = 4, // P0.28 / AIN4
    ADC_PIN_A3 = 5, // P0.29 / AIN5
    ADC_PIN_A4 = 2, // P0.04 / AIN2
    ADC_PIN_A5 = 3, // P0.05 / AIN3
    // Add other AIN pins if needed, up to AIN7 (P0.31)
} adc_analog_input_pin_t;

// ADC Resolution
typedef enum {
    ADC_RESOLUTION_8BIT  = 0, // NRF_SAADC_RESOLUTION_8BIT
    ADC_RESOLUTION_10BIT = 1, // NRF_SAADC_RESOLUTION_10BIT
    ADC_RESOLUTION_12BIT = 2, // NRF_SAADC_RESOLUTION_12BIT
    ADC_RESOLUTION_14BIT = 3, // NRF_SAADC_RESOLUTION_14BIT (Requires longer acquisition time >= 10us)
} adc_resolution_t;

// ADC Oversampling (Hardware Averaging)
// Note: Higher oversampling increases conversion time and power consumption.
typedef enum {
    ADC_OVERSAMPLE_DISABLED = 0, // NRF_SAADC_OVERSAMPLE_DISABLED
    ADC_OVERSAMPLE_2X       = 1, // NRF_SAADC_OVERSAMPLE_2X
    ADC_OVERSAMPLE_4X       = 2, // NRF_SAADC_OVERSAMPLE_4X
    ADC_OVERSAMPLE_8X       = 3, // NRF_SAADC_OVERSAMPLE_8X
    ADC_OVERSAMPLE_16X      = 4, // NRF_SAADC_OVERSAMPLE_16X
    ADC_OVERSAMPLE_32X      = 5, // NRF_SAADC_OVERSAMPLE_32X
    ADC_OVERSAMPLE_64X      = 6, // NRF_SAADC_OVERSAMPLE_64X
    ADC_OVERSAMPLE_128X     = 7, // NRF_SAADC_OVERSAMPLE_128X
    ADC_OVERSAMPLE_256X     = 8, // NRF_SAADC_OVERSAMPLE_256X
} adc_oversample_t;

// ADC Reference Voltage Selection
typedef enum {
    ADC_REFERENCE_INTERNAL = 0, // NRF_SAADC_CH_CONFIG_REFSEL_Internal (0.6V)
    ADC_REFERENCE_VDD_DIV_4 = 1, // NRF_SAADC_CH_CONFIG_REFSEL_VDD4 (VDD/4)
} adc_reference_t;

// ADC Gain Selection
typedef enum {
    ADC_GAIN_1_6 = 0, // NRF_SAADC_CH_CONFIG_GAIN_Gain1_6 (Input range = VREF * 6)
    ADC_GAIN_1_5 = 1, // NRF_SAADC_CH_CONFIG_GAIN_Gain1_5 (Input range = VREF * 5)
    ADC_GAIN_1_4 = 2, // NRF_SAADC_CH_CONFIG_GAIN_Gain1_4 (Input range = VREF * 4)
    ADC_GAIN_1_3 = 3, // NRF_SAADC_CH_CONFIG_GAIN_Gain1_3 (Input range = VREF * 3)
    ADC_GAIN_1_2 = 4, // NRF_SAADC_CH_CONFIG_GAIN_Gain1_2 (Input range = VREF * 2)
    ADC_GAIN_1   = 5, // NRF_SAADC_CH_CONFIG_GAIN_Gain1   (Input range = VREF)
    ADC_GAIN_2   = 6, // NRF_SAADC_CH_CONFIG_GAIN_Gain2   (Input range = VREF / 2)
    ADC_GAIN_4   = 7, // NRF_SAADC_CH_CONFIG_GAIN_Gain4   (Input range = VREF / 4)
} adc_gain_t;

// ADC Acquisition Time
typedef enum {
    ADC_ACQTIME_3US   = 0, // NRF_SAADC_CH_CONFIG_TACQ_3us
    ADC_ACQTIME_5US   = 1, // NRF_SAADC_CH_CONFIG_TACQ_5us
    ADC_ACQTIME_10US  = 2, // NRF_SAADC_CH_CONFIG_TACQ_10us
    ADC_ACQTIME_15US  = 3, // NRF_SAADC_CH_CONFIG_TACQ_15us
    ADC_ACQTIME_20US  = 4, // NRF_SAADC_CH_CONFIG_TACQ_20us
    ADC_ACQTIME_40US  = 5, // NRF_SAADC_CH_CONFIG_TACQ_40us
} adc_acqtime_t;


//=========================== typedef =========================================

// Structure to hold ADC configuration
typedef struct {
    adc_resolution_t       resolution;
    adc_oversample_t       oversample;
    adc_reference_t        reference;
    adc_gain_t             gain;
    adc_acqtime_t          acq_time;
    adc_analog_input_pin_t pin; // Which XIAO pin (A0-A5) to use
} adc_config_t;

//=========================== prototypes ======================================

/**
 * @brief Initializes the SAADC peripheral.
 *
 * Configures the ADC resolution, oversampling, channel settings (pin, gain, reference, acquisition time),
 * and enables the peripheral.
 *
 * @param[in] config Pointer to the configuration structure.
 */
void adc_init(const adc_config_t* config);

/**
 * @brief Performs a single blocking ADC conversion on the configured channel.
 *
 * @warning This function blocks until the conversion is complete.
 * @warning Ensure adc_init() has been called successfully before using this function.
 *
 * @param[out] result Pointer to store the ADC conversion result. The range depends
 * on the configured resolution (e.g., 0-1023 for 10-bit).
 * Negative values indicate single-ended mode results centered around 0
 * if the input can go below ground (not typical for simple VDD/GND measurements).
 * For standard 0-VDD measurements, the result will be positive.
 *
 * @return true if the conversion was successful, false otherwise (e.g., ADC not initialized or busy).
 */
bool adc_sample(uint16_t* result);

/**
 * @brief Disables the SAADC peripheral.
 *
 * Powers down the ADC to save energy. Re-initialization is required
 * before taking further samples.
 */
void adc_uninit(void);

#endif // __ADC_H__
