
#ifdef 		MODVERSIONS
    #ifndef 	MODULE
        #define 	MODULE
    #endif
#endif

#include <linux/version.h>
#define	VERSION_CODE(ver,rel,seq)	((ver << 16) | (rel << 8) | seq)

#ifdef MODULE
    #if (LINUX_VERSION_CODE < VERSION_CODE(2,6,18))
        #include <linux/config.h>
    #endif
    #ifdef MODVERSIONS
        #include <linux/modversions.h>
    #endif
    #include <linux/module.h>
#else
    #define	MOD_INC_USE_COUNT
    #define MOD_DEC_USE_COUNT
#endif



#define DRIVER_NAME         "PTLSER : "


#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,32))
#include <linux/autoconf.h>
#endif

#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_reg.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/ptrace.h>
#include <linux/ioport.h>
#include <linux/mm.h>
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,32))
#include <linux/autoconf.h>
#endif
#include <linux/delay.h>

#if (LINUX_VERSION_CODE < VERSION_CODE(3,4,0))
#include <asm/system.h>
#endif
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/segment.h>
#include <asm/bitops.h>

#include "ptlser.h"
#include "register.h"
#ifdef CONFIG_PCI
    #include <linux/pci.h>
#endif

#define	PTLSER_VERSION	"1.03"
#define	PTLSERMAJOR	31
#define	PTLSERCUMAJOR	34



enum	{
	PTLSER_PCIE_PORT_8 = 1,
	PTLSER_PCIE_PORT_4,
	PTLSER_PCIE_PORT_2,
	PTLSER_PCIE_PORT_1,
	PTLSER_PCI_PORT_8,
	PTLSER_PCI_PORT_4,
	PTLSER_PCI_PORT_2,
	PTLSER_PCI_PORT_1,

};

static char *ptlser_brdname[] = {
	"Pericom PI7C9X7958 PCI Express UART Device (8 ports)",
	"Pericom PI7C9X7954 PCI Express UART Device (4 ports)",
	"Pericom PI7C9X7952 PCI Express UART Device (2 ports)",
	"Pericom PI7C9X7951 PCI Express UART Device (1 port)",
	"Pericom PI7C9X8958 PCI UART Device (8 ports)",
	"Pericom PI7C9X8954 PCI UART Device (4 ports)",
	"Pericom PI7C9X8952 PCI UART Device (2 ports)",
	"Pericom PI7C9X8951 PCI UART Device (1 port)",
	
};

static int ptlser_numports[] = {
	8,
	4,
	2,
	1,	
	8,
	4,
	2,
	1,		
};


#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
typedef struct {
	unsigned short	vendor;
	unsigned short	device;
	unsigned short	driver_data;
}	ptlser_pciinfo;

static ptlser_pciinfo	ptlser_pcibrds[] = {
	{PTLSER_VENDOR_ID, PTLSER_DEVICE_7958, PTLSER_PCIE_PORT_8},
	{PTLSER_VENDOR_ID, PTLSER_DEVICE_7954, PTLSER_PCIE_PORT_4},
	{PTLSER_VENDOR_ID, PTLSER_DEVICE_7952, PTLSER_PCIE_PORT_2},
	{PTLSER_VENDOR_ID, PTLSER_DEVICE_7951, PTLSER_PCIE_PORT_1},
	{PTLSER_VENDOR_ID, PTLSER_DEVICE_8958, PTLSER_PCI_PORT_8},
	{PTLSER_VENDOR_ID, PTLSER_DEVICE_8954, PTLSER_PCI_PORT_4},
	{PTLSER_VENDOR_ID, PTLSER_DEVICE_8952, PTLSER_PCI_PORT_2},
	{PTLSER_VENDOR_ID, PTLSER_DEVICE_8951, PTLSER_PCI_PORT_1},	
	{0}
};
 
#else

static  struct pci_device_id	ptlser_pcibrds[] = {
    {PTLSER_VENDOR_ID, PTLSER_DEVICE_7958, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCIE_PORT_8},
    {PTLSER_VENDOR_ID, PTLSER_DEVICE_7954, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCIE_PORT_4},
    {PTLSER_VENDOR_ID, PTLSER_DEVICE_7952, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCIE_PORT_2},
    {PTLSER_VENDOR_ID, PTLSER_DEVICE_7951, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCIE_PORT_1},
    {PTLSER_VENDOR_ID, PTLSER_DEVICE_8958, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCI_PORT_8},
    {PTLSER_VENDOR_ID, PTLSER_DEVICE_8954, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCI_PORT_4},
    {PTLSER_VENDOR_ID, PTLSER_DEVICE_8952, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCI_PORT_2},
    {PTLSER_VENDOR_ID, PTLSER_DEVICE_8951, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCI_PORT_1},
		{0x494f, 0x10dc, PCI_ANY_ID, PCI_ANY_ID, 0, 0, PTLSER_PCI_PORT_4 },
	{0}
};

MODULE_DEVICE_TABLE(pci, ptlser_pcibrds);
#endif

typedef struct _ptlser_pci_info {
	unsigned short busNum;
	unsigned short devNum;
struct pci_dev	*pdev;	
} ptlser_pci_info;

static int              ttymajor=PTLSERMAJOR;
static int              calloutmajor=PTLSERCUMAJOR;
static unsigned char    interface=0;
static unsigned char    terminator=0;


#ifdef MODULE
/* Variables for insmod */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("simoncc");
MODULE_DESCRIPTION("Pericom PI7C9X795x/PI7C9X895x UART Device Driver");


#if defined SP2 || (LINUX_VERSION_CODE > VERSION_CODE(2,6,9))
int ptlser_ioaddr_array_num;
module_param(ttymajor, int, 0);
module_param(calloutmajor, int, 0);
module_param(interface, byte, 0);
module_param(terminator, byte, 0);
#else
MODULE_PARM(ttymajor,       "i");
MODULE_PARM(calloutmajor,   "i");
MODULE_PARM(interface, 	    "b");
MODULE_PARM(terminator,	    "b");
#endif

#endif /* MODULE */


struct ptlser_hwconf {
	int		        board_type;
	int		        ports;
	int		        irq;
	unsigned long	vector_mask;
	int		        uart_type;
	unsigned char	*ioaddr[PTLSER_PORTS_PER_BOARD];
	unsigned      io_port_addr[PTLSER_PORTS_PER_BOARD];
	int		        baud_base[PTLSER_PORTS_PER_BOARD];
	ptlser_pci_info	pciInfo;
	int		        MaxCanSetBaudRate[PTLSER_PORTS_PER_BOARD];
	unsigned long	*opmode_ioaddr[PTLSER_PORTS_PER_BOARD];
};


struct ptlser_struct {
	
#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))
	struct tty_port ttyPort;
#endif
	int			        port;
	unsigned char		*base;		/* port base address */
	unsigned        io_base;
	int			        irq;		/* port using irq no. */
	int			        baud_base;	/* max. speed */
	int			        flags;		/* defined in tty.h */
	int			        type;		/* UART type */
	struct tty_struct   *tty;
	int			        read_status_mask;
	int			        ignore_status_mask;
	int			        xmit_fifo_size;
	int			        custom_divisor;
	int			        close_delay;
	unsigned short		closing_wait;
	int			        IER;		/* Interrupt Enable Register */
	int			        MCR;		/* Modem control register */
	unsigned long		event;
	int			        count;		/* # of fd on device */
	int			        blocked_open;	/* # of blocked opens */
	unsigned char		*xmit_buf;
	int			        xmit_head;
	int			        xmit_tail;
	int			        xmit_cnt;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))	
	struct tq_struct	tqueue;
#else
	struct work_struct tqueue;
#endif	
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,19))
	struct ktermios		normal_termios;
	struct ktermios		callout_termios;
#else
	struct termios		normal_termios;
	struct termios		callout_termios;
#endif
	wait_queue_head_t   open_wait;
	wait_queue_head_t   close_wait;
	wait_queue_head_t   delta_msr_wait;
	struct async_icount	icount; 	/* kernel counters for the 4 input interrupts */
	int			        timeout;
	int			        MaxCanSetBaudRate;	
	long                realbaud;
//	unsigned char       err_shadow;
	spinlock_t		    slock;
	int			        speed;
	int			        custom_baud_rate;
	int			        board_type;
	unsigned char 		intr_status;
	unsigned char		lsr_status;
};


#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
static struct tty_driver	ptlser_var_sdriver;
static struct tty_driver	ptlser_var_cdriver;
static int			        ptlser_var_refcount;
#else
static struct tty_driver	*ptlser_var_sdriver;
#endif


#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,19))
static struct ktermios      *ptlser_var_termios[PTLSER_PORTS+1];
#if (LINUX_VERSION_CODE < VERSION_CODE(3,3,0))
static struct ktermios 		*ptlser_var_termios_locked[PTLSER_PORTS+1];
#endif
#else
static struct termios       *ptlser_var_termios[PTLSER_PORTS+1];
static struct termios       *ptlser_var_termios_locked[PTLSER_PORTS+1];
#endif

static struct ptlser_struct	ptlser_var_table[PTLSER_PORTS];
static struct tty_struct    *ptlser_var_tty[PTLSER_PORTS+1];


/*
 * This is used to figure out the divisor speeds and the timeouts
 */

static int ptlser_baud_table[] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600, 0 };
#define BAUD_TABLE_NO	(sizeof(ptlser_baud_table)/sizeof(int))

struct ptlser_hwconf ptlsercfg[PTLSER_BOARDS];

/*
 * static functions:
 */

#ifdef MODULE
int		init_module(void);
void	cleanup_module(void);
#endif

static int	ptlser_open(struct tty_struct *, struct file *);
static void	ptlser_close(struct tty_struct *, struct file *);

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,10))
static int	ptlser_write(struct tty_struct *, int from_user, const unsigned char *, int);
#else
static int	ptlser_write(struct tty_struct *, const unsigned char *, int);
#endif
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,25))
static int	ptlser_put_char(struct tty_struct *, unsigned char);
#else
static void	ptlser_put_char(struct tty_struct *, unsigned char);
#endif
static void	ptlser_flush_chars(struct tty_struct *);
static int	ptlser_write_room(struct tty_struct *);
static int	ptlser_chars_in_buffer(struct tty_struct *);
static void	ptlser_flush_buffer(struct tty_struct *);
//static int	ptlser_ioctl(struct tty_struct *, struct file *, uint, ulong);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,39))
static int ptlser_ioctl(struct tty_struct * tty, struct file * file, unsigned int cmd, unsigned long arg);
#else
static int ptlser_ioctl(struct tty_struct * tty, unsigned int cmd, unsigned long arg);
#endif
static void	ptlser_throttle(struct tty_struct *);
static void	ptlser_unthrottle(struct tty_struct *);
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,19))
static void	ptlser_set_termios(struct tty_struct *, struct ktermios *);
#else
static void	ptlser_set_termios(struct tty_struct *, struct termios *);
#endif
static void	ptlser_stop(struct tty_struct *);
static void	ptlser_start(struct tty_struct *);
static void	ptlser_hangup(struct tty_struct *);

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
static int	ptlser_tiocmget(struct tty_struct */*, struct file **/);
static int	ptlser_tiocmset(struct tty_struct */*, struct file **/, unsigned int, unsigned int);
#else
static int	ptlser_get_modem_info(struct ptlser_struct *, unsigned int *);
static int	ptlser_set_modem_info(struct ptlser_struct *, unsigned int, unsigned int *);
#endif

#if (LINUX_VERSION_CODE >= 131394 && LINUX_VERSION_CODE < VERSION_CODE(2,6,26))
static void	ptlser_break_ctl(struct tty_struct *, int);
static void 	ptlser_wait_until_sent(struct tty_struct *tty, int timeout);
#else
static int	ptlser_break_ctl(struct tty_struct *, int);
static void 	ptlser_wait_until_sent(struct tty_struct *tty, int timeout);
#endif

#ifdef CONFIG_PCI
static int      ptlser_get_PCI_conf(int ,int ,int ,struct ptlser_hwconf *);
#endif

#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,19))
static void     ptlser_do_softint(struct work_struct *work);
#else
static void	ptlser_do_softint(void *);
#endif
//static int	ptlser_ioctl_special(unsigned int, unsigned long);
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,18))
static irqreturn_t ptlser_interrupt(int irq, void *dev_id);
#else
static IRQ_RET ptlser_interrupt(int, void *, struct pt_regs *);
#endif
static void	ptlser_receive_chars(struct ptlser_struct *, int *);
static void	ptlser_transmit_chars(struct ptlser_struct *);
static void	ptlser_check_modem_status(struct ptlser_struct *, int);
static int	ptlser_block_til_ready(struct tty_struct *, struct file *, struct ptlser_struct *);
static int	ptlser_startup(struct ptlser_struct *);
static void	ptlser_shutdown(struct ptlser_struct *);
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,19))
static int	ptlser_change_speed(struct ptlser_struct *, struct ktermios *old_termios);
#else
static int	ptlser_change_speed(struct ptlser_struct *, struct termios *old_termios);
#endif
static int	ptlser_get_serial_info(struct ptlser_struct *, struct serial_struct *);
static int	ptlser_set_serial_info(struct ptlser_struct *, struct serial_struct *);
static int	ptlser_get_lsr_info(struct ptlser_struct *, unsigned int *);
static void	ptlser_send_break(struct ptlser_struct *, int);
static int ptlser_set_baud_ex(struct ptlser_struct *info, long newspd);
static int	ptlser_set_baud(struct ptlser_struct *info, long newspd);



