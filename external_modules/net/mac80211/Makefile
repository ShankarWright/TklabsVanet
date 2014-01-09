KBUILD_EXTRA_SYMBOLS := $(PWD)/../wireless/Module.symvers

obj-m += mac80211.o

# mac80211 objects
mac80211-y := \
	main.o status.o \
	sta_info.o \
	wep.o \
	wpa.o \
	scan.o offchannel.o \
	ht.o agg-tx.o agg-rx.o \
	ibss.o \
	wbss.o \
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
	
mac80211-y += \
	debugfs.o \
	debugfs_sta.o \
	debugfs_netdev.o \
	debugfs_key.o

CFLAGS_driver-trace.o := -I$(src)

# objects for PID algorithm
rc80211_pid-y := rc80211_pid_algo.o
rc80211_pid-y += rc80211_pid_debugfs.o

rc80211_minstrel-y := rc80211_minstrel.o
rc80211_minstrel-y += rc80211_minstrel_debugfs.o

rc80211_minstrel_ht-y := rc80211_minstrel_ht.o
rc80211_minstrel_ht-y += rc80211_minstrel_ht_debugfs.o

mac80211-y += $(rc80211_minstrel-y)
mac80211-y += $(rc80211_minstrel_ht-y)
mac80211-y += $(rc80211_pid-y)

ccflags-y += -D__CHECK_ENDIAN__
ccflags-y += -DCONFIG_MAC80211_RC_MINSTREL
ccflags-y += -DCONFIG_MAC80211_RC_MINSTREL_HT
ccflags-y += -DCONFIG_MAC80211_RC_PID
ccflags-y += -DCONFIG_MAC80211_DEBUGFS
#ccflags-y += -DCONFIG_MAC80211_IBSS_DEBUG
#ccflags-y += -DCONFIG_MAC80211_EXTRA_DEBUG
ccflags-y += -DCONFIG_MAC80211_RATE_DEBUG

all:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/mac80211 modules 

modules_install:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/mac80211 modules_install 

clean:
	make -C ../../../build/linux-3.4.39 M=../../external_modules/net/mac80211 clean
