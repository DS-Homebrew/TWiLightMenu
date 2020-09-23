<p align="center">
 <img src="https://github.com/DS-Homebrew/TWiLightMenu/blob/master/logo.png"><br>
  <a href="https://gbatemp.net/threads/ds-i-3ds-twilight-menu-gui-for-ds-i-games-and-ds-i-menu-replacement.472200/">
   <img src="https://img.shields.io/badge/GBAtemp-Thread-blue.svg" alt="GBAtemp Thread">
  </a>
  <a href="https://discord.gg/yD3spjv">
   <img src="https://img.shields.io/badge/Discord%20Server-%23twilightmenu-green.svg" alt="Discord Server">
  </a>
  <a href="https://github.com/DS-Homebrew/TWiLightMenu/actions?query=workflow%3A%22Build+TWiLight+Menu%2B%2B%22">
   <img src="https://github.com/DS-Homebrew/TWiLightMenu/workflows/Build%20TWiLight%20Menu++/badge.svg" height="20" alt="Build status on GitHub Actions">
  </a>
  <a href="https://crowdin.com/project/TwilightMenu">
    <img src="https://badges.crowdin.net/TwilightMenu/localized.svg" alt="Localized on Crowdin">
  </a>
</p>

TWiLight Menu++ is an open-source DSi Menu upgrade/replacement for the Nintendo DSi, the Nintendo 3DS, and Nintendo DS flashcards.
It can launch Nintendo DS, SNES, NES, GameBoy (color), GameBoy Advance, Sega GameGear/Master System & Mega Drive/Genesis ROMs, as well as DSTWO plugins (if you use a DSTWO) and videos.

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
- [Pk11](https://github.com/Epicpkmn11): Adding the ability to load sub-themes off the SD card for the R4 theme, implemented sorting & made manuals use PNG, improved font rendering, providing a custom background for Unlaunch, and various bug fixes.
## App Launchers
- [ahezard](https://github.com/ahezard) & [shutterbug2000](https://github.com/shutterbug2000): [nds-bootstrap](https://github.com/ahezard/nds-bootstrap) (used to launch Nintendo DS ROMs off the SD card), and NDMA SD read code.
- [Drenn](https://github.com/Drenn1): [GameYob](https://github.com/Drenn1/GameYob) (used to launch Gameboy ROMs)
- [Coto](https://coto88.bitbucket.io/): Working on [SNEmulDS](https://www.gamebrew.org/wiki/SNEmulDS) with acheid/Loopy (used to launch SNES ROMs) & [nesDS](https://github.com/RocketRobz/NesDS) ([TWL Edition](https://github.com/ApacheThunder/NesDS) by [Apache Thunder](https://github.com/ApacheThunder)) (used to launch NES ROMs).
- Lordus: [jEnesisDS](https://gamebrew.org/wiki/JEnesisDS) (used to launch Sega Mega Drive/Genesis ROMs)
- [Gericom](https://github.com/Gericom): [GBARunner2](https://github.com/Gericom/GBARunner2) (used to launch GameBoy Advance ROMs)
- [FluBBa](https://gbatemp.net/members/flubba.19963/): [S8DS](https://gbatemp.net/threads/s8ds.392855/) (used to launch Sega Master System/Game Gear ROMs)
- Alekmaul: [StellaDS](https://github.com/DS-Homebrew/StellaDS) (used to launch Atari 2600 ROMs)
## Graphics & Themes
- [spinal_cord](https://gbatemp.net/members/spinal_cord.90607/): [DSi4DS](https://gbatemp.net/threads/dsi4ds.173617/) and [DSision2](https://gbatemp.net/threads/dsision2.92740/) graphics.
- [StarvingArtist](https://www.deviantart.com/starvingartist/): Game Console icons.
- [FlameKat53](https://github.com/FlameKat53): Manual icon for DSi theme's `SELECT` menu.
- [fail0verflow](https://github.com/fail0verflow/), Fluto, and Arkhandar: Homebrew Channel/Launcher graphics.
- davi: Border for GBC theme (originally for GameYob).
- [NightScript](https://github.com/NightYoshi370/): Reworked Manual pages.
## Others
- [profi200](https://github.com/profi200): Improved SD code from fastboot3DS.
- [ahezard](https://github.com/ahezard): NDMA code from nds-bootstrap.
- [devkitPro](https://github.com/devkitPro): Code used in nds-hb-menu, and the use of the bootloader, devkitARM, libnds, and libfat.
- Another World & Yellow Wood Goblin: The original akMenu/Wood UI.
- [NightScript](https://github.com/NightYoshi370): Code cleanup, added functionality for Acekard theme in regards to flashcarts.
- [SNBeast](https://github.com/SNBeast): Unlaunch patches.
- retrogamefan & Rudolph: Included AP-patches for nds-bootstrap.
- fintogive, RadDude McCoolguy, KazoWAR, Vague Rant, gamemasterplc, RocketRobz & ChampionLeake: Widescreen Cheats.
## Translators
- Chinese Simplified: [Forbidden](https://crowdin.com/profile/Origami), [James-Makoto](https://crowdin.com/profile/VCMOD55), [R-YaTian](https://github.com/R-YaTian)
- Chinese Traditional: [Rintim](https://crowdin.com/profile/Rintim)
- Danish: [jonata](https://github.com/Jonatan6)
- French: [Arcky](https://github.com/ArckyTV), [cooolgamer](https://crowdin.com/profile/cooolgamer), [Dhalian.](https://crowdin.com/profile/DHALiaN3630), [Léo](https://crowdin.com/profile/leeo97one), [LinuxCat](https://github.com/L-i-n-u-x-C-a-t), [Martinez](https://github.com/flutterbrony), [NightScript](https://github.com/NightYoshi370), [T](https://crowdin.com/profile/---T---)
- German: [ariebe9115](https://crowdin.com/profile/ariebe9115), [Christian Schuhmann](https://github.com/c-schuhmann), [Dubsenbert Reaches](https://crowdin.com/profile/Bierjunge), [İlke Hür Eyiol](https://crowdin.com/profile/ilkecan51), [Julian](https://crowdin.com/profile/nailujx86), [Kazuto](https://crowdin.com/profile/Marcmario), [malekairmaroc7](https://github.com/malekairmaroc7), [Oleh Hatsenko](https://github.com/IRONKAGE), [SkilLP](https://github.com/SkilLP), [SuperSaiyajinStackZ](https://github.com/SuperSaiyajinStackZ), [T](https://crowdin.com/profile/---T---), [Tcm0](https://github.com/Tcm0)
- Hungarian: [Viktor Varga](http://github.com/vargaviktor)
- Italian: [Leonardo Ledda](https://github.com/LeddaZ), [T](https://crowdin.com/profile/---T---), [xavimel](https://github.com/xavimel)
- Japanese: [Chromaryu](https://crowdin.com/profile/knight-ryu12), [Pk11](https://github.com/Epicpkmn11), [Rintim](https://crowdin.com/profile/Rintim), [Ronny Chan](https://github.com/chyyran)
- Korean: [Myebyeol_NOTE](https://crowdin.com/profile/groovy-mint), [Oleh Hatsenko](https://github.com/IRONKAGE)
- Polish: [Mateusz Tobiasz](https://crowdin.com/profile/tobiaszmateusz)
- Portuguese: [bruwyvn](https://crowdin.com/profile/bruwyvn), [Christopher Rodrigues](https://crowdin.com/profile/chrismr197), [Gabz Almeida](https://crowdin.com/profile/connwcted), [jim](https://crowdin.com/profile/hnrwx), [joyrv](https://crowdin.com/profile/joyrv), [Tiago Silva](https://crowdin.com/profile/TheGameratorT), [Wodson de Andrade](https://crowdin.com/profile/CaptainCheep), [Zak](https://github.com/zekroman)
- Russian: [Alexey Barsukov](https://crowdin.com/profile/lps), [Ckau](https://crowdin.com/profile/Ckau), [manwithnoface](https://github.com/1upus), [mbhz](https://github.com/mbhz)
- Spanish: [ccccmark](https://github.com/ccccmark), [KplyAsteroid](https://crowdin.com/profile/Kplyasteroid), [mschifino](https://crowdin.com/profile/mschifino), [T](https://crowdin.com/profile/---T---), [Uriki](https://github.com/Uriki), [XxPhoenix1996xX](https://github.com/XxPhoenix1996xX)
- Swedish: [Max Hambraeus](https://github.com/maxhambraeus), [T](https://crowdin.com/profile/---T---), [Walter Lindell](https://crowdin.com/profile/walter.lindell)
- Turkish: [İlke Hür Eyiol](https://crowdin.com/profile/ilkecan51)
- Ukrainian: [Oleh Hatsenko](https://github.com/IRONKAGE), [T](https://crowdin.com/profile/---T---)