#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
static struct tty_operations ptlser_ops = {
	.open               = ptlser_open,
	.close              = ptlser_close,
	.write              = ptlser_write,
	.put_char           = ptlser_put_char,
	.flush_chars        = ptlser_flush_chars,
	.write_room         = ptlser_write_room,
	.chars_in_buffer    = ptlser_chars_in_buffer,
	.flush_buffer       = ptlser_flush_buffer,
	.ioctl              = ptlser_ioctl,
	.throttle           = ptlser_throttle,
	.unthrottle         = ptlser_unthrottle,
	.set_termios        = ptlser_set_termios,
	.stop               = ptlser_stop,
	.start              = ptlser_start,
	.hangup             = ptlser_hangup,
	.tiocmget           = ptlser_tiocmget,
	.tiocmset           = ptlser_tiocmset,
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,26))
	.break_ctl          = ptlser_break_ctl,
	.wait_until_sent    = ptlser_wait_until_sent,
#endif
};
#endif


static int ptlser_get_PCI_conf(int busnum, int devnum, int board_type, struct ptlser_hwconf *hwconf)
{
	int		i;
	unsigned char	*ioaddress;
	struct pci_dev	*pdev=hwconf->pciInfo.pdev;


	//io address
	hwconf->board_type = board_type;
	hwconf->ports = ptlser_numports[board_type-1];

	request_mem_region(pci_resource_start(pdev, 1), pci_resource_len(pdev, 1), "ptlser(MEM)");
	ioaddress = ioremap(pci_resource_start(pdev,1), pci_resource_len(pdev,1));
	printk (DRIVER_NAME "ioaddress = %p, start=0x%llx ", ioaddress, pci_resource_start(pdev,1));

	request_region(pci_resource_start(pdev, 0), pci_resource_len(pdev, 0), "ptlser(IO)");

	printk (DRIVER_NAME "region zero start=0x%llx ", pci_resource_start(pdev,0));
	
	for (i = 0; i < hwconf->ports; i++) {
		hwconf->ioaddr[i] = ioaddress + (i * PTLSER_MEMORY_RANGE);
		hwconf->io_port_addr[i] = pci_resource_start(pdev, 0) + (i * 8);
		if(i == 3){
			switch(board_type){
			case PTLSER_PCIE_PORT_4:
				hwconf->ioaddr[i] = ioaddress + (PTLSER_PORT8 * PTLSER_MEMORY_RANGE);
				hwconf->io_port_addr[i] = pci_resource_start(pdev, 0) + 0x38;
				break;
			default:
				break;
			}
		}
	}

	//irq
	hwconf->irq = hwconf->pciInfo.pdev->irq;
	hwconf->uart_type = PORT_16550A;
	
    for (i = 0; i < hwconf->ports; i++) {
		hwconf->baud_base[i] = hwconf->MaxCanSetBaudRate[i] = 921600;
	}

	return 0;
}


#if (LINUX_VERSION_CODE >  VERSION_CODE(2,6,19))
static void ptlser_do_softint(struct work_struct *work)
{
        struct ptlser_struct *info = container_of(work, struct ptlser_struct, tqueue);
#else
static void ptlser_do_softint(void *private_)
{
	struct ptlser_struct *info = (struct ptlser_struct *)private_;
#endif
	struct tty_struct *	tty = info->tty;

#if (LINUX_VERSION_CODE <  VERSION_CODE(2,6,9))
	if ( test_and_clear_bit(PTLSER_EVENT_TXLOW, &info->event) ) {
		if ( (tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) && tty->ldisc.write_wakeup )
			(tty->ldisc.write_wakeup)(tty);
	        wake_up_interruptible(&tty->write_wait);
	}
#else
	if ( test_and_clear_bit(PTLSER_EVENT_TXLOW, &info->event) ) 
		tty_wakeup(tty);
#endif	    	    
	if ( test_and_clear_bit(PTLSER_EVENT_HANGUP, &info->event) )
		tty_hangup(tty);

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
	PTLSER_MOD_DEC;
#endif
}

unsigned char getTxCount(struct ptlser_struct *	info)
{
	unsigned char value;
//	printk(DRIVER_NAME "Reading PTLSER_LTF_OFFSET\n");
	value = PTLSER_READ_REG(info->base + PTLSER_LTF_OFFSET);
	//printk(DRIVER_NAME "PTLSER_LTF_OFFSET\n = %d", value);
	if (value > PTLSER_FIFO_SIZE)
		value = PTLSER_FIFO_SIZE;
	return value;
}

unsigned char getRxCount(struct ptlser_struct *	info)
{
	unsigned char value;
	value = PTLSER_READ_REG(info->base + PTLSER_LRF_OFFSET);
	if (value > PTLSER_FIFO_SIZE)
		value = PTLSER_FIFO_SIZE;
	return value;
}


/*
 * This routine is called whenever a serial port is opened.  It
 * enables interrupts for a serial port, linking in its async structure into
 * the IRQ chain.   It also performs the serial-specific
 * initialization for the tty structure.
 */
static int ptlser_open(struct tty_struct * tty, struct file * filp)
{
	struct ptlser_struct *	info;
	int			retval, line;

	PTLSER_LOCK_INIT();
	
	line = PORTNO(tty);

	if ( line == PTLSER_PORTS )
		return 0;
	    
    	PTLSER_MOD_INC;
    	    
	if ( (line < 0) || (line > PTLSER_PORTS) )
		return -ENODEV;

	info = ptlser_var_table + line;

	if ( !info->base )
	    return -ENODEV;

	tty->driver_data = info;
	info->tty = tty;

	/*
	 * Start up serial port
	 */
	info->count++;
  printk(DRIVER_NAME "port %d  ptlser_open (jiff=%lu).\n", info->port, jiffies);
	retval = ptlser_startup(info);

	if ( retval )
	    return retval;

	retval = ptlser_block_til_ready(tty, filp, info);

	if ( retval )
	    return retval;

	if (info->count == 1) {
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	    if ( PTLSER_TTY_DRV(subtype) == SERIAL_TYPE_NORMAL )
		*tty->termios = info->normal_termios;
	    else
		*tty->termios = info->callout_termios;
#else
	    if ( PTLSER_TTY_DRV(subtype) == SERIAL_TYPE_NORMAL )
		(*tty).termios = info->normal_termios;
	    else
		(*tty).termios = info->callout_termios;	
#endif


	    PTLSER_LOCK(&info->slock);
	    ptlser_change_speed(info, 0);
	    PTLSER_UNLOCK(&info->slock);
	}

#ifdef TTY_DONT_FLIP
	clear_bit(TTY_DONT_FLIP, &tty->flags); // since VERSION_CODE >= 2.6.18
#endif

/* unmark here for very high baud rate (ex. 921600 bps) used
*/
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,1,0))
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	tty->low_latency = 1;
#endif
#endif
	printk(DRIVER_NAME "open completed\n");
	return 0;
}


/*
 * This routine is called when the serial port gets closed.  First, we
 * wait for the last remaining data to be sent.  Then, we unlink its
 * async structure from the interrupt chain if necessary, and we free
 * that IRQ if nothing is left in the chain.
 */
static void ptlser_close(struct tty_struct * tty, struct file * filp)
{
	struct ptlser_struct * info = (struct ptlser_struct *)tty->driver_data;
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,9))
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,28))
	struct tty_ldisc *ld;
#endif
#endif
	unsigned long timeout;
	unsigned char reg_flag;	
  printk(DRIVER_NAME "port %d  ptlser_close (jiff=%lu).\n", info->port, jiffies);
	PTLSER_LOCK_INIT();

	if ( PORTNO(tty) == PTLSER_PORTS )
	    return;

	if ( !info ){
	    PTLSER_MOD_DEC;

	    return;
	}

	PTLSER_LOCK(&info->slock);

	if ( tty_hung_up_p(filp) ) {
	    PTLSER_UNLOCK(&info->slock);
	    PTLSER_MOD_DEC;

	    return;
	}

#ifndef SP1
	if ( (tty->count == 1) && (info->count != 1) ) {
#else
#if (LINUX_VERSION_CODE < VERSION_CODE(2,4,21))
	if ( (tty->count == 1) && (info->count != 1) ) {
#else
	if ((atomic_read(&tty->count) == 1) && (info->count != 1)) {
#endif

#endif /* SP1 */
	    /*
	     * Uh, oh.	tty->count is 1, which means that the tty
	     * structure will be freed.  Info->count should always
	     * be one in these conditions.  If it's greater than
	     * one, we've got real problems, since it means the
	     * serial port won't be shutdown.
	     */
	    //printk(DRIVER_NAME "ptlser_close: bad serial port count; tty->count is 1, "
		//   "info->count is %d\n", info->count);
	    info->count = 1;
	}

	if ( --info->count < 0 ) {
	    //printk(DRIVER_NAME "ptlser_close: bad serial port count for ttyPPU%d: %d\n",
		//   info->port, info->count);
	    info->count = 0;
	}

	if ( info->count ) {
	    PTLSER_UNLOCK(&info->slock);
	    PTLSER_MOD_DEC;

	    return;
	}

	info->flags |= ASYNC_CLOSING;
	PTLSER_UNLOCK(&info->slock);
	/*
	 * Save the termios structure, since this port may have
	 * separate termios for callout and dialin.
	 */
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	if ( info->flags & ASYNC_NORMAL_ACTIVE )
	    info->normal_termios = *tty->termios;

	if ( info->flags & ASYNC_CALLOUT_ACTIVE )
	    info->callout_termios = *tty->termios;
#else
	if ( info->flags & ASYNC_NORMAL_ACTIVE )
	    info->normal_termios = (*tty).termios;

	if ( info->flags & ASYNC_CALLOUT_ACTIVE )
	    info->callout_termios = (*tty).termios;
#endif
	/*
	 * Now we wait for the transmit buffer to clear; and we notify
	 * the line discipline to only process XON/XOFF characters.
	 */
	
	reg_flag = PTLSER_READ_REG(info->base + PTLSER_EFR_OFFSET);
	reg_flag &= ~PTLSER_EFR_AUTO_RTS;
	PTLSER_WRITE_REG(reg_flag, info->base + PTLSER_EFR_OFFSET);
	tty->closing = 1;

	if ( info->closing_wait != ASYNC_CLOSING_WAIT_NONE )
	    tty_wait_until_sent(tty, info->closing_wait);
	/*
	 * At this point we stop accepting input.  To do this, we
	 * disable the receive line status interrupts, and tell the
	 * interrupt driver to stop checking the data ready bit in the
	 * line status register.
	 */
	info->IER &= ~UART_IER_RLSI;

	if ( info->flags & ASYNC_INITIALIZED ) {
		PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	    /*
	     * Before we drop DTR, make sure the UART transmitter
	     * has completely drained; this is especially
	     * important if there is a transmit FIFO!
	     */
		timeout = jiffies + HZ;
		while ( ptlser_chars_in_buffer(tty) ) {
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(5);
			set_current_state(TASK_RUNNING);

			if ( time_after(jiffies, timeout) )
				break;
		    }
	}

	ptlser_shutdown(info);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,26))
	if ( PTLSER_TTY_DRV(flush_buffer) )
		PTLSER_TTY_DRV(flush_buffer)(tty);
#else
	//tty_driver_flush_buffer(tty);
	//ptlser_flush_buffer(tty);
	
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,10))
	if ( tty->ldisc.flush_buffer )
		tty->ldisc.flush_buffer(tty);
#else	    
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,27))
	ld = tty_ldisc_ref(tty);
	if (ld) {
		if(ld->flush_buffer)
			ld->flush_buffer(tty);

		tty_ldisc_deref(ld);
	}
#else
	tty_ldisc_flush(tty);
#endif
#endif	    
	    
	tty->closing = 0;
	info->event = 0;
	info->tty = 0;

	if ( info->blocked_open ) {
		if ( info->close_delay ) {
			set_current_state(TASK_INTERRUPTIBLE);
			schedule_timeout(info->close_delay);
			set_current_state(TASK_RUNNING);
		}

		wake_up_interruptible(&info->open_wait);
	}

	info->flags &= ~(ASYNC_NORMAL_ACTIVE | ASYNC_CALLOUT_ACTIVE | ASYNC_CLOSING);
	wake_up_interruptible(&info->close_wait);

	PTLSER_MOD_DEC;
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,10))
static int ptlser_write(struct tty_struct * tty, int from_user, const unsigned char * buf, int count)
#else
static int ptlser_write(struct tty_struct * tty, const unsigned char * buf, int count)
#endif
{
	int c, total = 0;
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
//printk(DRIVER_NAME "port %d  ptlser_write (jiff=%lu).\n", info->port, jiffies);	 
	PTLSER_LOCK_INIT();

	if ( !tty || !info->xmit_buf  )
	    return(0);

	
	while ( 1 ) {
		c = MIN(count, MIN(SERIAL_XMIT_SIZE - info->xmit_cnt - 1,
			       SERIAL_XMIT_SIZE - info->xmit_head));
		if ( c <= 0 )
			break;

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,10))
		if ( from_user ) {
			if(copy_from_user(info->xmit_buf+info->xmit_head, buf, c)==c){
				total = -EFAULT;	
				break;
			}
		
		} else
			memcpy(info->xmit_buf + info->xmit_head, buf, c);
