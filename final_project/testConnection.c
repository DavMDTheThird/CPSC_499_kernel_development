#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/init.h>
#include <linux/input.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David D. Medina - 829423656");
MODULE_DESCRIPTION("USB-UART keyboard Driver");
MODULE_VERSION("0.5");

// Specific vendor and product ID of the USB to UART
#define USB_VENDOR_ID  0x10c4 //    Silicon Labs
#define USB_PRODUCT_ID 0xea60 //    CP210x USB to UART Bridge
// Global input device structure
static struct input_dev *keyboard_dev;
int complex_keys[2];

// ---------------------------------------------------------- Recieve the UART message (encrypted keyboard report)


// ---------------------------------------------------------- Decrypting the information (Needs key)


// ---------------------------------------------------------- Generate a keyboard report
// Function to send key events
static void generate_key_events(const int *keymodifiers, size_t mod_count, const int *keycodes, size_t key_count) {
    size_t i;
    if (!keyboard_dev) {
        pr_err("Input device is not initialized\n");
        return;
    }

    // Handle modifier key press
    for (i = 0; i < mod_count; i++) {
        int keycode = keymodifiers[i];
        
        // Key press
        input_report_key(keyboard_dev, keycode, 1);
        input_sync(keyboard_dev);
        pr_info("Modifier pressed: %d\n", keycode);
    }

    // Handle regular key events
    for (i = 0; i < key_count; i++) {
        int keycode = keycodes[i];
        
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
        int keycode = keymodifiers[i];
        
        // Key press
        input_report_key(keyboard_dev, keycode, 0);
        input_sync(keyboard_dev);
        pr_info("Modifier released: %d\n", keycode);
    }
}


// ---------------------------------------------------------- Reading the Devices
static struct usb_device_id usb_uart_table[] = {
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    {}
};
MODULE_DEVICE_TABLE(usb, usb_uart_table);

static int keyboard_UART_probe(struct usb_interface *interface, const struct usb_device_id *id){
    pr_info("USB UART device connected: Hello\n");

    generate_key_events((int[]){KEY_LEFTSHIFT}, 1, (int[]){26, 25, 27}, 3);
    return 0;
}

static void keyboard_UART_disconnect(struct usb_interface *interface){
    pr_info("USB UART device disconnected: Goodbye\n");
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

// ---------------------------------------------------------- Mapping of HID to Modifiers Keycodes
void getPressedModifiers(unsigned char modifierByte, int result[8]) {
    int count = 0;

    // Check each bit of the modifier byte and add the corresponding constant to result
    if (modifierByte & KEY_LEFTCTRL) {
        result[count++] = KEY_LEFTCTRL;
    }
    if (modifierByte & KEY_LEFTSHIFT) {
        result[count++] = KEY_LEFTSHIFT;
    }
    if (modifierByte & KEY_LEFTALT) {
        result[count++] = KEY_LEFTALT;
    }
    if (modifierByte & KEY_LEFTMETA) {
        result[count++] = KEY_LEFTMETA;
    }
    if (modifierByte & KEY_RIGHTCTRL) {
        result[count++] = KEY_RIGHTCTRL;
    }
    if (modifierByte & KEY_RIGHTSHIFT) {
        result[count++] = KEY_RIGHTSHIFT;
    }
    if (modifierByte & KEY_RIGHTALT) {
        result[count++] = KEY_RIGHTALT;
    }
    if (modifierByte & KEY_RIGHTMETA) {
        result[count++] = KEY_RIGHTMETA;
    }

    // Fill remaining entries with 0 if fewer than 8 modifiers are pressed
    for (int i = count; i < 8; i++) {
        result[i] = 0;
    }
}


// ---------------------------------------------------------- Mapping of HID to Keycodes
static const unsigned int hid_to_linux_keycode[256] = {
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
    // --- Key Modifiers ---
    [0xE0] = KEY_LEFTCTRL,
    [0xE1] = KEY_LEFTSHIFT,
    [0xE2] = KEY_LEFTALT,
    [0xE3] = KEY_LEFTMETA,
    [0xE4] = KEY_RIGHTCTRL,
    [0xE5] = KEY_RIGHTSHIFT,
    [0xE6] = KEY_RIGHTALT,
    [0xE7] = KEY_RIGHTMETA,
};