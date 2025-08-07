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


float table_2d_get_value(table_2d_t *table, float x, float y);


#endif // TABLE2D_H