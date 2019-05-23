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
  printf("           -d\n");
  printf("           --dry-run Just print the device index and name.\n");
  printf("\n\n");
  printf("       device-index := The device index to erase as passed to the aiousb\n");
  printf("          library. Default is 'only'\n");
  

}

#define CHUNK_LENGTH 0x100

void do_write(unsigned long device_index, unsigned short start, unsigned long length)
{
  unsigned long status;
  char zero_buff[0x2000] = {0};
  char ones_buff[0x2000];
  char rand_buff[0x2000];
  int fd = open("/dev/urandom", O_RDONLY);
  unsigned long bytes_written = 0;
  unsigned long bytes_to_write = 0;

  memset(ones_buff, 0xff, sizeof(ones_buff));

  if (read(fd, rand_buff, sizeof(rand_buff)) != sizeof(rand_buff))
  {
    err_print("Error reading random data\n");
  }
  close(fd); fd = -1;

  printf("start = 0x%x\n", start);
  printf("length = 0x%lx\n", length);
  
  while (bytes_written < length)
  {
    bytes_to_write = (CHUNK_LENGTH < (length - bytes_written) ? CHUNK_LENGTH : (length - bytes_written));

    printf("bytes_to_write = 0x%lx\n", bytes_to_write);
    printf("start = 0x%lx\n", start + bytes_written);

    status = GenericVendorWrite(device_index, 0xa2, start + bytes_written, 0,  &rand_buff[bytes_written], &bytes_to_write);
    if ( status ) { err_print ("GenericVendorWrite error %s\n", AIOUSB_GetResultCodeAsString(status)); exit(-1); }
    status = GenericVendorWrite(device_index, 0xa2, start + bytes_written, 0,  &zero_buff[bytes_written], &bytes_to_write);
    if ( status ) { err_print ("GenericVendorWrite error %s\n", AIOUSB_GetResultCodeAsString(status)); exit(-1); }
    status = GenericVendorWrite(device_index, 0xa2, start + bytes_written, 0,  &ones_buff[bytes_written], &bytes_to_write);
    if ( status ) { err_print ("GenericVendorWrite error %s\n", AIOUSB_GetResultCodeAsString(status)); exit(-1); }
    status = GenericVendorWrite(device_index, 0xa2, start + bytes_written, 0,  &zero_buff[bytes_written], &bytes_to_write);
    if ( status ) { err_print ("GenericVendorWrite error %s\n", AIOUSB_GetResultCodeAsString(status)); exit(-1); }

    bytes_written += bytes_to_write;
  }
}


int main (int argc, char **argv)
{
  int current_option = 0;
  int option_index = 0;
  int erase_pnp = 0;
  int no_erase_bulk = 0;
  int erase_serial_num = 0;
  int no_erase_user = 0;
  unsigned long device_index = diOnly;
  int args_parsed = 0;
  int dry_run = 0;
  

  struct option long_options[] = 
  {
    {"erase-pnp", no_argument,  0, 'a'},
    {"no-erase-bulk", no_argument, 0, 'b'},
    {"erase-serial-num", no_argument, 0, 's'},
    {"no-erase-user", no_argument, 0, 'u'},
    {"dry-run", no_argument, 0, 'd'},
    { 0, 0, 0, 0 }
  };

  do
  {
    current_option = getopt_long(argc, argv, "absu", long_options, &option_index);

    if (current_option != -1)
    {
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
        case 'd':
          dry_run = 1;
          break;
        case '?':
          print_usage();
          return -1;
      };
      args_parsed++;
    }
  } while (current_option != -1);

   if (args_parsed == argc - 2) //getopt_long was not behaving according to what
                                //the man page said it would. option_index was
                                //not being set as described so I did this.
   {
     device_index = strtoul(argv[argc - 1], NULL, 0);
   }
  // printf("args_parsed = %d\n", args_parsed);
  // printf("argc = %d\n", argc);
  // printf("erase_pnp = %d\n", erase_pnp);
  // printf("no-erase-bulk = %d\n", no_erase_bulk);
  // printf("erase_serial_num = %d\n", erase_serial_num);
  // printf("no_erase_user = %d\n", no_erase_user);
  // printf("option_index = 0x%x\n", option_index);
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

    printf("name is %s\n", name);
  }
  if (!dry_run)
  {
    
    unsigned long length;
    unsigned long status;
    int start_address;

    printf("Starting write operations\n");

#if ALLOW_DESTRUCTIVE_WRITES
    if(erase_pnp)
    {
      length = 0x8;
      do_write(device_index, 0, length);
      printf("PNP area erased\n");
      
    }
#endif

    if(!no_erase_bulk)
    {
      length = 0x1df0 - 0x8;
      do_write(device_index, 0x8, length);
      
      printf("Bulk area erased\n");
    }

#if ALLOW_DESTRUCTIVE_WRITES
    if(erase_serial_num)
    {
      length = 0x1e00 - 0x1df0;
      do_write(device_index, 0x1df0, length);
      printf("Serial number erased\n");
    }
#endif

    if(!no_erase_user)
    {
      length = 0x2000 - 0x1e00;
      do_write(device_index, 0x1e00, length);
      printf("User area erased\n");
    }

    printf("Requested operations completed\n");

  }
}