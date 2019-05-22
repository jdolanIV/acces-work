#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include "aiousb.h"

#define err_print(fmt, ...) \
        do { printf ("%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, ##__VA_ARGS__); } while (0)
                                


void print_usage ()
{
  printf("Usage: usb-eclear [options] [device-index]\n");
  printf("Where: options :=\n");
  printf("          -e\n");
#if ALLOW_DESTRUCTIVE_WRITES
  printf("          --erase-pnp Erase PNP area. *NOT RECOMMENDED*\n");
#else
  printf("          --erase-pnp Erase PNP area. Currently ignored, read warnings,\n");
  printf("                     recompile, and use at your own risk\n");
#endif
  printf("          -b\n");
  printf("          --no-erase-bulk Don't erase the bulk area. Default is to erase\n");
  printf("          -s\n");
#if ALLOW_DESTRUCTIVE_WRITES
  printf("          --erase-serial-num Erase the serial number. *NOT RECOMMENDED* Can void waranty\n");
#else
  printf("          --erase-serial-num Erase the serial number. Currently ignored, read warnings,\n");
  printf("                     recompile, and use at your own risk\n");
#endif
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
  unsigned long device_index = 0;
  

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
#if ALLOW_DESTRUCTIVE_WRITES
        erase_pnp = 1;
#endif 
        break;

      case 'b':
        no_erase_bulk = 1;
        break;
      case 's':
#if ALLOW_DESTRUCTIVE_WRITES
        erase_serial_num = 1;
#endif
        break;
      case 'u':
        no_erase_user = 1;
        break;
      case '?':
        print_usage();
    };
  } while (current_option != -1);

  // if (option_index <= argc)
  // {
  //   device_index = strtoul(argv[option_index - 1], NULL, 0);
  // }

  printf("option_index = %d\n", option_index);
  printf("argc = %d\n", argc);
  printf("erase_pnp = %d\n", erase_pnp);
  printf("no-erase-bulk = %d\n", no_erase_bulk);
  printf("erase_serial_num = %d\n", erase_serial_num);
  printf("no_erase_user = %d\n", no_erase_user);
  printf("device_index = 0x%lx\n", device_index);

  { //init AIOUSB and verify we can talk to a device at given index
    AIORET_TYPE aiousb_status;
    char name[128] = {0};
    unsigned long name_size = sizeof(name);

    aiousb_status = AIOUSB_Init();

    if ( aiousb_status )
    {
      err_print("Error opening AIOUSB library. %ld\n", aiousb_status);
      return -1;
    }

    aiousb_status = QueryDeviceInfo ( device_index, NULL, &name_size, name, NULL, NULL);

    if (aiousb_status)
    {
      err_print("Error trying to query device info. %ld\n", aiousb_status);
      return -1;
    }

    printf("name is %s", name);
  }
#if 0
  {
    char zero_buff[0x2000] = {0};
    char ones_buff[0x2000];
    char rand_buff[0x2000];
    int fd = open("/dev/urandom", O_RDONLY);
    unsigned long length;

    memset(ones_buff, 0xff, sizeof(ones_buff));

    if (read(fd, rand_buff, sizeof(rand_buff)) != sizeof(rand_buff))
    {
      err_print("Error reading random data\n");
    }
    close(fd); fd = -1;

#if ALLOW_DESTRUCTIVE_WRITES
    if(erase_pnp)
    {
      length = 0x8;
      GenericVendorWrite(device_index, 0xa2, 0, (unsigned long *)&rand_buff[0], &length);
      GenericVendorWrite(device_index, 0xa2, 0, (unsigned long *)&zero_buff[0], &length);
      GenericVendorWrite(device_index, 0xa2, 0, (unsigned long *)&ones_buff[0], &length);
      GenericVendorWrite(device_index, 0xa2, 0, (unsigned long *)&zero_buff[0], &length);
    }
#endif

    if(!no_erase_bulk)
    {
      length = 0x1df0 - 0x8;
      GenericVendorWrite(device_index, 0xa2, 0x8, 0,  (unsigned long *)&rand_buff[0x8], &length);
      GenericVendorWrite(device_index, 0xa2, 0x8, 0,  (unsigned long *)&zero_buff[0x8], &length);
      GenericVendorWrite(device_index, 0xa2, 0x8, 0,  (unsigned long *)&ones_buff[0x8], &length);
      GenericVendorWrite(device_index, 0xa2, 0x8, 0,  (unsigned long *)&zero_buff[0x8], &length);
    }

#if ALLOW_DESTRUCTIVE_WRITES
    if(erase_serial_num)
    {
      length = 0x1e00 - 0x1df0;
      GenericVendorWrite(device_index, 0xa2, 0x1df0, 0, (unsigned long *)&rand_buff[0x1df0], &length);
      GenericVendorWrite(device_index, 0xa2, 0x1df0, 0, (unsigned long *)&zero_buff[0x1df0], &length);
      GenericVendorWrite(device_index, 0xa2, 0x1df0, 0, (unsigned long *)&ones_buff[0x1df0], &length);
      GenericVendorWrite(device_index, 0xa2, 0x1df0, 0, (unsigned long *)&zero_buff[0x1df0], &length);
    }
#endif

    if(!no_erase_user)
    {
      length = 0x2000 - 0x1e00;
      GenericVendorWrite(device_index, 0xa2, 0x1e00, 0, (unsigned long *)&rand_buff[0x1e00], &length);
      GenericVendorWrite(device_index, 0xa2, 0x1e00, 0, (unsigned long *)&zero_buff[0x1e00], &length);
      GenericVendorWrite(device_index, 0xa2, 0x1e00, 0, (unsigned long *)&ones_buff[0x1e00], &length);
      GenericVendorWrite(device_index, 0xa2, 0x1e00, 0, (unsigned long *)&zero_buff[0x1e00], &length);
    }

  }
#endif
}