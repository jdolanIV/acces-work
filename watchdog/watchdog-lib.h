




void aio_watchdog_library_init();
void aio_watchdog_library_term();
void aio_watchdog_reset(int card_index);
void aio_watchdog_period_set (int card_index, int s);
void aio_watchdog_start (int card_index);
void aio_watchdog_pet (int card_index);
void aio_watchdog_stop (int card_index);
void aio_watchdog_reset_time_set (int card_index, uint8_t pulse_width);
void aio_watchdog_temp_read (int card_index, double *degrees_c);
void aio_watchdog_humidity_read (int card_index, double *humidity);