#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <systemd/sd-bus.h>
#include "i2c-dev.h"
#include "log.h"


#define NUM_DIMM	32
#define NUM_FAN		12
#define NUM_FAN_MODULE	6
#define NUM_CPU_CORE	12
#define NUM_PWM		6

const char *gService = "org.openbmc.Sensors";
const char *fanService = "org.openbmc.control.Fans";
const char *gService_Power = "org.openbmc.control.Chassis";
const char *gObjPath_Power = "/org/openbmc/control/chassis0";
const char *gIntPath_Power = "org.openbmc.control.Chassis";

const char *fanInObjPath [NUM_FAN] = {"/org/openbmc/sensors/tach/fan0H",
				"/org/openbmc/sensors/tach/fan0L",
				"/org/openbmc/sensors/tach/fan1H",
				"/org/openbmc/sensors/tach/fan1L",
				"/org/openbmc/sensors/tach/fan2H",
				"/org/openbmc/sensors/tach/fan2L",
				"/org/openbmc/sensors/tach/fan3H",
				"/org/openbmc/sensors/tach/fan3L",
				"/org/openbmc/sensors/tach/fan4H",
				"/org/openbmc/sensors/tach/fan4L",
				"/org/openbmc/sensors/tach/fan5H",
				"/org/openbmc/sensors/tach/fan5L",
				};

const char *gCPU0ObjPath[NUM_CPU_CORE] = {"/org/openbmc/sensors/temperature/cpu0/core0",
						 "/org/openbmc/sensors/temperature/cpu0/core1",
						 "/org/openbmc/sensors/temperature/cpu0/core2",
						 "/org/openbmc/sensors/temperature/cpu0/core3",
						 "/org/openbmc/sensors/temperature/cpu0/core4",
						 "/org/openbmc/sensors/temperature/cpu0/core5",
						 "/org/openbmc/sensors/temperature/cpu0/core6",
						 "/org/openbmc/sensors/temperature/cpu0/core7",
						 "/org/openbmc/sensors/temperature/cpu0/core8",
						 "/org/openbmc/sensors/temperature/cpu0/core9",
						 "/org/openbmc/sensors/temperature/cpu0/core10",
						 "/org/openbmc/sensors/temperature/cpu0/core11"};
								
const char *gCPU1ObjPath[NUM_CPU_CORE] = {"/org/openbmc/sensors/temperature/cpu1/core0",
						 "/org/openbmc/sensors/temperature/cpu1/core1",
						 "/org/openbmc/sensors/temperature/cpu1/core2",
						 "/org/openbmc/sensors/temperature/cpu1/core3",
						 "/org/openbmc/sensors/temperature/cpu1/core4",
						 "/org/openbmc/sensors/temperature/cpu1/core5",
						 "/org/openbmc/sensors/temperature/cpu1/core6",
						 "/org/openbmc/sensors/temperature/cpu1/core7",
						 "/org/openbmc/sensors/temperature/cpu1/core8",
						 "/org/openbmc/sensors/temperature/cpu1/core9",
						 "/org/openbmc/sensors/temperature/cpu1/core10",
						 "/org/openbmc/sensors/temperature/cpu1/core11"};

