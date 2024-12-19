#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/init.h>
#include <linux/input.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David D. Medina - 829423656");
MODULE_DESCRIPTION("USB-UART-keyboard Driver");
MODULE_VERSION("0.5");

// Specific vendor and product ID of the USB to UART
#define USB_VENDOR_ID_1 0x10c4 
#define USB_PRODUCT_ID_1 0xea60
#define USB_VENDOR_ID_2 0x67b
#define USB_PRODUCT_ID_2 0x2303 
// Interrupt handling structures
static struct urb *interrupt_urb;
static unsigned char *interrupt_buffer;
static struct usb_device *udev; // Global reference to the connected USB device
// Global input device structure
static struct input_dev *keyboard_dev;
int complex_keys[2];
// AES 256 decription
#define AES_KEY_SIZE 32   // 256-bit key
#define AES_IV_SIZE 12    // 12-byte IV for AES-GCM
#define AES_TAG_SIZE 16   // 16-byte GCM tag
// ---------------------------------------------------------- Mapping of HID to Keycodes
static const uint8_t hid_to_linux_keycode[256] = {
    // --- Alphabet --- 
    [0x04] = KEY_A,
    [0x05] = KEY_B,
    [0x06] = KEY_C,
    [0x07] = KEY_D,
    [0x08] = KEY_E,
    [0x09] = KEY_F,
    [0x0A] = KEY_G,
    [0x0B] = KEY_H,
    [0x0C] = KEY_I,
    [0x0D] = KEY_J,
    [0x0E] = KEY_K,
    [0x0F] = KEY_L,
    [0x10] = KEY_M,
    [0x11] = KEY_N,
    [0x12] = KEY_O,
    [0x13] = KEY_P,
    [0x14] = KEY_Q,
    [0x15] = KEY_R,
    [0x16] = KEY_S,
    [0x17] = KEY_T,
    [0x18] = KEY_U,
    [0x19] = KEY_V,
    [0x1A] = KEY_W,
    [0x1B] = KEY_X,
    [0x1C] = KEY_Y,
    [0x1D] = KEY_Z,
    // --- Numbers ---
    [0x1E] = KEY_1,
    [0x1F] = KEY_2,
    [0x20] = KEY_3,
    [0x21] = KEY_4,
    [0x22] = KEY_5,
    [0x23] = KEY_6,
    [0x24] = KEY_7,
    [0x25] = KEY_8,
    [0x26] = KEY_9,
    [0x27] = KEY_0,
    // --- Punctuation Symbols ---
    [0x2C] = KEY_SPACE,
    [0x2D] = KEY_MINUS,
    [0x2E] = KEY_EQUAL,
    [0x2F] = KEY_LEFTBRACE,
    [0x30] = KEY_RIGHTBRACE,
    [0x31] = KEY_BACKSLASH,
    [0x32] = KEY_SEMICOLON,
    [0x33] = KEY_APOSTROPHE,
    [0x34] = KEY_GRAVE,
    [0x35] = KEY_COMMA,
    [0x36] = KEY_DOT,
    [0x37] = KEY_SLASH,
    [0x38] = KEY_KPLEFTPAREN,
    [0x39] = KEY_KPRIGHTPAREN,
    // [0x3A] = KEY_LEFTANGLE,
    // [0x3B] = KEY_RIGHTANGLE,
    // [0x3C] = KEY_LEFTCURLY,
    // [0x3D] = KEY_RIGHTCURLY,
    // [0x3E] = KEY_DQUOT,
    // [0x3F] = KEY_LQUOT,
    // [0x40] = KEY_RQUOT,
    // [0x41] = KEY_LDQUOT,
    // [0x42] = KEY_RDQUOT,
    // --- Arrow Keys ---
    [0x4F] = KEY_RIGHT,
    [0x50] = KEY_LEFT,
    [0x51] = KEY_DOWN,
    [0x52] = KEY_UP,
};

// ---------------------------------------------------------- 1.0 Recieve the UART message (encrypted keyboard report)
static void interrupt_callback(struct urb *urb){
    if (urb->status == 0) {
        pr_info("Interrupt data received: %*ph\n", urb->actual_length, interrupt_buffer);
    } else {
        pr_err("Interrupt URB failed with status %d\n", urb->status);
    }

    // Resubmit the URB to keep listening for interrupts
    if (usb_submit_urb(urb, GFP_ATOMIC)) {
        pr_err("Failed to resubmit interrupt URB\n");
    }
}

// ---------------------------------------------------------- 2.0 Decrypting the information
void decryptMessage(const uint8_t ciphertext[64]){
    // decript
    // check hash
    // check sequence number
}

