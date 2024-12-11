#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>

// Specific vendor and product ID of the USB to UART
#define USB_VENDOR_ID  0x10c4 //    Silicon Labs
#define USB_PRODUCT_ID 0xea60 //    CP210x USB to UART Bridge


// ---------------------------------------------------------- Recieve the UART message (encrypted keyboard report)


// ---------------------------------------------------------- Decrypting the information (Needs key)


// ---------------------------------------------------------- Generate a keyboard report



// ---------------------------------------------------------- Reading the Devices
static struct usb_device_id usb_uart_table[] = {
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    {}
};
MODULE_DEVICE_TABLE(usb, usb_uart_table);

static int keyboard_UART_probe(struct usb_interface *interface, const struct usb_device_id *id){
    printk(KERN_INFO "USB UART device connected: Hello\n");
    return 0;
}

static void keyboard_UART_disconnect(struct usb_interface *interface){
    printk(KERN_INFO "USB UART device disconnected: Goodbye\n");
}

// USB driver structure
static struct usb_driver usb_uart_driver = {
    .name = "keyboard_UART_driver",
    .id_table = usb_uart_table,
    .probe = keyboard_UART_probe,
    .disconnect = keyboard_UART_disconnect,
};

// Module initialization
static int __init keyboard_UART_init(void){
    return usb_register(&usb_uart_driver);
}

// Module cleanup
static void __exit keyboard_UART_exit(void){
    usb_deregister(&usb_uart_driver);
}

module_init(keyboard_UART_init);
module_exit(keyboard_UART_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Medina - 829423656");
MODULE_DESCRIPTION("USB-UART keyboard Driver");
MODULE_VERSION("0.5");

