#ifndef USB_AUDIO_DEFS_H
#define USB_AUDIO_DEFS_H

#define USB_AUDIO_VID  0x001f
#define USB_AUDIO_PID 0x0b21

#define USB_AUDIO_CLASS 0x01
#define USB_AUDIO_SUBCLASS_CONTROL 0x01
#define USB_AUDIO_SUBCLASS_STREAMING 0x02

#endif 

#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME "usb_audio_driver_"
#endif
