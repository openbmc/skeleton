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


gboolean
get_threshold_state (SensorIntegerThreshold    *sen,
                   GDBusMethodInvocation  *invocation,
                   gpointer               user_data)
{
  guint state = sensor_integer_threshold_get_state(sen);
  sensor_integer_threshold_complete_get_state(sen,invocation,state);
  return TRUE;
}


gboolean
set_thresholds (SensorIntegerThreshold        *sen,
                   GDBusMethodInvocation  *invocation,
		   guint                  lc,
		   guint                  lw,
		   guint                  uw,
		   guint                  uc,
                   gpointer               user_data)
{
  sensor_integer_threshold_set_lower_critical(sen,lc);
  sensor_integer_threshold_set_lower_warning(sen,lw);
  sensor_integer_threshold_set_upper_warning(sen,uw);
  sensor_integer_threshold_set_upper_critical(sen,uc);
  sensor_integer_threshold_complete_set(sen,invocation);
  //sensor_integer_threshold_set_state(sen,NORMAL);
  return TRUE;
}


void check_thresholds(SensorIntegerThreshold* sensor,guint value)
{
  	threshold_states current_state = sensor_integer_threshold_get_state(sensor);
 	//if (current_state != NOT_SET) 
	//{
		threshold_states state = NORMAL;
		if (value < sensor_integer_threshold_get_lower_critical(sensor)) {
    			state = LOWER_CRITICAL;
  		}
		else if(value < sensor_integer_threshold_get_lower_warning(sensor)) {
    			state = LOWER_WARNING;
		}
		else if(value > sensor_integer_threshold_get_upper_critical(sensor)) {
 			state = UPPER_CRITICAL;
		}
		else if(value > sensor_integer_threshold_get_upper_warning(sensor)) {
 			state = UPPER_WARNING;
		}
		// only emit signal if threshold state changes
		if (state != sensor_integer_threshold_get_state(sensor))
		{
			sensor_integer_threshold_set_state(sensor,state);
			if (state == LOWER_CRITICAL || state == UPPER_CRITICAL)
			{
				sensor_integer_threshold_emit_critical(sensor);
			}
 			else if (state == LOWER_WARNING || state == UPPER_WARNING)
			{
 				sensor_integer_threshold_emit_warning(sensor);
			}
			else if (state == NORMAL)
			{
				sensor_integer_threshold_emit_normal(sensor);
			}
		}
	//}
}

