#ifndef GOVERNER_H
#define GOVERNER_H

#include "electronic_throttle.h"
#include "pid.h"
#include "error_handling.h"

typedef enum
{
    GOVERNER_STATUS_IGNORED,
    GOVERNER_STATUS_IDLE,
    GOVERNER_STATUS_TARGET,
} governer_status_e;

void governer_init(electronic_throttle_t *etb);

governer_status_e governer_get_status();

void governer_update();

#endif // GOVERNER_H