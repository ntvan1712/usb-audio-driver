#include <linux/soundcard.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/control.h>
#include <sound/pcm.h>

#include "audio_control.h"

static int index[SNDRV_CARDS] = SNDRV_DEFAULT_IDX;
static char *id_str[SNDRV_CARDS] = SNDRV_DEFAULT_STR;
static bool enable[SNDRV_CARDS] = SNDRV_DEFAULT_ENABLE_PNP;

static int get_dev_num(void)
{
    static int devNum = 1; // Bắt đầu từ 0.

    // Kiểm tra giới hạn trên.
    if (devNum >= SNDRV_CARDS)
    {
        printk(KERN_ERR "[AudioControl.get_dev_num] All cards checked, no valid card found.\n");
        return -1; // Báo lỗi.
    }

    // Bỏ qua các card không được kích hoạt.
    while (!enable[devNum])
    {
        devNum++;
        if (devNum >= SNDRV_CARDS)
        {
            printk(KERN_ERR "[AudioControl.get_dev_num] All cards checked, no valid card found.\n");
            return -1; // Báo lỗi.
        }
    }

    printk(KERN_INFO "[AudioControl.get_dev_num] DevNum %d \n", devNum);
    return devNum;
}

int get_card_index(void)
{
    int devNum = get_dev_num();
    if (devNum < 0)
    {
        printk(KERN_ERR "[AudioControl.get_card_index] Invalid devNum returned.\n");
        return -1; // Báo lỗi.
    }

    return index[devNum];
}

char *get_card_id_str(void)
{
    int devNum = get_dev_num();
    if (devNum < 0)
    {
        printk(KERN_ERR "[AudioControl.get_card_id_str] Invalid devNum returned.\n");
        return NULL; // Báo lỗi.
    }

    return id_str[devNum];
}

int my_pcm_open(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct my_pcm_device *pcm;

    // Cấp phát bộ nhớ cho cấu trúc thiết bị PCM
    pcm = kzalloc(sizeof(*pcm), GFP_KERNEL);
    if (!pcm)
        return -ENOMEM;

    // Lưu trữ thông tin thiết bị trong runtime
    runtime->private_data = pcm;

    // Thiết lập hardware parameters
    runtime->hw = (struct snd_pcm_hardware){
        .info = SNDRV_PCM_INFO_INTERLEAVED | SNDRV_PCM_INFO_BLOCK_TRANSFER,
        .formats = SNDRV_PCM_FMTBIT_S16_LE, // 16-bit signed little endian
        .rates = SNDRV_PCM_RATE_48000,      // Chỉ hỗ trợ 48kHz
        .rate_min = 48000,
        .rate_max = 48000,
        .channels_min = 2, // Stereo
        .channels_max = 2,
        .buffer_bytes_max = 128 * 1024, // Tối đa 128KB buffer
        .period_bytes_min = 1024,
        .period_bytes_max = 128 * 1024,
        .periods_min = 2,
        .periods_max = 16,
        .fifo_size = 0,
    };

    printk(KERN_INFO "[AudioControl.my_pcm_open] PCM device opened\n");

    return 0; // Thành công
}

int my_pcm_close(struct snd_pcm_substream *substream)
{
    struct snd_pcm_runtime *runtime = substream->runtime;
    struct my_pcm_device *pcm = runtime->private_data;

    if (pcm)
    {
        kfree(pcm); // Giải phóng bộ nhớ cho thiết bị PCM
    }

    printk(KERN_INFO "[AudioControl.my_pcm_close] PCM device closed\n");

    return 0; // Thành công
}

// PCM hw params
int my_pcm_hw_params(struct snd_pcm_substream *substream,
                     struct snd_pcm_hw_params *params)
{
    struct my_pcm_device *pcm = substream->runtime->private_data;
    int ret;

    // Cấu hình DMA buffer
    ret = snd_pcm_lib_malloc_pages(substream, params_buffer_bytes(params));
    if (ret < 0)
        return ret;

