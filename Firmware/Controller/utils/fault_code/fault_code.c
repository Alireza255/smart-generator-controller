#include "fault_code.h"


const fault_info_t fault_info[FAULT_COUNT] = {
    /* Engine Sensors */
    [FAULT_CRANK_SENSOR_FAULT] = {
        .code = FAULT_CRANK_SENSOR_FAULT,
        .name = "Crank Sensor Fault",
        .description = "No signal or electrical fault detected on crank sensor",
        .suggested_fix = "Check wiring, connector, sensor gap, replace sensor",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_MAP_SENSOR1_FAULT] = {
        .code = FAULT_MAP_SENSOR1_FAULT,
        .name = "MAP Sensor Fault",
        .description = "MAP sensor electrical fault or signal missing",
        .suggested_fix = "Check 5V supply, ground, signal wire, replace sensor",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_IAT_SENSOR_FAULT] = {
        .code = FAULT_IAT_SENSOR_FAULT,
        .name = "IAT Sensor Fault",
        .description = "Intake air temperature sensor electrical fault",
        .suggested_fix = "Check wiring or replace IAT sensor",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_CLT_SENSOR_FAULT] = {
        .code = FAULT_CLT_SENSOR_FAULT,
        .name = "CLT Sensor Fault",
        .description = "Coolant temperature sensor electrical fault",
        .suggested_fix = "Check wiring or replace coolant temp sensor",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_EGT_SENSOR_FAULT] = {
        .code = FAULT_EGT_SENSOR_FAULT,
        .name = "EGT Sensor Fault",
        .description = "Exhaust gas temperature sensor fault",
        .suggested_fix = "Inspect sensor, wiring, amplifier",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_LAMBDA_SENSOR_FAULT] = {
        .code = FAULT_LAMBDA_SENSOR_FAULT,
        .name = "Lambda Sensor Fault",
        .description = "Oxygen sensor electrical fault",
        .suggested_fix = "Check heater, wiring, replace sensor",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_DBW1_SENSOR_FAULT] = {
        .code = FAULT_DBW1_SENSOR_FAULT,
        .name = "DBW1 Position Sensor Fault",
        .description = "Throttle position sensor 1 fault",
        .suggested_fix = "Check wiring, sensor calibration",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_DBW2_SENSOR_FAULT] = {
        .code = FAULT_DBW2_SENSOR_FAULT,
        .name = "DBW2 Position Sensor Fault",
        .description = "Throttle position sensor 2 fault",
        .suggested_fix = "Check wiring, sensor calibration",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },

    /* Sensor validity */
    [FAULT_CRANK_POSITION_INVALID] = {
        .code = FAULT_CRANK_POSITION_INVALID,
        .name = "Crank Position Invalid",
        .description = "Crank signal present but timing invalid",
        .suggested_fix = "Check trigger wheel and sensor alignment",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_CAM_POSITION_INVALID] = {
        .code = FAULT_CAM_POSITION_INVALID,
        .name = "Cam Position Invalid",
        .description = "Camshaft position inconsistent with crank",
        .suggested_fix = "Check cam sensor timing and wiring",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_CLT_INVALID] = {
        .code = FAULT_CLT_INVALID,
        .name = "CLT Value Invalid",
        .description = "Coolant temperature out of plausible range",
        .suggested_fix = "Inspect sensor and calibration",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_MAP_INVALID] = {
        .code = FAULT_MAP_INVALID,
        .name = "MAP Value Invalid",
        .description = "MAP reading outside physical limits",
        .suggested_fix = "Check sensor and vacuum line",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_IAT_INVALID] = {
        .code = FAULT_IAT_INVALID,
        .name = "IAT Value Invalid",
        .description = "IAT reading outside physical limits",
        .suggested_fix = "Check sensor placement",
        .severity = FAULT_SEVERITY_INFO,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_EGT_INVALID] = {
        .code = FAULT_EGT_INVALID,
        .name = "EGT Value Invalid",
        .description = "EGT value outside valid range",
        .suggested_fix = "Inspect EGT probe",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_LAMBDA_INVALID] = {
        .code = FAULT_LAMBDA_INVALID,
        .name = "Lambda Value Invalid",
        .description = "Lambda reading implausible",
        .suggested_fix = "Check exhaust leaks or sensor",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },

    /* Outputs */
    [FAULT_OUTPUT_IGNITION_COIL_1] = {
        .code = FAULT_OUTPUT_IGNITION_COIL_1,
        .name = "Ignition Coil 1 Fault",
        .description = "Ignition coil 1 driver fault",
        .suggested_fix = "Check coil and wiring",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_IGNITION_COIL_2] = {
        .code = FAULT_OUTPUT_IGNITION_COIL_2,
        .name = "Ignition Coil 2 Fault",
        .description = "Ignition coil 2 driver fault",
        .suggested_fix = "Check coil and wiring",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_IGNITION_COIL_3] = {
        .code = FAULT_OUTPUT_IGNITION_COIL_3,
        .name = "Ignition Coil 3 Fault",
        .description = "Ignition coil 3 driver fault",
        .suggested_fix = "Check coil and wiring",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_IGNITION_COIL_4] = {
        .code = FAULT_OUTPUT_IGNITION_COIL_4,
        .name = "Ignition Coil 4 Fault",
        .description = "Ignition coil 4 driver fault",
        .suggested_fix = "Check coil and wiring",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_INJECTOR_1] = {
        .code = FAULT_OUTPUT_INJECTOR_1,
        .name = "Injector 1 Fault",
        .description = "Injector 1 driver fault",
        .suggested_fix = "Check injector and wiring",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_INJECTOR_2] = {
        .code = FAULT_OUTPUT_INJECTOR_2,
        .name = "Injector 2 Fault",
        .description = "Injector 2 driver fault",
        .suggested_fix = "Check injector and wiring",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_INJECTOR_3] = {
        .code = FAULT_OUTPUT_INJECTOR_3,
        .name = "Injector 3 Fault",
        .description = "Injector 3 driver fault",
        .suggested_fix = "Check injector and wiring",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_INJECTOR_4] = {
        .code = FAULT_OUTPUT_INJECTOR_4,
        .name = "Injector 4 Fault",
        .description = "Injector 4 driver fault",
        .suggested_fix = "Check injector and wiring",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_MAIN_RELAY] = {
        .code = FAULT_OUTPUT_MAIN_RELAY,
        .name = "Main Relay Fault",
        .description = "Main relay not responding",
        .suggested_fix = "Check relay and power supply",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_FUEL_PUMP] = {
        .code = FAULT_OUTPUT_FUEL_PUMP,
        .name = "Fuel Pump Fault",
        .description = "Fuel pump driver fault",
        .suggested_fix = "Check pump, relay, wiring",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_GAS_SOLENOID] = {
        .code = FAULT_OUTPUT_GAS_SOLENOID,
        .name = "Gas Solenoid Fault",
        .description = "Gas solenoid driver fault",
        .suggested_fix = "Check solenoid and wiring",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_FAN_1] = {
        .code = FAULT_OUTPUT_FAN_1,
        .name = "Cooling Fan 1 Fault",
        .description = "Cooling fan 1 driver fault",
        .suggested_fix = "Check fan motor and relay",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OUTPUT_FAN_2] = {
        .code = FAULT_OUTPUT_FAN_2,
        .name = "Cooling Fan 2 Fault",
        .description = "Cooling fan 2 driver fault",
        .suggested_fix = "Check fan motor and relay",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_LAMBDA_SENSOR_HEATER] = {
        .code = FAULT_LAMBDA_SENSOR_HEATER,
        .name = "Lambda Heater Fault",
        .description = "Lambda sensor heater circuit fault",
        .suggested_fix = "Check heater wiring",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_DBW1_MOTOR_FAULT] = {
        .code = FAULT_DBW1_MOTOR_FAULT,
        .name = "DBW Motor 1 Fault",
        .description = "Throttle motor 1 fault",
        .suggested_fix = "Check motor and H-bridge",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_DBW2_MOTOR_FAULT] = {
        .code = FAULT_DBW2_MOTOR_FAULT,
        .name = "DBW Motor 2 Fault",
        .description = "Throttle motor 2 fault",
        .suggested_fix = "Check motor and H-bridge",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },

    /* Communications */
    [FAULT_CANBUS_COMM] = {
        .code = FAULT_CANBUS_COMM,
        .name = "CAN Communication Fault",
        .description = "CAN bus communication error",
        .suggested_fix = "Check termination and wiring",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_USB_COMM] = {
        .code = FAULT_USB_COMM,
        .name = "USB Communication Fault",
        .description = "USB communication error",
        .suggested_fix = "Reconnect USB cable",
        .severity = FAULT_SEVERITY_INFO,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_I2C_COMM] = {
        .code = FAULT_I2C_COMM,
        .name = "I2C Communication Fault",
        .description = "I2C bus error",
        .suggested_fix = "Check pull-ups and wiring",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_SPI_COMM] = {
        .code = FAULT_SPI_COMM,
        .name = "SPI Communication Fault",
        .description = "SPI communication error",
        .suggested_fix = "Check CS and clock signals",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },

    /* Conditions */
    [FAULT_OVERHEATING] = {
        .code = FAULT_OVERHEATING,
        .name = "Engine Overheating",
        .description = "Coolant temperature above limit",
        .suggested_fix = "Check cooling system",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_OVERHEATING_LOAD_DISCONNECT] = {
        .code = FAULT_OVERHEATING_LOAD_DISCONNECT,
        .name = "Overheating Load Disconnect",
        .description = "Non-essential loads disconnected due to heat",
        .suggested_fix = "Allow engine to cool",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_OVERHEATING_SHUTDOWN] = {
        .code = FAULT_OVERHEATING_SHUTDOWN,
        .name = "Overheating Shutdown",
        .description = "Engine shutdown due to overheating",
        .suggested_fix = "Investigate cooling failure",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_LOW_OIL_PRESSURE] = {
        .code = FAULT_LOW_OIL_PRESSURE,
        .name = "Low Oil Pressure",
        .description = "Oil pressure below safe limit",
        .suggested_fix = "Stop engine, check oil system",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_BATTERY_VOLTAGE_LOW] = {
        .code = FAULT_BATTERY_VOLTAGE_LOW,
        .name = "Battery Voltage Low",
        .description = "Battery voltage below threshold",
        .suggested_fix = "Check battery and charging system",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_BATTERY_VOLTAGE_HIGH] = {
        .code = FAULT_BATTERY_VOLTAGE_HIGH,
        .name = "Battery Voltage High",
        .description = "Battery voltage above threshold",
        .suggested_fix = "Check alternator regulator",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_ALTERNATOR] = {
        .code = FAULT_ALTERNATOR,
        .name = "Alternator Fault",
        .description = "Charging system malfunction",
        .suggested_fix = "Inspect alternator",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_OVERSPEED] = {
        .code = FAULT_OVERSPEED,
        .name = "Engine Overspeed",
        .description = "RPM exceeded safe limit",
        .suggested_fix = "Reduce engine speed",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_OVERBOOST] = {
        .code = FAULT_OVERBOOST,
        .name = "Overboost",
        .description = "Boost pressure above limit",
        .suggested_fix = "Check boost control",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_KNOCK] = {
        .code = FAULT_KNOCK,
        .name = "Knock Detected",
        .description = "Engine knock detected",
        .suggested_fix = "Reduce load or check fuel quality",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_MISFIRE] = {
        .code = FAULT_MISFIRE,
        .name = "Misfire Detected",
        .description = "Engine misfire detected",
        .suggested_fix = "Check ignition and fueling",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },

    /* Fuel system */
    [FAULT_PETROL_PRESSURE_LOW] = {
        .code = FAULT_PETROL_PRESSURE_LOW,
        .name = "Petrol Pressure Low",
        .description = "Fuel pressure below limit",
        .suggested_fix = "Check pump and filter",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_PETROL_PRESSURE_HIGH] = {
        .code = FAULT_PETROL_PRESSURE_HIGH,
        .name = "Petrol Pressure High",
        .description = "Fuel pressure above limit",
        .suggested_fix = "Check regulator",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_GAS_PRESSURE_LOW] = {
        .code = FAULT_GAS_PRESSURE_LOW,
        .name = "Gas Pressure Low",
        .description = "Gas pressure below limit",
        .suggested_fix = "Check gas supply",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },
    [FAULT_GAS_PRESSURE_HIGH] = {
        .code = FAULT_GAS_PRESSURE_HIGH,
        .name = "Gas Pressure High",
        .description = "Gas pressure above limit",
        .suggested_fix = "Check regulator",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_TEMPORARY
    },

    /* Firmware */
    [FAULT_FIRMWARE_DATA_SAVE] = {
        .code = FAULT_FIRMWARE_DATA_SAVE,
        .name = "Firmware Data Save Error",
        .description = "Failed to save data to flash",
        .suggested_fix = "Check flash memory",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_FIRMWARE_DATA_LOAD] = {
        .code = FAULT_FIRMWARE_DATA_LOAD,
        .name = "Firmware Data Load Error",
        .description = "Failed to load data from flash",
        .suggested_fix = "Reflash firmware",
        .severity = FAULT_SEVERITY_WARNING,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
    [FAULT_FIRMWARE_DATA_LOAD_CHECKSUM] = {
        .code = FAULT_FIRMWARE_DATA_LOAD_CHECKSUM,
        .name = "Firmware Data Checksum Error",
        .description = "Checksum mismatch on loaded data",
        .suggested_fix = "Reflash firmware",
        .severity = FAULT_SEVERITY_CRITICAL,
        .persistency = FAULT_PERSISTENCY_PERSISTENT
    },
};

static fault_state_t fault_state[FAULT_COUNT];

void fault_set(fault_code_t code)
{
    if (code >= FAULT_COUNT)
        return;
    if (!fault_state[code].active)
    {
        fault_state[code].active = true;
        fault_state[code].date_time = (date_time_t){0};
    }
    if (fault_state[code].occurrence_count >= 9999)
    {
        return;
    }
    fault_state[code].occurrence_count++;
}

void fault_clear(fault_code_t code)
{
    if (code >= FAULT_COUNT)
        return;
    fault_state[code].active = false;
}

bool fault_is_active(fault_code_t code)
{
    if (code >= FAULT_COUNT)
        return false;
    return fault_state[code].active;
}

bool fault_any_critical_active(void)
{
    for (fault_code_t code = 0; code < FAULT_COUNT; code++)
    {
        if (fault_is_active(code))
        {
            if (fault_info[code].severity == FAULT_SEVERITY_CRITICAL)
            {
                return true;
            }
        }
    }
    return false;
}
bool fault_any_warning_active(void)
{
    for (fault_code_t code = 0; code < FAULT_COUNT; code++)
    {
        if (fault_is_active(code))
        {
            if (fault_info[code].severity == FAULT_SEVERITY_WARNING)
            {
                return true;
            }
        }
    }
    return false;
}
bool fault_any_info_active(void)
{
    for (fault_code_t code = 0; code < FAULT_COUNT; code++)
    {
        if (fault_is_active(code))
        {
            if (fault_info[code].severity == FAULT_SEVERITY_INFO)
            {
                return true;
            }
        }
    }
    return false;
}