#else
			memcpy(info->xmit_buf + info->xmit_head, buf, c);
#endif
		PTLSER_LOCK(&info->slock);    
		info->xmit_head = (info->xmit_head + c) & (SERIAL_XMIT_SIZE - 1);
		info->xmit_cnt += c;
		PTLSER_UNLOCK(&info->slock);

		buf += c;
		count -= c;
		total += c;
	}

	if ( info->xmit_cnt && !tty->stopped ) {
        	    PTLSER_LOCK(&info->slock); 	
		    info->IER &= ~UART_IER_THRI	;
	            PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	            info->IER |= UART_IER_THRI;
	            PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	            PTLSER_UNLOCK(&info->slock);
	}
	
	
	return total;
}

#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,25))
static int ptlser_put_char(struct tty_struct * tty, unsigned char ch)
#else
static void ptlser_put_char(struct tty_struct * tty, unsigned char ch)
#endif
{
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	PTLSER_LOCK_INIT();

	if ( !tty || !info->xmit_buf )
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,25))
		return 0;
#else
		return;
#endif

	if ( info->xmit_cnt >= SERIAL_XMIT_SIZE - 1 )
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,25))
		return 0;
#else
		return;
#endif
	PTLSER_LOCK(&info->slock);
	info->xmit_buf[info->xmit_head++] = ch;
	info->xmit_head &= SERIAL_XMIT_SIZE - 1;
	info->xmit_cnt++;
	PTLSER_UNLOCK(&info->slock);

	if ( !tty->stopped ) {
        	if (!tty->hw_stopped) {
        		PTLSER_LOCK(&info->slock);
		        info->IER &= ~UART_IER_THRI	;
	                PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	    		info->IER |= UART_IER_THRI;
	    		PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	    		PTLSER_UNLOCK(&info->slock);
		}
	}
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,25))
	return 1;
#endif

}


static void ptlser_flush_chars(struct tty_struct * tty)
{
	char fcr;
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	PTLSER_LOCK_INIT();

	if ( info->xmit_cnt <= 0 || tty->stopped || !info->xmit_buf)
		return;
	PTLSER_LOCK(&info->slock);
        info->IER &= ~UART_IER_THRI;
        PTLSER_WRITE_REG(info->IER, info->base + UART_IER);

//printk(DRIVER_NAME "port %d  flush_chars (jiff=%lu).\n", info->port, jiffies);
	//simoncc ++++++++++++++++++++
/*
	PTLSER_WRITE_REG(0xFF, info->base + 0x24);
	PTLSER_WRITE_REG(0x01, info->base + 0x25);
	while( getTxCount(info) ) {
		PTLSER_WRITE_REG(PTLSER_FCR_TX_FLUSH, info->base + UART_FCR);
		PTLSER_WRITE_REG(PTLSER_FCR_TX_FLUSH, info->base + UART_FCR);
	} 
	while( getRxCount(info) ) {
		PTLSER_WRITE_REG(PTLSER_FCR_RX_FLUSH, info->base + UART_FCR);
		PTLSER_WRITE_REG(PTLSER_FCR_RX_FLUSH, info->base + UART_FCR);
	} 
	PTLSER_WRITE_REG(0x00, info->base + 0x24);
	PTLSER_WRITE_REG(0x02, info->base + 0x25);
	//simoncc --------------------
	PTLSER_WRITE_REG(fcr, info->base+UART_FCR);
*/
	info->IER |= UART_IER_THRI;
	PTLSER_WRITE_REG(info->IER, info->base + UART_IER);

	PTLSER_UNLOCK(&info->slock);
}


static int ptlser_write_room(struct tty_struct * tty)
{
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	int	ret;

	ret = SERIAL_XMIT_SIZE - info->xmit_cnt - 1;

	if ( ret < 0 )
	    ret = 0;
	    
	return ret;
}


static int ptlser_chars_in_buffer(struct tty_struct * tty)
{
	int len;
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	unsigned char t_cnt;
	
	len = info->xmit_cnt;
	t_cnt = getTxCount(info);

	if(t_cnt)
		len+=(int)t_cnt;

	return len;
}


static void ptlser_flush_buffer(struct tty_struct * tty)
{
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	char fcr;
	PTLSER_LOCK_INIT();

	PTLSER_LOCK(&info->slock);
	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
	PTLSER_UNLOCK(&info->slock);
//printk(DRIVER_NAME "port %d  flush_buffer (jiff=%lu).\n", info->port, jiffies);	
//	fcr = PTLSER_READ_REG(info->base + UART_FCR);
	//simoncc ++++++++++++++++++++

	PTLSER_WRITE_REG(0xFF, info->base + 0x24);
	PTLSER_WRITE_REG(0x01, info->base + 0x25);
	while( getTxCount(info) ) {
		PTLSER_WRITE_REG(PTLSER_FCR_TX_FLUSH, info->base + UART_FCR);
		PTLSER_WRITE_REG(PTLSER_FCR_TX_FLUSH, info->base + UART_FCR);
	} 
	while( getRxCount(info) ) {
		PTLSER_WRITE_REG(PTLSER_FCR_RX_FLUSH, info->base + UART_FCR);
		PTLSER_WRITE_REG(PTLSER_FCR_RX_FLUSH, info->base + UART_FCR);
	} 
	PTLSER_WRITE_REG(0x00, info->base + 0x24);
	PTLSER_WRITE_REG(0x02, info->base + 0x25);

	//simoncc --------------------
//	PTLSER_WRITE_REG(fcr, info->base+UART_FCR);

	wake_up_interruptible(&tty->write_wait);

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,27))
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) && tty->ldisc.write_wakeup)
		(tty->ldisc.write_wakeup)(tty);
#else
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,31))
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) && tty->ldisc.ops->write_wakeup)
		tty_wakeup(tty);
#else
	if ((tty->flags & (1 << TTY_DO_WRITE_WAKEUP)) && tty->ldisc->ops->write_wakeup)
		tty_wakeup(tty);
#endif
#endif
}

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,39))
static int ptlser_ioctl(struct tty_struct * tty, struct file * file, unsigned int cmd, unsigned long arg)
#else
static int ptlser_ioctl(struct tty_struct * tty, unsigned int cmd, unsigned long arg)
#endif
{
	int			error;
	struct ptlser_struct *	info = (struct ptlser_struct *)tty->driver_data;
	int			retval;
	struct async_icount	cprev, cnow;	    /* kernel counter temps */
	struct serial_icounter_struct *p_cuser;     /* user space */
	unsigned long 		templ;
	PTLSER_LOCK_INIT();

	if ( (cmd != TIOCGSERIAL) && (cmd != TIOCMIWAIT) &&
	     (cmd != TIOCGICOUNT) ) {

		if ( tty->flags & (1 << TTY_IO_ERROR) )
			return -EIO;
	}

	switch ( cmd ) {
		case TCSBRK:	/* SVID version: non-zero arg --> no break */
			retval = tty_check_change(tty);

			if ( retval )
				return retval;

			tty_wait_until_sent(tty, 0);

			if ( !arg )
				ptlser_send_break(info, HZ/4);		/* 1/4 second */

			return 0;
		case TCSBRKP:	/* support for POSIX tcsendbreak() */
			retval = tty_check_change(tty);

			if ( retval )
				return retval;

			tty_wait_until_sent(tty, 0);
			ptlser_send_break(info, arg ? arg*(HZ/10) : HZ/4);

			return 0;
		case TIOCGSOFTCAR:
			error = PTLSER_ACCESS_CHK(VERIFY_WRITE, (void *)arg, sizeof(long));

			if ( PTLSER_ERR(error) )
				return error;

			put_to_user(C_CLOCAL(tty) ? 1 : 0, (unsigned long *)arg);

			return 0;
		case TIOCSSOFTCAR:
			error = PTLSER_ACCESS_CHK(VERIFY_READ, (void *)arg, sizeof(long));

			if ( PTLSER_ERR(error) )
				return error;

			get_from_user(templ,(unsigned long *)arg);
			arg = templ;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))				
			tty->termios->c_cflag = ((tty->termios->c_cflag & ~CLOCAL) | (arg ? CLOCAL : 0));
#else
			tty->termios.c_cflag = ((tty->termios.c_cflag & ~CLOCAL) | (arg ? CLOCAL : 0));
#endif
			return 0;
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
		case TIOCMGET:
			error = PTLSER_ACCESS_CHK(VERIFY_WRITE, (void *)arg, sizeof(unsigned int));

			if ( PTLSER_ERR(error) )
				return error;

			return ptlser_get_modem_info(info, (unsigned int *)arg);
		case TIOCMBIS:
		case TIOCMBIC:
		case TIOCMSET:
			return ptlser_set_modem_info(info, cmd, (unsigned int *)arg);
#endif	    
		case TIOCGSERIAL:
			error = PTLSER_ACCESS_CHK(VERIFY_WRITE, (void *)arg,sizeof(struct serial_struct));

			if ( PTLSER_ERR(error) )
				return error;

			return ptlser_get_serial_info(info, (struct serial_struct *)arg);
		case TIOCSSERIAL:
			error = PTLSER_ACCESS_CHK(VERIFY_READ, (void *)arg,sizeof(struct serial_struct));

			if ( PTLSER_ERR(error) )
				return error;

			return ptlser_set_serial_info(info, (struct serial_struct *)arg);
		case TIOCSERGETLSR: /* Get line status register */
			error = PTLSER_ACCESS_CHK(VERIFY_WRITE, (void *)arg,sizeof(unsigned int));

			if ( PTLSER_ERR(error) )
				return error;
			else
				return ptlser_get_lsr_info(info, (unsigned int *)arg);
	/*
	 * Wait for any of the 4 modem inputs (DCD,RI,DSR,CTS) to change
	 * - mask passed in arg for lines of interest
	 *   (use |'ed TIOCM_RNG/DSR/CD/CTS for masking)
	 * Caller should use TIOCGICOUNT to see which one it was
	 */
		case TIOCMIWAIT:{
			DECLARE_WAITQUEUE(wait, current);
			int ret;

			PTLSER_LOCK(&info->slock);
			cprev = info->icount;   /* note the counters on entry */
			PTLSER_UNLOCK(&info->slock);
	    
			add_wait_queue(&info->delta_msr_wait, &wait);
			while ( 1 ) {
				PTLSER_LOCK(&info->slock);
				cnow = info->icount;	/* atomic copy */
				PTLSER_UNLOCK(&info->slock);
				set_current_state(TASK_INTERRUPTIBLE);

				if ( ((arg & TIOCM_RNG) && (cnow.rng != cprev.rng)) ||
				     ((arg & TIOCM_DSR) && (cnow.dsr != cprev.dsr)) ||
                                     ((arg & TIOCM_CD) && (cnow.dcd != cprev.dcd)) ||
				     ((arg & TIOCM_CTS) && (cnow.cts != cprev.cts)) ) {
					ret = 0;
					break;
				}
		/* see if a signal did it */
				if ( signal_pending(current) ){
					ret = -ERESTARTSYS;
					break;
				}

				cprev = cnow;
			}

			set_current_state(TASK_RUNNING);
			remove_wait_queue(&info->delta_msr_wait, &wait);
			break;
		}
	    /* NOTREACHED */
	/*
	 * Get counter of input serial line interrupts (DCD,RI,DSR,CTS)
	 * Return: write counters to the user passed counter struct
	 * NB: both 1->0 and 0->1 transitions are counted except for
	 *     RI where only 0->1 is counted.
	 */
		case TIOCGICOUNT:
			error = PTLSER_ACCESS_CHK(VERIFY_WRITE, (void *)arg,sizeof(struct serial_icounter_struct));

			if ( PTLSER_ERR(error) )
				return error;

			PTLSER_LOCK(&info->slock);
			cnow = info->icount;
			PTLSER_UNLOCK(&info->slock);
			p_cuser = (struct serial_icounter_struct *)arg;

			if (put_user(cnow.frame, &p_cuser->frame))
				return -EFAULT;
			
			if (put_user(cnow.brk, &p_cuser->brk))
				return -EFAULT;

			if (put_user(cnow.overrun, &p_cuser->overrun))
				return -EFAULT;

			if (put_user(cnow.buf_overrun, &p_cuser->buf_overrun))
				return -EFAULT;

			if (put_user(cnow.parity, &p_cuser->parity))
				return -EFAULT;

			if (put_user(cnow.rx, &p_cuser->rx))
				return -EFAULT;

			if (put_user(cnow.tx, &p_cuser->tx))
				return -EFAULT;

			put_to_user(cnow.cts, &p_cuser->cts);
			put_to_user(cnow.dsr, &p_cuser->dsr);
			put_to_user(cnow.rng, &p_cuser->rng);
			put_to_user(cnow.dcd, &p_cuser->dcd);

			return 0;

		default:
			return(-ENOIOCTLCMD);
	}

	return 0;
}



