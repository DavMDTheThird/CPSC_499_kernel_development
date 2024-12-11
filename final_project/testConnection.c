#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

/* Define Vendor ID and Product ID of the USB device */
#define USB_VENDOR_ID  0x10c4 // Replace with your USB device's Vendor ID
#define USB_PRODUCT_ID 0xea60 // Replace with your USB device's Product ID

/* USB device ID table */
static struct usb_device_id usb_uart_table[] = {
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    {} /* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, usb_uart_table);

/* Probe function: called when the device is connected */
static int usb_uart_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    printk(KERN_INFO "USB UART device connected: Hello\n");
    return 0;
}

/* Disconnect function: called when the device is disconnected */
static void usb_uart_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "USB UART device disconnected: Goodbye\n");
}

/* USB driver structure */
static struct usb_driver usb_uart_driver = {
    .name = "usb_uart_driver",
    .id_table = usb_uart_table,
    .probe = usb_uart_probe,
    .disconnect = usb_uart_disconnect,
};

/* Module initialization */
static int __init usb_uart_init(void)
{
    return usb_register(&usb_uart_driver);
}

/* Module cleanup */
static void __exit usb_uart_exit(void)
{
    usb_deregister(&usb_uart_driver);
}

module_init(usb_uart_init);
module_exit(usb_uart_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple USB UART Kernel Module");
