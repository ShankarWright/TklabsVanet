obj-m += cfg80211.o

cfg80211-y += core.o sysfs.o radiotap.o util.o reg.o scan.o nl80211.o
cfg80211-y += mlme.o ibss.o sme.o chan.o ethtool.o mesh.o
cfg80211-y += debugfs.o
cfg80211-y += regdb.o

ccflags-y += -DCFG80211_REG_DEBUG
ccflags-y += -DCONFIG_CFG80211_INTERNAL_REGDB
ccflags-y += -DCONFIG_CFG80211_DEBUGFS
ccflags-y += -D__CHECK_ENDIAN__

$(obj)/regdb.c: $(src)/db.txt $(src)/genregdb.awk
	@$(AWK) -f $(srctree)/$(src)/genregdb.awk < $< > $@

all:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/wireless modules 

modules_install:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/wireless modules_install 

clean:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/wireless clean

clean-files := regdb.c
