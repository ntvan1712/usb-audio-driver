CONFIG_MODULE_SIG=n
CONFIG_MODULE_SIG_ALL=n

obj-m += usb_audio_driver.o
usb_audio_driver-objs := src/usb_audio_driver.o src/audio_control.o
EXTRA_CFLAGS += -I$(PWD)/include


all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
