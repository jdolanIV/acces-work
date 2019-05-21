#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "aiousb.h"

void print_usage ()
{
  printf("Usage: usb-eclear [options] [device-index]\n");
  printf("Where: options :=\n");
  printf("          -e\n");
#if ALLOW_PNP
  printf("          --erase-pnp Erase PNP area. *NOT RECOMMENDED*\n");
#else
  printf("          --erase-pnp Erase PNP area. Currently ignored, read warnings,\n");
  printf("                     recompile, and use at your own risk\n");
#endif
  printf("          -b\n");
  printf("          --no-erase-bulk Don't erase the bulk area. Default is to erase\n");
  printf("          -s\n");
  printf("          --erase-serial-num Erase the serial number. Can void waranty\n");
  printf("          -u\n");
  printf("          --no-erase-user Don't erase the user area. Default is to erase\n");
  printf("\n\n");
  printf("       device-index := The device index to erase as passed to the aiousb\n");
  printf("          library. Default is 'only'\n");

}


int main (int argc, char **argv)
{
  int current_option = 0;
  int option_index = 0;
  int erase_pnp = 0;
  int no_erase_bulk = 0;
  int erase_serial_num = 0;
  int no_erase_user = 0;
  unsigned long device_index = 0xfffffffd;
  

  struct option long_options[] = 
  {
    {"erase-pnp", no_argument,  0, 'a'},
    {"no-erase-bulk", no_argument, 0, 'b'},
    {"erase-serial-num", no_argument, 0, 's'},
    {"no-erase-user", no_argument, 0, 'u'},
    { 0, 0, 0, 0 }
  };

  do
  {
    current_option = getopt_long(argc, argv, "absu", long_options, &option_index);

    switch(current_option)
    {
      case 'a':
#if ALLOW_PNP
        erase_pnp = 1;
#endif 
        break;

      case 'b':
        no_erase_bulk = 1;
        break;
      case 's':
        erase_serial_num = 1;
        break;
      case 'u':
        no_erase_user = 1;
        break;
      case '?':
        print_usage();
    };
  } while (current_option != -1);

  if (option_index <= argc)
  {
    device_index = strtoul(argv[option_index - 1], NULL, 0);
  }

  printf("option_index = %d\n", option_index);
  printf("argc = %d\n", argc);
  printf("erase_pnp = %d\n", erase_pnp);
  printf("no-erase-bulk = %d\n", no_erase_bulk);
  printf("erase_serial_num = %d\n", erase_serial_num);
  printf("no_erase_user = %d\n", no_erase_user);
  printf("device_index = 0x%lx\n", device_index);


}