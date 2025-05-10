#include "controller.h"


angle_t ignition_table[TABLE_PRIMARY_SIZE_X][TABLE_PRIMARY_SIZE_Y];
percent_t ve_table[TABLE_PRIMARY_SIZE_X][TABLE_PRIMARY_SIZE_Y];


void aleads()
{
    // Calculate the sizes
    uint8_t x_size = sizeof(ignition_table) / sizeof(ignition_table[0]); // Number of rows
    uint8_t y_size = sizeof(ignition_table[0]) / sizeof(ignition_table[0][0]); // Number of columns

}