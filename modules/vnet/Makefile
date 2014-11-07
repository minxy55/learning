
obj-m	:= vnet.o

KERNELDIR	:= /lib/modules/$(shell uname -r)/build
#KERNELDIR	:= /mnt/dwork/hi3720/kernel/linux-2.6.35
PWD			:= $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

