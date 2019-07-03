#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "watchdog-lib.h"

#include <sys/time.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#define WATCHDOG_PERIOD 5000
#define WATCHDOG_PET_INTERVAL 3
#define WATCHDOG_NUM_PETS 999
#define WATCHDOG_PULSE_WIDTH 254

/* Helpful conversion constants. */
static const unsigned usec_per_sec = 1000000;
static const unsigned usec_per_msec = 1000;

/* These functions are written to match the win32
   signatures and behavior as closely as possible.
*/
bool QueryPerformanceFrequency(int64_t *frequency)
{
    /* Sanity check. */
    assert(frequency != NULL);

    /* gettimeofday reports to microsecond accuracy. */
    *frequency = usec_per_sec;

    return true;
}

bool QueryPerformanceCounter(int64_t *performance_count)
{
    struct timeval time;

    /* Sanity check. */
    assert(performance_count != NULL);

    /* Grab the current time. */
    gettimeofday(&time, NULL);
    *performance_count = time.tv_usec + /* Microseconds. */
                         time.tv_sec * usec_per_sec; /* Seconds. */

    return true;
}
int main (int argc, char **argv)
{
	double degrees = 0.0, humidity = 0.0;
	int status = 0, i = 0;
    int loopcount = 0;
	int64_t sum=0,now=0,den=0,freq=0;
	double avg;

	QueryPerformanceFrequency(&freq);
	printf("freq = %ld ticks per second (1million in Linux at the moment)\n", freq);

	
	do{
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
		
		sum = 0;
		for (i = 0 ; i < WATCHDOG_NUM_PETS ; i++)
		{
			//sleep(WATCHDOG_PET_INTERVAL);
			printf("Petting watchdog ... ");
			QueryPerformanceCounter(&now);
			aio_watchdog_pet(0);
			QueryPerformanceCounter(&den);
			sum += den-now;
			printf("%ldus      \r", den-now);
		}
		avg =(double)sum/(double)WATCHDOG_NUM_PETS;
		printf("\n\nAverage aio_watchdog_pet() took %fus (ticks, on this linux)\n\n", avg);
		aio_watchdog_library_term();
		loopcount++;
		printf("loopcount: %d\n\r", loopcount);
	}while (loopcount < 1000);
}
