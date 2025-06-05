#ifndef __APP_ADCREAD_H__
#define __APP_ADCREAD_H__

#include <stdint.h>

//=========================== prototypes ======================================

/**
 * @brief Initializes the ADC reader application component.
 *
 * Sets up the ADC peripheral via the adc driver and starts a periodic timer
 * to sample the ADC. Must be called after opentimers_init().
 */
void adcread_init(void);

/**
 * @brief Retrieves the last measured ADC value.
 *
 * @return The latest raw ADC value read by the periodic task.
 */
uint16_t adcread_get_value(void);

#endif // __APP_ADC_READER_H__
