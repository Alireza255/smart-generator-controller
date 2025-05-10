#include "thermistor.h"
#include "utils.h"


void thermistor_init(thermistor_s *thermistor, thermistor_conf_s cfg)
{
    if (thermistor == NULL)
    {
        log_error("Thermistor init failed!");
        return;
    }

    // https://en.wikipedia.org/wiki/Steinhart%E2%80%93Hart_equation
	float l1 = logf(cfg.resistance_1);
	float l2 = logf(cfg.resistance_2);
	float l3 = logf(cfg.resistance_3);

	float y1 = 1 / CELSIUS_TO_KELVIN(cfg.tempC_1);
	float y2 = 1 / CELSIUS_TO_KELVIN(cfg.tempC_2);
	float y3 = 1 / CELSIUS_TO_KELVIN(cfg.tempC_3);

	float u2 = (y2 - y1) / (l2 - l1);
	float u3 = (y3 - y1) / (l3 - l1);

	thermistor->m_c = ((u3 - u2) / (l3 - l2)) / (l1 + l2 + l3);
	thermistor->m_b = u2 - thermistor->m_c * (l1 * l1 + l1 * l2 + l2 * l2);
	thermistor->m_a = y1 - (thermistor->m_b + l1 * l1 * thermistor->m_c) * l1;

    /**
     * check disabled for now
     */
    /*
	float resistance10percent = cfg.resistance_1 + 0.1 * (cfg.resistance_2 - cfg.resistance_1);
	float tempAt10percentPoint = thermistor_get_temp(resistance10percent);

	if (tempAt10percentPoint < cfg.tempC_1)
    {
        log_error("Thermistor configuration has failed 10% test");
	}

	float resistance90percent = cfg.resistance_2 + 0.9 * (cfg.resistance_3 - cfg.resistance_2);
	float tempAt90percentPoint = thermistor_get_temp(resistance90percent);
	if (tempAt90percentPoint > cfg.tempC_3)
    {
        log_error("Thermistor configuration has failed 90% test");
	}
    */
}

temperature_t thermistor_get_temp(thermistor_s *thermistor)
{
    // This resistance should have already been validated - only
	// thing we can check is that it's non-negative
    float ohms = resistor_get_resistance(thermistor->resistor);
    if (ohms == NAN)
    {
        log_error("Thermistor invalid resistance!");
        return NAN;
    }

	float lnR = logf(ohms);

	float lnR3 = lnR * lnR * lnR;

	float recip = thermistor->m_a + thermistor->m_b * lnR + thermistor->m_c * lnR3;

	float kelvin = 1 / recip;

	float celsius = KELVIN_TO_CELSIUS(kelvin);

	// bounds check result - please don't try to run this SGC when it is colder than -50C :)
	if (celsius < -50) {
		log_error("Thermistor temp bound check failed!");
        return NAN;
	}

	if (celsius > 250) {
		log_error("Thermistor temp bound check failed!");
        return NAN;
	}

	return celsius;
}