/*
 * This routine is called by the upper-layer tty layer to signal that
 * incoming characters should be throttled.
 */
static void ptlser_throttle(struct tty_struct * tty)
{
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	PTLSER_LOCK_INIT();
	PTLSER_LOCK(&info->slock);
	info->IER &= ~UART_IER_RDI;
	PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	PTLSER_UNLOCK(&info->slock);        
}


static void ptlser_unthrottle(struct tty_struct * tty)
{
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	PTLSER_LOCK_INIT();
	PTLSER_LOCK(&info->slock);
	info->IER |= UART_IER_RDI;
	PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	PTLSER_UNLOCK(&info->slock);        
}


#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,19))
static void ptlser_set_termios(struct tty_struct * tty, struct ktermios * old_termios)
#else
static void ptlser_set_termios(struct tty_struct * tty, struct termios * old_termios)
#endif
{
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	PTLSER_LOCK_INIT();
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	if ( (tty->termios->c_cflag != old_termios->c_cflag) ||
	     (RELEVANT_IFLAG(tty->termios->c_iflag) != RELEVANT_IFLAG(old_termios->c_iflag)) ) {
		PTLSER_LOCK(&info->slock);
		ptlser_change_speed(info, old_termios);
		PTLSER_UNLOCK(&info->slock);

		if ( (old_termios->c_cflag & CRTSCTS) && !(tty->termios->c_cflag & CRTSCTS) ) {
	    		tty->hw_stopped = 0;
	    		ptlser_start(tty);
		}
	}
#else
	if ( (tty->termios.c_cflag != old_termios->c_cflag) ||
	     (RELEVANT_IFLAG(tty->termios.c_iflag) != RELEVANT_IFLAG(old_termios->c_iflag)) ) {
		PTLSER_LOCK(&info->slock);
		ptlser_change_speed(info, old_termios);
		PTLSER_UNLOCK(&info->slock);

		if ( (old_termios->c_cflag & CRTSCTS) && !(tty->termios.c_cflag & CRTSCTS) ) {
	    		tty->hw_stopped = 0;
	    		ptlser_start(tty);
		}
	}
#endif


/* Handle sw stopped */
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	if ( (old_termios->c_iflag & IXON) && !(tty->termios->c_iflag & IXON) ) {
#else
	if ( (old_termios->c_iflag & IXON) && !(tty->termios.c_iflag & IXON) ) {
#endif
    		tty->stopped = 0;
    		ptlser_start(tty);
	}
}


/*
 * ptlser_stop() and ptlser_start()
 *
 * This routines are called before setting or resetting tty->stopped.
 * They enable or disable transmitter interrupts, as necessary.
 */
static void ptlser_stop(struct tty_struct * tty)
{
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	PTLSER_LOCK_INIT();
	PTLSER_LOCK(&info->slock);
	if ( info->IER & UART_IER_THRI ) {
		info->IER &= ~UART_IER_THRI;
		PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	}
	PTLSER_UNLOCK(&info->slock);
}


static void ptlser_start(struct tty_struct * tty)
{
	struct ptlser_struct *info = (struct ptlser_struct *)tty->driver_data;
	PTLSER_LOCK_INIT();
	PTLSER_LOCK(&info->slock);
	if ( info->xmit_cnt && info->xmit_buf ) {
		info->IER &= ~UART_IER_THRI;
		PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
		info->IER |= UART_IER_THRI;
		PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
	}
	PTLSER_UNLOCK(&info->slock);
}
//#define SERIAL_DEBUG_RS_WAIT_UNTIL_SENT
/*
 * ptlser_wait_until_sent() --- wait until the transmitter is empty
 */
#if (LINUX_VERSION_CODE >= 131394)
static void ptlser_wait_until_sent(struct tty_struct *tty, int timeout)
{
	struct ptlser_struct * info = (struct ptlser_struct *)tty->driver_data;
	unsigned long orig_jiffies, char_time;
	int lsr;
	if (info->type == PORT_UNKNOWN)
		return;

	if (info->xmit_fifo_size == 0)
		return; /* Just in case.... */

	orig_jiffies = jiffies;
	/*
	 * Set the check interval to be 1/5 of the estimated time to
	 * send a single character, and make it at least 1.  The check
	 * interval should also be less than the timeout.
	 *
	 * Note: we have to use pretty tight timings here to satisfy
	 * the NIST-PCTS.
	 */
	char_time = (info->timeout - HZ/50) / info->xmit_fifo_size;
	char_time = char_time / 5;

	if (char_time == 0)
		char_time = 1;

	if (timeout && timeout < char_time)
		char_time = timeout;
	/*
	 * If the transmitter hasn't cleared in twice the approximate
	 * amount of time to send the entire FIFO, it probably won't
	 * ever clear.  This assumes the UART isn't doing flow
	 * control, which is currently the case.  Hence, if it ever
	 * takes longer than info->timeout, this is probably due to a
	 * UART bug of some kind.  So, we clamp the timeout parameter at
	 * 2*info->timeout.
	 */

	if (!timeout || timeout > 2*info->timeout)
		timeout = 2*info->timeout;
#ifdef SERIAL_DEBUG_RS_WAIT_UNTIL_SENT
	printk(DRIVER_NAME "port %d  In rs_wait_until_sent(%d) check=%lu...\n", info->port, timeout, char_time);
	printk(DRIVER_NAME "port %d  jiff=%lu...\n", info->port, jiffies);	
	printk(DRIVER_NAME "port %d  tx count = %d \n", info->port, getTxCount(info));
#endif
	while (!((lsr = PTLSER_READ_REG(info->base+ UART_LSR)) & UART_LSR_TEMT)) {
#ifdef SERIAL_DEBUG_RS_WAIT_UNTIL_SENT
		printk(DRIVER_NAME "port %d  lsr = %d (jiff=%lu)...\n", info->port, lsr, jiffies);
		printk(DRIVER_NAME "port %d  tx count = %d \n", info->port, getTxCount(info));
#endif
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(char_time);

		if (signal_pending(current))
			break;

		if (timeout && time_after(jiffies, orig_jiffies + timeout))
			break;
	}
	set_current_state(TASK_RUNNING);

#ifdef SERIAL_DEBUG_RS_WAIT_UNTIL_SENT
	printk(DRIVER_NAME "port %d  lsr = %d (jiff=%lu)...done\n", info->port, lsr, jiffies);
#endif
}
#endif

/*
 * This routine is called by tty_hangup() when a hangup is signaled.
 */
void ptlser_hangup(struct tty_struct * tty)
{
	struct ptlser_struct * info = (struct ptlser_struct *)tty->driver_data;
	ptlser_flush_buffer(tty);
	ptlser_shutdown(info);
	info->event = 0;
	info->count = 0;
	info->flags &= ~(ASYNC_NORMAL_ACTIVE|ASYNC_CALLOUT_ACTIVE);
	info->tty = 0;
	wake_up_interruptible(&info->open_wait);
}


/*
 * ptlser_break_ctl() --- routine which turns the break handling on or off
 */
#if (LINUX_VERSION_CODE >= 131394 && LINUX_VERSION_CODE < VERSION_CODE(2,6,26))
static void ptlser_break_ctl(struct tty_struct *tty, int break_state)
{
#else
static int ptlser_break_ctl(struct tty_struct *tty, int break_state)
{
#endif
	struct ptlser_struct * info = (struct ptlser_struct *)tty->driver_data;
	PTLSER_LOCK_INIT();
	PTLSER_LOCK(&info->slock);
	if (break_state == -1){
		PTLSER_WRITE_REG(PTLSER_READ_REG(info->base + UART_LCR) | UART_LCR_SBC, info->base + UART_LCR);
	} else {
		PTLSER_WRITE_REG(PTLSER_READ_REG(info->base + UART_LCR) & ~UART_LCR_SBC, info->base + UART_LCR);
	}
	PTLSER_UNLOCK(&info->slock);
#if (LINUX_VERSION_CODE >= 131394 && LINUX_VERSION_CODE < VERSION_CODE(2,6,26))
#else
	return 0;
#endif
}

/*
 * This is the serial driver's generic interrupt routine
 */

#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,18))
static irqreturn_t ptlser_interrupt(int irq, void *dev_id)
#else
static IRQ_RET ptlser_interrupt(int irq, void *dev_id, struct pt_regs * regs)
#endif
{
	int			status, iir, i;
	struct ptlser_struct *	info;
	struct ptlser_struct *	port;
	int			max, msr;
	int			int_cnt;
	int			handled=0;

	port = 0;

    for(i=0; i<PTLSER_BOARDS; i++){
        if(dev_id == &(ptlser_var_table[i*PTLSER_PORTS_PER_BOARD])){
            port = dev_id;
            break;
        }
    }

    if (i==PTLSER_BOARDS)
        goto irq_stop;

    if (port==0)
        goto irq_stop;

    max = ptlser_numports[ptlsercfg[i].board_type-1];

	PTLSER_WRITE_REG(0, port->base + PTLSER_GIE_OFFSET);
	do{

		for (i=0; i<max; i++) {
			info = port + i;
			spin_lock(&info->slock);
			info->intr_status = PTLSER_READ_REG(info->base+PTLSER_ISR2_OFFSET) & PTLSER_ISR2_INT_STATUS_MASK;
			info->lsr_status = PTLSER_READ_REG(info->base+PTLSER_GLSR_OFFSET);
			spin_unlock(&info->slock);
		}


		
		for(i=0; i<max; i++){
			info = port + i;			
			if (info->intr_status == 0x00 && info->lsr_status == 0x00 ) {
				continue;
			}
			handled = 1;
			
			spin_lock(&info->slock);
			status = PTLSER_READ_REG(info->base+UART_LSR) | info->lsr_status;

			if ((info->intr_status & PTLSER_ISR2_RX_MASK) || (status & (~(PTLSER_LSR_TX_EMPTY | PTLSER_LSR_TX_COMPLETE))  ) || getRxCount(info)){
					ptlser_receive_chars(info, &status);
			} 				

			if(info->intr_status & PTLSER_ISR2_MODEM_STATUS_CHANGE){
				msr = PTLSER_READ_REG(info->base + UART_MSR);	 			
				if ( msr & UART_MSR_ANY_DELTA ) {
					ptlser_check_modem_status(info, msr);
				}
			}

			if ((info->intr_status & PTLSER_ISR2_TX_EMPTY) || ( status & (PTLSER_LSR_TX_EMPTY | PTLSER_LSR_TX_COMPLETE) ) ){
					ptlser_transmit_chars(info);
			}

			spin_unlock(&info->slock);

		}

	}while(0); 
	PTLSER_WRITE_REG(0xFF, port->base + PTLSER_GIE_OFFSET);

irq_stop:
	return IRQ_RETVAL(handled);	
}

#if defined(_SCREEN_INFO_H) || (LINUX_VERSION_CODE >= VERSION_CODE(2,6,16))
static void ptlser_receive_chars(struct ptlser_struct *info, int *status)
{
	struct tty_struct *	tty = info->tty;
	unsigned char	ch, gdl=0;
	int			    cnt = 0;
	int 			recv_room;
	int			    max = 256;
	unsigned long 	flags;

	if ( *status & UART_LSR_SPECIAL )
		goto intr_old;
//printk(DRIVER_NAME "port %d  ptlser_receive (jiff=%lu).\n", info->port, jiffies);	 
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	recv_room = tty_buffer_request_room(tty, PTLSER_FIFO_SIZE);
#else
	recv_room = tty_buffer_request_room((struct tty_port *) info, PTLSER_FIFO_SIZE);
#endif

	if(recv_room){
		gdl = getRxCount(info);
//printk(DRIVER_NAME "port %d  receive %d bytes \n", info->port, gdl);	
		if(gdl > recv_room)
			gdl = recv_room;

		if(gdl){
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
			tty_insert_flip_string(tty, info->base + PTLSER_FIFO_D_OFFSET, gdl);
#else
			tty_insert_flip_string((struct tty_port *) info, info->base + PTLSER_FIFO_D_OFFSET, gdl);
#endif
			cnt = gdl;
		}
	}
	else{
		if (getRxCount(info))
			printk(DRIVER_NAME "port %d  ptlser_receive  %02x (jiff=%lu).\n", info->port, getRxCount(info), jiffies);
		set_bit(TTY_THROTTLED, &tty->flags);
	}

	goto end_intr;

intr_old:

	do {
		if(max-- <0)
			break;

		ch = PTLSER_READ_REG(info->base + UART_RX);

		if ( *status & UART_LSR_SPECIAL ) {
			if ( *status & UART_LSR_BI ) {
				flags = TTY_BREAK;
				info->icount.brk++;

				if ( info->flags & ASYNC_SAK )
					do_SAK(tty);
			} else if ( *status & UART_LSR_PE ) {
				flags = TTY_PARITY;
				info->icount.parity++;
			} else if ( *status & UART_LSR_FE ) {
				flags = TTY_FRAME;			
				info->icount.frame++;
			} else if ( *status & UART_LSR_OE ) {
				flags = TTY_OVERRUN;
				info->icount.overrun++;
			} else
				flags = TTY_BREAK;

		} else
			flags = 0;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
		tty_insert_flip_char(tty, ch, flags);
#else
		tty_insert_flip_char((struct tty_port *) info, ch, flags);
#endif
		cnt++;

		*status = PTLSER_READ_REG(info->base+UART_LSR);
	} while ( *status & UART_LSR_DR );

end_intr:	// add by Victor Yu. 09-02-2002

#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	tty_flip_buffer_push(tty);
#else
	tty_flip_buffer_push((struct tty_port *) info);
#endif
}
#else
static void ptlser_receive_chars(struct ptlser_struct *info,
					 int *status)
{
	struct tty_struct *	tty = info->tty;
	unsigned char		ch, gdl;
	int			cnt = 0;
	unsigned char           *cp;
	char                    *fp;
	int 			recv_room;
	int			max = 256;
	
        cp = tty->flip.char_buf;
        fp = tty->flip.flag_buf;
	if ( *status & UART_LSR_SPECIAL )
		goto intr_old;

	recv_room = tty->ldisc.receive_room(tty);

	if(recv_room){
		gdl = getRxCount(info);
//printk(DRIVER_NAME "port %d  receive %d bytes \n", info->port, gdl);	
		if(gdl > recv_room)
			gdl = recv_room;

		if(gdl){
			cnt = gdl;
			memcpy(cp,info->base + PTLSER_FIFO_D_OFFSET, cnt);
			memset(fp, TTY_NORMAL, cnt);
			cp+=cnt;
			fp+=cnt;
		}
	}
	else{
		set_bit(TTY_THROTTLED, &tty->flags);
	}

	goto end_intr;

intr_old:
	do {
		if(max-- <0)
			break;

		ch = PTLSER_READ_REG(info->base + UART_RX);

		if ( *status & UART_LSR_SPECIAL ) {
			if ( *status & UART_LSR_BI ) {
				*fp++ = TTY_BREAK;
				info->icount.brk++;
				if ( info->flags & ASYNC_SAK )
					do_SAK(tty);
			} else if ( *status & UART_LSR_PE ) {
				*fp++ = TTY_PARITY;
				info->icount.parity++;
			} else if ( *status & UART_LSR_FE ) {
				*fp++ = TTY_FRAME;
				info->icount.frame++;
			} else if ( *status & UART_LSR_OE ) {
				*fp++ = TTY_OVERRUN;
				info->icount.overrun++;
			} else
				*fp++ = 0;
		} else
			*fp++ = 0;

		*cp++ = ch;
		cnt++;

		*status = PTLSER_READ_REG(info->base+UART_LSR);
	} while ( *status & UART_LSR_DR );

end_intr:

	tty->ldisc.receive_buf(tty, tty->flip.char_buf, tty->flip.flag_buf, cnt);

}
#endif


static void ptlser_transmit_chars(struct ptlser_struct *info)
{
	int	count, cnt;
	int tx_cnt;

	if ( info->xmit_buf == 0 ){
	    return;
	}
#if 0
//printk(DRIVER_NAME "port %d  ptlser_transmit (jiff=%lu).\n", info->port, jiffies);	 
	if(info->xmit_cnt==0){
		if ( info->xmit_cnt < WAKEUP_CHARS ) {
			set_bit(PTLSER_EVENT_TXLOW,&info->event);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
    		MOD_INC_USE_COUNT;
    		if (schedule_task(&info->tqueue) == 0)
    			MOD_DEC_USE_COUNT;
#else
    	    schedule_work(&info->tqueue);
#endif
		}
		return;
	}
#endif
	if (info->tty->stopped){
		info->IER &= ~UART_IER_THRI;
		PTLSER_WRITE_REG(info->IER, info->base + UART_IER);
		return;
	}

	cnt = info->xmit_cnt;
	count = info->xmit_fifo_size;

	tx_cnt = PTLSER_FIFO_SIZE - getTxCount(info);

	//cnt = MIN(info->xmit_cnt, MIN(tx_cnt, SERIAL_XMIT_SIZE - info->xmit_tail));
	cnt = MIN(info->xmit_cnt, tx_cnt);
//printk(DRIVER_NAME "port %d  send  %d bytes\n", info->port, cnt);
	if(cnt){
		memcpy(info->base + PTLSER_FIFO_D_OFFSET, info->xmit_buf+info->xmit_tail, cnt);
		info->xmit_tail += cnt;
		info->xmit_tail &= (SERIAL_XMIT_SIZE - 1);
		info->xmit_cnt -= cnt;
	}
//printk(DRIVER_NAME "port %d  tx count = %d \n", info->port, getTxCount(info));
    	info->icount.tx += cnt;
#if 1
	if ( info->xmit_cnt < WAKEUP_CHARS ) {
		set_bit(PTLSER_EVENT_TXLOW,&info->event);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
		MOD_INC_USE_COUNT;
		if (schedule_task(&info->tqueue) == 0)
			MOD_DEC_USE_COUNT;
#else
    	schedule_work(&info->tqueue);
#endif
	}
#endif

}

static void ptlser_check_modem_status(struct ptlser_struct *info,
					      int status)
{
	/* update input line counters */
	if ( status & UART_MSR_TERI )
	    info->icount.rng++;
	if ( status & UART_MSR_DDSR )
	    info->icount.dsr++;
	if ( status & UART_MSR_DDCD )
	    info->icount.dcd++;
	if ( status & UART_MSR_DCTS )
	    info->icount.cts++;

	wake_up_interruptible(&info->delta_msr_wait);

	if ( (info->flags & ASYNC_CHECK_CD) && (status & UART_MSR_DDCD) ) {
		if ( status & UART_MSR_DCD )
			wake_up_interruptible(&info->open_wait);
		else if ( !((info->flags & ASYNC_CALLOUT_ACTIVE) && (info->flags & ASYNC_CALLOUT_NOHUP)) )
	        set_bit(PTLSER_EVENT_HANGUP,&info->event);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
		MOD_INC_USE_COUNT;
		if (schedule_task(&info->tqueue) == 0)
			MOD_DEC_USE_COUNT;
#else
    	schedule_work(&info->tqueue);
#endif
	}

	if ( info->flags & ASYNC_CTS_FLOW ) {
		if ( info->tty->hw_stopped ) {
			if (status & UART_MSR_CTS ){
			    	info->tty->hw_stopped = 0;
		       		set_bit(PTLSER_EVENT_TXLOW,&info->event);
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
            		MOD_INC_USE_COUNT;
            		if (schedule_task(&info->tqueue) == 0)
            			MOD_DEC_USE_COUNT;
#else
    	            schedule_work(&info->tqueue);
#endif
	        	}
		} else {
			if ( !(status & UART_MSR_CTS) ){
			    	info->tty->hw_stopped = 1;
			}
		}
	}
}

static int ptlser_block_til_ready(struct tty_struct *tty, struct file * filp, struct ptlser_struct *info)
{
	DECLARE_WAITQUEUE(wait, current);
	int			retval;
	int			do_clocal = 0;
	PTLSER_LOCK_INIT();

	/*
	 * If non-blocking mode is set, or the port is not enabled,
	 * then make the check up front and then exit.
	 */
	if ( (filp->f_flags & O_NONBLOCK) || (tty->flags & (1 << TTY_IO_ERROR)) ) {
		if ( info->flags & ASYNC_CALLOUT_ACTIVE )
			return -EBUSY;

		info->flags |= ASYNC_NORMAL_ACTIVE;

		return 0;
	}

	if ( info->flags & ASYNC_CALLOUT_ACTIVE ) {
		if ( info->normal_termios.c_cflag & CLOCAL )
			do_clocal = 1;
	} else {
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))	
		if ( tty->termios->c_cflag & CLOCAL )
#else
		if ( tty->termios.c_cflag & CLOCAL )
#endif
			do_clocal = 1;
	}

	/*
	 * Block waiting for the carrier detect and the line to become
	 * free (i.e., not in use by the callout).  While we are in
	 * this loop, info->count is dropped by one, so that
	 * ptlser_close() knows when to free things.  We restore it upon
	 * exit, either normal or abnormal.
	 */
	retval = 0;
	add_wait_queue(&info->open_wait, &wait);
	
	PTLSER_LOCK(&info->slock);
	if ( !tty_hung_up_p(filp) )
	    info->count--;
	PTLSER_UNLOCK(&info->slock);

	info->blocked_open++;

	while ( 1 ) {

		PTLSER_LOCK(&info->slock);
		if ( !(info->flags & ASYNC_CALLOUT_ACTIVE) )
			PTLSER_WRITE_REG(PTLSER_READ_REG(info->base + UART_MCR) | UART_MCR_DTR | UART_MCR_RTS,
		     info->base + UART_MCR);
		PTLSER_UNLOCK(&info->slock);

		set_current_state(TASK_INTERRUPTIBLE);

		if ( tty_hung_up_p(filp) || !(info->flags & ASYNC_INITIALIZED) ) {
#ifdef SERIAL_DO_RESTART
			if ( info->flags & ASYNC_HUP_NOTIFY )
				retval = -EAGAIN;
			else
				retval = -ERESTARTSYS;
#else
			retval = -EAGAIN;
#endif
			break;
		}

		if ( !(info->flags & ASYNC_CALLOUT_ACTIVE) && !(info->flags & ASYNC_CLOSING) &&
		 (do_clocal || (PTLSER_READ_REG(info->base + UART_MSR) & UART_MSR_DCD)) )

			break;
		if ( signal_pending(current) ) {
			retval = -ERESTARTSYS;
			break;
		}

		schedule();
	}

	set_current_state(TASK_RUNNING);
	remove_wait_queue(&info->open_wait, &wait);

	if ( !tty_hung_up_p(filp) )
		info->count++;

	info->blocked_open--;

	if ( retval )
		return retval;

	info->flags |= ASYNC_NORMAL_ACTIVE;

	return 0;
}

