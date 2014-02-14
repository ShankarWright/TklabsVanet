KBUILD_EXTRA_SYMBOLS := $(PWD)/../../../../net/mac80211/Module.symvers

obj-m	+= ath5k/
obj-m	+= ath.o

ath-objs :=	main.o \
		regd.o \
		hw.o \
		key.o 

ath-y	+= debug.o

ccflags-y += -D__CHECK_ENDIAN__
ccflags-y += -DCONFIG_ATH_COMMON
ccflags-y += -DCONFIG_ATH_DEBUG
ccflags-y += -DCONFIG_ATH5K
ccflags-y += -DCONFIG_ATH5K_DEBUG
ccflags-y += -DCONFIG_ATH5K_TRACER
ccflags-y += -DCONFIG_ATH5K_PCI



all:
	make -C ../../../../../build/linux-3.4.39 M=../../external_modules/drivers/net/wireless/ath modules 

modules_install:
	make -C ../../../../../build/linux-3.4.39 M=../../external_modules/drivers/net/wireless/ath modules_install 

clean:
	make -C ../../../../../build/linux-3.4.39 M=../../external_modules/drivers/net/wireless/ath clean
