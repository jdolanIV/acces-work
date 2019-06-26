#include <linux/ioctl.h>

#define AIOWDM_MAGIC 0x0E

#ifndef AIOWDM_MAGIC
#error AIOWDM_MAGIC  not defined.
#endif

struct aiowdm_card_info
{
  uint16_t device_id;
  unsigned short int port_base;
  unsigned long name_size;
  char *name;
};



#define AIOWDM_CARD_INFO_GET _IOR(AIOWDM_MAGIC, 1, struct aiowdm_card_info *)