#ifndef __SENSOR_THRESHOLDS_H__
#define __SENSOR_THRESHOLDS_H__

#include <stdint.h>
#include "interfaces/openbmc_intf.h"

typedef enum { NOT_SET,NORMAL,LOWER_CRITICAL,LOWER_WARNING,UPPER_WARNING,UPPER_CRITICAL } threshold_states;

gboolean get_threshold_state(SensorThreshold*,
                   GDBusMethodInvocation*,gpointer);

void check_thresholds(SensorThreshold*,GVariant*);


#endif
