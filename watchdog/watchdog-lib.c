#include "watchdog-lib.h"
#include <libusb-1.0/libusb.h>
#include <stdlib.h>
#include <stdio.h>


static int watchdog_library_init;
static libusb_context *usb_context;
static int num_cards;
static struct libusb_device_handle **watchdog_card_list;
#pragma pack(push, 1)
struct watchdog_status
{
	uint8_t Length;
	struct time_stamp
	{
		uint8_t DI_Raw;
		uint16_t USBFRAME; 
		uint8_t ReasonCode;
	}ts;
	struct readings
	{
		uint8_t DI_Raw;
		uint8_t DI_Debounced;
		uint8_t DI_Rising; 

		uint8_t DI_Falling;
		uint16_t Temp_Counts;
		uint16_t Humidity_Counts; 
		uint16_t AD0_Counts; 
		uint16_t AD1_Counts;
		uint16_t AD2_Counts;
		uint16_t AD3_Counts;
		uint8_t DI_Changes;
		uint8_t DI_Raw_Changes;
		uint8_t DI_Rising_CoS_Enabled; 
		uint8_t DI_Rising_CoS_Occurred;
		uint8_t DI_Falling_CoS_Enabled;
		uint8_t DI_Falling_CoS_Occurred;
	}r;
	uint8_t DO_READBACK;
};
#pragma pack(pop)

//Internal function declarations
int aio_watchdog_read_status(int card_index, struct watchdog_status *stat);

#define aio_watchdog_library_err_print(status, fmt, ...) \
	do { if (status < 0) printf( "%s:%d:%s(): " fmt, __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__); } while (0);

#define aio_watchdog_debug_print(fmt, ...) \
        do { printf ("%s:%d:%s(): " fmt, __FILE__, \
                                __LINE__, __func__, ##__VA_ARGS__); } while (0)

int aio_watchdog_library_init()
{
	int status = 0;
	if (!watchdog_library_init)
	{
		libusb_device                   **device_list   = NULL;
		struct libusb_device_descriptor usb_descriptor = { 0 };
		ssize_t                         device_count    = 0;
		ssize_t                         i               = 0;
		ssize_t                         opened          = 0;


		libusb_init(&usb_context);

		device_count = libusb_get_device_list(usb_context, &device_list);
		num_cards = 0;
		for ( i = 0 ; i < device_count ; i++ )
		{
			libusb_get_device_descriptor(device_list[i], &usb_descriptor);
			if ((usb_descriptor.idVendor == 0x1605) && 
				((usb_descriptor.idProduct == 0x8080) || (usb_descriptor.idProduct == 0x8081)))
			{
				num_cards++;
			}
		}

		watchdog_card_list = malloc(sizeof(libusb_device_handle *) * num_cards);

		for ( i = 0 ; i < device_count ; i++)
		{
			libusb_get_device_descriptor(device_list[i], &usb_descriptor);
			if ((usb_descriptor.idVendor == 0x1605) && (usb_descriptor.idProduct == 0x8080))
			{
				status = libusb_open(device_list[i], &watchdog_card_list[opened]);
				aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
				status = libusb_claim_interface(watchdog_card_list[opened], 0);
				aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
				//enable ADC
				status = libusb_control_transfer(watchdog_card_list[opened], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 0x1, 0x1, NULL, 0, 0);
				aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
				if (0 == status) opened++;
				
			}
		}
		watchdog_library_init = 1;
	}
	return num_cards;
}

void aio_watchdog_library_term()
{
	if(watchdog_library_init)
	{
		int status = 0;
		int i      = 0;
		watchdog_library_init = 0;
		for ( i = 0 ; i < num_cards ; i++ )
		{
			status = libusb_control_transfer(watchdog_card_list[i], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 0x0, 0x1, NULL, 0, 0);
			aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
			status = libusb_release_interface(watchdog_card_list[i], 0);
			aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
			libusb_close(watchdog_card_list[i]);
		}

		libusb_exit(usb_context);
		num_cards = 0;
	}
}

int aio_watchdog_reinit(int card_index)
{
	int status = 0;
	if (num_cards > card_index)
	{
		status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 0, 0x0052, NULL, 0, 0);
		aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
	}
	else
	{
		status = -1;
		aio_watchdog_debug_print("Invalid card index\n");
	}
	return status;
}

int aio_watchdog_period_set (int card_index, int ms)
{
	int status = 0;
	if (num_cards > card_index)
	{
		status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, ms, 0x0055, NULL, 0, 0);
		aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
	}
	else
	{
		status = -1;
		aio_watchdog_debug_print("Invalid card index\n");
	}
	return status;
}

