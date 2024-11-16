#include <linux/usb.h>
#include <linux/slab.h>
#include "audio_control.h"

/* Thiết lập âm lượng */
int audio_control_set_volume(struct usb_device *dev, u16 volume)
{
    int ret;
    unsigned char *data;

    /* Allocating memory for the control request */
    data = kmalloc(2, GFP_KERNEL); // 2 bytes for volume control
    if (!data)
    {
        printk(KERN_ERR "Failed to allocate memory for volume control\n");
        return -ENOMEM;
    }

    /* Setting the volume value */
    data[0] = (volume & 0xFF);        // Low byte
    data[1] = ((volume >> 8) & 0xFF); // High byte

    /* Gửi yêu cầu điều khiển đến thiết bị USB */
    ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0), AUDIO_CONTROL_VOLUME,
                          USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          0, 0, data, 2, 1000);

    kfree(data);
    return ret;
}

/* Thiết lập trạng thái mute (on/off) */
int audio_control_set_mute(struct usb_device *dev, bool mute)
{
    int ret;
    unsigned char *data;

    data = kmalloc(1, GFP_KERNEL); // 1 byte for mute control
    if (!data)
    {
        printk(KERN_ERR "Failed to allocate memory for mute control\n");
        return -ENOMEM;
    }

    /* Nếu mute = true, byte = 0x01, nếu không mute = false, byte = 0x00 */
    data[0] = mute ? 0x01 : 0x00;

    ret = usb_control_msg(dev, usb_sndctrlpipe(dev, 0), AUDIO_CONTROL_MUTE,
                          USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          0, 0, data, 1, 1000);

    kfree(data);
    return ret;
}

/* Lấy giá trị âm lượng */
int audio_control_get_volume(struct usb_device *dev, u16 *volume)
{
    int ret;
    unsigned char *data;

    data = kmalloc(2, GFP_KERNEL); // 2 bytes for volume
    if (!data)
    {
        printk(KERN_ERR "Failed to allocate memory for get volume\n");
        return -ENOMEM;
    }

    ret = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), AUDIO_CONTROL_VOLUME,
                          USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          0, 0, data, 2, 1000);

    if (ret == 0)
    {
        *volume = data[0] | (data[1] << 8);
    }

    kfree(data);
    return ret;
}

/* Lấy trạng thái mute */
int audio_control_get_mute(struct usb_device *dev, bool *mute)
{
    int ret;
    unsigned char *data;

    data = kmalloc(1, GFP_KERNEL); // 1 byte for mute state
    if (!data)
    {
        printk(KERN_ERR "Failed to allocate memory for get mute state\n");
        return -ENOMEM;
    }

    ret = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0), AUDIO_CONTROL_MUTE,
                          USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                          0, 0, data, 1, 1000);

    if (ret == 0)
    {
        *mute = (data[0] == 0x01);
    }

    kfree(data);
    return ret;
}
