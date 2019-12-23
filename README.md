<p align="center">
 <img src="https://github.com/DS-Homebrew/TWiLightMenu/blob/master/logo.png"><br>
  <a href="https://gbatemp.net/threads/ds-i-3ds-twilight-menu-gui-for-ds-i-games-and-ds-i-menu-replacement.472200/">
   <img src="https://img.shields.io/badge/GBATemp-Thread-blue.svg">
  </a>
  <a href="https://dev.azure.com/DS-Homebrew/Builds/_build?definitionId=17">
   <img src="https://dev.azure.com/DS-Homebrew/Builds/_apis/build/status/DS-Homebrew.TWiLightMenu?branchName=master" height="20">
  </a>
  <a href="https://discord.gg/dzu8Mp">
   <img src="https://img.shields.io/badge/Discord%20Server-%23twilightmenu-green.svg">
  </a>
</p>

TWiLight Menu++ is an open-source DSi Menu upgrade/replacement for the Nintendo DSi, the Nintendo 3DS, and Nintendo DS flashcards.
It can launch Nintendo DS, SNES, NES, GameBoy (color), GameBoy Advance, Sega GameGear/Master System & Mega Drive/Genesis ROMs, as well as DSTWO plugins (if you use a DSTWO) and `.rvid` videos using RocketVideo Technology.

# Compiling

Compiling this app requires devkitPRO with devkitARM and libnds. Be sure you have grit and mmutil installed.

## Using Docker

Using the included [Docker](https://docker.com) image, you can easily compile TWiLight Menu++ without having to manually set up the required version of devkitARM using the provided PowerShell (`.ps1`) scripts.

The script accepts `make` arguments as well. For example, `.\compile_docker.ps1 clean` will clean the directories of all the compiled code. If you would like to build all artifacts, run `.\compile_docker.ps1 package`.

Please note that Docker compilation is not compatible with native compilation on Windows. You should run `.\compile_docker.ps1 clean` to clean the artifacts before attempting to build with Docker. If a notification appears asking you to share your drive, you must choose to enable drive sharing for Docker to work on Windows.

# Credits
## Main Developers
- [RocketRobz](https://github.com/RocketRobz): Lead Developer, implementing the auto-reset power button function used in NTR-mode.
- [chyyran](https://github.com/chyyran): Porting the akMenu/Wood UI to TWiLight Menu++ & adding the ability to load sub-themes off the SD card for DSi/3DS themes.
- [Epicpkmn11](https://github.com/Epicpkmn11): Adding the ability to load sub-themes off the SD card for the R4 theme.
## App Launchers
- [ahezard](https://github.com/ahezard) & [shutterbug2000](https://github.com/shutterbug2000): [nds-bootstrap](https://github.com/ahezard/nds-bootstrap) (used to launch Nintendo DS ROMs off the SD card), and NDMA SD read code.
- [Drenn](https://github.com/Drenn1): [GameYob](https://github.com/Drenn1/GameYob) (used to launch Gameboy ROMs)
- [Coto](https://coto88.bitbucket.io/): Working on [SNEmulDS](https://www.gamebrew.org/wiki/SNEmulDS) with acheid/Loopy (used to launch SNES ROMs) & [nesDS](https://github.com/RocketRobz/NesDS) ([TWL Edition](https://github.com/ApacheThunder/NesDS) by [Apache Thunder](https://github.com/ApacheThunder)) (used to launch NES ROMs).
- Lordus: [jEnesisDS](https://gamebrew.org/wiki/JEnesisDS) (used to launch Sega Mega Drive/Genesis ROMs)
- [Gericom](https://github.com/Gericom): [GBARunner2](https://github.com/Gericom/GBARunner2) (used to load GameBoy Advance ROMs)
- [FluBBa](https://gbatemp.net/members/flubba.19963/): [S8DS](https://gbatemp.net/threads/s8ds.392855/) (used to launch Sega Master System/Game Gear ROMs)
## Graphics & Themes
- [spinal_cord](https://gbatemp.net/members/spinal_cord.90607/): [DSi4DS](https://gbatemp.net/threads/dsi4ds.173617/) and [DSision2](https://gbatemp.net/threads/dsision2.92740/) graphics.
- [StarvingArtist](https://www.deviantart.com/starvingartist/): Game Console icons.
- [FlameKat53](https://github.com/FlameKat53): Manual icon for DSi theme's `SELECT` menu.
## Others
- [profi200](https://github.com/profi200): Improved SD code from fastboot3DS.
- [devkitPro](https://github.com/devkitPro): Code used in nds-hb-menu, and the use of the bootloader, devkitARM, libnds, and libfat.
- Another World and Yellow Wood Goblin: The original akMenu/Wood UI.
- [NightYoshi370](https://github.com/NightYoshi370): Code cleanup
