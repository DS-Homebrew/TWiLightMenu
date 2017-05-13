# About
Aura Launcher is a modern NDS homebrew launcher focussing on aesthetics, based off of DevkitPro's hbmenu which can be used to replace the menu on several DS flashcards. Read more about the core of the launcher [on the hbmenu page][hbmenu].

![Screenshot][Top Screen]

# Installation
To try out the launcher, place `Aura-Launcher.nds` in the root of your card and start it as you would any other homebrew. To replace it as your default launcher, copy the contents of the install folder to the root of your SD card (Overwriting duplicate files).

# Compatible Flash Cards
Currently the following cards are supported:
- Original R4
- EZ Flash 5
- DSTT
- Acekard 2(i)

Many R4i knockoffs are also supported (Such as R4i SDHC and Gold) thanks to fincs, but not included in the download. To use Aura Launcher with one of those cards:

1. [Download fincs' bootstrap][fincs' blog page]
2. Copy the `_DS_MENU.DAT` and `R4.dat` files from that download to the root of the SD card
3. Copy `Aura-Launcher.nds` to the root of the SD card and rename it to `_BOOT_DS.NDS`

# Build Instructions
### Setup DevkitPro
##### Windows
- Download the latest exe devkitpro [installer from here][devkitPro]
- Run the downloaded installer and check "devkitARM" in the custom install

##### Linux
- Download the latest perl script [installer (devkitARMupdate.pl) from here][devkitPro]
- Navigate to the downloaded file and type "perl devkitARMupdate.pl"

### Setup External Library (Easy GL2D DS)
 - Download the [latest GL2D library][GL2D]
 - Extract and merge into your devkitPro folder (In Windows, `C:\devkitPro\` and in Linux, `/opt/devkitPro`)

### Compiling
Download the source code to a directory and in a command prompt, type "make".
Everything should compile successfully. Otherwise, don't hesitate to contact me.

[hbmenu]:https://github.com/devkitPro/nds-hb-menu
[fincs' blog page]:http://fincs.drunkencoders.com/2010/04/13/r4i-christmas-hbmenu-bootstub/
[Top Screen]:https://cloud.githubusercontent.com/assets/5875019/8685470/6dcbc466-2a44-11e5-92af-256503349dbb.png
[devkitPro]:http://sourceforge.net/projects/devkitpro/files/Automated%20Installer/
[GL2D]:http://www.mediafire.com/download/4f8ne79hlci35bc/GL2D.zip
