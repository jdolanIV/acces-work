#include <stdio.h>
#include <unistd.h>
#include "watchdog-lib.h"

int main (int argc, char **argv)
{
        double degrees = 0.0, humidity = 0.0;

        aio_watchdog_library_init();
        aio_watchdog_reset(0);
        aio_watchdog_period_set (0, 500);
//        aio_watchdog_start (0);
        aio_watchdog_temp_read(0, &degrees);
        printf("degrees = %f\n", degrees);
        // aio_watchdog_humidity_read(0, &humidity);
        // printf("humidiy = %f\n", humidity);
        aio_watchdog_library_term();
        sleep(10);
}