// ---------------------------------------------------------- 3.0 Generate a keyboard report
// Function to get the modifiers (modifiers bit to key_codes)
void getPressedModifiers(unsigned char modifierByte, uint8_t modifier_keys[8], size_t *mod_count) {
    // Check each bit of the modifier byte and add the corresponding constant to modifier_keys
    if (modifierByte & KEY_LEFTCTRL) {
        modifier_keys[(*mod_count)++] = KEY_LEFTCTRL;
    }
    if (modifierByte & KEY_LEFTSHIFT) {
        modifier_keys[(*mod_count)++] = KEY_LEFTSHIFT;
    }
    if (modifierByte & KEY_LEFTALT) {
        modifier_keys[(*mod_count)++] = KEY_LEFTALT;
    }
    if (modifierByte & KEY_LEFTMETA) {
        modifier_keys[(*mod_count)++] = KEY_LEFTMETA;
    }
    if (modifierByte & KEY_RIGHTCTRL) {
        modifier_keys[(*mod_count)++] = KEY_RIGHTCTRL;
    }
    if (modifierByte & KEY_RIGHTSHIFT) {
        modifier_keys[(*mod_count)++] = KEY_RIGHTSHIFT;
    }
    if (modifierByte & KEY_RIGHTALT) {
        modifier_keys[(*mod_count)++] = KEY_RIGHTALT;
    }
    if (modifierByte & KEY_RIGHTMETA) {
        modifier_keys[(*mod_count)++] = KEY_RIGHTMETA;
    }

    // Fill remaining entries with 0 if fewer than 8 modifiers are pressed
    for (int i = *mod_count; i < 8; i++) {
        modifier_keys[i] = 0;
    }
}

// Function to get the pressed keys (HID to key_codes)
void getPressedKeys(const uint8_t data[64], uint8_t key_codes[8], size_t *key_count) {
    for (int i = 2; i < 8; i++) {
        if(data[i] != 0){
            (*key_count)++;
            uint8_t keycode; 
            keycode = hid_to_linux_keycode[data[i]];

            if(keycode != (uint8_t)0){
                key_codes[*key_count] = keycode;
                (*key_count)++;
            }
            else{
                pr_err("Keycode: %i not found!!\n", data[i]);
            }
        }
        else{
            break;
        }
    }
}

// Function to send key events
static void generate_key_events(uint8_t *modifiers_codes, size_t mod_count, uint8_t *key_codes, size_t key_count) {
    size_t i;
    if (!keyboard_dev) {
        pr_err("Input device is not initialized\n");
        return;
    }

    // Handle modifier key press
    for (i = 0; i < mod_count; i++) {
        int keycode = modifiers_codes[i];
        
        // Key press
        input_report_key(keyboard_dev, keycode, 1);
        input_sync(keyboard_dev);
        pr_info("Modifier pressed: %d\n", keycode);
    }

    // Handle regular key events
    for (i = 0; i < key_count; i++) {
        int keycode = key_codes[i];
        
        // Key press
        input_report_key(keyboard_dev, keycode, 1);
        input_sync(keyboard_dev);
        
        // Key release
        input_report_key(keyboard_dev, keycode, 0);
        input_sync(keyboard_dev);

        pr_info("Key pressed: %d\n", keycode);
    }

    // Handle modifier key release
    for (i = 0; i < mod_count; i++) {
        int keycode = modifiers_codes[i];
        
        // Key press
        input_report_key(keyboard_dev, keycode, 0);
        input_sync(keyboard_dev);
        pr_info("Modifier released: %d\n", keycode);
    }
}

// Function that takes the HID report and generates key events
void hid_to_key_events(const uint8_t data[64]) {
    uint8_t modifier_keys[8];
    uint8_t key_codes[8];
    size_t mod_count = 0;
    size_t key_count = 0;

    // Get the modifier keys
    getPressedModifiers(data[0], modifier_keys, &mod_count);
    // Get the pressed keys
    getPressedKeys(data, key_codes, &key_count);

    // Generate key events
    generate_key_events(modifier_keys, mod_count, key_codes, key_count);
}

// ---------------------------------------------------------- 4.0 Device creation and module initial and exit defitions
static struct usb_device_id usb_uart_table[] = {
    { USB_DEVICE(USB_VENDOR_ID_1, USB_PRODUCT_ID_1) },
    { USB_DEVICE(USB_VENDOR_ID_2, USB_PRODUCT_ID_2) },
    {}
};
MODULE_DEVICE_TABLE(usb, usb_uart_table);

