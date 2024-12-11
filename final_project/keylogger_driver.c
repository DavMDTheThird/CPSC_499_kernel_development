#include <linux/module.h>
#include <linux/usb.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Medina - 829423656");
MODULE_DESCRIPTION("Vendor-Specific USB Driver");
MODULE_VERSION("0.5");

#define VENDOR_ID  0x067B // Prolific Technology Inc.
#define PRODUCT_ID 0x2303 // USB-to-Serial Converter

// ---------------------------------------------------------- Recieve the UART message (encrypted keyboard report)


// ---------------------------------------------------------- Decrypting the information (Needs key)


// ---------------------------------------------------------- Generate a keyboard report



// ---------------------------------------------------------- Reading the Devices
// Table of devices
static const struct usb_device_id my_usb_table[] = {
    {USB_DEVICE(VENDOR_ID, PRODUCT_ID)},{}
};
MODULE_DEVICE_TABLE(usb, my_usb_table);

// Probe function when device is connected
static int my_usb_probe(struct usb_interface *interface, const struct usb_device_id *id){
    printk(KERN_INFO "Vendor-Specific USB Device Connected (VID:PID = %04X:%04X)\n",
           id->idVendor, id->idProduct);
    return 0;
}

// When device is disconnected 
static void my_usb_disconnect(struct usb_interface *interface){
    printk(KERN_INFO "Vendor-Specific USB Device Disconnected\n");
}

/* USB driver structure */
static struct usb_driver my_usb_driver = {
    .name = "keylogger_driver",
    .id_table = my_usb_table,
    .probe = my_usb_probe,
    .disconnect = my_usb_disconnect,
};

// Initiacion funciton that registers the USB driver
static int __init my_usb_init(void){
    return usb_register(&my_usb_driver);
}

// Unregisters the USB driver
static void __exit my_usb_exit(void){
    usb_deregister(&my_usb_driver);
}

module_init(my_usb_init);
module_exit(my_usb_exit);