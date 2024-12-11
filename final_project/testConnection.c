#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/init.h>
#include <linux/input.h>

// Specific vendor and product ID of the USB to UART
#define USB_VENDOR_ID  0x10c4 //    Silicon Labs
#define USB_PRODUCT_ID 0xea60 //    CP210x USB to UART Bridge
// Global input device structure
#define DRIVER_NAME "keyboard_dr"
static struct input_dev *keyboard_dev;


// ---------------------------------------------------------- Recieve the UART message (encrypted keyboard report)


// ---------------------------------------------------------- Decrypting the information (Needs key)


// ---------------------------------------------------------- Generate a keyboard report
// Function to send key events
static void generate_key_events(const int *keycodes, size_t count) {
    size_t i;
    if (!keyboard_dev) {
        pr_err("Input device is not initialized\n");
        return;
    }

    for (i = 0; i < count; i++) {
        int keycode = keycodes[i];
        
        // Key press
        input_report_key(keyboard_dev, keycode, 1);
        
        // Key release
        input_report_key(keyboard_dev, keycode, 0);
    }

    // Sync events
    input_sync(keyboard_dev);
}

// ---------------------------------------------------------- Reading the Devices
static struct usb_device_id usb_uart_table[] = {
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    {}
};
MODULE_DEVICE_TABLE(usb, usb_uart_table);

static int keyboard_UART_probe(struct usb_interface *interface, const struct usb_device_id *id){
    pr_info("USB UART device connected: Hello\n");

    generate_key_events((int[]){KEY_A, KEY_B, KEY_C}, 3);
    
    return 0;
}

static void keyboard_UART_disconnect(struct usb_interface *interface){
    pr_info("USB UART device disconnected: Goodbye\n");
}

// USB driver structure
static struct usb_driver keyboard_UART_driver = {
    .name = "keyboard_UART_driver",
    .id_table = usb_uart_table,
    .probe = keyboard_UART_probe,
    .disconnect = keyboard_UART_disconnect,
};

// Module initialization
static int __init keyboard_UART_init(void){
    int error;
    // Allocate and register the keyboard input device
    keyboard_dev = input_allocate_device();
    if(!keyboard_dev){
        pr_err("Failed to allocate input device\n");
        return -ENOMEM; // Error NO MEMory
    }

    keyboard_dev->name = DRIVER_NAME;
    keyboard_dev->id.bustype = BUS_HOST;
    set_bit(EV_KEY, keyboard_dev->evbit); // Set it so it can make key events

    // Register the input device
    error = input_register_device(keyboard_dev);
    if(error){
        pr_err("Failed to register input device\n");
        input_free_device(keyboard_dev);
        return error;
    }
    pr_info("Keyboard driver initialized\n");

    return usb_register(&keyboard_UART_driver);
}

// Module cleanup
static void __exit keyboard_UART_exit(void){
    if(keyboard_dev){
        input_unregister_device(keyboard_dev);
        keyboard_dev = NULL;
    }
    pr_info("Keyboard driver terminated\n");

    usb_deregister(&keyboard_UART_driver);
}

module_init(keyboard_UART_init);
module_exit(keyboard_UART_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Medina - 829423656");
MODULE_DESCRIPTION("USB-UART keyboard Driver");
MODULE_VERSION("0.5");

