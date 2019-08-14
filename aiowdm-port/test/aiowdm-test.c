#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>


int main (int argc, char **argv)
{
  int fd = -1;
  volatile __uint8_t *bar_mem = NULL;
  int i;
  fd = open ("/dev/aiowdm/mPCIe-DIO-24S", O_RDWR);
  if ( fd < 0)
  {
    printf("Unable to open device file\n");
    return 1;
  }

  bar_mem = mmap(NULL, getpagesize(), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  for (i = 0; i < 4 ; i++)
  {
    printf("bar_mem[%d] = 0x%x\n", i, bar_mem[i]);
  }
  printf("bar_mem[0xA] = 0x%x\n", bar_mem[0xA]);
  printf("bar_mem[0x28] = 0x%x\n",bar_mem[0x28]);
  printf("bar_mem[0x29] = 0x%x\n",bar_mem[0x29]);

  munmap(bar_mem, getpagesize());

  close(fd);

  return 0;
}