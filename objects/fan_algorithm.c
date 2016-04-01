#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <systemd/sd-bus.h>
#include "i2c-dev.h"
#include "log.h"

const char *gService = "org.openbmc.Sensors";
const char *fanService = "org.openbmc.control.Fans";

const char *gfanflagObjPath = "/org/openbmc/sensors/FanParameter/flag";
const char *gfanCPUvaribleObjPath = "/org/openbmc/sensors/FanParameter/flag";
const char *gfanDIMMvaribleObjPath = "/org/openbmc/sensors/FanParameter/flag";
			
const char *gfanpidObjPath [3] = {"/org/openbmc/sensors/FanParameter/Kp",
							      "/org/openbmc/sensors/FanParameter/Ki",
							      "/org/openbmc/sensors/FanParameter/Kd"};


const char *gCPU0ObjPath [12] = {"/org/openbmc/sensors/temperature/cpu0/core0",
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
								
const char *gCPU1ObjPath [12] = {"/org/openbmc/sensors/temperature/cpu0/core0",
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

const char *gDIMMObjPath [32] = {"/org/openbmc/sensors/temperature/dimm0",
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


char *gMessage = NULL;

#define MAX_BYTES 255

int g_use_pec = 0;
int g_has_write = 1;
int g_n_write = 0;
uint8_t g_write_bytes[MAX_BYTES];
uint8_t g_write_color_bytes[MAX_BYTES];

int g_has_read = 1;
int g_n_read = -1;
uint8_t g_read_bytes[MAX_BYTES];
uint8_t g_read_tmp[MAX_BYTES];
uint8_t g_bus = -1;
uint8_t g_slave_addr = 0xff;
double g_Kp = 0.7;
double g_Ki = -0.025;
double g_Kd = 1.0;
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

static int i2c_open()
{
	int rc = 0, fd = -1;
	char fn[32];

	g_bus = 6;
	snprintf(fn, sizeof(fn), "/dev/i2c-%d", g_bus);
	fd = open(fn, O_RDWR);
	if (fd == -1) {
		LOG_ERR(errno, "Failed to open i2c device %s", fn);
		close(fd);
		return -1;
	}
	return fd;
}

static int SetFanLed(int fd,int color)
{
	struct i2c_rdwr_ioctl_data data;
	struct i2c_msg msg[2];
	int rc = 0, n_msg = 0;

	memset(&msg, 0, sizeof(msg));

	g_slave_addr = 0x20;
	g_use_pec = 0;
	g_n_write = 2;

	if(color == 1) {
		//blue light
		g_write_bytes[0] = 0x03;
		g_write_bytes[1] = 0x55;
		g_write_color_bytes[0] = 0x02;
		g_write_color_bytes[1] = 0xaa;
	} else {
		//red light
		g_write_bytes[0] = 0x03;
		g_write_bytes[1] = 0xaa;
		g_write_color_bytes[0] = 0x02;
		g_write_color_bytes[1] = 0x55;
	}
  
	if (1) {
		msg[n_msg].addr = g_slave_addr;
		msg[n_msg].flags = (g_use_pec) ? I2C_CLIENT_PEC : 0;
		msg[n_msg].len = g_n_write;
		msg[n_msg].buf = g_write_bytes;
		n_msg++;
	}
	 if (1) {
		msg[n_msg].addr = g_slave_addr;
		msg[n_msg].flags = (g_use_pec) ? I2C_CLIENT_PEC : 0;
		msg[n_msg].len = g_n_write;
		msg[n_msg].buf = g_write_color_bytes;
		n_msg++;
	}
	data.msgs = msg;
	data.nmsgs = n_msg;
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
	int Up_Amb = 40;
	
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


int Fan_control_algorithm(void)
{
	sd_bus *bus = NULL;
	sd_bus_error bus_error = SD_BUS_ERROR_NULL;
	sd_bus_message *response = NULL;
	int Ambient_reading = 0, rc = 0, i = 0;
	int CPU0_core_temperature[12];
	int CPU1_core_temperature[12];
	int DIMM_temperature[32];
	int HighestCPUtemp = 0;
	int HighestDIMMtemp = 0;
	int CPUnocore[2];
	int fd = -1;
	int FinalFanSpeed = 0;
	int CPUtemp = 0;
	
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
		CPUtemp = 0;
		for(i=0; i<12; i++) {
			rc = sd_bus_call_method(bus,                   // On the System Bus
						gService,               // Service to contact
						gCPU0ObjPath[i],            // Object path
						gIntPath,              // Interface name
						"getValue",          // Method to be called
						&bus_error,                 // object to return error
						&response,                  // Response message on success
						NULL);                       // input message (string,byte)
			if(rc < 0) {
//				fprintf(stderr, "Failed to get CPU 0 temperature from dbus: %s\n", bus_error.message);
				CPU0_core_temperature[i] = 0;
			} else {
				rc = sd_bus_message_read(response, "v","i", &CPU0_core_temperature[i]);
				if (rc < 0 ) {
					fprintf(stderr, "Failed to parse GetCpu0Temp response message:[%s]\n", strerror(-rc));
					CPU0_core_temperature[i] = 0;
				}
			}
//			fprintf(stderr, "CPU0 core %d temperature is %d\n",i ,CPU0_core_temperature[i]);
			if(CPU0_core_temperature[i] > HighestCPUtemp) {
				HighestCPUtemp = CPU0_core_temperature[i];
				CPUnocore[0] = 0;
				CPUnocore[1] = i;
			}
			CPUtemp = CPUtemp + CPU0_core_temperature[i];
			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}

		for(i=0; i<12; i++) {
			rc = sd_bus_call_method(bus,                   // On the System Bus
						gService,               // Service to contact
						gCPU1ObjPath[i],            // Object path
						gIntPath,              // Interface name
						"getValue",          // Method to be called
						&bus_error,                 // object to return error
						&response,                  // Response message on success
						NULL);                       // input message (string,byte)
			if(rc < 0) {
//				fprintf(stderr, "Failed to get CPU 1 temperature from dbus: %s\n", bus_error.message);
				CPU1_core_temperature[i] = 0;
			} else {
				rc = sd_bus_message_read(response, "v","i", &CPU1_core_temperature[i]);
				if (rc < 0 ) {
					fprintf(stderr, "Failed to parse GetCpu1Temp response message:[%s]\n", strerror(-rc));
					CPU1_core_temperature[i] = 0;
				}
			}
//			fprintf(stderr, "CPU1 core %d temperature is %d\n",i ,CPU1_core_temperature[i]);
			if(CPU1_core_temperature[i] > HighestCPUtemp ) {
				HighestCPUtemp = CPU1_core_temperature[i];
				CPUnocore[0] = 1;
				CPUnocore[1] = i;
			}
			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}
//		fprintf(stderr, "Highest CPU temperature = [%d]\n", HighestCPUtemp);
	
		for(i=0; i<32; i++) {
			rc = sd_bus_call_method(bus,                   // On the System Bus
						gService,               // Service to contact
						gDIMMObjPath[i],            // Object path
						gIntPath,              // Interface name
						"getValue",          // Method to be called
						&bus_error,                 // object to return error
						&response,                  // Response message on success
						NULL);                       // input message (string,byte)
			if(rc < 0) {
//				fprintf(stderr, "Failed to get DIMM temperature from dbus: %s\n", bus_error.message);
				DIMM_temperature[i] = 0;
			} else {
				rc = sd_bus_message_read(response, "v","i", &DIMM_temperature[i]);
				if (rc < 0 ) {
					fprintf(stderr, "Failed to parse GetDimmTemp response message:[%s]\n", strerror(-rc));
					DIMM_temperature[i] = 0;
				}
			}
//			fprintf(stderr, "DIMM %d temperature is %d\n", i, DIMM_temperature[i]);
			if(DIMM_temperature[i] > HighestDIMMtemp )
				HighestDIMMtemp = DIMM_temperature[i];
			sd_bus_error_free(&bus_error);
			response = sd_bus_message_unref(response);
		}
//		fprintf(stderr, "Highest DIMM temperature = [%d]\n",HighestDIMMtemp);

		rc = sd_bus_call_method(bus,                   // On the System Bus
					gService,               // Service to contact
					gObjPath_Ambient,            // Object path
					gIntPath,              // Interface name
					"getValue",          // Method to be called
					&bus_error,                 // object to return error
					&response,                  // Response message on success
					NULL);                       // input message (string,byte)
		if(rc < 0) {
//			fprintf(stderr, "Failed to get ambient temperature from dbus: %s\n", bus_error.message);
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
//		fprintf(stderr, "Highest ambient inlet temperature = [%d]\n", HighestCPUtemp);

		if (CPUtemp == 0)	{
			HighestCPUtemp = 0;
			HighestDIMMtemp = 0;
		}

		CloseLoop(HighestCPUtemp,HighestDIMMtemp);
		OpenLoop(Ambient_reading);

		if(Openloopspeed > Closeloopspeed)
			g_fanspeed = Openloopspeed;
		else
			g_fanspeed = Closeloopspeed;
	
		fd = i2c_open();
		if (fd == -1) {
			fprintf(stderr, "Fail to set FAN LED\n");
		} else {
			if(g_fanspeed > 30)
				SetFanLed(fd,1);
			else
				SetFanLed(fd,2);
			close(fd);
		}

		FinalFanSpeed = g_fanspeed * 255;
		FinalFanSpeed = FinalFanSpeed / 100;

		if(HighestCPUtemp == 0) //OCC sensor does not enable
			FinalFanSpeed = 255;

		for(i=0; i<6; i++) {
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
	
finish:
		sd_bus_error_free(&bus_error);
		response = sd_bus_message_unref(response);
		sd_bus_flush(bus);
		sleep(1);
	}
	bus = sd_bus_flush_close_unref(bus);
	return rc < 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}


int main(int argc, char *argv[]) {

	return Fan_control_algorithm();
}
