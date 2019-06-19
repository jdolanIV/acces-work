#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  int fd = -1;
  fd = open ("/dev/aiowdm/mPCIe-DIO-24S", O_RDWR);
  if ( fd < 0)
  {
    printf("Unable to open device file\n");
    return 1;
  }
  close(fd);

  return 0;
}