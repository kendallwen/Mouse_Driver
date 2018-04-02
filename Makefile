obj-m = KW_IA_Mouse_Driver.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc driver.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm driver

driver: driver.c
	cc -o driver driver.c
