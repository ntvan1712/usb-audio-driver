#ifndef AUDIO_CONTROL_H
#define AUDIO_CONTROL_H

#include <linux/soundcard.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>
#include <sound/initval.h>

#define AUDIO_CONTROL_VOLUME 0x01
#define AUDIO_CONTROL_MUTE 0x02

// #define CARD_INDEX 1
// #define CARD_ID "UsbAudioDriverCard"

struct my_pcm_device
{
    struct snd_pcm_substream *substream;
    void *dma_area;
    size_t dma_bytes;
    spinlock_t lock;
};

// int get_dev_num(void);  
int get_card_index(void);  
char *get_card_id_str(void);  


// ** PCM config
int my_pcm_open(struct snd_pcm_substream *substream);
int my_pcm_close(struct snd_pcm_substream *substream);
int my_pcm_hw_params(struct snd_pcm_substream *substream,
                     struct snd_pcm_hw_params *params);
int my_pcm_hw_free(struct snd_pcm_substream *substream);
int my_pcm_prepare(struct snd_pcm_substream *substream);
int my_pcm_trigger(struct snd_pcm_substream *substream, int cmd);
snd_pcm_uframes_t my_pcm_pointer(struct snd_pcm_substream *substream);
int create_alsa_pcm(struct snd_card *card);

static const struct snd_pcm_ops my_pcm_ops = {
    .open = my_pcm_open,
    .close = my_pcm_close,
    .ioctl = snd_pcm_lib_ioctl,
    .hw_params = my_pcm_hw_params,
    .hw_free = my_pcm_hw_free,
    .prepare = my_pcm_prepare,
    .trigger = my_pcm_trigger,
    .pointer = my_pcm_pointer,
};

// ** ALSA control
// Hàm tạo ALSA control cho volume
int create_alsa_volume_control(struct snd_card *card);

// Hàm tạo ALSA control cho mute
int create_alsa_mute_control(struct snd_card *card);

// Hàm khởi tạo ALSA controls (volume, mute, ... )
int create_alsa_control(struct snd_card *card);

// Hàm khi dispose ALSA controls (volume, mute, ... )
void dispose_alsa_control(struct snd_card *card);

// Hàm để lấy và thay đổi giá trị volume từ ALSA control
int get_alsa_volume(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
int set_alsa_volume(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);

// Hàm để lấy và thay đổi trạng thái mute từ ALSA control
int get_alsa_mute(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);
int set_alsa_mute(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol);

#endif // AUDIO_CONTROL_H
