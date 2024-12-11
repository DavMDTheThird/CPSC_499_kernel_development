#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/hid.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("David Medina - 829423656");
MODULE_DESCRIPTION("HID keylogger Driver");
MODULE_VERSION("0.5");

#define HID_VENDOR_ID  0x067B // Prolific Technology Inc.
#define HID_PRODUCT_ID 0x2303 // USB-to-Serial Converter

/* Probe function is called when the device is connected */
static int example_hid_probe(struct hid_device *hdev, const struct hid_device_id *id) {
    int ret;

    printk(KERN_INFO "HID example: Device connected.\n");

    /* Initialize the HID device */
    ret = hid_parse(hdev);
    if (ret) {
        printk(KERN_ERR "HID example: Failed to parse HID device.\n");
        return ret;
    }

    ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
    if (ret) {
        printk(KERN_ERR "HID example: Failed to start HID device.\n");
        return ret;
    }

    printk(KERN_INFO "HID example: HID device successfully started.\n");
    return 0;
}

/* Remove function is called when the device is disconnected */
static void example_hid_remove(struct hid_device *hdev) {
    printk(KERN_INFO "HID example: Device disconnected.\n");
    hid_hw_stop(hdev);
}

/* Define the HID device ID table */
static const struct hid_device_id example_hid_table[] = {
    { HID_USB_DEVICE(HID_VENDOR_ID, HID_PRODUCT_ID) },
    { }
};
MODULE_DEVICE_TABLE(hid, example_hid_table);

/* Register the probe and remove functions */
static struct hid_driver example_hid_driver = {
    .name = "hid_example",
    .id_table = example_hid_table,
    .probe = example_hid_probe,
    .remove = example_hid_remove,
};

/* Initialize the HID driver */
static int __init example_hid_init(void) {
    int ret;
    printk(KERN_INFO "HID example: Initializing driver.\n");

    ret = hid_register_driver(&example_hid_driver);
    if (ret) {
        printk(KERN_ERR "HID example: Failed to register driver.\n");
        return ret;
    }

    printk(KERN_INFO "HID example: Driver registered successfully.\n");
    return 0;
}

/* Exit and clean up the HID driver */
static void __exit example_hid_exit(void) {
    printk(KERN_INFO "HID example: Exiting driver.\n");
    hid_unregister_driver(&example_hid_driver);
}

module_init(example_hid_init);
module_exit(example_hid_exit);