static int ptlser_startup(struct ptlser_struct * info)
{
	unsigned long page;
	unsigned char reg_flag;
	PTLSER_LOCK_INIT();

	page = GET_FPAGE(GFP_KERNEL);
	if ( !page )
	    return -ENOMEM;

	PTLSER_LOCK(&info->slock);

	if ( info->flags & ASYNC_INITIALIZED ) {
		free_page(page);
		PTLSER_UNLOCK(&info->slock);

		return 0;
	}

	if ( !info->base || !info->type ) {
		if ( info->tty )
			set_bit(TTY_IO_ERROR, &info->tty->flags);
		free_page(page);
		PTLSER_UNLOCK(&info->slock);

		return 0;
	}

	if ( info->xmit_buf )
		free_page(page);
	else
		info->xmit_buf = (unsigned char *)page;

	ptlser_change_speed(info, 0);

	/*
	 * Clear the FIFO buffers and disable them
	 * (they will be reenabled in ptlser_change_speed())
	 */
	//simoncc ++++++++++++++++++++
//	PTLSER_WRITE_REG((PTLSER_FCR_RX_FLUSH | PTLSER_FCR_TX_FLUSH),info->base + UART_FCR);
//	PTLSER_WRITE_REG((PTLSER_FCR_RX_FLUSH | PTLSER_FCR_TX_FLUSH),info->base + UART_FCR);
	//simoncc --------------------
	/*
	 * At this point there's no way the LSR could still be 0xFF;
	 * if it is, then bail out, because there's likely no UART
	 * here.
	 */

	if ( PTLSER_READ_REG(info->base + UART_LSR) == 0xff ) {
		PTLSER_UNLOCK(&info->slock);

		if (capable(CAP_SYS_ADMIN)) {
			if ( info->tty )
				set_bit(TTY_IO_ERROR, &info->tty->flags);

			return(0);
		} else
			return -ENODEV;
	}


//printk(DRIVER_NAME "port %d  **receive %d bytes \n", info->port, getRxCount(info));
	/*
	 * Clear the interrupt registers.
	 */
	(void)PTLSER_READ_REG(info->base + UART_LSR);
	(void)PTLSER_READ_REG(info->base + UART_RX);
	(void)PTLSER_READ_REG(info->base + UART_IIR);
	(void)PTLSER_READ_REG(info->base + UART_MSR);

	/*
	 * Now, initialize the UART
	 */
	PTLSER_WRITE_REG(UART_LCR_WLEN8, info->base + UART_LCR);	/* reset DLAB */
	info->MCR = UART_MCR_DTR | UART_MCR_RTS;
	PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);

	/*
	 * Initialize enhance mode register
	 */
	reg_flag = PTLSER_EFR_ENHANCE_MODE;
	PTLSER_WRITE_REG(reg_flag, info->base + PTLSER_EFR_OFFSET);

	reg_flag = PTLSER_SFR_950 | PTLSER_SFR_TX_FIFO_COUNT;
	PTLSER_WRITE_REG(reg_flag, info->base + PTLSER_SFR_OFFSET);

	PTLSER_WRITE_REG(1, info->base + PTLSER_TTL_OFFSET);
	PTLSER_WRITE_REG(96, info->base + PTLSER_RTL_OFFSET); //96
	PTLSER_WRITE_REG(16, info->base + PTLSER_FCL_OFFSET);
	PTLSER_WRITE_REG(110, info->base + PTLSER_FCH_OFFSET);

	/*
	 * Finally, enable interrupts
	 */
	info->IER = UART_IER_MSI | UART_IER_RLSI | UART_IER_RDI;
	PTLSER_WRITE_REG(info->IER, info->base + UART_IER); /* enable interrupts */

	/*
	 * And clear the interrupt registers again for luck.
	 */
	(void)PTLSER_READ_REG(info->base + UART_LSR);
	(void)PTLSER_READ_REG(info->base + UART_RX);
	(void)PTLSER_READ_REG(info->base + UART_IIR);
	(void)PTLSER_READ_REG(info->base + UART_MSR);

	if ( info->tty )
		test_and_clear_bit(TTY_IO_ERROR, &info->tty->flags);

	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;

	/*
	 * and set the speed of the serial port
	 */
	ptlser_change_speed(info, 0);
	PTLSER_UNLOCK(&info->slock);

	info->flags |= ASYNC_INITIALIZED;

	return 0;
}

