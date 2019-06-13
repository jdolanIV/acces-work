#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/pci.h>
#include <linux/cdev.h>

#include "devices.h"

MODULE_DESCRIPTION("Linux counterpart to AIOWDM");
MODULE_AUTHOR("ACCES");
MODULE_LICENSE("GPL");

#define aio_driver_err_print(fmt, ...) \
				do { printk( "%s:%d:%s(): " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } while (0);

#ifndef AIO_DEBUG
#define AIO_DEBUG 0
#endif

#define aio_driver_debug_print(fmt, ...) \
				do { if (AIO_DEBUG) printk ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)

//TODO: remove when done with initial driver development
#define aio_driver_dev_print(fmt, ...) \
				do { if (AIO_DEBUG) printk ("%s:%d:%s(): " fmt "\n" , __FILE__, \
																__LINE__, __func__, ##__VA_ARGS__); } while (0)


int aio_driver_pci_probe (struct pci_dev *dev, const struct pci_device_id *id);
void aio_driver_pci_remove (struct pci_dev *dev);
int  aio_driver_pci_suspend (struct pci_dev *dev, pm_message_t state);
int  aio_driver_pci_suspend_late (struct pci_dev *dev, pm_message_t state);
int  aio_driver_pci_resume_early (struct pci_dev *dev);
int  aio_driver_pci_resume (struct pci_dev *dev);
void aio_driver_pci_shutdown (struct pci_dev *dev);
int  aio_driver_pci_sriov_configure (struct pci_dev *dev, int num_vfs);

static struct pci_driver aio_pci_driver = {
.name = "aio_driver_pci",
.probe = aio_driver_pci_probe,
.remove = aio_driver_pci_remove,
.suspend=aio_driver_pci_suspend,
.suspend_late=aio_driver_pci_suspend_late,
.resume_early=aio_driver_pci_resume_early,
.resume=aio_driver_pci_resume,
.shutdown=aio_driver_pci_shutdown,
.sriov_configure=aio_driver_pci_sriov_configure,
};

static struct pci_device_id *aio_driver_pci_device_table;
#define AIO_DRIVER_PCI_TABLE_SIZE sizeof(struct pci_device_id) * (NUM_ACCES_PCI_DEVICES + 1)

//structure to hold per device information
struct aio_device_context
{
  //TODO: verify we need to hold onto the pci_dev
  struct pci_dev pci_dev;
  struct cdev cdev;
  const struct aio_pci_dev_cfg *dev_cfg;
  void *bar_bases[6]; //use ioport_map() and ioremap() to get these
};

static int aio_driver_init(void)
{
	int status;
	int i;
	aio_driver_debug_print("Enter");

	aio_driver_pci_device_table = kmalloc(AIO_DRIVER_PCI_TABLE_SIZE, GFP_KERNEL);
	memset(aio_driver_pci_device_table, 0, AIO_DRIVER_PCI_TABLE_SIZE);

	for ( i = 0; i < NUM_ACCES_PCI_DEVICES ; i++)
	{
		aio_driver_pci_device_table[i].vendor = AIO_PCI_VEN_ID;
		aio_driver_pci_device_table[i].device = aio_pci_dev_table[i].pciDevId;
		aio_driver_pci_device_table[i].subvendor = PCI_ANY_ID;
		aio_driver_pci_device_table[i].subdevice = PCI_ANY_ID;
    aio_driver_pci_device_table[i].driver_data = (kernel_ulong_t)&aio_pci_dev_table[i];
	}

	aio_pci_driver.id_table = aio_driver_pci_device_table;

	status = pci_register_driver(&aio_pci_driver);
	if (status)
	{
		aio_driver_err_print("registration failure: %d", status);
		goto err_register;
	}
	return 0;

err_register:
	#warning magic number
	return -1;
}

static void aio_driver_exit(void)
{
	aio_driver_debug_print("Enter");
	pci_unregister_driver(&aio_pci_driver);
}

module_init(aio_driver_init);
module_exit(aio_driver_exit);

int aio_driver_pci_probe (struct pci_dev *dev, const struct pci_device_id *id)
{
  int status = 0;
  struct aio_device_context *context = NULL;
  int i = 0;
  unsigned long flags;
	aio_driver_debug_print("Enter");


	status = pci_request_regions(dev, "aio"); //TODO: Figure out if res_name should be defined somewhere

  if (status)
  {
    aio_driver_err_print("pci_request_region failed. status = 0x%x", status);
    goto err_out;
  }

  context = kmalloc(sizeof(struct aio_device_context), GFP_KERNEL);

  for (i = 0; i < 6 ; i++) //TODO:look into magic number for BARs
  {
    flags = pci_resource_flags(dev, i);
    if ( flags & ( IORESOURCE_IO | IORESOURCE_MEM))
    {
      context->bar_bases[i] = pci_iomap(dev, i, 0);
      if (NULL == context->bar_bases[i])
      {
        aio_driver_err_print("Could not map bar %d", i);
      }
      aio_driver_dev_print("attempted mapping of bar %d", i);
    }
  }

  i = 0;

  do
  {
    if (aio_pci_dev_table[i].pciDevId == dev->device)
    {
      context->dev_cfg = &aio_pci_dev_table[i];
      aio_driver_dev_print("Found config entry");
    }
    i++;
  }while(NULL == context->dev_cfg && i < NUM_ACCES_PCI_DEVICES);

  alloc_chrdev_region(



err_out:
	return status;
}

void aio_driver_pci_remove (struct pci_dev *dev)
{

  pci_release_selected_regions(dev, 0x3f);
	return;
}

int  aio_driver_pci_suspend (struct pci_dev *dev, pm_message_t state)
{
  aio_driver_err_print("Entering stub");
  return 0;
}

int  aio_driver_pci_suspend_late (struct pci_dev *dev, pm_message_t state)
{
  aio_driver_err_print("Entering stub");
  return 0;
}

int  aio_driver_pci_resume_early (struct pci_dev *dev)
{
  aio_driver_err_print("Entering stub");
  return 0;
}

int  aio_driver_pci_resume (struct pci_dev *dev)
{
  aio_driver_err_print("Entering stub");
  return 0;
}

void aio_driver_pci_shutdown (struct pci_dev *dev)
{
  aio_driver_err_print("Entering stub");
}
int  aio_driver_pci_sriov_configure (struct pci_dev *dev, int num_vfs)
{
  aio_driver_err_print("Entering stub");
  return 0;
}