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
It can launch Nintendo DS, SNES, NES, GameBoy (Color), GameBoy Advance, Sega GameGear/Master System & Mega Drive/Genesis ROMs, as well as DSTWO plugins (if you use a DSTWO) and videos.

# Compiling

## Setting up

Compiling this app requires devkitPro's devkitARM, libnds, grit, and mmutil. These can be installed using [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman) with the following command:
```
sudo dkp-pacman -S nds-dev
```
(Note: Command will vary by OS, sudo may not be needed and it may be just `pacman` instead)

## Building

Once you have devkitPro's toolchains installed you can build the entirety of TWiLight Menu++ by simply running `make package` in the root of the repository. If you only want to build a specific part of TWiLight Menu++ you can `cd` to that folder and run `make dist`.

Once it finishes building, the output files will be in the `7zfile` folder following the same directory structure as the official `TWiLightMenu.7z` builds.

## Using Docker

Using the included [Docker](https://docker.com) image, you can easily compile TWiLight Menu++ without having to manually set up the required version of devkitARM using the provided PowerShell (`.ps1`) scripts.

The script accepts `make` arguments as well. For example, `.\compile_docker.ps1 clean` will clean the directories of all the compiled code. If you would like to build all artifacts, run `.\compile_docker.ps1 package`.

Please note that Docker compilation is not compatible with native compilation on Windows. You should run `.\compile_docker.ps1 clean` to clean the artifacts before attempting to build with Docker. If a notification appears asking you to share your drive, you must choose to enable drive sharing for Docker to work on Windows.

## Manual Pages

The manual pages are stored in a separate repository and downloaded from a release when building TWiLight Menu++. For more information, see the [twilight-manual](https://github.com/DS-Homebrew/twilight-manual) repository.

# Translating

You can help translate TWiLight Menu++ on the [Crowdin project](https://crowdin.com/project/TwilightMenu). If you'd like to request a new language be added then please ask on the [Discord server](https://ds-homebrew.com/discord).

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
- Ryan FB & [xonn83](https://github.com/xonn83): [PicoDriveDS](https://github.com/xonn83/PicodriveDS_GBMacro) (used to launch large Sega Mega Drive/Genesis ROMs)
- [Gericom](https://github.com/Gericom): [GBARunner2](https://github.com/Gericom/GBARunner2) (used to launch GameBoy Advance ROMs outside of the DS Phat/lite's GBA mode)
- [FluBBa](https://gbatemp.net/members/flubba.19963/): [S8DS](https://gbatemp.net/threads/s8ds.392855/) (used to launch Sega Master System/Game Gear ROMs)
- Alekmaul & [wavemotion](https://github.com/wavemotion-dave): [StellaDS](https://github.com/wavemotion-dave/StellaDS), [A5200DS](https://github.com/wavemotion-dave/A5200DS), [A7800DS](https://github.com/wavemotion-dave/A7800DS), and [XEGS-DS](https://github.com/wavemotion-dave/XEGS-DS) (used to launch Atari 2600/5200/7800/XEGS ROMs)
## Graphics & Themes
- [spinal_cord](https://gbatemp.net/members/spinal_cord.90607/): [DSi4DS](https://gbatemp.net/threads/dsi4ds.173617/) and [DSision2](https://gbatemp.net/threads/dsision2.92740/) graphics.
- [StarvingArtist](https://www.deviantart.com/starvingartist/): Game Console icons.
- [FlameKat53](https://github.com/FlameKat53): Manual icon for DSi theme's `SELECT` menu.
- [fail0verflow](https://github.com/fail0verflow/), Fluto, and Arkhandar: Homebrew Channel/Launcher graphics.
- davi: Border for GBC theme (originally for GameYob).
- [NightScript](https://github.com/NightYoshi370/): Reworked Manual pages.
## Music
- IkaMusumeYiyaRoxie: General N64 MIDI Soundfont, used for the title splash fanfare
## Others
- [profi200](https://github.com/profi200): Improved SD code from fastboot3DS.
- [ahezard](https://github.com/ahezard): NDMA code from nds-bootstrap.
- [Gericom](https://github.com/Gericom), TrolleyDave, and FAST6191: GBA SRAM-patching code, used in gbapatcher ([SRAM patching thread at GBATemp](https://gbatemp.net/threads/reverse-engineering-gba-patching.60168/))
- [devkitPro](https://github.com/devkitPro): Code used in nds-hb-menu, and the use of the bootloader, devkitARM, libnds, and libfat.
- Another World & Yellow Wood Goblin: The original akMenu/Wood UI.
- [NightScript](https://github.com/NightYoshi370): Code cleanup, added functionality for Acekard theme in regards to flashcarts.
- [SNBeast](https://github.com/SNBeast): Unlaunch patches.
- retrogamefan & Rudolph: Included AP-patches for nds-bootstrap.
- fintogive, RadDude McCoolguy, KazoWAR, Vague Rant, gamemasterplc, RocketRobz & ChampionLeake: Widescreen Cheats.
## Translators
- Chinese Simplified: [Forbidden](https://crowdin.com/profile/Origami), [James-Makoto](https://crowdin.com/profile/VCMOD55), [R-YaTian](https://github.com/R-YaTian), [Yukino Song](https://crowdin.com/profile/ClassicOldSong), [百地 希留耶](https://crowdin.com/profile/FIve201)
- Chinese Traditional: [ccccchoho](https://crowdin.com/profile/ccccchoho), [James-Makoto](https://crowdin.com/profile/VCMOD55), [Rintim](https://crowdin.com/profile/Rintim), [奇诺比奥](https://crowdin.com/profile/Counta6_233)
- Danish: [jonata](https://github.com/Jonatan6), [Nadia Pedersen](https://crowdin.com/profile/nadiaholmquist)
- French: [Arcky](https://github.com/ArckyTV), [cooolgamer](https://crowdin.com/profile/cooolgamer), [Dhalian.](https://crowdin.com/profile/DHALiaN3630), [maximesharp](https://crowdin.com/profile/maximesharp), [Ghost0159](https://crowdin.com/profile/Ghost0159), [Léo](https://crowdin.com/profile/leeo97one), [LinuxCat](https://github.com/L-i-n-u-x-C-a-t), [Martinez](https://github.com/flutterbrony), [NightScript](https://github.com/NightYoshi370), [TM-47](https://crowdin.com/profile/-tm-)
- German: [ariebe9115](https://crowdin.com/profile/ariebe9115), [Christian Schuhmann](https://github.com/c-schuhmann), [Dubsenbert Reaches](https://crowdin.com/profile/Bierjunge), [İlke Hür Eyiol](https://crowdin.com/profile/ilkecan51), [Julian](https://crowdin.com/profile/nailujx86), [Kazuto](https://crowdin.com/profile/Marcmario), [malekairmaroc7](https://github.com/malekairmaroc7), [Oleh Hatsenko](https://github.com/IRONKAGE), [SkilLP](https://github.com/SkilLP), [SuperSaiyajinStackZ](https://github.com/SuperSaiyajinStackZ), [Tcm0](https://github.com/Tcm0), [TM-47](https://crowdin.com/profile/-tm-), [Uriki](https://github.com/Uriki)
- Hebrew: [Barawer](https://crowdin.com/profile/barawer), [Yaniv](https://crowdin.com/profile/y4niv)
- Hungarian: [Viktor Varga](http://github.com/vargaviktor), [ハトヴィング -- ハット](https://ja.crowdin.com/profile/hatoving)
- Italian: [Leonardo Ledda](https://github.com/LeddaZ), [TM-47](https://crowdin.com/profile/-tm-), [xavimel](https://github.com/xavimel)
- Japanese: [Chromaryu](https://crowdin.com/profile/knight-ryu12), [Pk11](https://github.com/Epicpkmn11), [rinrinrin2002](https://crowdin.com/profile/rinrinrin2002), [Rintim](https://crowdin.com/profile/Rintim), [Ronny Chan](https://github.com/chyyran), [Uriki](https://github.com/Uriki)
- Korean: [I'm Not Cry](https://ja.crowdin.com/profile/cryental), [Myebyeol_NOTE](https://crowdin.com/profile/groovy-mint), [Oleh Hatsenko](https://github.com/IRONKAGE), [그그기그](https://crowdin.com/profile/gigueguegue0803)
- Norwegian: [Nullified Block](https://crowdin.com/profile/elasderas123)
- Polish: [Mateusz Tobiasz](https://crowdin.com/profile/tobiaszmateusz), [RedstonekPL](https://crowdin.com/profile/redstonekpl)
- Portuguese: [bruwyvn](https://crowdin.com/profile/bruwyvn), [Christopher Rodrigues](https://crowdin.com/profile/chrismr197), [Gabz Almeida](https://crowdin.com/profile/connwcted), [jim](https://crowdin.com/profile/hnrwx), [joyrv](https://crowdin.com/profile/joyrv), [leteka 1234](https://crowdin.com/profile/Leaqua21), [Rodrigo Tavares](https://crowdin.com/profile/rodrigodst), [Tiago Silva](https://crowdin.com/profile/TheGameratorT), [Wodson de Andrade](https://crowdin.com/profile/CaptainCheep), [Wodson de Andrade](https://crowdin.com/profile/WodsonKun), [Zak](https://github.com/zekroman)
- Russian: [Alexey Barsukov](https://crowdin.com/profile/lps), [Ckau](https://crowdin.com/profile/Ckau), [manwithnoface](https://github.com/1upus), [mbhz](https://github.com/mbhz)
- Spanish: [Adrin Ramen](https://crowdin.com/profile/adiiramen), [Adrian Rodriguez](https://crowdin.com/profile/ar9555997), [Allinxter](https://crowdin.com/profile/allinxter), [ccccmark](https://github.com/ccccmark), [dimateos](https://crowdin.com/profile/dimateos), [KplyAsteroid](https://crowdin.com/profile/KplyAsteroid), [mschifino](https://crowdin.com/profile/mschifino), [Nicolás Herrera Concha](https://crowdin.com/profile/noname141203), [nuxa17](https://twitter.com/TimeLordJean), [SofyUchiha](https://crowdin.com/profile/sofyuchiha), [TM-47](https://crowdin.com/profile/-tm-), [Uriki](https://github.com/Uriki), [XxPhoenix1996xX](https://github.com/XxPhoenix1996xX)
- Swedish: [Max Hambraeus](https://github.com/maxhambraeus), [Nullified Block](https://crowdin.com/profile/elasderas123), [TM-47](https://crowdin.com/profile/-tm-), [Victor Ahlin](https://crowdin.com/profile/VSwede), [Walter Lindell](https://crowdin.com/profile/walter.lindell)
- Turkish: [İlke Hür Eyiol](https://crowdin.com/profile/ilkecan51), [Yağmur Celep](https://crowdin.com/profile/FixingCarp)
- Ukrainian: [Oleh Hatsenko](https://github.com/IRONKAGE), [TM-47](https://crowdin.com/profile/-tm-)
