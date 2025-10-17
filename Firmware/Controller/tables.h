#ifndef TABLE2D_H
#define TABLE2D_H

#include "types.h"
#include "constants.h"


typedef struct
{
    float data[TABLE_PRIMARY_SIZE_X][TABLE_PRIMARY_SIZE_Y];
    float x_bins[TABLE_PRIMARY_SIZE_X];
    float y_bins[TABLE_PRIMARY_SIZE_Y];
} table_2d_t;

typedef struct
{
    float data[TABLE_PRIMARY_SIZE_X];
    float x_bins[TABLE_PRIMARY_SIZE_X];
} table_1d_t;

typedef struct
{
    float data[TABLE_SMALL_SIZE_X][TABLE_SMALL_SIZE_Y];
    float x_bins[TABLE_SMALL_SIZE_X];
    float y_bins[TABLE_SMALL_SIZE_Y];
} table_2d_small_t;


float table_2d_get_value(table_2d_t *table, float x, float y);

float table_1d_get_value(table_1d_t *table, float x);

#endif // TABLE2D_H