#ifndef ENUMS_H
#define ENUMS_H

/**
 * @brief Trigger wheel type
 */
typedef enum __attribute__ ((__packed__)) {
    /**
     * 58 teeth with 2 missing tooth wheel
     */
    TW_58_TOOTH_2_MISSING = 0,
    /**
     * 29 teeth with 1 missing tooth wheel
     */
    TW_29_TOOTH_1_MISSING = 1,

} trigger_wheel_type_e;

/**
 * @brief Ignition Mode
 */
typedef enum __attribute__ ((__packed__)) {
    /**
     * in this mode the ecu won't produce any sparks
     */
    IM_NO_IGNITION = 0,
    /**
     * in this mode only one spark output is used
     */
    IM_ONE_COIL = 1,
    /**
     * in this mode we use as many coils as we have cylinders
     */
    IM_INDIVIDUAL_COILS = 2,
    /**
     * in this mode we use one coil for every two cylinders
     */
    IM_WASTED_SPARK = 3,
 
} ignition_mode_e;

/**
 * @brief Injection Mode
 */
typedef enum __attribute__ ((__packed__)) {
    /**
     * each cylinder has it's own injector but they all works in parallel
     */
    IM_SIMULTANEOUS = 0,
    /**
     * each cylinder has it's own injector, each injector is wired separately
     */
    IM_SEQUENTIAL = 1,
    /**
     * each cylinder has it's own injector but these injectors work in pairs. Injectors could be wired in pairs or separately.
     * Each pair is fired once per engine cycle
     */
    IM_BATCH = 2,
    /**
     * only one injector for the whole engine
     */
    IM_SINGLE_POINT = 3,

} injection_mode_e;

/**
 * @brief Fuel Type
 */
typedef enum __attribute__ ((__packed__)) {
    /**
     * Gasoline fuel type
     */
    FT_GASOLINE = 0,
    /**
     * Natural gas fuel
     */
    FT_NATURAL_GAS = 1,
    /**
     * both fuel types are used, useful for when we want to start on gasoline and switch to natural gas after startup
     */
    FT_DUAL_FUEL = 2,

} fuel_type_e;

typedef enum __attribute__ ((__packed__)) {
    MT_CUSTOM = 0,
    MT_DENSO183 = 1,
    /**
     * 20 to 250 kPa (2.9 to 36.3 psi) 0.2 to 4.9 V OUTPUT
     */
    MT_MPX4250 = 2,
    MT_HONDA3BAR = 3,
    MT_DODGE_NEON_2003 = 4,
    /**
     * 22012AA090
     */
    MT_SUBY_DENSO = 5,
    /**
     * 16040749
     */
    MT_GM_3_BAR = 6,
 
    /**
     * 20 to 105 kPa (2.9 to 15.2 psi) 0.3 to 4.9 V Output
     */
    MT_MPX4100 = 7,
 
    /**
     * http://rusefi.com/forum/viewtopic.php?f=3&t=906&p=18976#p18976
     * Toyota 89420-02010
     */
    MT_TOYOTA_89420_02010 = 8,
 
    /**
     * 20 to 250 kPa (2.9 to 36.3 psi) 0.25 to 4.875 OUTPUT
     * More precise calibration data for new NXP sensor revisions MPX4250A and MPXA4250A.
     * For an old Freescale MPX4250D use "MT_MPX4250".
     * See https://www.nxp.com/docs/en/data-sheet/MPX4250A.pdf
     */
    MT_MPX4250A = 9,
 
 
    /**
     * Bosch 2.5 Bar TMap Map Sensor with IAT
     * 20 kPa at 0.40V, 250 kPa at 4.65V
     * 4 pin:
     *    Pin 1 : Sensor Ground
     *    Pin 2 : Temp Signal
     *    Pin 3 : 5v
     *    Pin 4 : Map Signal
     * Volkswagen Passat B6
     */
 
    MT_BOSCH_2_5 = 10,
 
    MT_MAZDA_1_BAR = 11,
 
    MT_GM_2_BAR = 12,
 
    MT_GM_1_BAR = 13,
 
    /**
     * 4 bar
     */
    MT_MPXH6400 = 14,
    /**
     * 3 bar
     */
    MT_MPXH6300 = 15,
 
} intake_manifold_pressure_sensor_type_e;


typedef enum __attribute__ ((__packed__)) {
    /**
     * This is the default mode in which ECU controls timing dynamically
     */
    TM_DYNAMIC = 0,
    /**
     * Fixed timing is useful while we are playing with a timing gun - you need to have fixed
     * timing if you want to install your distributor at some specific angle
     */
    TM_FIXED = 1,
 
} ignition_timing_mode_e;

typedef enum __attribute__ ((__packed__))
{
    /**
     * Normal 4 cylinder firing order
     */
    FO_1342,
} firing_order_e;

typedef enum __attribute__ ((__packed__))
{
    SENSOR_IAT_TYPE_BOSCH_816,
} sensor_iat_type_e;


typedef enum __attribute__ ((__packed__))
{
    SENSOR_MAP_TYPE_BOSCH_816,
} sensor_map_type_e;



#endif // ENUMS_H