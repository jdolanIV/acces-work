#include <stdint.h>




/******************************
 * This needs to be the first function called in the library. It returns the
 * number of watchdog cards found.
 * ***************************/
int aio_watchdog_library_init();
void aio_watchdog_library_term();
/*******************************
 * All of these functions return 0 on success. If an error is encountered
 * In the libusb function they wrap, they will print the error message to stdout
 * and return the libusb error code.
 * card_index starts at 0. If an invlid card_index is passed in it will print
 * an error message to stdout and return -1
 * ****************************/
int aio_watchdog_reset(int card_index);
int aio_watchdog_period_set (int card_index, int s);
int aio_watchdog_start (int card_index);
int aio_watchdog_pet (int card_index);
int aio_watchdog_stop (int card_index);
int aio_watchdog_pulse_width_set (int card_index, uint8_t pulse_width);
int aio_watchdog_temp_read (int card_index, double *degrees_c);
int aio_watchdog_humidity_read (int card_index, double *humidity);