    pcm->dma_area = substream->runtime->dma_area;
    pcm->dma_bytes = substream->runtime->dma_bytes;

    return 0;
}

int my_pcm_hw_free(struct snd_pcm_substream *substream)
{
    return snd_pcm_lib_free_pages(substream);
}

// Hàm chuẩn bị playback
int my_pcm_prepare(struct snd_pcm_substream *substream)
{
    struct my_pcm_device *pcm = substream->runtime->private_data;

    // Reset trạng thái buffer
    memset(pcm->dma_area, 0, pcm->dma_bytes);

    return 0;
}

// Điều khiển trigger (start/stop)
int my_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
    // struct my_pcm_device *pcm = substream->runtime->private_data;

    switch (cmd)
    {
    case SNDRV_PCM_TRIGGER_START:
        // Bắt đầu playback
        break;
    case SNDRV_PCM_TRIGGER_STOP:
        // Dừng playback
        break;
    default:
        return -EINVAL;
    }

    return 0;
}

// Trả về vị trí con trỏ DMA
snd_pcm_uframes_t my_pcm_pointer(struct snd_pcm_substream *substream)
{
    // struct my_pcm_device *pcm = substream->runtime->private_data;

    // Cần tính toán vị trí con trỏ
    return bytes_to_frames(substream->runtime, 0);
}

int create_alsa_pcm(struct snd_card *card)
{
    struct snd_pcm *pcm;
    struct my_pcm_device *my_pcm;
    int err;

    // Tạo một thiết bị PCM
    err = snd_pcm_new(card, get_card_id_str(), get_card_index(), 1, 0, &pcm); // 1 playback, 0 capture

    if (err < 0)
    {
        printk(KERN_ERR "Failed to create PCM device\n");
        return err;
    }

    snprintf(pcm->id, sizeof(pcm->id), "PlaybackDevice%d", card->number);
    printk(KERN_INFO "[AudioControl.create_alsa_pcm] snd_pcm_new err: %d \n", err);

    // Gán callback PCM
    snd_pcm_set_ops(pcm, SNDRV_PCM_STREAM_PLAYBACK, &my_pcm_ops);

    // Gán thông tin thiết bị
    my_pcm = kzalloc(sizeof(*my_pcm), GFP_KERNEL);
    if (!my_pcm)
        return -ENOMEM;

    spin_lock_init(&my_pcm->lock);
    pcm->private_data = my_pcm;
    pcm->info_flags = 0;
    strcpy(pcm->name, "My PCM Device");

    printk(KERN_INFO "[AudioControl.create_alsa_pcm] Init alsa pcm successfully. name=%s, id=%s\n", pcm->name, pcm->id);

    return 0;
}

// Hàm khởi tạo ALSA controls (volume, mute, ...)
int create_alsa_control(struct snd_card *card)
{
    int err;

    // Tạo control cho volume
    err = create_alsa_volume_control(card);
    printk(KERN_INFO "[UsbAudioDriver.create_alsa_control] create_alsa_volume_control err: %d \n", err);

    if (err < 0)
    {
        printk(KERN_ERR "[AudioControl.init_alsa_control] Failed to create volume control\n");
        return err;
    }

    // Tạo control cho mute
    err = create_alsa_mute_control(card);
    printk(KERN_INFO "[UsbAudioDriver.create_alsa_control] create_alsa_mute_control err: %d \n", err);

    if (err < 0)
    {
        printk(KERN_ERR "[AudioControl.init_alsa_control] Failed to create mute control\n");
        return err;
    }

    printk(KERN_INFO "[AudioControl.init_alsa_control] Init alsa control successfully\n");

    return 0;
}

static int volume_info(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_info *uinfo)
{
    uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
    uinfo->count = 1; // Mono (hoặc 2 nếu Stereo)
    uinfo->value.integer.min = 0;  // Giá trị nhỏ nhất
    uinfo->value.integer.max = 100; // Giá trị lớn nhất
    return 0;
}

