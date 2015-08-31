#ifndef __SENSOR_THRESHOLDS_H__
#define __SENSOR_THRESHOLDS_H__

#include <stdint.h>
#include "interfaces/sensor2.h"

typedef enum { NOT_SET,NORMAL,LOWER_CRITICAL,LOWER_WARNING,UPPER_WARNING,UPPER_CRITICAL } threshold_states;

gboolean get_threshold_state(SensorIntegerThreshold*,
                   GDBusMethodInvocation*,gpointer);

gboolean set_thresholds(SensorIntegerThreshold*,
                   GDBusMethodInvocation*,guint,guint,guint,guint,gpointer);
void check_thresholds(SensorIntegerThreshold*,guint);


#endif
