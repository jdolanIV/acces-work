
all: watchdog-sample watchdog-library

watchdog-sample: watchdog-sample.c watchdog-lib.c
	cc -o watchdog-sample watchdog-lib.c watchdog-sample.c  -lusb-1.0

watchdog-library: watchdog-lib.c
	cc -fPIC -shared -o libaiowatchdog.so watchdog-lib.c -lusb-1.0