const char *gDIMMObjPath [NUM_DIMM] = {"/org/openbmc/sensors/temperature/dimm0",
								 "/org/openbmc/sensors/temperature/dimm1",
								 "/org/openbmc/sensors/temperature/dimm2",
								 "/org/openbmc/sensors/temperature/dimm3",
								 "/org/openbmc/sensors/temperature/dimm4",
								 "/org/openbmc/sensors/temperature/dimm5",
								 "/org/openbmc/sensors/temperature/dimm6",
								 "/org/openbmc/sensors/temperature/dimm7",
								 "/org/openbmc/sensors/temperature/dimm8",
								 "/org/openbmc/sensors/temperature/dimm9",
								 "/org/openbmc/sensors/temperature/dimm10",
								 "/org/openbmc/sensors/temperature/dimm11",
								 "/org/openbmc/sensors/temperature/dimm12",
								 "/org/openbmc/sensors/temperature/dimm13",
								 "/org/openbmc/sensors/temperature/dimm14",
								 "/org/openbmc/sensors/temperature/dimm15",
								 "/org/openbmc/sensors/temperature/dimm16",
								 "/org/openbmc/sensors/temperature/dimm17",
								 "/org/openbmc/sensors/temperature/dimm18",
								 "/org/openbmc/sensors/temperature/dimm19",
								 "/org/openbmc/sensors/temperature/dimm20",
								 "/org/openbmc/sensors/temperature/dimm21",
								 "/org/openbmc/sensors/temperature/dimm22",
								 "/org/openbmc/sensors/temperature/dimm23",
								 "/org/openbmc/sensors/temperature/dimm24",
								 "/org/openbmc/sensors/temperature/dimm25",
								 "/org/openbmc/sensors/temperature/dimm26",
								 "/org/openbmc/sensors/temperature/dimm27",
								 "/org/openbmc/sensors/temperature/dimm28",
								 "/org/openbmc/sensors/temperature/dimm29",
								 "/org/openbmc/sensors/temperature/dimm30",
								 "/org/openbmc/sensors/temperature/dimm31"};


const char *gObjPath_Ambient = "/org/openbmc/sensors/temperature/ambient";
const char *fanObjPath [6] ={"/org/openbmc/sensors/speed/fan0",
							 "/org/openbmc/sensors/speed/fan1",
							 "/org/openbmc/sensors/speed/fan2",
							 "/org/openbmc/sensors/speed/fan3",
							 "/org/openbmc/sensors/speed/fan4",
							 "/org/openbmc/sensors/speed/fan5"};



const char *gIntPath = "org.openbmc.SensorValue";


double g_Kp = 0.45;
double g_Ki = -0.017;
double g_Kd = 0.3;
int g_CPUVariable = 80;
int g_DIMMVariable = 75;

int g_Sampling_N = 20;
int intergral_i = 0;
int Interal_CPU_Err[20]={0};
int Interal_DIMM_Err[20]={0};

int g_fanspeed = 0;

int Openloopspeed = 0;
int Closeloopspeed = 0;
int Finalfanspeed = 0;

static int i2c_open(int bus)
{
	int rc = 0, fd = -1;
	char fn[32];

	snprintf(fn, sizeof(fn), "/dev/i2c-%d", bus);
	fd = open(fn, O_RDWR);
	if (fd == -1) {
		LOG_ERR(errno, "Failed to open i2c device %s", fn);
		close(fd);
		return -1;
	}
	return fd;
}

#define CMD_OUTPUT_PORT_0 2
#define PCA9535_ADDR 0x20
static int SetFanLed(int fd, uint8_t port0, uint8_t port1)
{
	struct i2c_rdwr_ioctl_data data;
	struct i2c_msg msg[1];
	int rc = 0, use_pec = 0;
	uint8_t write_bytes[3];

//	fprintf(stderr,"SetFanLed: port0 = %02X,port1 = %02X\n",port0,port1);

	memset(&msg, 0, sizeof(msg));

	write_bytes[0] = CMD_OUTPUT_PORT_0;
	write_bytes[1] = port0;
	write_bytes[2] = port1;
  
	msg[0].addr = PCA9535_ADDR;
	msg[0].flags = (use_pec) ? I2C_CLIENT_PEC : 0;
	msg[0].len = sizeof(write_bytes);
	msg[0].buf = write_bytes;

	data.msgs = msg;
	data.nmsgs = 1;
	rc = ioctl(fd, I2C_RDWR, &data);
	if (rc < 0) {
		LOG_ERR(errno, "Failed to do raw io");
		close(fd);
		return -1;
	}

	return 0;
}


