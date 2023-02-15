<p align="center">
 <img src="https://github.com/DS-Homebrew/TWiLightMenu/blob/master/logo.png"><br>
  <a href="https://gbatemp.net/threads/ds-i-3ds-twilight-menu-gui-for-ds-i-games-and-ds-i-menu-replacement.472200/">
   <img src="https://img.shields.io/badge/GBAtemp-Thread-blue.svg" alt="GBAtemp Thread">
  </a>
  <a href="https://discord.gg/yD3spjv">
   <img src="https://img.shields.io/badge/Discord%20Server-%23twilight--menu--dev-green.svg" alt="Discord Server">
  </a>
  <a href="https://github.com/DS-Homebrew/TWiLightMenu/actions?query=workflow%3A%22Build+TWiLight+Menu%2B%2B%22">
   <img src="https://github.com/DS-Homebrew/TWiLightMenu/actions/workflows/nightly.yml/badge.svg" height="20" alt="Build status on GitHub Actions">
  </a>
  <a href="https://crowdin.com/project/TwilightMenu">
    <img src="https://badges.crowdin.net/TwilightMenu/localized.svg" alt="Localized on Crowdin">
  </a>
</p>

TWiLight Menu++ is an open-source DSi Menu upgrade/replacement for the Nintendo DSi, the Nintendo 3DS, and Nintendo DS flashcards.
It can launch Nintendo DS(i), SNES, NES, GameBoy (Color), GameBoy Advance, Sega GameGear/Master System/SG-1000 & Mega Drive/Genesis, Atari 2600/5200/7800/XEGS, Intellivision, Neo Geo Pocket, Sord M5, PC Engine/TurboGrafx-16, WonderSwan, and ColecoVision ROMs, as well as DSTWO plugins (if you use a DSTWO).

# Compiling

## Setting up

