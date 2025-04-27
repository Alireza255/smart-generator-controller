#ifndef RPM_CALCULATOR_H
#define RPM_CALCULATOR_H

#include "enums.h"
#include "types.h"

// we use this value in case of noise on trigger input lines
#define NOISY_RPM -1
#define UNREALISTIC_RPM 30000

typedef enum {
    /**
     * The engine is not spinning, RPM=0
     */
    STOPPED,
    /**
     * The engine is spinning up (reliable RPM is not detected yet).
     * In this state, rpmValue is >= 0 (can be zero).
     */
    SPINNING_UP,
    /**
     * The engine is cranking (0 < RPM < cranking rpm)
     */
    CRANKING,
    /**
     * The engine is running (RPM >= cranking rpm)
     */
    RUNNING,
} spinning_state_e;

angle_t crankshaft_get_angle(void);
rpm_t crankshaft_get_rpm(void);

#endif // RPM_CALCULATOR_H