int CloseLoop (int cpureading,int dimmreading)
{
	int i,rc;
	int flag = 0;
	double pid [3] = {0};
	int CPUvarible;
	int DIMMvarible = 0;
	
	int CPU_PWM_speed = 0;
	static int CPU_PID_value = 0;
	static int CPU_tracking_error = 0; 
	static int CPU_integral_error = 0;
	static int CPU_differential_error = 0;
	static int CPU_last_error = 0;

	int DIMM_PWM_speed = 0;
	static int DIMM_PID_value = 0;
	static int DIMM_tracking_error = 0;
	static int DIMM_integral_error = 0;
	static int DIMM_differential_error = 0;
	static int DIMM_last_error = 0;
	int CPU_Warning=85;
	int	DIMM_Warning=85;
	//CPU closeloop
	CPU_tracking_error = cpureading - g_CPUVariable;
	Interal_CPU_Err[intergral_i] = CPU_tracking_error;
	CPU_integral_error = 0;

	for(i=0;i<g_Sampling_N;i++)
		CPU_integral_error += Interal_CPU_Err[i] ;

	CPU_differential_error = CPU_tracking_error - CPU_last_error;
	CPU_PID_value = g_Kp * CPU_tracking_error +  g_Ki * CPU_integral_error + g_Kd * CPU_differential_error;
	CPU_PWM_speed = CPU_PID_value + g_fanspeed;

	if(CPU_PWM_speed > 100)
		CPU_PWM_speed = 100;

	if(CPU_PWM_speed < 0)
		CPU_PWM_speed = 0;

	CPU_last_error = CPU_tracking_error;

	//DIMM closeloop
	DIMM_tracking_error = dimmreading - g_DIMMVariable;
	Interal_DIMM_Err[intergral_i] = DIMM_tracking_error;
	intergral_i++;
	DIMM_integral_error = 0;

	for(i=0;i<g_Sampling_N;i++)
		DIMM_integral_error += Interal_DIMM_Err[i] ;

	if(intergral_i == g_Sampling_N)
		intergral_i = 0;
		

    	DIMM_differential_error = DIMM_tracking_error - DIMM_last_error;			
	DIMM_PID_value = g_Kp * DIMM_tracking_error +  g_Ki * DIMM_integral_error + g_Kd * DIMM_differential_error;
	DIMM_PWM_speed = DIMM_PID_value + g_fanspeed;

	if(DIMM_PWM_speed > 100)
		DIMM_PWM_speed = 100;

	if(DIMM_PWM_speed < 0)
		DIMM_PWM_speed = 0;
	
		DIMM_last_error = DIMM_tracking_error;

	if (DIMM_PWM_speed > CPU_PWM_speed)
		Closeloopspeed = DIMM_PWM_speed;
	else
		Closeloopspeed = CPU_PWM_speed;

	if((cpureading>=CPU_Warning)||(dimmreading>=DIMM_Warning))
		Closeloopspeed = 100;

	if(intergral_i == g_Sampling_N)
		intergral_i = 0;
}


int OpenLoop (int sensorreading)
{
	int speed = 0;
	float paramA= 0;
	float paramB= 2; 
	float paramC= 0;
	int Low_Amb = 20;
	int Up_Amb = 38;

	sensorreading=sensorreading-1;
		
	if (sensorreading >= Up_Amb) {
		speed = 100;
//		printf("## Ambient >=%dC, the Fan duty is %d \n",Up_Amb,speed);
	} else if (sensorreading <= Low_Amb) {
		speed = 40;
//		printf("## Ambient <=%dC, the Fan duty is %d \n",Low_Amb,speed);
	} else {
		speed = ( paramA * sensorreading * sensorreading ) + ( paramB * sensorreading ) + paramC;

		if(speed > 100)
			speed = 100;

		if(speed < 40)
			speed = 40;
//		printf("The Fan duty is %d \n",speed);
	}

	Openloopspeed = speed;

	return 0;
}