Compiling this app requires devkitPro's devkitARM, libnds, grit, and mmutil. These can be installed using [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman) with the following command:
```
sudo dkp-pacman -S nds-dev
```
(Note: Command will vary by OS, sudo may not be needed and it may be just `pacman` instead.)

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
- [Rocket Robz](https://github.com/RocketRobz): Lead Developer
- [chyyran](https://github.com/chyyran): Porting the akMenu/Wood UI to TWiLight Menu++ & adding the ability to load sub-themes off the SD card for DSi/3DS themes
- [Pk11](https://github.com/Epicpkmn11): Adding the ability to load skins off the SD card for the R4 theme, implemented sorting & made manuals use PNG, improved font rendering, providing a custom background for Unlaunch, managing translations, and various bug fixes
## Secondary Developers
- [DieGo](https://github.com/DieGo367): Adding support for custom ROM/folder icons and improvements for custom skins
- [lifehackerhansol](https://github.com/lifehackerhansol): Improving support for flashcard autobooting and kernel loading
- [NightScript](https://github.com/NightScript370): Code cleanup, defining code standards, added flashcard functionality for Wood UI theme, manual pages
## App Launchers
- [ahezard](https://github.com/ahezard), [shutterbug2000](https://github.com/shutterbug2000) and [Rocket Robz](https://github.com/RocketRobz): [nds-bootstrap](https://github.com/ahezard/nds-bootstrap) (used for launching Nintendo DS(i) ROMs off the SD card)
- [Alekmaul](https://github.com/alekmaul): [AmeDS](https://www.gamebrew.org/wiki/AmeDS) (used for launching Amstrad CPC ROMs)
- [Alekmaul](https://github.com/alekmaul) & [wavemotion](https://github.com/wavemotion-dave): [StellaDS](https://github.com/wavemotion-dave/StellaDS), [A5200DS](https://github.com/wavemotion-dave/A5200DS), [A7800DS](https://github.com/wavemotion-dave/A7800DS), [XEGS-DS](https://github.com/wavemotion-dave/XEGS-DS) (used for launching Atari 2600/5200/7800/XEGS ROMs), and [ColecoDS](https://github.com/wavemotion-dave/ColecoDS) (used for launching ColecoVision, Sord M5, and Sega SG-1000 ROMs)
- archiede: [SNEmulDS](https://www.gamebrew.org/wiki/SNEmulDS) (used for launching SNES ROMs)
     - [Coto](https://coto88.bitbucket.io/): [SNEmulDS Revival](https://bitbucket.org/Coto88/snemulds)
- [chishm](https://github.com/chishm): [tuna-viDS](https://github.com/chishm/tuna-vids) (used for playing Xvid videos)
- [Drenn](https://github.com/Drenn1): [GameYob](https://github.com/Drenn1/GameYob) (used for launching Gameboy ROMs)
- [FluBBaOfWard](https://github.com/FluBBaOfWard): [S8DS](https://github.com/FluBBaOfWard/S8DS) (used for launching Sega Master System, Game Gear, SG-1000, and ColecoVision ROMs), [NitroGrafx](https://github.com/FluBBaOfWard/NitroGrafx) (used for launching PC Engine/TurboGrafx-16 ROMs), [NitroSwan](https://github.com/FluBBaOfWard/NitroSwan) (used for launching WonderSwan ROMs), and [NGPDS](https://github.com/FluBBaOfWard/NGPDS) (used for launching Neo Geo Pocket ROMs)
- [FluBBaOfWard](https://github.com/FluBBaOfWard), [Coto](https://coto88.bitbucket.io/), and Loopy: [nesDS](https://github.com/DS-Homebrew/NesDS) (used for launching NES ROMs)
     - [Apache Thunder](https://github.com/ApacheThunder): [TWL Edition](https://github.com/ApacheThunder/NesDS) of nesDS
- [Gericom](https://github.com/Gericom): [FastVideoDSPlayer](https://github.com/Gericom/FastVideoDSPlayer) (used for launching FastVideoDS videos) & [GBARunner2](https://github.com/Gericom/GBARunner2) (used for launching GameBoy Advance ROMs outside of the DS Phat/Lite's GBA mode)
     - [therealteamplayer](https://github.com/therealteamplayer): [Hicode+DSP](https://github.com/therealteamplayer/GBARunner2) merged builds of GBARunner2 included for DSi/3DS
     - [unresolvedsymbol](https://github.com/unresolvedsymbol): [rom3M+master "DSL-Enhanced"](https://github.com/unresolvedsymbol/GBARunner2-DSL-Enhanced) builds of GBARunner2 included for flashcards
- Lordus: [jEnesisDS](https://gamebrew.org/wiki/JEnesisDS) (used for launching Sega Mega Drive/Genesis ROMs)
     - [xonn83](https://github.com/xonn83): GBMacro version of [jEnesisDS](https://github.com/xonn83/jEnesisDS_macro)
- [redbug26](https://github.com/redbug26): [CrocoDS](https://github.com/redbug26/crocods-nds) (used for launching Amstrad CPC ROMs)
- Ryan FB, [Rocket Robz](https://github.com/RocketRobz), and [xonn83](https://github.com/xonn83): [PicoDriveTWL](https://github.com/DS-Homebrew/PicoDriveTWL) (used for launching large Sega Mega Drive/Genesis ROMs)
- [wavemotion](https://github.com/wavemotion-dave): [Nintellivision/NINTV-DS](https://github.com/wavemotion-dave/NINTV-DS) (used for launching Intellivision ROMs)
## Graphics & Themes
- [Absent-Reality](http://pixeljoint.com/p/19283.htm): Intellivision icon
- davi: Border for GBC theme (originally for GameYob)
- [fail0verflow](https://github.com/fail0verflow/), Fluto, and Arkhandar: Homebrew Channel/Launcher graphics
- [FlameKat53](https://github.com/FlameKat53): Manual icon for DSi theme's `SELECT` menu
- [Mr. Start](https://github.com/Arthur-Start): Super Nintendo DS splash screen
- [spinal_cord](https://gbatemp.net/members/spinal_cord.90607/): [DSi4DS](https://gbatemp.net/threads/dsi4ds.173617/) and [DSision2](https://gbatemp.net/threads/dsision2.92740/) graphics
- [StarvingArtist](https://www.deviantart.com/starvingartist/): Game Console icons
## Music
- [BlastoiseVeteran](https://soundcloud.com/blastyveteran): Remastered version of Nintendo DSi Shop music
- IkaMusumeYiyaRoxie: General N64 MIDI Soundfont, used for the title splash fanfare
## Sound
- [dbry](https://github.com/dbry): Xtreme Quality IMA-ADPCM decoder code from [adpcm-xq](https://github.com/dbry/adpcm-xq).
- [Firexploit](https://github.com/Firexploit): 3DS UI sounds for the 3DS theme, which some are also used in the DSi theme.
## Others
- [ahezard](https://github.com/ahezard): NDMA code from nds-bootstrap
- Another World & Yellow Wood Goblin: The original akMenu/Wood UI
- [Arisotura](https://github.com/Arisotura): ROM list from melonDS, and BIOS dumper code from [dsibiosdumper](https://github.com/Arisotura/dsibiosdumper)
- [devkitPro](https://github.com/devkitPro): Code used in nds-hb-menu, and the use of the bootloader, devkitARM, libnds, and libfat
- [Gericom](https://github.com/Gericom), TrolleyDave, and FAST6191: GBA SRAM-patching code, used in gbapatcher ([SRAM patching thread at GBAtemp](https://gbatemp.net/threads/reverse-engineering-gba-patching.60168/))
- Nikokaro: Found no-tilt patches for *WarioWare: Twisted!*, and *Yoshi Topsy-Turvy*. ([GBAtemp thread](https://gbatemp.net/threads/gba-no-tilt-patches-for-ds-users.584128/))
- [profi200](https://github.com/profi200): Improved SD code from fastboot3DS
- RadDude McCoolguy, fintogive, KazoWAR, Vague Rant, [gamemasterplc](https://github.com/gamemasterplc), [Rocket Robz](https://github.com/RocketRobz), [ChampionLeake](https://github.com/ChampionLeake), [DeadSkullzJr](https://github.com/DeadSkullzJr), [spellboundtriangle](https://github.com/spellboundtriangle), [LedyBacer](https://github.com/LedyBacer): Widescreen cheats
- retrogamefan & Rudolph: Included AP-patches for nds-bootstrap
   - [enler](https://github.com/enler): Fixing AP-patch for Pokemon Black 2 (Japan) for DS⁽ⁱ⁾ mode compatibility
   - [Rocket Robz](https://github.com/RocketRobz): Fixing some DS⁽ⁱ⁾-Enhanced game AP-patches for DS⁽ⁱ⁾ mode compatibility
- [SNBeast](https://github.com/SNBeast): Unlaunch patches
- Taiju Yamada: Code used to bypass R4i-SDHC boot file protection
## Translators
- Arabic: [Ken Brown](https://crowdin.com/profile/kenkenkenneth), [SLG3](https://crowdin.com/profile/slg3)
- Bulgarian: [Peter0x44](https://github.com/Peter0x44), [Tescu](https://crowdin.com/profile/tescu48)
- Catalan:
- Chinese Simplified: [cai_miao](https://crowdin.com/profile/cai_miao), [Chris](https://crowdin.com/profile/z0287yyy), [Forbidden](https://crowdin.com/profile/Origami), [James-Makoto](https://crowdin.com/profile/VCMOD55), [R-YaTian](https://github.com/R-YaTian), [Yukino Song](https://crowdin.com/profile/ClassicOldSong), [曾国立](https://crowdin.com/profile/notthingtosay), [天天地地人人](https://crowdin.com/profile/realworld), [百地 希留耶](https://crowdin.com/profile/FIve201)
- Chinese Traditional: [cai_miao](https://crowdin.com/profile/cai_miao), [ccccchoho](https://crowdin.com/profile/ccccchoho), [James-Makoto](https://crowdin.com/profile/VCMOD55), [Rintim](https://crowdin.com/profile/Rintim), [奇诺比奥](https://crowdin.com/profile/Counta6_233)
- Danish: [jonata](https://github.com/Jonatan6), [Michael Millet](https://crowdin.com/profile/duroluro), [Nadia Pedersen](https://crowdin.com/profile/nadiaholmquist)
- Dutch: [Arthur](https://crowdin.com/profile/arthurr2014.tl), [guusbuk](https://crowdin.com/profile/guusbuk), [Mikosu](https://crowdin.com/profile/miko303), [Minionguyjpro](https://crowdin.com/profile/minionguyjpro), [Xtremegamer007](https://crowdin.com/profile/xtremegamer007)
- French: [Arcky](https://github.com/ArckyTV), [Benjamin](https://crowdin.com/profile/sombrabsol), [cooolgamer](https://crowdin.com/profile/cooolgamer), [Dhalian](https://crowdin.com/profile/DHALiaN3630), [maximesharp](https://crowdin.com/profile/maximesharp), [Ghost0159](https://crowdin.com/profile/Ghost0159), [Léo](https://crowdin.com/profile/leeo97one), [LinuxCat](https://github.com/LinUwUxCat), [Martinez](https://github.com/flutterbrony), [NightScript](https://github.com/NightScript370), [SLG3](https://crowdin.com/profile/slg3), [TM-47](https://crowdin.com/profile/-tm-), [Yolopix](https://crowdin.com/profile/yolopix)
- German: [ariebe9115](https://crowdin.com/profile/ariebe9115), [Blurry Knight](https://crowdin.com/profile/blurryknight), [Christian Schuhmann](https://github.com/c-schuhmann), [Dubsenbert Reaches](https://crowdin.com/profile/Bierjunge), [Fırat Tay](https://crowdin.com/profile/paradox-), [hehe](https://crowdin.com/profile/znime), [İlke Hür Eyiol](https://crowdin.com/profile/ilkecan51), [Julian](https://crowdin.com/profile/nailujx86), [Kazuto](https://crowdin.com/profile/Marcmario), [malekairmaroc7](https://github.com/malekairmaroc7), [Michael Brey](https://crowdin.com/profile/xxmichibxx), [Oleh Hatsenko](https://github.com/IRONKAGE), [SkilLP](https://github.com/SkilLP), [SuperSaiyajinStackZ](https://github.com/SuperSaiyajinStackZ), [Tcm0](https://github.com/Tcm0), [TheDude](https://crowdin.com/profile/the6771), [TM-47](https://crowdin.com/profile/-tm-), [Uriki](https://github.com/Uriki)
- Greek: [Anestis1403](https://crowdin.com/profile/anestis1403)
- Hebrew: [Barawer](https://crowdin.com/profile/barawer), [Yaniv Levin](https://crowdin.com/profile/y4niv)
- Hungarian: [Viktor Varga](http://github.com/vargaviktor), [ハトヴィング -- ハット](https://crowdin.com/profile/hatoving)
- Indonesian: [Cyruz Wings](https://crowdin.com/profile/cyruzwings), [Farid Irwan](https://crowdin.com/profile/farid1991), [heydootdoot](https://crowdin.com/profile/heydootdoot), [Shiori](https://crowdin.com/profile/egoistamamono)
- Italian: [Alessandro Tavolieri](https://crowdin.com/profile/ale2197), [Leonardo Ledda](https://github.com/LeddaZ), [Mattia](https://crowdin.com/profile/mattiau59), [TM-47](https://crowdin.com/profile/-tm-), [Vendicatorealato](https://crowdin.com/profile/vendicatorealato), [xavimel](https://github.com/xavimel)
- Japanese: [Chromaryu](https://crowdin.com/profile/knight-ryu12), [inucat](https://crowdin.com/profile/inucat), [Pk11](https://github.com/Epicpkmn11), [kuragehime](https://crowdin.com/profile/kuragehimekurara1), [rinrinrin2002](https://crowdin.com/profile/rinrinrin2002), [Rintim](https://crowdin.com/profile/Rintim), [Ronny Chan](https://github.com/chyyran), [Uriki](https://github.com/Uriki)
- Korean: [lifehackerhansol](https://github.com/lifehackerhansol), [I'm Not Cry](https://crowdin.com/profile/cryental), [Myebyeol_NOTE](https://crowdin.com/profile/groovy-mint), [Oleh Hatsenko](https://github.com/IRONKAGE), [그그기그](https://crowdin.com/profile/gigueguegue0803)
- Norwegian: [Nullified Block](https://crowdin.com/profile/elasderas123)
- Polish: [Avginike](https://crowdin.com/profile/avginike), [gierkowiec tv](https://crowdin.com/profile/krystianbederz), [Kipi000](https://crowdin.com/profile/kipi000), [Konrad Borowski](https://crowdin.com/profile/xfix), [MaksCROWDIN0](https://crowdin.com/profile/makscrowdin0), [Mateusz Tobiasz](https://crowdin.com/profile/tobiaszmateusz), [Michał Słonina](https://crowdin.com/profile/badis_), [RedstonekPL](https://crowdin.com/profile/redstonekpl), [TheCasachii](https://crowdin.com/profile/thecasachii)
- Portuguese (Brazil): [César Memère](https://crowdin.com/profile/blueo110), [Jeff Sousa](https://crowdin.com/profile/lordeilluminati), [themasterf](https://crowdin.com/profile/themasterf), [Victor Coronado](https://crowdin.com/profile/raulcoronado)
- Portuguese (Portugal): [bruwyvn](https://crowdin.com/profile/bruwyvn), [Christopher Rodrigues](https://crowdin.com/profile/chrismr197), [Gabz Almeida](https://crowdin.com/profile/connwcted), [jim](https://crowdin.com/profile/hnrwx), [joyrv](https://crowdin.com/profile/joyrv), [leteka 1234](https://crowdin.com/profile/Leaqua21), [Rodrigo Tavares](https://crowdin.com/profile/rodrigodst), [Tiago Silva](https://crowdin.com/profile/TheGameratorT), [Wodson de Andrade](https://crowdin.com/profile/CaptainCheep), [Wodson de Andrade](https://crowdin.com/profile/WodsonKun), [Zak](https://github.com/zekroman)
- Romanian: [Tescu](https://crowdin.com/profile/tescu48)
- Russian: [Alexey Barsukov](https://crowdin.com/profile/lps), [Ckau](https://crowdin.com/profile/Ckau), [manwithnoface](https://github.com/1upus), [mbhz](https://github.com/mbhz), [MMR Marler](https://crowdin.com/profile/bessmertnyi_mikhail), [Nikita](https://crowdin.com/profile/bacer), [Молодая Кукуруза](https://crowdin.com/profile/bessmertnyi_mikhail)
- Ryukyuan: [kuragehime](https://crowdin.com/profile/kuragehimekurara1)
- Spanish: [Adrin Ramen](https://crowdin.com/profile/adiiramen), [Adrian Rodriguez](https://crowdin.com/profile/ar9555997), [Allinxter](https://crowdin.com/profile/allinxter), [beta215](https://crowdin.com/profile/beta215), [ccccmark](https://github.com/ccccmark), [dimateos](https://crowdin.com/profile/dimateos), [KplyAsteroid](https://crowdin.com/profile/KplyAsteroid), [mschifino](https://crowdin.com/profile/mschifino), [Nicolás Herrera Concha](https://crowdin.com/profile/noname141203), [Nintendo R](https://crowdin.com/profile/nintendor), [nuxa17](https://twitter.com/TimeLordJean), [Radriant](https://crowdin.com/profile/radriant), [SofyUchiha](https://crowdin.com/profile/sofyuchiha), [TM-47](https://crowdin.com/profile/-tm-), [Uriki](https://github.com/Uriki), [XxPhoenix1996xX](https://github.com/XxPhoenix1996xX)
- Swedish: [Max Hambraeus](https://github.com/maxhambraeus), [Nullified Block](https://crowdin.com/profile/elasderas123), [TM-47](https://crowdin.com/profile/-tm-), [Victor Ahlin](https://crowdin.com/profile/VSwede), [Walter Lindell](https://crowdin.com/profile/walter.lindell)
- Turkish: [Alp](https://crowdin.com/profile/alpcinar), [Egehan.TWL](https://crowdin.com/profile/egehan.twl), [Emir](https://crowdin.com/profile/dirt3009), [GlideGuy06](https://crowdin.com/profile/glideguy06), [Grandmaquil](https://crowdin.com/profile/grandmaquil), [imbeegboi22](https://crowdin.com/profile/imbeegboi22), [İlke Hür Eyiol](https://crowdin.com/profile/ilkecan51), [rewix32](https://crowdin.com/profile/rewix32), [rewold20](https://crowdin.com/profile/rewold20), [Yağmur Celep](https://crowdin.com/profile/FixingCarp)
- Ukrainian: [Oleh Hatsenko](https://github.com/IRONKAGE), [Mykola Pukhalskyi](https://crowdin.com/profile/sensetivity), [TM-47](https://crowdin.com/profile/-tm-), [вухаста гітара](https://crowdin.com/profile/earedguitr)
- Valencian: [tsolo](https://crowdin.com/profile/tsolo)
- Vietnamese: [Chử Tiến Bình](https://crowdin.com/profile/okabe_zero-link), [daicahuyoi](https://crowdin.com/profile/daicahuyoi) [Đỗ Minh Hiếu](https://crowdin.com/profile/hieu2097), [hotungkhanh](https://crowdin.com/profile/hotungkhanh), [Trương Hồng Sơn](https://crowdin.com/profile/truonghongson2005)
