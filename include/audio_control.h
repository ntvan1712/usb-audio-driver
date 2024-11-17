#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H

#include <linux/types.h>
#include <linux/usb.h>

#define AUDIO_CONTROL_INTERFACE  0x01  
#define AUDIO_STREAM_INTERFACE   0x02  
#define AUDIO_CONTROL_FEATURE_UNIT 0x06 

#define AUDIO_CONTROL_VOLUME      0x01
#define AUDIO_CONTROL_MUTE        0x02
#define AUDIO_CONTROL_BASS        0x03
#define AUDIO_CONTROL_TREBLE      0x04
#define AUDIO_CONTROL_EQUALIZER   0x05

int audio_control_set_volume(struct usb_device *dev, u16 volume);
int audio_control_set_mute(struct usb_device *dev, bool mute);
int audio_control_get_volume(struct usb_device *dev, u16 *volume);
int audio_control_get_mute(struct usb_device *dev, bool *mute);

#endif 
