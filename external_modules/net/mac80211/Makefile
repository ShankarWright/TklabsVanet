#obj-$(CONFIG_MAC80211) += mac80211.o

obj-m += mac80211.o
ccflags-y += -DCONFIG_MAC80211

# mac80211 objects
mac80211-y := \
	main.o status.o \
	sta_info.o \
	wep.o \
	wpa.o \
	scan.o offchannel.o \
	ht.o agg-tx.o agg-rx.o \
	ibss.o \
	work.o \
	iface.o \
	rate.o \
	michael.o \
	tkip.o \
	aes_ccm.o \
	aes_cmac.o \
	cfg.o \
	rx.o \
	spectmgmt.o \
	tx.o \
	key.o \
	util.o \
	wme.o \
	event.o \
	chan.o \
	driver-trace.o mlme.o

#mac80211-$(CONFIG_MAC80211_LEDS) += led.o
mac80211-y += led.o
ccflags-y += -DCONFIG_MAC80211_LEDS

#mac80211-$(CONFIG_MAC80211_DEBUGFS) += 
mac80211-y += debugfs.o
mac80211-y += debugfs_sta.o
mac80211-y += debugfs_netdev.o
mac80211-y += debugfs_key.o
ccflags-y += -DCONFIG_MAC80211_DEBUGFS

#mac80211-$(CONFIG_MAC80211_MESH) += 
mac80211-y += mesh.o
mac80211-y += mesh_pathtbl.o
mac80211-y += mesh_plink.o
mac80211-y += mesh_hwmp.o
ccflags-y += -DCONFIG_MAC80211_MESH

#mac80211-$(CONFIG_PM) += pm.o
mac80211-y += pm.o
ccflags-y += -DCONFIG_PM

CFLAGS_driver-trace.o := -I$(src)

# objects for PID algorithm
rc80211_pid-y := rc80211_pid_algo.o

#rc80211_pid-$(CONFIG_MAC80211_DEBUGFS) += rc80211_pid_debugfs.o
rc80211_pid-y += rc80211_pid_debugfs.o
ccflags-y += -DCONFIG_MAC80211_DEBUGFS

rc80211_minstrel-y := rc80211_minstrel.o
#rc80211_minstrel-$(CONFIG_MAC80211_DEBUGFS) += rc80211_minstrel_debugfs.o
rc80211_minstrel-y += rc80211_minstrel_debugfs.o

rc80211_minstrel_ht-y := rc80211_minstrel_ht.o
#rc80211_minstrel_ht-$(CONFIG_MAC80211_DEBUGFS) += rc80211_minstrel_ht_debugfs.o
rc80211_minstrel_ht-y += rc80211_minstrel_ht_debugfs.o

## RC PID not enabled
##mac80211-$(CONFIG_MAC80211_RC_PID) += $(rc80211_pid-y)
#mac80211-$(CONFIG_MAC80211_RC_MINSTREL) += $(rc80211_minstrel-y)
mac80211-y += $(rc80211_minstrel-y)
ccflags-y += -DCONFIG_MAC80211_RC_MINSTREL

#mac80211-$(CONFIG_MAC80211_RC_MINSTREL_HT) += $(rc80211_minstrel_ht-y)
mac80211-y += $(rc80211_minstrel_ht-y)
ccflags-y += -DCONFIG_MAC80211_RC_MINSTREL_HT

ccflags-y += -D__CHECK_ENDIAN__
ccflags-y += -DCONFIG_MAC80211_MESH
ccflags-y += -DCONFIG_NL80211_TESTMODE
ccflags-y += -DCONFIG_PM
ccflags-y += -DCONFIG_MAC80211_DEBUG_COUNTERS
ccflags-y += -DCONFIG_MAC80211_DEBUGFS
ccflags-y += -DCONFIG_MAC80211_MESH
#ccflags-y += -DCONFIG_MAC80211_HT_DEBUG
#ccflags-y += -DCONFIG_MAC80211_VERBOSE_TDLS_DEBUG
ccflags-y += -DCONFIG_MAC80211_IBSS_DEBUG
ccflags-y += -DCONFIG_MAC80211_VERBOSE_DEBUG
ccflags-y += -DCONFIG_MAC80211_LEDS
#ccflags-y += -DCONFIG_MAC80211_NOINLINE
ccflags-y += -DCONFIG_INET
#ccflags-y += -DCONFIG_PROVE_LOCKING
ccflags-y += -DCONFIG_MODULE_UNLOAD
#ccflags-y += -DCONFIG_MAC80211_VERBOSE_MHWMP_DEBUG
#ccflags-y += -DCONFIG_MAC80211_VERBOSE_MPATH_DEBUG
#ccflags-y += -DCONFIG_MAC80211_VERBOSE_MPL_DEBUG
ccflags-y += -DCONFIG_MAC80211_RC_MINSTREL
ccflags-y += -DCONFIG_MAC80211_RC_MINSTREL_HT
#ccflags-y += -DCONFIG_MAC80211_RC_PID
#ccflags-y += -DCONFIG_MAC80211_VERBOSE_PS_DEBUG
#ccflags-y += -DCONFIG_MAC80211_TKIP_DEBUG
ccflags-y += -I../../../build/linux-3.4.39/include/generated

all:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/mac80211 modules 

modules_install:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/mac80211 modules_install 

clean:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/mac80211 clean

