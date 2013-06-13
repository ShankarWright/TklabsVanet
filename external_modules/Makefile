obj-m += drivers/net/wireless/ath/
obj-m += drivers/net/wireless/ath/ath5k/
obj-m += net/wireless/
obj-m += net/mac80211/


all:
	make -C ../build/linux-3.4.39 M=../../external_modules modules 

modules_install:
	make -C ../build/linux-3.4.39 M=../../external_modules modules_install 

clean:
	make -C ../build/linux-3.4.39 M=../../external_modules clean



