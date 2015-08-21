# Custom-Firmware-Extender
Originally Developed by Cpasjuste. My modificaton is to allow it to work efficently on 6.XX Custom Firmwares

Custom Firmware Extender (aka cfe) is a set of plugins that will add some extra options to your SONY PSP.
With Custom Firmware Extender you should be able to :


•	Change the speed of the PSP CPU/FSB while under the VSH (PSP Menu) or in-game (Umd/Iso/Homebrews) from the cfe menu or with a quick combo key.

•	Change the brightness of the PSP while under the VSH or in-game from the cfe menu or with a quick combo key.

•	Take some screenshot while under the VSH or in-game from the cfe menu or with a quick combo key.

•	Load some PRX plugins while under the VSH or in-game from the cfe menu. Note that on slim PSP, they are loaded in the extra slim memory, so you should be able to load more plugins than you usally can.

•	Start and stop USB under the VSH or in-game from the cfe menu.

•	Play your Umd/Iso/Homebrews on your PC screen and make some in-game videos with the help of remotejoyLite from Akind, based on Tyranid work.

•	Stream/play Iso/Homebrews hosted on your computer via USB (based on HostCore by Poison).

•	Reset and Power off the PSP while under the VSH or in-game.

•	Listen custom music (mp3, at3, oma, omg and aa3) while under the VSH or in-game, based on music.prx by joek2100. Note that if you have some subfolders under an album folder, files won't be found for now.

•	"Time Machine" feature, you can reboot to any TM compatible firmwares, they are automatically detected by cfe 3.0. Note that you must configure each TM firmware to load CFE under the VSH to be able to easly reboot to your original firmware. There is already a sample configuration under the TM folder in this archive. If you encounter some problem's to go back to your original firmware, connect to your PSP via USB, go to the "TM" folder and replace "config.txt" with "config.txt.back" to restore your original configuration.

•	Change/mute volume of each music channel's of the psp.

•	Grab some informations about the PSP memory (Usefull to know how much space a prx plugin require) and battery.

•	Set a default CPU/BUS speed at startup.

•	Set a default brightness level at startup.

•	Configure all the buttons/settings from the cfe menu.


# Changes in 3.1


•	Removed the USB autostart feature since the 5.00 fw include a similar function.

•	Fixed a bug that prevented the auto-sleep and backlight auto-off features to work.

•	Combo button's are back (change the cpu/brightness, take screenshots with a quick combo key).

•	Fully configurable via the menu, changes can be saved. Note that there is now a different configuration file for game and vsh modes.

•	Usb streaming recoded, based on the great work of Poison (hostcore). PC hard drive is redirected to the PSP, alowing streaming of homebrews, music, videos, and isos in any mode (sony, m33, umd..). Just select "Remap usb to ms" in the menu then browse your games like you usally do.

•	Music player improved, can now be completly stopped from the menu, and/or properly restarted with another music directory source.

•	PSP phat compatibility added, but in-game music player is removed for now (not tested).

•	Gui colors configurable via the configuration files ("game.cfg" and "vsh.cfg") in ABGR hex format.

•	Added the possibility to prevent the plugin to be loaded (hold L trigger) while starting a game, or load the light version of cfe (hold R trigger) so extra slim memory is not used, preventing some plugins (pspstates) to not work.


# Installation

•	Copy the content of the "seplugins" folder to the seplugins folder of your memory stick.

•	Take a look at the file "game.cfg" and "vsh.cfg" files under "ms0:/seplugins/cfe" and adjust to your needs.

•	Enable cfe_loader.prx under the m33 recovery menu (Hold R while starting the PSP) in vsh and game. (you may need to activate it under pops to make it work on FAT PSP, strange bug).

•	Set cpu speed to default under the m33 recovery menu.


# Usage

•	Press NOTE + R to load the cfe menu (default configuration).

•	You can DISABLE the cfe plugin by holding the L trigger button while starting the psp or a game if needed.

•	On slim PSP, you can  load the light version of cfe by holding R trigger button  while starting the psp or a game so extra slim memory is not used. This can prevent some plugins (pspstates) to not work.


# Know bugs

•	Waiting for reports @ http://mydedibox.fr/forums/


# Credits

•	Team wildcard, the gui is based on theire sources.

•	Tyranid for psplink.

•	joek2100 for his work on the mp3prx module for CF, giving the source of his work.

•	brethren and AllyOmega for the beta test.

•	People I may forget.

http://mydedibox.fr/
cpasjuste@gmail.com
