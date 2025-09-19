#include "opendefs.h"        // OpenWSN type definitions and component IDs
#include "adcread.h" // Header for this module
#include "adc.h"            // Include your ADC driver header
#include "opentimers.h"     // OpenWSN Kernel Timer service
#include "openserial.h"     // OpenWSN Serial communication (for errors/debug)
#include "scheduler.h"
#include "databus.h"
#include "idmanager.h"
#include <string.h>         // For memset

//=========================== defines =========================================

// Define the period for reading the ADC in milliseconds
#define ADC_READER_PERIOD_MS 5000 // Read ADC every 5000ms (5 seconds)

//=========================== variables =======================================

typedef struct {
    opentimers_id_t timer_id;          // Stores the ID assigned by opentimers
    uint16_t        latest_adc_value; // The variable holding the latest raw ADC reading
    adc_config_t   adc_config;       // Store the ADC configuration used
    bool           initialized;      // Flag to prevent double initialization
} adcread_vars_t;

// State variable for this module
static adcread_vars_t adcread_vars;

//=========================== prototypes ======================================

void _adcread_timer_cb(opentimers_id_t id);
void _adcread_task_cb(void);

//=========================== public ==========================================

/**
 * @brief Initializes the ADC reader application component.
 * @warning This function assumes opentimers_init() has been called previously.
 */
void adcread_init() {
    // Prevent double initialization
    if (adcread_vars.initialized) {
        return;
    }
    // Clear module variables
    memset(&adcread_vars, 0, sizeof(adcread_vars_t));

    // --- 1. Configure the ADC ---
    adcread_vars.adc_config.resolution = ADC_RESOLUTION_12BIT;
    adcread_vars.adc_config.oversample = ADC_OVERSAMPLE_DISABLED;
    adcread_vars.adc_config.reference  = ADC_REFERENCE_INTERNAL; // 0.6V reference
    adcread_vars.adc_config.gain       = ADC_GAIN_1_6;          // Input range = 0.6V / (1/6) = 3.6V
    adcread_vars.adc_config.acq_time   = ADC_ACQTIME_10US;
    adcread_vars.adc_config.pin        = ADC_PIN_A0;             // Example: Read from XIAO pin A0 (P0.02)

    // --- 2. Initialize the ADC driver ---
    adc_init(&adcread_vars.adc_config);

    // --- 3. Create and schedule the periodic timer ---
    adcread_vars.timer_id = opentimers_create(TIMER_GENERAL_PURPOSE, TASKPRIO_ADC);

    if (adcread_vars.timer_id == ERROR_NO_AVAILABLE_ENTRIES) {
        openserial_printLog(
            SERFRAME_MOTE2PC_ERROR,
            COMPONENT_ADCREAD, // Component ID (add to opendefs.h)
            ERROR_NO_AVAILABLE_ENTRIES,        // Error code (needs definition in openwsn.h/opendefs.h)
            (errorparameter_t)0,
            (errorparameter_t)0
        );
        adcread_vars.initialized = false; // Mark as not successfully initialized
    } else {
        // Schedule the timer to run periodically (similar to uinject example)
        opentimers_scheduleIn(
            adcread_vars.timer_id, // The ID of the timer we just created
            ADC_READER_PERIOD_MS,         // Timer duration/period
            TIME_MS,                      // Unit of the duration (milliseconds)
            TIMER_PERIODIC,               // Type: periodic timer
            _adcread_timer_cb       // Function to call when timer expires
        );
        // Mark as initialized successfully
         adcread_vars.initialized = true;
         // Log initialization (optional)
         databus_init(BUFFER_SIZE, ADC);
    }
}

/**
 * @brief Retrieves the last measured ADC value.
 */
uint16_t adcread_get_value() {
    // Return the latest value stored by the timer callback
    return adcread_vars.latest_adc_value;
}

//=========================== private =========================================

/**
 * @brief Timer callback function executed periodically by opentimers.
 *
 * This function reads the ADC and updates the global variable.
 * @param[in] id The timer ID that expired (should match adcread_vars.timer_id).
 */
void _adcread_timer_cb(opentimers_id_t id) {

    _adcread_task_cb();
   
}


void _adcread_task_cb(void) {
        // don't run on dagroot
    if (idmanager_getIsDAGroot()) {
        opentimers_destroy(adcread_vars.timer_id);
        return;
    }
 // --- 4. Sample the ADC ---
    bool success = adc_sample(&adcread_vars.latest_adc_value);
    if(success)
    {
      //payload[len++] = (uint8_t)(reading & 0x00ff);
      //payload[len++] = (uint8_t)((reading & 0xff00) >> 8);
      //databus_write(ADC, adcread_vars.latest_adc_value, 2);
    }
    // --- 5. Handle Result (Optional) ---
    if (!success) {
        // Log an error if the ADC sample failed
        openserial_printLog(
            SERFRAME_MOTE2PC_ERROR,
            COMPONENT_ADCREAD, 
            ERR_ADC_READ_FAILED,     
            (errorparameter_t)0,
            (errorparameter_t)0
        );
    }
}