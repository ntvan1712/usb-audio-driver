#include <linux/usb.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>

#include "usb_audio_defs.h"
#include "audio_control.h"

// Bảng ID thiết bị USB Audio
static struct usb_device_id usb_audio_id_table[] = {
    {USB_DEVICE(USB_AUDIO_VID, USB_AUDIO_PID)},
    {},
};

MODULE_DEVICE_TABLE(usb, usb_audio_id_table);

// Khởi tạo ALSA card
static struct snd_card *init_snd_card_from_usb_device(struct usb_device *dev)
{
    // static int devNum;
    struct snd_card *card;
    int err;

    if (!dev)
    {
        printk(KERN_ERR "[UsbAudioDriver.init_snd_card_from_usb_device] Invalid USB device\n");
        return NULL;
    }

    // if (devNum >= SNDRV_CARDS)
    //     return NULL;
    // if (!enable[devNum])
    // {
    //     devNum++;
    //     return NULL;
    // }

    // Tạo một card âm thanh mới
    err = snd_card_new(&dev->dev, get_card_index(), get_card_id_str(), THIS_MODULE, 0, &card);
    printk(KERN_INFO "[UsbAudioDriver.init_snd_card_from_usb_device] snd_card_new err: %d \n", err);

    if (err < 0)
    {
        printk(KERN_ERR "[UsbAudioDriver.init_snd_card_from_usb_device] Failed to create snd_card for USB device: %d\n", err);
        return NULL;
    }

    snprintf(card->shortname, sizeof(card->shortname), "USB Audio Device %d", card->number);
    snprintf(card->longname, sizeof(card->longname), "USB Audio Device at %s", dev_name(&dev->dev));
    snd_card_set_id(card, "MyUSBCard");

    err = snd_card_register(card);

    printk(KERN_INFO "[UsbAudioDriver.init_snd_card_from_usb_device] snd_card_register err: %d \n", err);

    if (err < 0)
    {
        printk(KERN_ERR "[UsbAudioDriver.init_snd_card_from_usb_device] Failed to register snd_card: %d\n", err);
        snd_card_free(card);
        return NULL;
    }

    // snd_card_info_init();

    printk(KERN_INFO "[UsbAudioDriver.init_snd_card_from_usb_device] snd_card created successfully\n");

    return card; // Trả về card đã được tạo
}

static int create_snd_device(struct snd_card *card)
{
    int err;
    static struct snd_device_ops ops = {
        // .dev_free = snd_card_free,
    };

    err = snd_device_new(card, SNDRV_DEV_LOWLEVEL, card, &ops);
    if (err < 0)
    {
        snd_card_free(card);
        return err;
    }
    printk(KERN_INFO "[UsbAudioDriver.snd_device_create] snd_device_new SNDRV_DEV_LOWLEVEL err: %d \n", err);

    err = snd_device_new(card, SNDRV_DEV_PCM, card, &ops);
    if (err < 0)
    {
        snd_card_free(card);
        return err;
    }
    printk(KERN_INFO "[UsbAudioDriver.snd_device_create] snd_device_new SNDRV_DEV_PCM err: %d \n", err);

    err = snd_device_register(card, card);
    if (err < 0)
    {
        snd_card_free(card);
        return err;
    }
    printk(KERN_INFO "[UsbAudioDriver.snd_device_create] snd_device_register err: %d \n", err);

    return 0;
}

// Khi USB Audio device được cắm vào
static int usb_audio_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    struct usb_device *dev = interface_to_usbdev(interface);
    struct snd_card *card;
    int err;

    // Lấy ALSA card từ thiết bị USB
    card = init_snd_card_from_usb_device(dev);
    if (!card)
    {
        return -ENODEV;
    }

    printk(KERN_INFO "[UsbAudioDriver.usb_audio_probe] init_snd_card_from_usb_device ALSA Card Info: name=%s, id=%s\n",
           card->shortname, card->id);

    err = create_snd_device(card);
    if (err < 0)
    {
        printk(KERN_ERR "[UsbAudioDriver.usb_audio_probe] Failed to initialize create_snd_device\n");
        return err;
    }

    interface->dev.driver_data = card; // Lưu trữ card vào driver_data của usb_device

    usb_set_intfdata(interface, card);

    // Khởi tạo các điều khiển ALSA (volume, mute...)
    err = create_alsa_control(card);
    if (err < 0)
    {
        printk(KERN_ERR "[UsbAudioDriver.usb_audio_probe] Failed to initialize ALSA controls\n");
        return err;
    }

    err = create_alsa_pcm(card);
    if (err < 0)
    {
        printk(KERN_ERR "[UsbAudioDriver.usb_audio_probe] Failed to initialize PCM controls\n");
        return err;
    }

    printk(KERN_INFO "[UsbAudioDriver.usb_audio_probe] USB Audio Device Detected: VID:PID = %04x:%04x \n",
           dev->descriptor.idVendor, dev->descriptor.idProduct);

    return 0;
}

// Khi USB Audio device bị ngắt kết nối
static void usb_audio_disconnect(struct usb_interface *interface)
{
    // Giải phóng ALSA card và tài nguyên
    dispose_alsa_control(interface->dev.driver_data);
    printk(KERN_INFO "[UsbAudioDriver.usb_audio_disconnect] USB Audio Device Disconnected\n");
}

// Đăng ký USB driver
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
