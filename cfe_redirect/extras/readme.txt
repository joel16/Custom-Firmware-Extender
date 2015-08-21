         (\___/)
         (='.'=)
         ( \ / )
        ('')_('')
         To Alien
http://www.2alien.info
http://www.2alien.net

HostCore 1.2 for 3.80+ by Poison
==============================

A module "bridges" your PC contents to PSP XMB

-=FEATURE=-
With HostCore enabled, you can access your files which locate at PC via USBHostFs/NetHostFs directly from XMB menu.


-=UPDATE=-
1.2:
+add 5.00m33 support.
1.1:
+ add 3.80m33 support.
+ improve wifi compatibility in playing game(bug #0008 some games might not start on wifi mode due to some user memory issues fixed).
- fix bug #0007(randomly fail on loading eboot.bin with error 0x80010013[SCE_ERROR_ERRNO_ENODEV])
- fix bug #0009(crash on exiting some games)

1.0:
+ play ISO/CSO games in Normal mode(need UMD) via wifi.
+ no umd mode support.

before 1.0:
+ play media files locate in PC via USB.
+ play ISO/CSO games in Normal mode(need UMD) via USB.
- fix bug #0003(show double items of the same file in XMB menu)
- fix bug #0001(photo show as corrupted data)
+ access photos locate in PC via Wifi.
- fix bug #0004(crash on photo slideshow)
- fix bug #0002(wrong metadata)
+ play video via Wifi.
- fix bug #0005(crash on video playing)
+ add a mini ui to switch USBHost/NetHost on.
- fix bug #0006(icon corrupted up).
+ add a config file to store setting.


-=REQUIREMENTS=-
A PSP (either fat or slim one) with 3.80+ m33 custom firmware.
Start usbhostfs_pc or nethostfs_pc at your PC side. if you use nethostfs, make sure your computer connect to a wifi router or you are sharing your network on a wifi card.


-=INSTALLATION=-
Copy HostCore folder to your memory stick root.
Add a line ms0:/HostCore/hostcore.prx in your vsh.txt (ms0:/seplugins/vsh.txt), don't forget to enable it.
Open ms0:/HostCore/conf.txt and change some values if you need.


-=HOW TO USE=-
Run usbhostfs_pc or nethostfs_pc on your PC, host a folder which has same structure as ms0:(it contains MUSIC, VIDEO, ISO etc..).
for example, type the following command on your console
usbhostfs_pc /Volumes/Home/PSP/
in wifi case
nethostfs_pc -c 4 -d 20 /Volumes/Home/PSP/
(If you don't pass a folder path to it, it would use the current work path. when you are using windows, you can just place usbhostfs_pc or nethostfs_pc in a folder which contains MUSIC, VIDEO, ISO etc. and double click on the usbhostfs_pc or nethostfs_pc binary to host.)
Go to PSP XMB, press the hotkey(default Note key) to switch on the mode you need.

-=SOURCE=-
http://code.google.com/p/hostcore-psp/source/checkout


-=NOTICE=-
If you find some bugs, feel free to report it to me. you can either leave a message at my blog or send me an email.
In wifi case, only infrastructure mode connection support.


-=CREDIT=-
Thanks to Matchung, Gilleco, Cooleyes, Real.KK, SilverSpring for advise and testing.
Thanks to everyone who has contributed to nethostfs, usbhostfs.
Thanks to everyone who has contributed to the PSPSDK.
Thanks to everyone who has contributed to PSP CFW.
