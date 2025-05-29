#include "tables.h"
#include "error_handling.h"
#include "utils.h"

float table_2d_get_value(table_2d_t *table, float x, float y)
{
    uint8_t num_x_bins = sizeof(table->x_bins) / sizeof(table->x_bins[0]);
    uint8_t num_y_bins = sizeof(table->y_bins) / sizeof(table->y_bins[0]);
    x = CLAMP(x, table->x_bins[0], table->x_bins[num_x_bins - 1]);
    y = CLAMP(y, table->y_bins[0], table->y_bins[num_y_bins - 1]);

    // Find x_bin (lower index)
    uint8_t x_bin = 0;
    while (x_bin < num_x_bins - 1 && x >= table->x_bins[x_bin + 1])
    {
        x_bin++;
    }
    uint8_t x_bin_next = (x_bin < num_x_bins - 1) ? x_bin + 1 : x_bin;

    // Find y_bin (lower index)
    uint8_t y_bin = 0;
    while (y_bin < num_y_bins - 1 && y >= table->y_bins[y_bin + 1])
    {
        y_bin++;
    }
    uint8_t y_bin_next = (y_bin < num_y_bins - 1) ? y_bin + 1 : y_bin;

    // Get bin edges
    float x0 = table->x_bins[x_bin];
    float x1 = table->x_bins[x_bin_next];
    float y0 = table->y_bins[y_bin];
    float y1 = table->y_bins[y_bin_next];

    // Get table values at corners
    float q11 = table->data[x_bin][y_bin];
    float q21 = table->data[x_bin_next][y_bin];
    float q12 = table->data[x_bin][y_bin_next];
    float q22 = table->data[x_bin_next][y_bin_next];

    // Handle edge cases (avoid division by zero)
    float x_frac = (x1 != x0) ? (x - x0) / (x1 - x0) : 0.0f;
    float y_frac = (y1 != y0) ? (y - y0) / (y1 - y0) : 0.0f;

    // Bilinear interpolation
    float interp =
        q11 * (1 - x_frac) * (1 - y_frac) +
        q21 * x_frac * (1 - y_frac) +
        q12 * (1 - x_frac) * y_frac +
        q22 * x_frac * y_frac;

    return interp;
}


