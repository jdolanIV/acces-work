<<<<<<< HEAD
#include <stdint.h>
//returns handle to be used. device name is name in /dev/aiowdm
int aiowdm_card_open(const char *fname);
int aiowdm_card_info_get(int handle, uint16_t *device_id, unsigned short int *port_base, size_t port_size, unsigned long *name_size, char *name);
//int aiowdm_bar_resource_get(int handle, unsigned int bar, struct resource *resource);

int aiowdm_irq_enable(int handle);
int aiowdm_irq_disable(int handle);
int aiowdm_irq_wait(int handle);
int aiowdm_irq_wait_cancel(int def_fd);

int aiowdm_card_close(int handle);
=======
#include <linux/ioport.h>
//returns handle to be used
int aiowdm_card_open(const char *device_path);
int aiowdm_card_info_get(int dev_fd, uint16_t *device_id, unsigned short int *port_base, unsigned long *name_size, char *name);
int aiowdm_bar_resource_get(int dev_fd, unsigned int bar, struct resource *resource);

int aiowdm_irq_enable(int, dev_fd);
int aiowdm_irq_disable(
int aiowdm_irq_wait(int dev_fd);
int aiowdm_irq_wait_cancel(int def_fd);

int aiowdm_card_close(int dev_fd);
>>>>>>> aiowdm-port

//Following all use the card's default I/O address as a base
//This is the address filled out in aiowdm_card_info_get and documented
//in the card's manual
//modeled after port io functions in sys/io.h
<<<<<<< HEAD
unsigned char aiowdm_inb(int handle, unsigned short int port);
void aiowdm_outb(int handle, unsigned char value, unsigned short int port);

unsigned short int aiowdm_inw(int handle, unsigned short int port);
void aiowdm_outw(int handle, unsigned short int value, unsigned short int port);

unsigned int aiowdm_inl(int handle, unsigned short int port);
void aiowdm_outl(int handle, unsigned int value, unsigned short int port);

void aiowdm_insb (int handle, unsigned short int port, void *addr, unsigned long int count);
void aiowdm_outsb (int handle, unsigned short int port, const void *addr, unsigned long int count);

void aiowdm_insw(int handle, unsigned short int port, void *addr, unsigned long int count);
void aiowdm_outsw(int handlem, unsigned short int port, const void *addr, unsigned long int count);

void aiowdm_insl(int handle, unsigned short int port, void *addr, unsigned long int count);
void aiowdm_outsl (int handle, unsigned short int port, const void *addr, unsigned long int count);
=======
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
>>>>>>> aiowdm-port