int aio_watchdog_start (int card_index)
{
	int status = 0;
	if (num_cards > card_index)
	{
		status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 1, 0x0044, NULL, 0, 0);
		aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
	}
	else
	{
		status = -1;
		aio_watchdog_debug_print("Invalid card index\n");
	}
	return status;
}

int aio_watchdog_pet (int card_index)
{
		int status = 0;
	if (num_cards > card_index)
	{
		status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 0, 0x0050, NULL, 0, 0);
		aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
	}
	else
	{
		status = -1;
		aio_watchdog_debug_print("Invalid card index\n");
	}
	return status;
}

int aio_watchdog_stop (int card_index)
{
	int status = 0;
	if (num_cards > card_index)
	{
		status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 0, 0x0044, NULL, 0, 0);
		aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
	}
	else
	{
		status = -1;
		aio_watchdog_debug_print("Invalid card index\n");
	}
	return status;
}

int aio_watchdog_pulse_width_set (int card_index, uint8_t pulse_width)
{
	int status = 0;
	if (num_cards > card_index)
	{
		status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, pulse_width, 0x0045, NULL, 0, 0);
		aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
	}
	else
	{
		status = -1;
		aio_watchdog_debug_print("Invalid card index\n");
	}
	return status;
}

int aio_watchdog_temp_read (int card_index, double *degrees_c)
{
	int status = 0;
	struct watchdog_status stat = { 0 };
	status = aio_watchdog_read_status(card_index, &stat);
	if (0 == status)
	{
		*degrees_c = 175.72/65536 * stat.r.Temp_Counts - 46.85;
	}
	return status;
}

int aio_watchdog_humidity_read (int card_index, double *humidity)
{
	int status = 0;
	struct watchdog_status stat = { 0 };
	aio_watchdog_read_status(card_index, &stat);
	if (0 == status)
	{
		*humidity = 125.0/65536.0 * stat.r.Humidity_Counts - 6;
	}
	return status;
}

int aio_watchdog_a2d_read (int card_index, 
                            double *a2d0, 
                            double *a2d1, 
                            double *a2d2, 
                            double *a2d3)
{
	int status = 0;
	struct watchdog_status stat = { 0 };
	aio_watchdog_read_status(card_index, &stat);
	if (0 == status)
	{
		*a2d0 = stat.r.AD0_Counts;
		*a2d1 = stat.r.AD1_Counts;
		*a2d2 = stat.r.AD2_Counts;
		*a2d3 = stat.r.AD3_Counts;
	}
	return status;
}
int aio_watchdog_read_status(int card_index, struct watchdog_status *stat)
{
	int status = 0;
	if (num_cards > card_index)
	{
		status = libusb_control_transfer(watchdog_card_list[card_index], 
				LIBUSB_REQUEST_TYPE_VENDOR | 0x80, 
				0x10, 
				0, 
				0x0001, 
				(unsigned char *)stat, 
				sizeof(struct watchdog_status), 
				0);
		aio_watchdog_library_err_print(status, "%s\n", libusb_error_name(status));
		if (status > 0)
		{
			status = 0;
		}
		
	}
	else
	{
		status = -1;
		aio_watchdog_debug_print("Invalid card index\n");
	}
	return status;
}

