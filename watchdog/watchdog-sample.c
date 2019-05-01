#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "watchdog-lib.h"

#define WATCHDOG_PERIOD 5
#define WATCHDOG_PET_INTERVAL 3
#define WATCHDOG_NUM_PETS 3
#define WATCHDOG_PULSE_WIDTH 254

int main (int argc, char **argv)
{
	double degrees = 0.0, humidity = 0.0;
	int status = 0, i = 0;

	printf("Initializing aio_watchdog library\n");
	status = aio_watchdog_library_init();
	printf ("Found %d watchdog cards.\n", status);
	if (status == 0) exit(0);
	printf("Reinitalizing watchdog card\n");
	aio_watchdog_reinit(0);
	sleep(2);
	printf("Setting watchdog period to %d\n", WATCHDOG_PERIOD);
	aio_watchdog_period_set (0, WATCHDOG_PERIOD);
	printf("Setting watchdog pulse width\n");
	aio_watchdog_pulse_width_set(0, 128);
	printf("Starting watchdog\n");
	aio_watchdog_start(0);

	for (i = 0 ; i < WATCHDOG_NUM_PETS ; i++)
	{
		aio_watchdog_temp_read(0, &degrees);
		printf("degrees = %f\n", degrees);
		aio_watchdog_humidity_read(0, &humidity);
		printf("humidiy = %f\n", humidity);
		sleep(WATCHDOG_PET_INTERVAL);
		printf("Petting watchdog\n");
		aio_watchdog_pet(0);
	}
	aio_watchdog_library_term();
}