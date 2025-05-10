#ifndef TABLE2D_H
#define TABLE2D_H

#include "types.h"
#include "tables.h"
#include "constants.h"

typedef struct
{
    angle_t data[TABLE_PRIMARY_SIZE_X][TABLE_PRIMARY_SIZE_Y];

} table_ignition_t;

typedef struct
{
    percent_t data[TABLE_PRIMARY_SIZE_X][TABLE_PRIMARY_SIZE_Y];
} table_ve_t;



/**
 * @brief Retrieves the value from the VE (Volumetric Efficiency) table based on the given RPM and load.
 * 
 * @param rpm The engine speed in revolutions per minute (RPM).
 * @param load The engine load, typically represented as pressure.
 * @return The corresponding value from the VE table as a percentage.
 * @note There is no interpolation for now.
 * @todo Add interpolation.
 */
percent_t table_ve_get_value(table_ve_t *table, rpm_t rpm, pressure_t load);


/**
 * @brief Retrieves the ignition angle based on engine RPM and load pressure.
 * 
 * @param rpm The engine speed in revolutions per minute (RPM).
 * @param load The engine load pressure.
 * @return The ignition angle corresponding to the given RPM and load.
 * @note There is no interpolation for now.
 * @todo Add interpolation.
 */
angle_t table_ignition_get_value(table_ignition_t *table, rpm_t rpm, pressure_t load);



#endif // TABLE2D_H