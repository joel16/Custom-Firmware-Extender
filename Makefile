VERSION = 3.1

all:
	$(MAKE) -C cfe_redirect/launcher all
	$(MAKE) -C cfe_redirect all
	$(MAKE) -C cfe_redirect/usbhostfs all
	psp-build-exports -s -k -v cfe_redirect/exports.exp
	mv cfe_redirect.S cfe_main
	$(MAKE) -C cfe_main clean
	$(MAKE) -C cfe_main VSH=1
	$(MAKE) -C cfe_main clean
	$(MAKE) -C cfe_main GAME=1
	$(MAKE) -C cfe_main clean
	$(MAKE) -C cfe_main LIGHT=1
	$(MAKE) -C cfe_loader all
	

release: all
	mkdir -p release/seplugins/cfe
	mkdir -p release/TM/150_340/kd
	mkdir -p release/windows/remotejoy
	cp -rf extras/remotejoy_gui/* release/windows/remotejoy
	cp cfe_main/cfe_game.prx release/seplugins/cfe
	cp cfe_main/cfe_vsh.prx release/seplugins/cfe
	cp cfe_main/cfe_light.prx release/seplugins/cfe
	cp cfe_loader/cfe_loader.prx release/seplugins/cfe
	cp cfe_redirect/cfe_redirect.prx release/seplugins/cfe
	cp cfe_redirect/usbhostfs/usbhostfs.prx release/seplugins/cfe
	cp cfe_redirect/launcher/EBOOT.PBP release/seplugins/cfe
	cp extras/RemoteJoyLite.prx release/seplugins/cfe
	cp extras/fake.iso release/seplugins/cfe
	cp extras/game.cfg release/seplugins/cfe
	cp extras/vsh.cfg release/seplugins/cfe
	cp extras/game.txt release
	cp extras/vsh.txt release
	cp extras/TM/150_340/kd/* release/TM/150_340/kd
	cp cfe_main/cfe_vsh.prx release/TM/150_340/
	cp cfe_main/cfe_light.prx release/TM/150_340/
	cp README.odt release
	cp README.doc release
	zip -r -9 Custom_Firmware_Extender_$(VERSION).zip release

clean:
	$(MAKE) -C cfe_redirect/launcher clean
	$(MAKE) -C cfe_redirect clean
	$(MAKE) -C cfe_redirect/usbhostfs clean
	$(MAKE) -C cfe_main VSH=1 clean
	$(MAKE) -C cfe_main GAME=1 clean
	$(MAKE) -C cfe_main LIGHT=1 clean
	$(MAKE) -C cfe_loader clean
	rm -rf release
	rm -f Custom_Firmware_Extender_$(VERSION).zip

