<p align="center">
 <img src="https://github.com/RocketRobz/TWiLightMenu/blob/master/logo.png"><br>
 <span style="padding-right: 5px;">
  <a href="https://travis-ci.org/RocketRobz/TWiLightMenu">
   <img src="https://travis-ci.org/RocketRobz/TWiLightMenu.svg?branch=master">
  </a>
 </span>
 <span style="padding-left: 5px;">
  <a href="https://dshomebrew.serveo.net/">
   <img src="https://github.com/ahezard/nds-bootstrap/blob/master/images/Rocket.Chat button.png" height="20">
  </a>
 </span>
</p>

TWiLight Menu++ is an open-source DSi Menu upgrade/replacement, and frontend for nds-bootstrap for DSi, 3DS, and flashcards.

# Building

Building this app by yourself requires devkitPRO with devkitARM and libnds. Be sure you have grit and mmutil installed.

## Building with Docker

TWiLight Menu++ comes included with a Docker image for easy building without having to manually set up the required version of devkitARM.

You can compile TWiLight Menu++ with Docker using the provided PowerShell (`.ps1`) scripts. First, install Docker at https://docker.com or through your package manager of choice. Then run `compile_docker.ps1` or `compile_docker.sh`. The script accepts `make` arguments as well, for example `.\compile_docker.ps1 clean`, if you would like to build all artifacts, run `.\compile_docker.ps1 package`.

Please note that Docker compilation is not compatible with native compilation if on Windows. You should run `.\compile_docker.ps1 clean` to clean the artifacts before attempting to build with Docker. If a notification appears asking you to share your drive, you must choose to enable drive sharing for Docker to work on Windows.


# Credits
## Main Developers
- [RocketRobz](https://github.com/RocketRobz): Lead Developer, implementing the auto-reset power button function used in NTR-mode, and LED functions to nds-bootstrap.
- Another World and Yellow Wood Goblin: The original akMenu/Wood UI.
- [chyyran](https://github.com/chyyran): Port of akMenu/Wood UI to TWiLightMenu++ as a theme, as well as custom theming on the DSi and 3DS themes, and a bunch of bug fixes.
- [Epicpkmn11](https://github.com/Epicpkmn11): Adding new features to the DSi/3DS Themes, custom themes on the R4 theme, and new features & bug fixes to Acekard theme, and the Switch themes for the Acekard theme.
- [shutterbug2000](https://github.com/shutterbug2000): NDMA SD read code, and the muted sound/touchscreen fix for nds-bootstrap.
## App Launchers
- [ahezard](https://github.com/ahezard): [nds-bootstrap](https://github.com/ahezard/nds-bootstrap), and improved NDMA SD read code.
- [Drenn](https://github.com/Drenn1): [GameYob](https://github.com/Drenn1/GameYob)
- [Coto](https://github.com/cotodevel) & [Apache Thunder](https://github.com/ApacheThunder): [nesDS](https://sourceforge.net/projects/nesds/) ([TWL Edition](https://github.com/ApacheThunder/NesDS)).
- Lordus: [jEnesisDS](https://gamebrew.org/wiki/JEnesisDS)
- archeid (Loopy): [SNEmulDS](https://www.gamebrew.org/wiki/SNEmulDS)
## Graphics & Themes
- [spinal_cord](https://gbatemp.net/members/spinal_cord.90607/): [DSi4DS](https://gbatemp.net/threads/dsi4ds.173617/) and [DSision2](https://gbatemp.net/threads/dsision2.92740/) graphics.
- [StarvingArtist](https://www.deviantart.com/starvingartist/): Some game console icons used.
- [Vulpes-Vulpeos](https://www.deviantart.com/vulpes-vulpeos): MHGen and DS Menu themes for Acekard theme.
- [FlameKat53](https://github.com/FlameKat53): Manual icon for DSi theme's `SELECT` menu.
- [Daniel](https://github.com/XD-Daniel-XD): Loading screen for 3DS theme.
## Others
- [profi200](https://github.com/profi200): Improved SD code from fastboot3DS.
- [devkitPro](https://github.com/devkitPro), [WinterMute](https://github.com/WinterMute): Code used in nds-hb-menu, and the use of the bootloader, devkitARM, libnds, and libfat.
- [DeadSkullzjr](https://github.com/DeadSkullzJr): His [cheat database](https://gbatemp.net/threads/deadskullzjrs-flashcart-cheat-databases.488711/) which is the recommended one for use in nds-bootstrap.

## Rocket.Chat Server

Would you like a place to talk about your experiences with TWiLight Menu++ or need some assistance? Well, why not join our Rocket.Chat server!

Rocket.Chat is a self-hosted communication platform with the ability to share files and switch to an video/audio conferencing.

Come join today by following this link: https://dshomebrew.serveo.net/ ([alternative link](https://b2b38a00.ngrok.io))
