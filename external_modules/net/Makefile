obj-m += mac80211/
obj-m += wireless/


all:
	make -C ../../build/linux-3.4.39 M=../../external_modules/net modules 

modules_install:
	make -C ../../build/linux-3.4.39 M=../../external_modules/net modules_install 

clean:
	make -C ../../build/linux-3.4.39 M=../../external_modules/net clean



