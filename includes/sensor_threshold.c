#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <argp.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "sensor_threshold.h"
#include "openbmc.h"

gboolean
get_threshold_state (SensorThreshold    *sen,
                   GDBusMethodInvocation  *invocation,
                   gpointer               user_data)
{
  guint state = sensor_threshold_get_state(sen);
  sensor_threshold_complete_get_state(sen,invocation,state);
  return TRUE;
}



void check_thresholds(SensorThreshold* sensor,GVariant* value)
{
  	threshold_states current_state = sensor_threshold_get_state(sensor);
 	//if (current_state != NOT_SET) 
	//{
		threshold_states state = NORMAL;
		if (VARIANT_COMPARE(value,sensor_threshold_get_lower_critical(sensor)) < 0) {
    			state = LOWER_CRITICAL;
  		}
		else if(VARIANT_COMPARE(value,sensor_threshold_get_lower_warning(sensor)) < 0) {
    			state = LOWER_WARNING;
		}
		else if(VARIANT_COMPARE(value,sensor_threshold_get_upper_critical(sensor)) > 0) {
 			state = UPPER_CRITICAL;
		}
		else if(VARIANT_COMPARE(value,sensor_threshold_get_upper_warning(sensor)) > 0) {
 			state = UPPER_WARNING;
		}
		// only emit signal if threshold state changes
		if (state != sensor_threshold_get_state(sensor))
		{
			sensor_threshold_set_state(sensor,state);
			if (state == LOWER_CRITICAL || state == UPPER_CRITICAL)
			{
				sensor_threshold_emit_critical(sensor);
			}
 			else if (state == LOWER_WARNING || state == UPPER_WARNING)
			{
 				sensor_threshold_emit_warning(sensor);
			}
			else if (state == NORMAL)
			{
				sensor_threshold_emit_normal(sensor);
			}
		}
	//}
}