static int keyboard_UART_probe(struct usb_interface *interface, const struct usb_device_id *id){
    int retval;
    struct usb_endpoint_descriptor *endpoint;

    struct usb_device *udev = interface_to_usbdev(interface);

    pr_info("USB-UART connected\n");
    // pr_info("  Serial No   : %s\n", udev->serial ? udev->serial : "Unknown");


    struct usb_device *udev = interface_to_usbdev(interface);
    struct usb_host_interface *iface_desc;
    struct usb_endpoint_descriptor *endpoint;

    pr_info("USB device connected:\n");
    pr_info("  Vendor ID : 0x%04x\n", id->idVendor);
    pr_info("  Product ID: 0x%04x\n", id->idProduct);
    pr_info("  Manufacturer: %s\n", udev->manufacturer ? udev->manufacturer : "Unknown");
    pr_info("  Product     : %s\n", udev->product ? udev->product : "Unknown");
    pr_info("  Serial No   : %s\n", udev->serial ? udev->serial : "Unknown");

    iface_desc = interface->cur_altsetting;
    pr_info("Number of endpoints: %d\n", iface_desc->desc.bNumEndpoints);

    // Iterate through endpoints
    for (int i = 0; i < iface_desc->desc.bNumEndpoints; i++) {
        endpoint = &iface_desc->endpoint[i].desc;

        pr_info("Endpoint[%d]:\n", i);
        pr_info("  Address: 0x%02x\n", endpoint->bEndpointAddress);
        pr_info("  Attributes: 0x%02x\n", endpoint->bmAttributes);
        pr_info("  Max Packet Size: %d\n", le16_to_cpu(endpoint->wMaxPacketSize));
        pr_info("  Interval: %d\n", endpoint->bInterval);

        if (usb_endpoint_is_int_in(endpoint)) {
            pr_info("  -> Interrupt IN endpoint found!\n");
        }
    }






    // Find the interrupt IN endpoint
    endpoint = &interface->cur_altsetting->endpoint[0].desc; // Assuming endpoint[0] is interrupt IN
    if (!usb_endpoint_is_int_in(endpoint)) {
        pr_err("No interrupt IN endpoint found\n");
        return -ENODEV;
    }

    // Allocate buffer for interrupt data
    interrupt_buffer = kmalloc(le16_to_cpu(endpoint->wMaxPacketSize), GFP_KERNEL);
    if (!interrupt_buffer) {
        pr_err("Failed to allocate interrupt buffer\n");
        return -ENOMEM;
    }

    // Allocate URB for interrupt endpoint
    interrupt_urb = usb_alloc_urb(0, GFP_KERNEL);
    if (!interrupt_urb) {
        pr_err("Failed to allocate interrupt URB\n");
        kfree(interrupt_buffer);
        return -ENOMEM;
    }

    // Fill the URB
    usb_fill_int_urb(interrupt_urb, udev,
                     usb_rcvintpipe(udev, endpoint->bEndpointAddress),
                     interrupt_buffer,
                     le16_to_cpu(endpoint->wMaxPacketSize),
                     interrupt_callback,
                     NULL,
                     endpoint->bInterval);

    // Submit the URB to start listening for interrupts
    retval = usb_submit_urb(interrupt_urb, GFP_KERNEL);
    if (retval) {
        pr_err("Failed to submit interrupt URB: %d\n", retval);
        usb_free_urb(interrupt_urb);
        kfree(interrupt_buffer);
        return retval;
    }

    pr_info("Interrupt URB submitted\n");

    uint8_t test[64] = {0x02, 0x00, 0x04, 0x05, 0x06};
    hid_to_key_events(test);
    return 0;
}

static void keyboard_UART_disconnect(struct usb_interface *interface){
    pr_info("USB-UART disconnected\n");

    // Free interrupt handling resources
    if (interrupt_urb) {
        usb_kill_urb(interrupt_urb);
        usb_free_urb(interrupt_urb);
        kfree(interrupt_buffer);
    }

    udev = NULL;
}

// USB driver structure
static struct usb_driver keyboard_UART_driver = {
    .name = "keyboard UART driver",
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

    keyboard_dev->name = "Virtual Keyboard Device";
    keyboard_dev->id.bustype = BUS_VIRTUAL;
    keyboard_dev->evbit[0] = BIT_MASK(EV_KEY);
    
    
    // Enable all key codes
    // For extra safety, banning certain key could be implemented (for example the windows key)
    for (int i = 0; i < KEY_MAX; i++) {
        set_bit(i, keyboard_dev->keybit);
    }

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