// Hàm tạo ALSA control cho volume
int create_alsa_volume_control(struct snd_card *card)
{
    // struct snd_kcontrol_new control_new = {
    //     .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
    //     .name = "Master Volume",
    //     .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
    //     .info = volume_info,
    //     .get = get_alsa_volume,
    //     .put = set_alsa_volume,
    // };

    struct snd_kcontrol control = {
        .info = volume_info,
        .get = get_alsa_volume,
        .put = set_alsa_volume,
    };

    int *volume = kzalloc(sizeof(int), GFP_KERNEL);
    if (!volume)
    {
        printk(KERN_ERR "[AudioControl.create_alsa_volume_control] Failed to allocate memory for volume\n");
        return -ENOMEM;
    }

    *volume = 50; // Giá trị mặc định
    // struct snd_kcontrol *control = snd_ctl_new1(&control_new, volume);

    // if (!control)
    // {
    //     kfree(volume);
    //     printk(KERN_ERR "[AudioControl.create_alsa_volume_control] Failed to create control\n");
    //     return -ENOMEM;
    // }

    return snd_ctl_add(card, &control);
}

// Hàm tạo ALSA control cho mute
int create_alsa_mute_control(struct snd_card *card)
{
    struct snd_kcontrol_new control_new = {
        .iface = SNDRV_CTL_ELEM_IFACE_MIXER,
        .name = "Mute",
        .access = SNDRV_CTL_ELEM_ACCESS_READWRITE,
        .info = snd_ctl_boolean_mono_info,
        .get = get_alsa_mute,
        .put = set_alsa_mute,
    };

    bool *mute = kzalloc(sizeof(bool), GFP_KERNEL);
    if (!mute)
    {
        printk(KERN_ERR "[AudioControl.create_alsa_mute_control] Failed to allocate memory for mute\n");
        return -ENOMEM;
    }

    *mute = false; // Giá trị mặc định
    struct snd_kcontrol *control = snd_ctl_new1(&control_new, mute);
    if (!control)
    {
        kfree(mute);
        printk(KERN_ERR "[AudioControl.create_alsa_mute_control] Failed to create control\n");
        return -ENOMEM;
    }

    return snd_ctl_add(card, control);
}

void dispose_alsa_control(struct snd_card *card)
{
    snd_card_free(card);
}

// return 0 nếu ko có lỗi, ngược lại, return err code
int get_alsa_volume(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    int *volume = kcontrol->private_data;
    ucontrol->value.integer.value[0] = *volume;
    return 0;
}

int set_alsa_volume(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    int *volume = kcontrol->private_data;
    *volume = ucontrol->value.integer.value[0];
    printk(KERN_INFO "[AudioControl.set_alsa_volume] New volume value: %d\n", *volume);
    return 0;
}

// Hàm lấy trạng thái mute từ ALSA control
int get_alsa_mute(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    bool *mute = kcontrol->private_data;              // Truy cập dữ liệu mute
    ucontrol->value.integer.value[0] = *mute ? 1 : 0; // Ghi trạng thái mute vào ucontrol
    return 0;                                         // Thành công
}

// Hàm thay đổi trạng thái mute trong ALSA control
int set_alsa_mute(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
    bool *mute = kcontrol->private_data;              // Truy cập dữ liệu mute
    bool new_mute = ucontrol->value.integer.value[0]; // Lấy trạng thái mới từ ucontrol

    if (*mute == new_mute)
        return 0; // Không thay đổi, không cần xử lý

    *mute = new_mute; // Cập nhật trạng thái mute
    printk(KERN_INFO "[AudioControl.set_alsa_volume] New mute State: %d\n", *mute);
    return 1; // Thay đổi thành công, trả về số lượng control được cập nhật
}