/*
 * This routine will shutdown a serial port; interrupts maybe disabled, and
 * DTR is dropped if the hangup on close termio flag is on.
 */
static void ptlser_shutdown(struct ptlser_struct * info)
{
	unsigned char reg_flag;
//printk(DRIVER_NAME "port %d  ptlser_shutdown (jiff=%lu).\n", info->port, jiffies);	 
	PTLSER_LOCK_INIT();

	if ( !(info->flags & ASYNC_INITIALIZED) )
		return;

	PTLSER_LOCK(&info->slock);

	/*
	 * clear delta_msr_wait queue to avoid mem leaks: we may free the irq
	 * here so the queue might never be waken up
	 */
	wake_up_interruptible(&info->delta_msr_wait);

	/*
	 * Free the IRQ, if necessary
	 */
	if ( info->xmit_buf ) {
		free_page((unsigned long)info->xmit_buf);
		info->xmit_buf = 0;
	}

	reg_flag = 0;
	PTLSER_WRITE_REG(reg_flag, info->base + PTLSER_EFR_OFFSET);
	PTLSER_WRITE_REG(reg_flag, info->base + PTLSER_SFR_OFFSET);

	info->IER = 0;
	PTLSER_WRITE_REG(0x00, info->base + UART_IER);
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	if ( !info->tty || (info->tty->termios->c_cflag & HUPCL) )
#else
	if ( !info->tty || (info->tty->termios.c_cflag & HUPCL) )
#endif
		info->MCR &= ~(UART_MCR_DTR | UART_MCR_RTS);
	PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);

	/* read data port to reset things */
	(void)PTLSER_READ_REG(info->base + UART_RX);

	if ( info->tty )
		set_bit(TTY_IO_ERROR, &info->tty->flags);

	info->flags &= ~ASYNC_INITIALIZED;

	PTLSER_UNLOCK(&info->slock);
}

/*
 * This routine is called to set the UART divisor registers to match
 * the specified baud rate for a serial port.
 */
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,19))
static int ptlser_change_speed(struct ptlser_struct *info, struct ktermios *old_termios)
#else
static int ptlser_change_speed(struct ptlser_struct *info, struct termios *old_termios)
#endif
{
	unsigned	cflag, cval, fcr;
        int             ret = 0;
	long baud;
	int reg_flag;

#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	if ( !info->tty || !info->tty->termios )
		return ret;

	cflag = info->tty->termios->c_cflag;
#else	
	if ( !info->tty )
		return ret;

	cflag = info->tty->termios.c_cflag;
#endif

	if ( !(info->base) )
		return ret;

#ifndef B921600
#define B921600 (B460800 +1)
#endif
	switch( cflag & (CBAUD | CBAUDEX) ){
				case B4000000: info->speed = baud = 4000000; break;
				case B3500000: info->speed = baud = 3500000; break;
				case B3000000: info->speed = baud = 3000000; break;
				case B2500000: info->speed = baud = 2500000; break;
				case B2000000: info->speed = baud = 2000000; break;
				case B1500000: info->speed = baud = 1500000; break;
				case B1152000: info->speed = baud = 1152000; break;
				case B1000000: info->speed = baud = 1000000; break;


        case B921600 : info->speed = baud = 921600; break;
        case B460800 : info->speed = baud = 460800; break;
        case B230400 : info->speed = baud = 230400; break;
        case B115200 : info->speed = baud = 115200; break;
        case B57600 : info->speed = baud = 57600; break;
        case B38400 : info->speed = baud = 38400;break;
        case B19200 : info->speed = baud = 19200; break;
        case B9600 : info->speed = baud = 9600; break;
        case B4800 : info->speed = baud = 4800; break;
        case B2400 : info->speed = baud = 2400; break;
        case B1800 : info->speed = baud = 1800; break;
        case B1200 : info->speed = baud = 1200; break;
        case B600 : info->speed = baud = 600; break;
        case B300 : info->speed = baud = 300; break;
        case B200 : info->speed = baud = 200; break;
        case B150 : info->speed = baud = 150; break;
        case B134 : info->speed = baud = 134; break;
        case B110 : info->speed = baud = 110; break;
        case B75 : info->speed = baud = 75; break;
        case B50 : info->speed = baud = 50; break;
        default: info->speed = baud = 0; break;
	}

		if (baud > 921600)
			ptlser_set_baud_ex(info, baud);
		else
    	ptlser_set_baud(info, baud);

	/* byte size and parity */
	switch ( cflag & CSIZE ) {
		case CS5: cval = 0x00; break;
		case CS6: cval = 0x01; break;
		case CS7: cval = 0x02; break;
		case CS8: cval = 0x03; break;
		default:  cval = 0x00; break;	/* too keep GCC shut... */
	}

	if ( cflag & CSTOPB )
		cval |= 0x04;

	if ( cflag & PARENB )
		cval |= UART_LCR_PARITY;

#ifndef CMSPAR
#define	CMSPAR 010000000000
#endif
	if ( !(cflag & PARODD) )
		cval |= UART_LCR_EPAR;

	if ( cflag & CMSPAR )
		cval |= UART_LCR_SPAR;

	fcr = UART_FCR_ENABLE_FIFO | PTLSER_FCR_DMA_ENABLE;

	/* CTS flow control flag and modem status interrupts */
	info->IER &= ~UART_IER_MSI;
//	info->MCR &= ~UART_MCR_AFE;

	reg_flag = PTLSER_READ_REG(info->base + PTLSER_EFR_OFFSET);

	if ( cflag & CRTSCTS ) {
		info->flags |= ASYNC_CTS_FLOW;
		info->IER |= UART_IER_MSI;
		reg_flag |= (PTLSER_EFR_AUTO_RTS | PTLSER_EFR_AUTO_CTS);
	} else {
		info->flags &= ~ASYNC_CTS_FLOW;
		reg_flag &= ~(PTLSER_EFR_AUTO_RTS | PTLSER_EFR_AUTO_CTS);
	}

	PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);

	if ( cflag & CLOCAL ){
		info->flags &= ~ASYNC_CHECK_CD;
	}else {
		info->flags |= ASYNC_CHECK_CD;
		info->IER |= UART_IER_MSI;
	}

	PTLSER_WRITE_REG(info->IER, info->base + UART_IER);

	/*
	 * Set up parity check flag
	 */
	info->read_status_mask = UART_LSR_OE | UART_LSR_THRE | UART_LSR_DR;

	if ( I_INPCK(info->tty) )
		info->read_status_mask |= UART_LSR_FE | UART_LSR_PE;

	if ( I_BRKINT(info->tty) || I_PARMRK(info->tty) )
		info->read_status_mask |= UART_LSR_BI;

	info->ignore_status_mask = 0;

	if ( I_IGNBRK(info->tty) ) {
		info->ignore_status_mask |= UART_LSR_BI;
		info->read_status_mask |= UART_LSR_BI;
	    /*
	     * If we're ignore parity and break indicators, ignore
	     * overruns too.  (For real raw support).
	     */
		if ( I_IGNPAR(info->tty) ) {
			info->ignore_status_mask |= UART_LSR_OE|UART_LSR_PE|UART_LSR_FE;
			info->read_status_mask |= UART_LSR_OE|UART_LSR_PE|UART_LSR_FE;
		}
	}

	PTLSER_WRITE_REG(START_CHAR(info->tty), info->base + PTLSER_XON1_OFFSET);
	PTLSER_WRITE_REG(START_CHAR(info->tty), info->base + PTLSER_XON2_OFFSET);
	PTLSER_WRITE_REG(STOP_CHAR(info->tty), info->base + PTLSER_XOFF1_OFFSET);
	PTLSER_WRITE_REG(STOP_CHAR(info->tty), info->base + PTLSER_XOFF2_OFFSET);

	if ( I_IXON(info->tty) )
		reg_flag |= PTLSER_EFR_INBAND_RX_MODE_1;
	else
		reg_flag &= ~PTLSER_EFR_INBAND_RX_MODE_1;

	if ( I_IXOFF(info->tty) )
		reg_flag |= PTLSER_EFR_INBAND_TX_MODE_1;
	else
		reg_flag &= ~PTLSER_EFR_INBAND_TX_MODE_1;

	PTLSER_WRITE_REG(reg_flag, info->base + PTLSER_EFR_OFFSET);
	PTLSER_WRITE_REG(fcr, info->base + UART_FCR);		    /* set fcr */
	PTLSER_WRITE_REG(cval, info->base + UART_LCR);  
	
        return ret;
}

static int ptlser_set_baud_ex(struct ptlser_struct *info, long newspd)
{
	int		i;
	int		quot = 0;
	unsigned char	cval;
	int scr;
	int actual_baud;
	int tolerance;
	int             ret = 0;

	printk(DRIVER_NAME "Enter %s, newspd = %lu\n", __FUNCTION__, newspd);

#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	if ( !info->tty || !info->tty->termios )
		return ret;
#else
	if ( !info->tty )
		return ret;
#endif

	if ( !(info->base) )
		return ret;

	info->realbaud = newspd;

	for (scr = 5 ; scr <= 15 ; scr++)
	{
		actual_baud = 921600 * 16 /scr;
		tolerance = actual_baud / 50;

		if ((newspd < actual_baud + tolerance) &&
			(newspd > actual_baud - tolerance))
		{
			printk(DRIVER_NAME "Found match scr = %d\n", scr);
			break;
		}
	}
	quot = 1;

	info->timeout = (int)((unsigned int)(info->xmit_fifo_size*HZ*10*quot) / (unsigned int)info->baud_base);
	info->timeout += HZ/50;		/* Add .02 seconds of slop */

	if ( quot ) {
		info->MCR |= UART_MCR_DTR;
		PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);
	} else {
		info->MCR &= ~UART_MCR_DTR;
		PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);
		return ret;
	}

	printk(DRIVER_NAME "info->base = %p, UART_DLL = %d", info->base, UART_DLL);

	// cval = inb(info->io_base + UART_LCR);
	// outb(cval | UART_LCR_DLAB, info->io_base + UART_LCR);  /* set DLAB */
	// outb(1, info->io_base + UART_DLL);	    /* LS of divisor */
	// outb(0, info->io_base + UART_DLM); 	    /* MS of divisor */
	// outb(scr, info->io_base + 2); 	    /* scr */
	// outb(cval, info->io_base + UART_LCR);		    /* reset DLAB */

	cval = PTLSER_READ_REG(info->base + UART_LCR);
	PTLSER_WRITE_REG(cval | UART_LCR_DLAB, info->base + UART_LCR);  /* set DLAB */
	PTLSER_WRITE_REG(1, info->base + UART_DLL);	    /* LS of divisor */
	PTLSER_WRITE_REG(0, info->base + UART_DLM); 	    /* MS of divisor */
	PTLSER_WRITE_REG(16 - scr, info->base + 2); 	    /* scr */
	PTLSER_WRITE_REG(cval, info->base + UART_LCR);		    /* reset DLAB */

	// cval = PTLSER_READ_REG(info->base + 0x7);
	// PTLSER_WRITE_REG(cval | 0x80, info->base + 0x7);
	// PTLSER_WRITE_REG(15, info->base + 0x16);
	// PTLSER_WRITE_REG(cval, info->base + 0x7);

	PTLSER_WRITE_REG(96, info->base + PTLSER_RTL_OFFSET);
	//PTLSER_WRITE_REG(256, info->base + PTLSER_RTL_OFFSET);

    return ret;
}

