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
                        if ((usb_descriptor.idVendor == 0x1605) && (usb_descriptor.idProduct == 0x8080))
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
                                if (status < 0)
                                {
                                        printf("%s: libusb_open error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                                }
                                status = libusb_claim_interface(watchdog_card_list[opened], 0);
                                opened++;
                                if (status < 0)
                                {
                                        printf("%s: libusb_claim_interface error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                                }
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
                        status = libusb_release_interface(watchdog_card_list[i], 0);
                        if (status < 0)
                        {
                                printf("%s: libusb_release_interface error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                        }
                        libusb_close(watchdog_card_list[i]);
                }

                libusb_exit(usb_context);
                num_cards = 0;
        }
}

int aio_watchdog_reset(int card_index)
{
        int status = 0;
        if (num_cards > card_index)
        {
                status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 0, 0x52, NULL, 0, 0);
                if (status < 0)
                {
                        printf("%s: libusb_control_transfer error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                }
        }
        else
        {
                printf("%s: Invalid card_index\n", __FUNCTION__);
                status = -1;
        }
        return status;
}

int aio_watchdog_period_set (int card_index, int s)
{
                int status = 0;
        if (num_cards > card_index)
        {
                status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, s, 0x54, NULL, 0, 0);
                if (status < 0)
                {
                        printf("%s: libusb_control_transfer error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                }
        }
        else
        {
                printf("%s: Invalid card_index\n", __FUNCTION__);
                status = -1;
        }
        return status;
}

int aio_watchdog_start (int card_index)
{
        int status = 0;
        if (num_cards > card_index)
        {
                status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 1, 0x44, NULL, 0, 0);
                if (status < 0)
                {
                        printf("%s: libusb_control_transfer error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                }
        }
        else
        {
                printf("%s: Invalid card_index\n", __FUNCTION__);
                status = -1;
        }
        return status;
}

int aio_watchdog_pet (int card_index)
{
                int status = 0;
        if (num_cards > card_index)
        {
                status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 1, 0x50, NULL, 0, 0);
                if (status < 0)
                {
                        printf("%s: libusb_control_transfer error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                }
        }
        else
        {
                printf("%s: Invalid card_index\n", __FUNCTION__);
                status = -1;
        }
        return status;
}
int aio_watchdog_stop (int card_index)
{
        int status = 0;
        if (num_cards > card_index)
        {
                status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, 0, 0x44, NULL, 0, 0);
                if (status < 0)
                {
                        printf("%s: libusb_control_transfer error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                }
        }
        else
        {
                printf("%s: Invalid card_index\n", __FUNCTION__);
                status = -1;
        }
        return status;
}
int aio_watchdog_pulse_width_set (int card_index, uint8_t pulse_width)
{
        int status = 0;
        if (num_cards > card_index)
        {
                status = libusb_control_transfer(watchdog_card_list[card_index], LIBUSB_REQUEST_TYPE_VENDOR, 0x40, pulse_width, 0x45, NULL, 0, 0);
                if (status < 0)
                {
                        printf("%s: libusb_control_transfer error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                }
        }
        else
        {
                printf("%s: Invalid card_index\n", __FUNCTION__);
                status = -1;
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
                *humidity = 125/65536 * stat.r.Humidity_Counts - 6;
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
                                0x1, 
                                (unsigned char *)stat, 
                                sizeof(struct watchdog_status), 
                                0);
                if (status < 0)
                {
                        printf("%s: libusb_control_transfer error \"%s\"\n", __FUNCTION__, libusb_error_name(status));
                }
                else
                {
                        status = 0;
                }
                
        }
        else
        {
                printf("%s: Invalid card_index\n", __FUNCTION__);
                status = -1;
        }
        return status;
}
