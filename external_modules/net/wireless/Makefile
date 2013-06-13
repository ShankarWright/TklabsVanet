#obj-$(CONFIG_LIB80211) += lib80211.o
obj-m += lib80211.o
ccflags-y += -DCONFIG_LIB80211

#obj-$(CONFIG_CFG80211) += cfg80211.o
obj-m += cfg80211.o
ccflags-y += -DCONFIG_CFG80211

#T##obj-$(CONFIG_LIB80211_CRYPT_WEP) += lib80211_crypt_wep.o
#T#obj-m += lib80211_crypt_wep.o
#T#ccflags-y += -DCONFIG_LIB80211_CRYPT_WEP

#T##obj-$(CONFIG_LIB80211_CRYPT_CCMP) += lib80211_crypt_ccmp.o
#T#obj-m += lib80211_crypt_ccmp.o
#T#ccflags-y += -DCONFIG_LIB80211_CRYPT_CCMP

#T##obj-$(CONFIG_LIB80211_CRYPT_TKIP) += lib80211_crypt_tkip.o
#T#obj-m += lib80211_crypt_tkip.o
#T#ccflags-y += -DCONFIG_LIB80211_CRYPT_TKIP

#obj-$(CONFIG_WEXT_CORE) += wext-core.o
lib80211-y += wext-core.o
ccflags-y += -DCONFIG_WEXT_CORE
ccflags-y += -DCONFIG_WIRELESS_EXT

#obj-$(CONFIG_WEXT_PROC) += wext-proc.o
lib80211-y += wext-proc.o
ccflags-y += -DCONFIG_WEXT_PROC

#obj-$(CONFIG_WEXT_SPY) += wext-spy.o
lib80211-y += wext-spy.o
ccflags-y += -DCONFIG_WEXT_SPY

#obj-$(CONFIG_WEXT_PRIV) += wext-priv.o
lib80211-y += wext-priv.o
ccflags-y += -DCONFIG_WEXT_PRIV

cfg80211-y += core.o sysfs.o radiotap.o util.o reg.o scan.o nl80211.o
cfg80211-y += mlme.o ibss.o sme.o chan.o ethtool.o mesh.o

#cfg80211-$(CONFIG_CFG80211_DEBUGFS) += debugfs.o
cfg80211-y += debugfs.o
ccflags-y += -DCONFIG_CFG80211_DEBUGFS

#cfg80211-$(CONFIG_CFG80211_WEXT) += wext-compat.o wext-sme.o
cfg80211-y += wext-compat.o wext-sme.o
ccflags-y += -DCONFIG_CFG80211_WEXT

#cfg80211-$(CONFIG_CFG80211_INTERNAL_REGDB) += regdb.o
cfg80211-y += regdb.o
ccflags-y += -DCONFIG_CFG80211_INTERNAL_REGDB

#ccflags-y += -D__CHECK_ENDIAN__
ccflags-y += -D__CHECK_ENDIAN__ 
ccflags-y += -DCONFIG_CFG80211_DEFAULT_PS
ccflags-y += -DCONFIG_MAC80211_MESH
ccflags-y += -DCONFIG_CFG80211_DEVELOPER_WARNINGS
ccflags-y += -DCONFIG_NL80211_TESTMODE
ccflags-y += -DCONFIG_CFG80211_DEBUGFS
ccflags-y += -DCONFIG_LIB80211_DEBUG
ccflags-y += -DCONFIG_MODULE_UNLOAD
ccflags-y += -DCONFIG_CFG80211_WEXT
ccflags-y += -DCONFIG_CFG80211_REG_DEBUG
ccflags-y += -DCONFIG_HOTPLUG
#ccflags-y += -DCONFIG_COMPAT


all:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/wireless modules 

modules_install:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/wireless modules_install 

clean:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/wireless clean



$(obj)/regdb.c: $(src)/db.txt $(src)/genregdb.awk
	@$(AWK) -f $(srctree)/$(src)/genregdb.awk < $< > $@

clean-files := regdb.c
