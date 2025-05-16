#include "iat.h"

resistor_s sensor_iat_resistor = {0};
thermistor_s sensor_iat = {0};

void sensor_iat_init()
{
    switch (configuration.iat_sensor_type)
    {
        case SENSOR_IAT_TYPE_BOSCH_816:
        resistor_init(&sensor_iat_resistor, 10000, RESISTOR_PULL_UP, ANALOG_INPUT_SENSOR_IAT_PIN);
        thermistor_conf_s bosch_816_iat_conf = 
        {
            .resistance_1 = 9395.0f,
            .resistance_2 = 2056.0f,
            .resistance_3 = 322.5f,
            .tempC_1 = -10.0f,
            .tempC_2 = 25.0f,
            .tempC_3 = 80.0f
        };
        thermistor_init(&sensor_iat, bosch_816_iat_conf);
        break;
    
    default:
        log_error("iat sensor type not configured!");
        break;
    }
}

temperature_t sensor_iat_get()
{
    temperature_t temperature = SENSOR_IAT_FAIL_SAFE;
    switch (configuration.iat_sensor_type)
    {
    case SENSOR_IAT_TYPE_BOSCH_816:
        temperature = thermistor_get_temp(&sensor_iat);
        break;
    
    default:
        temperature = SENSOR_IAT_FAIL_SAFE;
        log_warning("iat sensor type not configured!");
        break;
    }
    return temperature;
}
