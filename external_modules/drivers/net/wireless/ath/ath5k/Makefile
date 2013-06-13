obj-m		:= ath5k.o

ath5k-y		+= caps.o
ath5k-y		+= initvals.o
ath5k-y		+= eeprom.o
ath5k-y		+= gpio.o
ath5k-y		+= desc.o
ath5k-y		+= dma.o
ath5k-y		+= qcu.o
ath5k-y		+= pcu.o
ath5k-y		+= phy.o
ath5k-y		+= reset.o
ath5k-y		+= attach.o
ath5k-y		+= base.o
ath5k-y		+= led.o
ath5k-y		+= rfkill.o
ath5k-y		+= ani.o
ath5k-y		+= sysfs.o
ath5k-y		+= mac80211-ops.o
ath5k-y		+= pci.o
ath5k-y		+= debug.o
#ath5k-y		+= ahb.o  

ccflags-y += -D__CHECK_ENDIAN__
ccflags-y += -DCONFIG_ATH_COMMON
ccflags-y += -DCONFIG_ATH_DEBUG
ccflags-y += -DCONFIG_ATH5K
ccflags-y += -DCONFIG_ATH5K_DEBUG
ccflags-y += -DCONFIG_ATH5K_TRACER
ccflags-y += -DCONFIG_ATH5K_PCI

all: 
    make -C ../../../../../../build/linux-3.4.39 M=../../external_modules/drivers/net/wireless/ath/ath5k modules

modules_install: 
    make -C ../../../../../../build/linux-3.4.39 M=../../external_modules/drivers/net/wireless/ath/ath5k modules_install

clean: 
    make -C ../../../../../../build/linux-3.4.39 M=../../external_modules/drivers/net/wireless/ath/ath5k clean

