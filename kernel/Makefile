ifneq ($(KERNELRELEASE),)

obj-m := psmon.o
psmon-y := \
	main.o \
	process_watch/proc_watch.o \
	process_watch/pw_procfs.o \
	process_watch/pw_info.o \
	processes_monitor/ps_monitor.o \
	processes_monitor/processes.o

else

KDIR ?= /lib/modules/$(shell uname -r)/build

.PHONY: all modules clean

all: modules

modules:
	$(MAKE) -C $(KDIR) M=$(CURDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(CURDIR) clean

endif
