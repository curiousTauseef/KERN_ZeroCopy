kernel-dir:=/usr/src/linux-headers-$(shell uname -r)
pwd := $(shell pwd)
mod-name := zcopy_mod
ccflags-y := -std=gnu99
obj-m += $(mod-name).o
$(mod-name)-objs := ./src/node_rule.o ./src/node_ip.o ./src/node_port.o ./src/main.o

all:
	$(MAKE) -C $(kernel-dir) M=$(pwd) modules

install:
	insmod ./$(mod-name).ko

remove:
	rmmod -f $(mod-name)

clean:
	rm -rf *.o *.ko *.order *.symvers *.mod.c .tmp* *.order .$(mod-name).* src/.*

log:
	dmesg | tail -n 50

check:
	./../../checkpatch.pl --no-tree -f $(mod-name).c