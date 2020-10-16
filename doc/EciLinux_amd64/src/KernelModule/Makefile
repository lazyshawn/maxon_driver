SHELL := /bin/bash
MOD_NAME := ecikm

TARGET_PATH := /usr/lib/
SOURCE_PATH := ../../bin/release/
LIB_MASK := libeci*DriverLinux.so.*.*.*
LIBUSB1_MASK := libeci*DriverLinux-usb-1.0.so.*.*.*
LIBDFU_MASK := libdfu*DriverLinux.so.*.*.*
OLD_PATH :=$(PATH)
PATH = $(OLD_PATH):/usr/local/sbin:/sbin

ifneq ($(KERNELRELEASE),)
	obj-m := ${MOD_NAME}.o
else
	KERNELBASE ?= /lib/modules/$(shell uname -r)
	KERNELBUILD ?= $(KERNELBASE)/build
	KERNELMOD ?= $(KERNELBASE)/kernel/drivers/misc
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELBUILD) M=$(PWD) modules

install: default install-udev install-module install-lib

install-usb: install-udev install-lib

install-pci: install-udev install-module install-lib

install-module:
	rm -f $(KERNELMOD)/$(MOD_NAME).ko
	install $(MOD_NAME).ko $(KERNELMOD)/
	depmod -a
	modprobe $(MOD_NAME)

install-udev:
	install $(PWD)/60-eci.rules /etc/udev/rules.d/
	@(if [ -f /sbin/initctl ] && [ -f /etc/init/udev.conf ]; then \
	/sbin/initctl reload udev; \
	fi; \
	if [ -f /etc/init.d/udev ]; then \
	/etc/init.d/udev reload > /dev/null; \
	fi)
	
install-lib: install-libusb1.0 install-dfu link-libusb1.0
	install $(SOURCE_PATH)$(LIB_MASK) $(TARGET_PATH)
	ldconfig -n $(TARGET_PATH)
	@(for file in $$(ls $(SOURCE_PATH)$(LIB_MASK) ); \
        do \
        LIB_NAME=$$(echo $$file |awk -F'/' '{print $$(NF)}'); \
        LIB_SONAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2,3); \
        LIB_LNKNAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2); \
        ln -sf $$LIB_SONAME $(TARGET_PATH)/$$LIB_LNKNAME; \
        done)

install-dfu:
	install $(SOURCE_PATH)$(LIBDFU_MASK) $(TARGET_PATH)
	ldconfig -n $(TARGET_PATH)
	@(for file in $$(ls $(SOURCE_PATH)$(LIBDFU_MASK) ); \
        do \
        LIB_NAME=$$(echo $$file |awk -F'/' '{print $$(NF)}'); \
        LIB_SONAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2,3); \
        LIB_LNKNAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2); \
        ln -sf $$LIB_SONAME $(TARGET_PATH)/$$LIB_LNKNAME; \
        done)

install-libusb1.0:
	install $(SOURCE_PATH)$(LIBUSB1_MASK) $(TARGET_PATH)
	ldconfig -n $(TARGET_PATH)
	@(for file in $$(ls $(SOURCE_PATH)$(LIBUSB1_MASK) ); \
        do \
        LIB_NAME=$$(echo $$file |awk -F'/' '{print $$(NF)}'); \
        LIB_SONAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2,3,4); \
        LIB_LNKNAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2,3); \
        ln -sf $$LIB_SONAME $(TARGET_PATH)/$$LIB_LNKNAME; \
        done)

link-libusb1.0:
	ln -sf libeci113DriverLinux-usb-1.0.so $(TARGET_PATH)libeci113DriverLinux.so
	ln -sf libeci116DriverLinux-usb-1.0.so $(TARGET_PATH)libeci116DriverLinux.so

uninstall: uninstall-lib uninstall-udev uninstall-module

uninstall-udev:
	rm -f /etc/udev/rules.d/60-eci.rules

uninstall-module:
	rmmod $(MOD_NAME)
	rm -f $(KERNELMOD)/$(MOD_NAME).ko
	depmod -a

uninstall-lib: uninstall-libusb1.0 uninstall-dfu
	@(for file in $$(ls $(SOURCE_PATH)$(LIB_MASK) ); \
        do \
        LIB_NAME=$$(echo $$file |awk -F'/' '{print $$(NF)}'); \
        LIB_SONAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2,3); \
        LIB_LNKNAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2); \
	rm -f $(TARGET_PATH)/$$LIB_SONAME $(TARGET_PATH)/$$LIB_LNKNAME $(TARGET_PATH)/$$LIB_NAME; \
	done)

uninstall-dfu: 
	@(for file in $$(ls $(SOURCE_PATH)$(LIBDFU_MASK) ); \
        do \
        LIB_NAME=$$(echo $$file |awk -F'/' '{print $$(NF)}'); \
        LIB_SONAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2,3); \
        LIB_LNKNAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2); \
        rm -f $(TARGET_PATH)/$$LIB_SONAME $(TARGET_PATH)/$$LIB_LNKNAME $(TARGET_PATH)/$$LIB_NAME; \
        done)
	
uninstall-libusb1.0: 
	@(for file in $$(ls $(SOURCE_PATH)$(LIBUSB1_MASK) ); \
        do \
        LIB_NAME=$$(echo $$file |awk -F'/' '{print $$(NF)}'); \
        LIB_SONAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2,3,4); \
        LIB_LNKNAME=$$(echo $$LIB_NAME | cut -d \. -f 1,2,3); \
	rm -f $(TARGET_PATH)/$$LIB_SONAME $(TARGET_PATH)/$$LIB_LNKNAME $(TARGET_PATH)/$$LIB_NAME; \
	done)	

clean:
	rm -f $(PWD)/*.o $(PWD)/*.ko $(PWD)/*.order $(PWD)/*.symvers $(PWD)/*.markers $(PWD)/*.mod.c $(PWD)/.*.cmd $(PWD)/.tmp_versions/*.mod
	@(if [ -d $(PWD)/.tmp_versions ]; \
	then rmdir --ignore-fail-on-non-empty $(PWD)/.tmp_versions; \
	fi)
endif