#define FAN_LED_OFF     0xFF
#define FAN_LED_PORT0_ALL_BLUE  0xAA
#define FAN_LED_PORT1_ALL_BLUE  0x55
#define FAN_LED_PORT0_ALL_RED   0x55
#define FAN_LED_PORT1_ALL_RED   0xAA
#define PORT0_FAN_LED_RED_MASK  0x02
#define PORT0_FAN_LED_BLUE_MASK	0x01
#define PORT1_FAN_LED_RED_MASK  0x40
#define PORT1_FAN_LED_BLUE_MASK	0x80
int Fan_control_algorithm(void)
{
	sd_bus *bus = NULL;
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int rc = 0, i = 0, fd = -1, offset = 0;
	int CPU0_core_temperature[NUM_CPU_CORE], CPU1_core_temperature[NUM_CPU_CORE], HighestCPUtemp = 0;
	int DIMM_temperature[NUM_DIMM], HighestDIMMtemp = 0;
	int Ambient_reading = 0;
	int Fan_tach[NUM_FAN], FinalFanSpeed = 255;
	int Power_state = 0, fan_led_port0 = 0xFF, fan_led_port1 = 0xFF;
	char fan_presence[NUM_FAN_MODULE] = {0}, fan_presence_previous[NUM_FAN_MODULE] = {0};
	char string[128] = {0};
	
	do {
		/* Connect to the user bus this time */
		rc = sd_bus_open_system(&bus);
		if(rc < 0) {
			fprintf(stderr, "Failed to connect to system bus for fan_algorithm: %s\n", strerror(-rc));
			bus = sd_bus_flush_close_unref(bus);
			sleep(1);
		}
	} while (rc < 0);

	while (1) {
		rc = sd_bus_call_method(bus,					// On the System Bus
					gService_Power,				// Service to contact
					gObjPath_Power,			 // Object path
					gIntPath_Power,			   // Interface name
					"getPowerState",			// Method to be called
					&bus_error,				  // object to return error
					&response,				  // Response message on success
					NULL);					   // input message (string,byte)
		if(rc < 0) {
			fprintf(stderr, "Failed to get power state from dbus: %s\n", bus_error.message);
			goto finish;
		}

		rc = sd_bus_message_read(response, "i", &Power_state);
		if (rc < 0 ) {
			fprintf(stderr, "Failed to parse GetPowerState response message:[%s]\n", strerror(-rc));
			goto finish;
		}
		sd_bus_error_free(&bus_error);
		response = sd_bus_message_unref(response);
//		fprintf(stderr,"Power State = [%d]\n",Power_state);

		if (Power_state == 1 ) {
			for(i=0; i<NUM_CPU_CORE; i++) {
				rc = sd_bus_call_method(bus,                   // On the System Bus
							gService,               // Service to contact
							gCPU0ObjPath[i],            // Object path
							gIntPath,              // Interface name
							"getValue",          // Method to be called
							&bus_error,                 // object to return error
							&response,                  // Response message on success
							NULL);                       // input message (string,byte)
				if(rc < 0) {
//					fprintf(stderr, "Failed to get CPU 0 temperature from dbus: %s\n", bus_error.message);
					CPU0_core_temperature[i] = 0;
				} else {
					rc = sd_bus_message_read(response, "v","i", &CPU0_core_temperature[i]);
					if (rc < 0 ) {
						fprintf(stderr, "Failed to parse GetCpu0Temp response message:[%s]\n", strerror(-rc));
						CPU0_core_temperature[i] = 0;
					}
				}
//				fprintf(stderr, "CPU0 core %d temperature is %d\n",i ,CPU0_core_temperature[i]);

				if(CPU0_core_temperature[i] > HighestCPUtemp)
					HighestCPUtemp = CPU0_core_temperature[i];

				sd_bus_error_free(&bus_error);
				response = sd_bus_message_unref(response);
			}

			for(i=0; i<NUM_CPU_CORE; i++) {
				rc = sd_bus_call_method(bus,                   // On the System Bus
							gService,               // Service to contact
							gCPU1ObjPath[i],            // Object path
							gIntPath,              // Interface name
							"getValue",          // Method to be called
							&bus_error,                 // object to return error
							&response,                  // Response message on success
							NULL);                       // input message (string,byte)
				if(rc < 0) {
//					fprintf(stderr, "Failed to get CPU 1 temperature from dbus: %s\n", bus_error.message);
					CPU1_core_temperature[i] = 0;
				} else {
					rc = sd_bus_message_read(response, "v","i", &CPU1_core_temperature[i]);
					if (rc < 0 ) {
						fprintf(stderr, "Failed to parse GetCpu1Temp response message:[%s]\n", strerror(-rc));
						CPU1_core_temperature[i] = 0;
					}
				}
//				fprintf(stderr, "CPU1 core %d temperature is %d\n",i ,CPU1_core_temperature[i]);

				if(CPU1_core_temperature[i] > HighestCPUtemp )
					HighestCPUtemp = CPU1_core_temperature[i];

				sd_bus_error_free(&bus_error);
				response = sd_bus_message_unref(response);
			}
//			fprintf(stderr, "Highest CPU temperature = [%d]\n", HighestCPUtemp);

			rc = sd_bus_call_method(bus,                   // On the System Bus
						gService,               // Service to contact
						gObjPath_Ambient,            // Object path
						gIntPath,              // Interface name
						"getValue",          // Method to be called
						&bus_error,                 // object to return error
						&response,                  // Response message on success
						NULL);                       // input message (string,byte)
			if(rc < 0) {
				fprintf(stderr, "Failed to get ambient temperature from dbus: %s\n", bus_error.message);
				Ambient_reading = 0;
			} else {
				rc = sd_bus_message_read(response, "v","i", &Ambient_reading);
				if (rc < 0 ) {
					fprintf(stderr, "Failed to parse GetDimmTemp response message:[%s]\n", strerror(-rc));
					Ambient_reading = 0;
				}
			}
			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);

//			fprintf(stderr, "Highest ambient inlet temperature = [%d]\n", Ambient_reading);

			if (HighestCPUtemp > 0 && Ambient_reading > 0) {
				for(i=0; i<NUM_DIMM; i++) {
					rc = sd_bus_call_method(bus,                   // On the System Bus
								gService,               // Service to contact
								gDIMMObjPath[i],            // Object path
								gIntPath,              // Interface name
								"getValue",          // Method to be called
								&bus_error,                 // object to return error
								&response,                  // Response message on success
								NULL);                       // input message (string,byte)
					if(rc < 0) {
//						fprintf(stderr, "Failed to get DIMM temperature from dbus: %s\n", bus_error.message);
						DIMM_temperature[i] = 0;
					} else {
						rc = sd_bus_message_read(response, "v","i", &DIMM_temperature[i]);
						if (rc < 0 ) {
							fprintf(stderr, "Failed to parse GetDimmTemp response message:[%s]\n", strerror(-rc));
							DIMM_temperature[i] = 0;
						}
					}
					
//					fprintf(stderr, "DIMM %d temperature is %d\n", i, DIMM_temperature[i]);

					if(DIMM_temperature[i] > HighestDIMMtemp )
						HighestDIMMtemp = DIMM_temperature[i];
					sd_bus_error_free(&bus_error);
					response = sd_bus_message_unref(response);
				}
//				fprintf(stderr, "Highest DIMM temperature = [%d]\n",HighestDIMMtemp);

				CloseLoop(HighestCPUtemp,HighestDIMMtemp);
				OpenLoop(Ambient_reading);

				if(Openloopspeed > Closeloopspeed)
					g_fanspeed = Openloopspeed;
				else
					g_fanspeed = Closeloopspeed;

				FinalFanSpeed = g_fanspeed * 255;
				FinalFanSpeed = FinalFanSpeed / 100;

				if(g_fanspeed > 30) {
					fan_led_port0 = FAN_LED_PORT0_ALL_BLUE;
					fan_led_port1 = FAN_LED_PORT1_ALL_BLUE;
				} else {
					fan_led_port0 = FAN_LED_PORT0_ALL_RED;
					fan_led_port1 = FAN_LED_PORT1_ALL_RED;
				}
			} else {//HighestCPUtemp == 0 || Ambient_reading == 0
				FinalFanSpeed = 255;
				fan_led_port0 = FAN_LED_PORT0_ALL_BLUE;
				fan_led_port1 = FAN_LED_PORT1_ALL_BLUE;
			}

//			fprintf(stderr,"fan_led compute: port0=%02X,port1=%02X\n",fan_led_port0,fan_led_port1);
			for(i=0;i<NUM_FAN;i++) {
				rc = sd_bus_call_method(bus,                               // On the System Bus
							gService,                               // Service to contact
							fanInObjPath[i],                        // Object path
							gIntPath,                          // Interface name
							"getValue",              // Method to be called
							&bus_error,                             // object to return error
							&response,                                      // Response message on success
							NULL);                                           // input message (string,byte)
				if(rc < 0) {
					fprintf(stderr, "Failed to get fan tach from dbus: %s\n", bus_error.message);
					Fan_tach[i] = 0;
				} else {
					rc = sd_bus_message_read(response, "v","i", &Fan_tach[i]);
					if (rc < 0 ) {
						fprintf(stderr, "Failed to parse GetFanTach response message:[%s]\n", strerror(-rc));
						Fan_tach[i] = 0;
					}
				}
				sd_bus_error_free(&bus_error);
				response = sd_bus_message_unref(response);

				if (Fan_tach[i] == 0) {
					FinalFanSpeed = 255;
					if (i <= 3) { //FAN 1 & 2
						offset = i / 2 * 2;
						fan_led_port1 &= ~(PORT1_FAN_LED_RED_MASK >> offset); //turn on red led
						fan_led_port1 |= PORT1_FAN_LED_BLUE_MASK >> offset; //turn off blue led
//						fprintf(stderr,"i=%d,offset=%d,fan_led_port1=%02X\n",i,offset,fan_led_port1);
					} else { //FAN 3~6
						offset = (i - 4) / 2 * 2;
						fan_led_port0 &= ~(PORT0_FAN_LED_RED_MASK << offset); //turn on red led
						fan_led_port0 |= PORT0_FAN_LED_BLUE_MASK << offset; //turn off blue led
//						fprintf(stderr,"i=%d,offset=%d,fan_led_port0=%02X\n",i,offset,fan_led_port0);
					}
				} else {
					fan_presence[i/2] = 1;
				}
			}
		} else {//Power_state == 0
			FinalFanSpeed = 255;
			fan_led_port0 = FAN_LED_OFF;
			fan_led_port1 = FAN_LED_OFF;
		}

		fd = i2c_open(6);
		if (fd != -1) {
			SetFanLed(fd,fan_led_port0,fan_led_port1);
			close(fd);
		}

		for(i=0; i<NUM_PWM; i++) {
			rc = sd_bus_call_method(bus,				   // On the System Bus
						gService,			   // Service to contact
						fanObjPath[i],			// Object path
						gIntPath,			  // Interface name
						"setValue",			// Method to be called
						&bus_error,				   // object to return error
						&response,				   // Response message on success
						"i",						// input message (string,byte)
						FinalFanSpeed);                  // First argument
			if(rc < 0)
				fprintf(stderr, "Failed to adjust fan speed via dbus: %s\n", bus_error.message);
			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}

		for(i=0; i<NUM_FAN_MODULE; i++) {
			if (fan_presence[i] == fan_presence_previous[i])
				continue;

			sprintf(string, "/org/openbmc/inventory/system/chassis/fan%d", i);
			rc = sd_bus_call_method(bus,				   // On the System Bus
						"org.openbmc.Inventory",
						string,
						"org.openbmc.InventoryItem",
						"setPresent",
						&bus_error,
						&response,
						"s",
						(fan_presence[i] == 1 ? "True" : "False"));
			if(rc < 0)
				fprintf(stderr, "Failed to update fan presence via dbus: %s\n", bus_error.message);
			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}
	
finish:
		sd_bus_error_free(&bus_error);
		response = sd_bus_message_unref(response);
		sd_bus_flush(bus);
		HighestCPUtemp = 0;
		HighestDIMMtemp = 0;
		Ambient_reading = 0;
		memcpy(fan_presence_previous, fan_presence, sizeof(fan_presence));
		memset(fan_presence, 0, sizeof(fan_presence));
		sleep(1);
	}
	bus = sd_bus_flush_close_unref(bus);
	return rc < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {

	return Fan_control_algorithm();
}
