#include "clt.h"

resistor_s sensor_clt_resistor;
thermistor_s sensor_clt;

void sensor_clt_init()
{
    resistor_init(&sensor_clt_resistor, 10000, RESISTOR_PULL_UP, ANALOG_INPUT_SENSOR_CLT_PIN);
    thermistor_init(&sensor_clt, configuration.clt_thermistor_calib);
}

temperature_t sensor_clt_get()
{
    temperature_t clt_temp = 0;
    clt_temp = thermistor_get_temp(&sensor_clt);
    if (clt_temp == NAN)
    {
        clt_temp = SENSOR_CLT_FAIL_SAFE;
    }
    return clt_temp;
}