static int ptlser_set_baud(struct ptlser_struct *info, long newspd)
{
	int		i;
	int		quot = 0;
	unsigned char	cval;
        int             ret = 0;

#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	if ( !info->tty || !info->tty->termios )
		return ret;
#else
	if ( !info->tty )
		return ret;
#endif

	if ( !(info->base) )
		return ret;

	if ( newspd > info->MaxCanSetBaudRate )
		return 0;

	info->realbaud = newspd;

	for ( i=0; i<BAUD_TABLE_NO && newspd != ptlser_baud_table[i]; i++ );

	if ( i == BAUD_TABLE_NO ){
		quot = info->baud_base / info->speed;
		if ( info->speed <= 0 || info->speed > info->MaxCanSetBaudRate )
			quot = 0;
	}else{
		if ( newspd == 134 ) {
			quot = (2 * info->baud_base / 269);
			info->speed = 134;
		} else if ( newspd ) {
			quot = info->baud_base / newspd;

			if(quot==0)
				quot = 1;
		} else {
			quot = 0;
		}
	}

	info->timeout = (int)((unsigned int)(info->xmit_fifo_size*HZ*10*quot) / (unsigned int)info->baud_base);
	info->timeout += HZ/50;		/* Add .02 seconds of slop */

	if ( quot ) {
		info->MCR |= UART_MCR_DTR;
		PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);
	} else {
		info->MCR &= ~UART_MCR_DTR;
		PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);

		return ret;
	}

	printk(DRIVER_NAME "info->base = %p, UART_DLL = %d", info->base, UART_DLL);

	cval = PTLSER_READ_REG(info->base + UART_LCR);
	PTLSER_WRITE_REG(cval | UART_LCR_DLAB, info->base + UART_LCR);  /* set DLAB */
	PTLSER_WRITE_REG(quot & 0xff, info->base + UART_DLL);	    /* LS of divisor */
	PTLSER_WRITE_REG(quot >> 8, info->base + UART_DLM); 	    /* MS of divisor */
	PTLSER_WRITE_REG(cval, info->base + UART_LCR);		    /* reset DLAB */
	// simoncc patch...
	if (info->realbaud < 9600) {				
		PTLSER_WRITE_REG(3, info->base + PTLSER_RTL_OFFSET);			
	} else if (info->realbaud < 38400) {
		PTLSER_WRITE_REG(8, info->base + PTLSER_RTL_OFFSET);
	} else if (info->realbaud < 115200) {
		PTLSER_WRITE_REG(14, info->base + PTLSER_RTL_OFFSET);
	} else {
		PTLSER_WRITE_REG(96, info->base + PTLSER_RTL_OFFSET);
	}

	if ( i == BAUD_TABLE_NO ){
		quot = info->baud_base % info->speed;
		quot *= 8;
		if ( (quot % info->speed) > (info->speed / 2) ) {
			quot /= info->speed;
			quot++;
		} else {
			quot /= info->speed;
		}
	}

    return ret;
}



/*
 * ------------------------------------------------------------
 * friends of ptlser_ioctl()
 * ------------------------------------------------------------
 */
static int ptlser_get_serial_info(struct ptlser_struct * info, struct serial_struct * retinfo)
{
	struct serial_struct	tmp;

	if ( !retinfo )
		return(-EFAULT);

	memset(&tmp, 0, sizeof(tmp));
	tmp.type = info->type;
	tmp.line = info->port;
	tmp.port = *info->base;
	tmp.irq = info->irq;
	tmp.flags = info->flags;
	tmp.baud_base = info->baud_base;
	tmp.close_delay = info->close_delay;
	tmp.closing_wait = info->closing_wait;
	tmp.custom_divisor = info->custom_divisor;
	tmp.hub6 = 0;

	if(copy_to_user(retinfo, &tmp, sizeof(*retinfo)))
		return -EFAULT;

	return 0;
}

static int ptlser_set_serial_info(struct ptlser_struct * info, struct serial_struct * new_info)
{
	struct serial_struct	new_serial;
	unsigned int		flags;
	int			retval = 0;

	PTLSER_LOCK_INIT();

	if ( !new_info || !info->base )
		return -EFAULT;

	if(copy_from_user(&new_serial, new_info, sizeof(new_serial)))
		return -EFAULT;

	if ( (new_serial.irq != info->irq) ||
	     (new_serial.port != *info->base) )
		return -EPERM;

	flags = info->flags & ASYNC_SPD_MASK;

       if ( !capable(CAP_SYS_ADMIN)) {
		if ( (new_serial.baud_base != info->baud_base) ||
		     (new_serial.close_delay != info->close_delay) ||
		    ((new_serial.flags & ~ASYNC_USR_MASK) !=
		    (info->flags & ~ASYNC_USR_MASK)) )
			return(-EPERM);

		info->flags = ((info->flags & ~ASYNC_USR_MASK) | (new_serial.flags & ASYNC_USR_MASK));
	} else {
	    /*
	     * OK, past this point, all the error checking has been done.
	     * At this point, we start making changes.....
	     */
		info->flags = ((info->flags & ~ASYNC_FLAGS) | (new_serial.flags & ASYNC_FLAGS));
		info->close_delay = new_serial.close_delay * HZ/100;
		info->closing_wait = new_serial.closing_wait * HZ/100;
//simoncc		info->tty->low_latency = (info->flags & ASYNC_LOW_LATENCY) ? 1 : 0;

		if( (new_serial.baud_base != info->baud_base) ||
		    (new_serial.custom_divisor != info->custom_divisor) )
			info->custom_baud_rate = new_serial.baud_base/new_serial.custom_divisor;
	}

	info->type = new_serial.type;
	info->xmit_fifo_size = PTLSER_FIFO_SIZE;

	if ( info->flags & ASYNC_INITIALIZED ) {
		if ( flags != (info->flags & ASYNC_SPD_MASK) ){
			PTLSER_LOCK(&info->slock);
			ptlser_change_speed(info,0);
			PTLSER_UNLOCK(&info->slock);
		}
	} else{
	    retval = ptlser_startup(info);
	}

	return retval;
}

/*
 * ptlser_get_lsr_info - get line status register info
 *
 * Purpose: Let user call ioctl() to get info when the UART physically
 *	    is emptied.  On bus types like RS485, the transmitter must
 *	    release the bus after transmitting. This must be done when
 *	    the transmit shift register is empty, not be done when the
 *	    transmit holding register is empty.  This functionality
 *	    allows an RS485 driver to be written in user space.
 */
static int ptlser_get_lsr_info(struct ptlser_struct * info, unsigned int *value)
{
	unsigned char	status;
	unsigned int	result;
	PTLSER_LOCK_INIT();

	PTLSER_LOCK(&info->slock);
	status = PTLSER_READ_REG(info->base + UART_LSR);
	PTLSER_UNLOCK(&info->slock);
	result = ((status & UART_LSR_TEMT) ? TIOCSER_TEMT : 0);
	put_to_user(result, value);

	return 0;
}

/*
 * This routine sends a break character out the serial port.
 */
static void ptlser_send_break(struct ptlser_struct * info, int duration)
{
	PTLSER_LOCK_INIT();

	if ( !info->base )
		return;

	set_current_state(TASK_INTERRUPTIBLE);

	PTLSER_LOCK(&info->slock);
	PTLSER_WRITE_REG(PTLSER_READ_REG(info->base + UART_LCR) | UART_LCR_SBC, info->base + UART_LCR);
	PTLSER_UNLOCK(&info->slock);

	schedule_timeout(duration);

	PTLSER_LOCK(&info->slock);
	PTLSER_WRITE_REG(PTLSER_READ_REG(info->base + UART_LCR) & ~UART_LCR_SBC, info->base + UART_LCR);
	PTLSER_UNLOCK(&info->slock);
	
	set_current_state(TASK_RUNNING);
}

#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
static int ptlser_tiocmget(struct tty_struct *tty/*, struct file *file*/)
{
	struct ptlser_struct *info = (struct ptlser_struct *) tty->driver_data;
	unsigned char control, status;
	PTLSER_LOCK_INIT();

	if (PORTNO(tty) == PTLSER_PORTS)
		return -ENOIOCTLCMD;

	if (tty->flags & (1 << TTY_IO_ERROR))
		return -EIO;

	control = info->MCR;
	
	PTLSER_LOCK(&info->slock);
	status = PTLSER_READ_REG(info->base + UART_MSR);

	if (status & UART_MSR_ANY_DELTA)
		ptlser_check_modem_status(info, status);

	PTLSER_UNLOCK(&info->slock);

	return ((control & UART_MCR_RTS) ? TIOCM_RTS : 0) |
	    ((control & UART_MCR_DTR) ? TIOCM_DTR : 0) |
	    ((status & UART_MSR_DCD) ? TIOCM_CAR : 0) |
	    ((status & UART_MSR_RI) ? TIOCM_RNG : 0) |
	    ((status & UART_MSR_DSR) ? TIOCM_DSR : 0) |
	    ((status & UART_MSR_CTS) ? TIOCM_CTS : 0);
}

static int ptlser_tiocmset(struct tty_struct *tty/*, struct file *file*/, unsigned int set, unsigned int clear)
{
	struct ptlser_struct *info = (struct ptlser_struct *) tty->driver_data;
	PTLSER_LOCK_INIT();

	if (PORTNO(tty) == PTLSER_PORTS)
		return (-ENOIOCTLCMD);

	if (tty->flags & (1 << TTY_IO_ERROR))
		return (-EIO);

	PTLSER_LOCK(&info->slock);

	if (set & TIOCM_RTS)
		info->MCR |= UART_MCR_RTS;
	if (set & TIOCM_DTR)
		info->MCR |= UART_MCR_DTR;
	if (clear & TIOCM_RTS)
		info->MCR &= ~UART_MCR_RTS;
	if (clear & TIOCM_DTR)
		info->MCR &= ~UART_MCR_DTR;

	PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);
	PTLSER_UNLOCK(&info->slock);

	return 0;
}

#else
static int ptlser_get_modem_info(struct ptlser_struct * info, unsigned int *value)
{
	unsigned char	control, status;
	unsigned int	result;
	PTLSER_LOCK_INIT();
	PTLSER_LOCK(&info->slock);
	control = info->MCR;
	status = PTLSER_READ_REG(info->base + UART_MSR); 
	PTLSER_UNLOCK(&info->slock);

	if ( status & UART_MSR_ANY_DELTA )
		ptlser_check_modem_status(info, status);

	result =((control & UART_MCR_RTS) ? TIOCM_RTS : 0) |
		    ((control & UART_MCR_DTR) ? TIOCM_DTR : 0) |
		    ((status  & UART_MSR_DCD) ? TIOCM_CAR : 0) |
		    ((status  & UART_MSR_RI)  ? TIOCM_RNG : 0) |
		    ((status  & UART_MSR_DSR) ? TIOCM_DSR : 0) |
		    ((status  & UART_MSR_CTS) ? TIOCM_CTS : 0);
	put_to_user(result, value);

	return 0;
}


static int ptlser_set_modem_info(struct ptlser_struct * info, unsigned int cmd, unsigned int *value)
{
	int		error;
	unsigned int	arg;
	error = PTLSER_ACCESS_CHK(VERIFY_READ, value, sizeof(int));

	if ( PTLSER_ERR(error) )
		return(error);

	get_from_user(arg,value);

	switch ( cmd ) {
		case TIOCMBIS:
			if ( arg & TIOCM_RTS )
				info->MCR |= UART_MCR_RTS;
			if ( arg & TIOCM_DTR )
				info->MCR |= UART_MCR_DTR;
			break;
		case TIOCMBIC:
			if ( arg & TIOCM_RTS )
				info->MCR &= ~UART_MCR_RTS;
			if ( arg & TIOCM_DTR )
				info->MCR &= ~UART_MCR_DTR;
			break;
		case TIOCMSET:
			info->MCR = ((info->MCR & ~(UART_MCR_RTS | UART_MCR_DTR)) |
			((arg & TIOCM_RTS) ? UART_MCR_RTS : 0) |
			((arg & TIOCM_DTR) ? UART_MCR_DTR : 0));
			break;
		default:
			return -EINVAL;
	}

	PTLSER_WRITE_REG(info->MCR, info->base + UART_MCR);

	return 0;
}
#endif


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ptlser_initHw(int board,struct ptlser_hwconf *hwconf)
{
	struct ptlser_struct *	info;
    int     retval;
	int	i,n;

	n = board*PTLSER_PORTS_PER_BOARD;
	info = &ptlser_var_table[n];
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
	printk(DRIVER_NAME "ttyPPU%d/coppu%d - ttyPPU%d/coppu%d\n", n, n, n+hwconf->ports-1, n+hwconf->ports-1);
#else
	printk(DRIVER_NAME "ttyPPU%d - ttyPPU%d\n", n, n+hwconf->ports-1);
#endif			
	printk(DRIVER_NAME "max. baud rate = %d bps.\n", hwconf->MaxCanSetBaudRate[0]);
	
	for ( i=0; i<hwconf->ports; i++, n++, info++ ) {
		info->port = n;
		info->base = hwconf->ioaddr[i];
		info ->io_base = hwconf->io_port_addr[i];
		info->irq = hwconf->irq;
		info->board_type = hwconf->board_type;
		info->flags = ASYNC_SHARE_IRQ;
		info->type = hwconf->uart_type;
		info->baud_base = hwconf->baud_base[i];
		info->MaxCanSetBaudRate = hwconf->MaxCanSetBaudRate[i];
		info->xmit_fifo_size = PTLSER_FIFO_SIZE;
		info->custom_divisor = hwconf->baud_base[i] * 16;
		info->close_delay = 5*HZ/10;
		info->closing_wait = 30*HZ;
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,19))
		INIT_WORK(&info->tqueue, ptlser_do_softint);
