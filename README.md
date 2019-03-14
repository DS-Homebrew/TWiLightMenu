<p align="center">
 <img src="https://github.com/RocketRobz/TWiLightMenu/blob/master/logo.png"><br>
 <span style="padding-right: 5px;">
  <a href="https://travis-ci.org/RocketRobz/TWiLightMenu">
   <img src="https://travis-ci.org/RocketRobz/TWiLightMenu.svg?branch=master">
    <a href="https://dev.azure.com/evan102112/sap-on-a-tree/_apis/build/status/RocketRobz.TWiLightMenu?branchName=master">
   <img src="https://dev.azure.com/evan102112/sap-on-a-tree/_apis/build/status/RocketRobz.TWiLightMenu?branchName=master">
  </a>
 </span>
 <span style="padding-left: 5px;">
  <a href="https://dshomebrew.serveo.net/">
   <img src="https://github.com/ahezard/nds-bootstrap/blob/master/images/Rocket.Chat button.png" height="20">
  </a>
 </span>
</p>

TWiLight Menu++ is an open-source DSi Menu upgrade/replacement, and frontend for nds-bootstrap for DSi, 3DS, and flashcards.

# Compiling

Compiling this app requires devkitPRO with devkitARM and libnds. Be sure you have grit and mmutil installed.

## Using Docker

Using the included [Docker](https://docker.com) image, you can easily compile TWiLight Menu++ without having to manually set up the required version of devkitARM using the provided PowerShell (`.ps1`) scripts.

The script accepts `make` arguments as well. For example, `.\compile_docker.ps1 clean` will clean the directories of all the compiled code. If you would like to build all artifacts, run `.\compile_docker.ps1 package`.

Please note that Docker compilation is not compatible with native compilation on Windows. You should run `.\compile_docker.ps1 clean` to clean the artifacts before attempting to build with Docker. If a notification appears asking you to share your drive, you must choose to enable drive sharing for Docker to work on Windows.

## Rocket.Chat Server

Would you like a place to talk about your experiences with TWiLight Menu++ or need some assistance? Well, why not join our Rocket.Chat server!

Rocket.Chat is a self-hosted communication platform with the ability to share files and switch to an video/audio conferencing.

Come join today by following this link: https://dshomebrew.serveo.net/ ([alternative link](https://b2b38a00.ngrok.io))

# Credits
## Main Developers
- [RocketRobz](https://github.com/RocketRobz): Lead Developer, implementing the auto-reset power button function used in NTR-mode.
- [chyyran](https://github.com/chyyran): Porting the akMenu/Wood UI to TWiLight Menu++ & custom themes to the DSi/3DS themes.
- [Epicpkmn11](https://github.com/Epicpkmn11): Adding new features to the DSi/3DS and Acekard themes & custom themes on the R4 theme.
## App Launchers
- [ahezard](https://github.com/ahezard): [nds-bootstrap](https://github.com/ahezard/nds-bootstrap), and improved NDMA SD read code.
- [Drenn](https://github.com/Drenn1): [GameYob](https://github.com/Drenn1/GameYob)
- [Coto](https://coto88.bitbucket.io/): Working on [SNEmulDS](https://www.gamebrew.org/wiki/SNEmulDS) (with acheid/Loopy) & [nesDS](https://github.com/RocketRobz/NesDS) ([TWL Edition](https://github.com/ApacheThunder/NesDS) by [Apache Thunder](https://github.com/ApacheThunder)).
- Lordus: [jEnesisDS](https://gamebrew.org/wiki/JEnesisDS)
## Graphics & Themes
- [spinal_cord](https://gbatemp.net/members/spinal_cord.90607/): [DSi4DS](https://gbatemp.net/threads/dsi4ds.173617/) and [DSision2](https://gbatemp.net/threads/dsision2.92740/) graphics.
- [StarvingArtist](https://www.deviantart.com/starvingartist/): Game Console icons.
- [FlameKat53](https://github.com/FlameKat53): Manual icon for DSi theme's `SELECT` menu.
- [Daniel](https://github.com/XD-Daniel-XD): Loading screen for 3DS theme.
## Others
- [profi200](https://github.com/profi200): Improved SD code from fastboot3DS.
- [devkitPro](https://github.com/devkitPro), [WinterMute](https://github.com/WinterMute): Code used in nds-hb-menu, and the use of the bootloader, devkitARM, libnds, and libfat.
- Another World and Yellow Wood Goblin: The original akMenu/Wood UI.
- [shutterbug2000](https://github.com/shutterbug2000): NDMA SD read code, and the muted sound/touchscreen fix for nds-bootstrap.
