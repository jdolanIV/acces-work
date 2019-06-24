#include <linux/ioport.h>
//returns handle to be used
int aiowdm_card_open(const char *device_path);
int aiowdm_card_info_get(int dev_fd, uint16_t *device_id, unsigned short int *port_base, unsigned long *name_size, char *name);
int aiowdm_bar_resource_get(int dev_fd, unsigned int bar, struct resource *resource);
int aiowdm_wait_for_irq(int dev_fd);
int aiowdm_card_close(int dev_fd);

//Following all use the card's default I/O address as a base
//This is the address filled out in aiowdm_card_info_get and documented
//in the card's manual
//modeled after port io functions in sys/io.h
unsigned char aiowdm_inb(int dev_fd, unsigned short int port);
void aiowdm_outb(int dev_fd, unsigned char value, unsigned short int port);

unsigned short int aiowdm_inw(int dev_fd, unsigned short int port);
void aiowdm_outw(int dev_fd, unsigned short int value, unsigned short int port);

unsigned int aiowdm_inl(int dev_fd, unsigned short int port);
void aiowdm_outl(int dev_fd, unsigned int value, unsigned short int port);

void aiowdm_insb (int dev_fd, unsigned short int port, void *addr, unsigned long int count);
void aiowdm_outsb (int dev_fd, unsigned short int port, const void *addr, unsigned long int count);

void aiowdm_insw(int dev_fd, unsigned short int port, void *addr, unsigned long int count);
void aiowdm_outsw(int dev_fdm, unsigned short int port, const void *addr, unsigned long int count);

void aiowdm_insl(int dev_fd, unsigned short int port, void *addr, unsigned long int count);
void aiowdm_outsl (int dev_fd, unsigned short int port, const void *addr, unsigned long int count);