#else
		INIT_WORK(&info->tqueue, ptlser_do_softint, info);
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
		info->callout_termios = ptlser_var_cdriver.init_termios;
#endif		
		info->normal_termios = DRV_VAR_P(init_termios);
		init_waitqueue_head(&info->open_wait);
		init_waitqueue_head(&info->close_wait);
		init_waitqueue_head(&info->delta_msr_wait);
		info->speed = 9600;
//		info->err_shadow = 0;
		spin_lock_init(&info->slock);
		printk(DRIVER_NAME "ttyPPU%d c %d %d\n", n, ttymajor, n);
	}


	/*
	 * Allocate the IRQ if necessary
	 */
	

	/* before set INT ISR, disable all int */
	for(i=0; i<hwconf->ports; i++)
		PTLSER_WRITE_REG(PTLSER_READ_REG(hwconf->ioaddr[i] + UART_IER) & 0xf0, hwconf->ioaddr[i]+UART_IER);

	n = board*PTLSER_PORTS_PER_BOARD;
	info = &ptlser_var_table[n];
    retval = request_irq(hwconf->irq, ptlser_interrupt, IRQF_SHARED, "ptlser", info);

	if ( retval ) {
	    printk(DRIVER_NAME "Board %d: %s", board, ptlser_brdname[hwconf->board_type-1]);
	    printk(DRIVER_NAME "Request irq fail,IRQ (%d) may be conflit with another device.\n",info->irq);
	    return retval;
	}
	
        return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
 * boot-time initialization
 */
static int __init	ptlser_module_init(void)
{		
	int			    i, m, retval, b;
    int             ret1, ret2;
#ifdef CONFIG_PCI
	struct pci_dev	*pdev=NULL;
	int			    index;
	unsigned char	busnum,devnum;
#endif
	struct ptlser_hwconf	hwconf;

	printk(DRIVER_NAME "Pericom PI7C9X795x/PI7C9X895x UART Device Driver Version %s\n",PTLSER_VERSION);
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
	ptlser_var_sdriver = alloc_tty_driver(PTLSER_PORTS + 1);
	if (!ptlser_var_sdriver)
		return -ENOMEM;
#endif		

	for(i=0; i<PTLSER_BOARDS; i++)
		ptlsercfg[i].board_type = -1;
	
	//printk(DRIVER_NAME "Pericom PI7C9X795x PCI Express UART Device Driver Version %s\n",PTLSER_VERSION);

	/* Initialize the tty_driver structure */
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,28))
	memset(DRV_VAR, 0, sizeof(struct tty_driver));
#endif
	DRV_VAR_P(magic)            = TTY_DRIVER_MAGIC;
	DRV_VAR_P(name)             = "ttyXR"; //simoncc "ttyPPU";
	DRV_VAR_P(major)            = ttymajor;
	DRV_VAR_P(minor_start)      = 0;
	DRV_VAR_P(num)              = PTLSER_PORTS + 1;
	DRV_VAR_P(type)             = TTY_DRIVER_TYPE_SERIAL;
	DRV_VAR_P(subtype)          = SERIAL_TYPE_NORMAL;
	DRV_VAR_P(init_termios)     = tty_std_termios;
	DRV_VAR_P(init_termios.c_cflag) = B9600|CS8|CREAD|HUPCL|CLOCAL;
	DRV_VAR_P(flags)            = TTY_DRIVER_REAL_RAW;
#if (LINUX_VERSION_CODE >= VERSION_CODE(2,6,0))
	tty_set_operations(DRV_VAR, &ptlser_ops);
	DRV_VAR_P(ttys)             = ptlser_var_tty;
#else
	DRV_VAR_P(refcount)         = &ptlser_var_refcount;
	DRV_VAR_P(table)            = ptlser_var_tty;
#endif
	DRV_VAR_P(termios)          = ptlser_var_termios;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,3,0))
	DRV_VAR_P(termios_locked)   = ptlser_var_termios_locked;
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,26))
	DRV_VAR_P(open)             = ptlser_open;
	DRV_VAR_P(close)            = ptlser_close;
	DRV_VAR_P(write)            = ptlser_write;
	DRV_VAR_P(put_char)         = ptlser_put_char;
	DRV_VAR_P(flush_chars)      = ptlser_flush_chars;
	DRV_VAR_P(write_room)       = ptlser_write_room;
	DRV_VAR_P(chars_in_buffer)  = ptlser_chars_in_buffer;
	DRV_VAR_P(flush_buffer)     = ptlser_flush_buffer;
	DRV_VAR_P(ioctl)            = ptlser_ioctl;
	DRV_VAR_P(throttle)         = ptlser_throttle;
	DRV_VAR_P(unthrottle)       = ptlser_unthrottle;
	DRV_VAR_P(set_termios)      = ptlser_set_termios;
	DRV_VAR_P(stop)             = ptlser_stop;
	DRV_VAR_P(start)            = ptlser_start;
	DRV_VAR_P(hangup)           = ptlser_hangup;
#if (LINUX_VERSION_CODE >= 131394 && LINUX_VERSION_CODE < VERSION_CODE(2,6,26))
	DRV_VAR_P(break_ctl)        = ptlser_break_ctl;
	DRV_VAR_P(wait_until_sent)  = ptlser_wait_until_sent;
#endif
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
	/*
	 * The callout device is just like normal device except for
	 * major number and the subtype code.
	 */
	ptlser_var_cdriver               = ptlser_var_sdriver;
	ptlser_var_cdriver.name          = "coppu";
	ptlser_var_cdriver.major         = calloutmajor;
	ptlser_var_cdriver.subtype       = SERIAL_TYPE_CALLOUT;

	printk(DRIVER_NAME "Tty devices major number = %d, callout devices major number = %d\n",ttymajor,calloutmajor);
#endif
	memset(ptlser_var_table, 0, PTLSER_PORTS * sizeof(struct ptlser_struct));
	memset(&hwconf, 0, sizeof(struct ptlser_hwconf));

	/* start finding PCI board here */
#ifdef CONFIG_PCI
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
	if (pci_present()) {
#else
	{		
#endif		
		int n = (sizeof(ptlser_pcibrds) / sizeof(ptlser_pcibrds[0])) - 1;
		index = 0;
		b = 0;
		m = 0;

		while (b < n) {
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,21))
		    pdev = pci_find_device(ptlser_pcibrds[b].vendor, ptlser_pcibrds[b].device, pdev);
#else
		    pdev = pci_get_device(ptlser_pcibrds[b].vendor, ptlser_pcibrds[b].device, pdev);
#endif	       		
			if(pdev==NULL){
				b++;
				continue;
			}

			hwconf.pciInfo.busNum = busnum = pdev->bus->number;
			hwconf.pciInfo.devNum = devnum = PCI_SLOT(pdev->devfn)<<3;
			hwconf.pciInfo.pdev = pdev;
			printk(DRIVER_NAME "Found PTLSER %s board(BusNo=%d,DevNo=%d)\n",ptlser_brdname[(int)(ptlser_pcibrds[b].driver_data)-1],busnum,devnum >> 3);
			index++;

			if ( m >= PTLSER_BOARDS) {
				printk(DRIVER_NAME "Too many boards find (maximum %d),board not configured\n",PTLSER_BOARDS);
			} else {
				if ( pci_enable_device(pdev) ) {
					printk(DRIVER_NAME "enable fail !\n");
					continue;
				}
				retval = ptlser_get_PCI_conf(busnum, devnum, (int)ptlser_pcibrds[b].driver_data, &hwconf);
				if (retval < 0) {
					if (retval == PTLSER_ERR_IRQ)
						printk(DRIVER_NAME "Invalid interrupt number,board not configured\n");
					else if (retval == PTLSER_ERR_IRQ_CONFLIT)
						printk(DRIVER_NAME "Invalid interrupt number,board not configured\n");
					else if (retval == PTLSER_ERR_VECTOR)
						printk(DRIVER_NAME "Invalid interrupt vector,board not configured\n");
					else if (retval == PTLSER_ERR_IOADDR)
						printk(DRIVER_NAME "Invalid I/O address,board not configured\n");
					continue;

				}
				
	    		ptlsercfg[m] = hwconf;
	    		//init ptlsercfg first, or ptlsercfg data is not correct on ISR.
	    		//ptlser_initHwwill hook ISR.
	    		if(ptlser_initHw(m,&hwconf)<0)
					continue;
				m++;
			}
		}
	}
#endif
	printk(DRIVER_NAME "Tty devices major number = %d, callout devices major number = %d\n",ttymajor,calloutmajor);

#if (LINUX_VERSION_CODE >= VERSION_CODE(3,7,0))        	
	for (m=0; m < PTLSER_BOARDS; m++) {
		for (i=0; i < PTLSER_PORTS_PER_BOARD; i++) {
			int port_idx = (m * PTLSER_PORTS_PER_BOARD) + i;
			tty_port_init(&ptlser_var_table[port_idx].ttyPort);
			tty_port_link_device(&ptlser_var_table[port_idx].ttyPort, ptlser_var_sdriver, port_idx);
		}
	}
#endif
        ret1 = 0;
        ret2 = 0;

        if ( !(ret1=tty_register_driver(DRV_VAR)) ){
#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))        	
            if ( !(ret2=tty_register_driver(&ptlser_var_cdriver)) ){

                return 0;
            }else{
                tty_unregister_driver(DRV_VAR);
		printk(DRIVER_NAME "Couldn't install PTLSER callout driver !\n");
            }
#else
	    return 0;	            
#endif            
        }else
	    printk(DRIVER_NAME "Couldn't install PTLSER driver !\n");


        if(ret1 || ret2) {
		for (i=0; i<PTLSER_BOARDS; i++) {
			if(ptlsercfg[i].board_type == -1)
				continue;
			else {
				free_irq(ptlsercfg[i].irq, &ptlser_var_table[i*PTLSER_PORTS_PER_BOARD]);
		    //todo: release io, vector
	                }
		}

		return -1;
        }
	
	return 0;	
}




static void __exit	ptlser_module_exit(void)
{
	int i, err=0;

#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,27))
	struct ktermios *tp;
	void *p;
#endif

#if (LINUX_VERSION_CODE < VERSION_CODE(2,6,0))
	if ((err |= tty_unregister_driver(&ptlser_var_cdriver)))
		printk(DRIVER_NAME "Couldn't unregister callout driver\n");
#endif		
	if ((err |= tty_unregister_driver(DRV_VAR)))
		printk(DRIVER_NAME "Couldn't unregister serial driver\n");
#if (LINUX_VERSION_CODE > VERSION_CODE(2,6,27))
	for (i = 0; i < DRV_VAR->num; i++) {
		tp = DRV_VAR->termios[i];
		if (tp) {
			DRV_VAR->termios[i] = NULL;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))		
			/* the ktermios of tty is not get through alloc after kernel 3.7, so we do not
			  *  have to free this */
			kfree(tp);
#endif
		}
#if (LINUX_VERSION_CODE < VERSION_CODE(3,3,0))
		tp = DRV_VAR->termios_locked[i];
		if (tp) {
			DRV_VAR->termios_locked[i] = NULL;
			kfree(tp);
		}
#endif
		if (!(DRV_VAR->flags & TTY_DRIVER_DYNAMIC_DEV))
			tty_unregister_device(DRV_VAR, i);
	}
	p = DRV_VAR->ttys;
	//proc_tty_unregister_driver(DRV_VAR);
	DRV_VAR->ttys = NULL;
#if (LINUX_VERSION_CODE < VERSION_CODE(3,3,0))
	DRV_VAR->termios = DRV_VAR->termios_locked = NULL;
#else
	DRV_VAR->termios = NULL;
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))	
	kfree(p);
#endif
#if (LINUX_VERSION_CODE < VERSION_CODE(3,7,0))
	cdev_del(&DRV_VAR->cdev);
#endif
#endif
 
        for(i=0; i<PTLSER_BOARDS; i++){
		struct pci_dev *pdev;

		if(ptlsercfg[i].board_type == -1){
			continue;
		}
		else{
			pdev = ptlsercfg[i].pciInfo.pdev;
			free_irq(ptlsercfg[i].irq, &ptlser_var_table[i*PTLSER_PORTS_PER_BOARD]);

			if(pdev!=NULL){ //PCI
				iounmap(ptlsercfg[i].ioaddr[1]);
				release_mem_region(pci_resource_start(pdev, 1),	pci_resource_len(pdev, 1));
				release_region(pci_resource_start(pdev,2), pci_resource_len(pdev,2));
			}	
		}
    }
    printk(DRIVER_NAME "unregister successfully\n");
}


module_init(ptlser_module_init);
module_exit(ptlser_module_exit); 
