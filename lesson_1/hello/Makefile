KVERSION := $(strip $(shell uname -r))
KDIR := /lib/modules/$(KVERSION)/build
PWD := $(shell pwd)

obj-m := hello.o

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@rm -f *.o .*.cmd .*.flags *.mod.c *.order .*.o *.ko *.mod
	@rm -f .*.*.cmd *.symvers *~ *.*~ TODO.*
	@rm -fR .tmp*
	@rm -rf .tmp_versions