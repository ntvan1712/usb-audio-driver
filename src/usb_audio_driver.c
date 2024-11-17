#include <linux/usb.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "usb_audio_defs.h"

// Bảng ID thiết bị USB Audio
static struct usb_device_id usb_audio_id_table[] = {
    {USB_DEVICE(USB_AUDIO_VID, USB_AUDIO_PID)},
    {},
};
MODULE_DEVICE_TABLE(usb, usb_audio_id_table);

// Kiểm tra xem đây có phải là giao diện chính hay không
// Vì 1 usb driver có nhiều interface, ví dụ vừa có thể là loa, vừa có thể là mic thoại hoặc cái gì đó
static bool _is_main_interface(struct usb_interface *interface)
{
    unsigned char bClass = interface->cur_altsetting->desc.bInterfaceClass;
    const char *class_name = NULL;

    switch (bClass)
    {
    case USB_AUDIO_CLASS:
        class_name = "USB_AUDIO_CLASS (Audio)";
        break;
    default:
        class_name = "Unknown class";
        break;
    }

    printk(KERN_INFO "[UsbAudioDriver._is_main_interface] bInterfaceClass: %s, code: %u \n", class_name, bClass);

    // return main interface nếu là audio class, vì project là cho loa:)
    return bClass == USB_AUDIO_CLASS;
}


// Hàm xử lý khi thiết bị USB Audio được kết nối
static int usb_audio_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usb_device *dev = interface_to_usbdev(interface);

    if (!_is_main_interface(interface))
    {
        return -ENODEV;
    }

    printk(KERN_INFO "[UsbAudioDriver.usb_audio_probe] USB Audio Device Detected: VID:PID = %04x:%04x\n",
           dev->descriptor.idVendor, dev->descriptor.idProduct);

    // TODO:: làm sau
    // Cấu hình ALSA hoặc tạo kết nối để phát nhạc

    return 0;
}

// Hàm xử lý khi thiết bị USB Audio bị ngắt kết nối
static void usb_audio_disconnect(struct usb_interface *interface)
{

    if (!_is_main_interface(interface))
    {
        return;
    }

    // TODO:: làm sau
    printk(KERN_INFO "[UsbAudioDriver.usb_audio_disconnect] USB Audio Device Disconnected\n");
}

// Register driver
static struct usb_driver usb_audio_driver = {
    .name = "usb_audio_driver",
    .id_table = usb_audio_id_table,
    .probe = usb_audio_probe,
    .disconnect = usb_audio_disconnect,
};

module_usb_driver(usb_audio_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Van");
MODULE_DESCRIPTION("USB Audio Driver");
MODULE_VERSION("1.0");
