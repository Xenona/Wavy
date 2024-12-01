obj-m += pikernely.o

KERN_DIR=/lib/modules/$(shell uname -r)/build/

modules:
	make -C $(KERN_DIR) M=$(PWD) modules
clean:
	make -C $(KERN_DIR) M=$(PWD) clean
help:
	make -C $(KERN_DIR) M=$(PWD) help

test_prog:
	gcc -o test ./test.c

all: modules test_prog
