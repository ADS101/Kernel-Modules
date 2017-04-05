obj-m += chardev.o
obj-m += procfs4.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build	M=$(PWD) clean
