KERNELDIR := /home/k/linux/kernel
CURRENT_PATH := $(shell pwd)

obj-m := atomicled.o

build: kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean