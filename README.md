<p align="center">
 <img src="https://github.com/DS-Homebrew/TWiLightMenu/blob/master/logo.png"><br>
  <a href="https://gbatemp.net/threads/ds-i-3ds-twilight-menu-gui-for-ds-i-games-and-ds-i-menu-replacement.472200/">
   <img src="https://img.shields.io/badge/GBAtemp-Thread-blue.svg" alt="GBAtemp Thread">
  </a>
  <a href="https://discord.gg/fCzqcWteC4">
   <img src="https://img.shields.io/badge/Discord%20Server-%23twilight--menu--dev-green.svg" alt="Discord Server">
  </a>
  <a href="https://github.com/DS-Homebrew/TWiLightMenu/actions?query=workflow%3A%22Build+TWiLight+Menu%2B%2B%22">
   <img src="https://github.com/DS-Homebrew/TWiLightMenu/actions/workflows/nightly.yml/badge.svg" height="20" alt="Build status on GitHub Actions">
  </a>
  <a href="https://crowdin.com/project/TwilightMenu">
    <img src="https://badges.crowdin.net/TwilightMenu/localized.svg" alt="Localized on Crowdin">
  </a>
</p>

**TW**i**L**ight Menu++ is an open-source DSi Menu upgrade/replacement for the Nintendo DSi, the Nintendo 3DS, and Nintendo DS flashcards.
It can launch games for the Nintendo DS, Nintendo DSi, and GameBoy Advance, as well as DSTWO plugins (if you use a DSTWO).

# Add-ons

Additional features can be added on to **TW**i**L**ight Menu++. See [this page](https://wiki.ds-homebrew.com/twilightmenu/installing-addons) for more information.

# Compiling

## Setting up

Compiling this app requires devkitPro's devkitARM, libnds, grit, and mmutil. These can be installed using [devkitPro pacman](https://devkitpro.org/wiki/devkitPro_pacman) with the following command:
```
sudo dkp-pacman -S nds-dev
```
(Note: Command will vary by OS, sudo may not be needed and it may be just `pacman` instead.)

## Cloning
The repository contains submodules, so you need to clone recursively:
```
git clone --recursive https://github.com/DS-Homebrew/TWiLightMenu.git
```

If you cloned without pulling the submodules, you may update them afterwards:
```
git submodule update --init --recursive
```

## Building

Once you have devkitPro's toolchains installed you can build the entirety of TWiLight Menu++ by simply running `make package` in the root of the repository. If you only want to build a specific part of TWiLight Menu++ you can `cd` to that folder and run `make dist`.

Once it finishes building, the output files will be in the `7zfile` folder following the same directory structure as the official `TWiLightMenu.7z` builds.

## Using Docker

Using the included [Docker](https://docker.com) image, you can easily compile TWiLight Menu++ without having to manually set up the required version of devkitARM using the provided PowerShell (`.ps1`) scripts.

The script accepts `make` arguments as well. For example, `.\compile_docker.ps1 clean` will clean the directories of all the compiled code. If you would like to build all artifacts, run `.\compile_docker.ps1 package`.

Please note that Docker compilation is not compatible with native compilation on Windows. You should run `.\compile_docker.ps1 clean` to clean the artifacts before attempting to build with Docker. If a notification appears asking you to share your drive, you must choose to enable drive sharing for Docker to work on Windows.

## Manual Pages

The manual pages are stored in a separate repository and downloaded from a release when building TWiLight Menu++. For more information, see the [twilight-manual](https://github.com/DS-Homebrew/twilight-manual) repository.

## Subfolders

TWiLight Menu++ is composed of multiple "sub-projects" which all work together to create the DSi Menu replacement. Most subfolders in the repository contain their own code which will compile a `xxx.nds` file, which is generally copied to `/_nds/TWiLightMenu/xxx.srldr` inside the `7z` file (or `7zfile` folder).

- **3dssplash**: opens 3ds and cia files (?)
  - Compiled to `/_nds/TWiLightMenu/3dssplash.srldr`
- **booter**: main entrypoint, the first file loaded by the console if using SD-card or CFW
  - Creates `/BOOT.nds` and `/title/00030004/53524c41/content/00000000.app` (same file).
- **booter_fc**: main entrypoint for flashcards
  - Creates `_DS_MENU.dat`, `dsedgei.dat`, `akmenu4.nds`, `_DSMENU.nds`, `SCFW.SC` and others.
- gbapatcher
- **imageview**: image viewer similar to DSi camera (gif, png, bmp) (Part of the Multimedia add-on)
  - `/_nds/TWiLightMenu/imageview.srldr`
- **manual**: instruction manual for TWiLight Menu++
  - Creates `/_nds/TWiLightMenu/manual.srldr`
- **quickmenu**: DS Lite menu, the old classic UI
  - Creates `/_nds/TWiLightMenu/mainmenu.srldr`
- **romsel_aktheme**: menu for the Wood UI
  - Creates `/_nds/TWiLightMenu/akmenu.srldr`
- **romsel_dsimenutheme**: menu for the "Nintendo DSi", "Nintendo 3DS", "SEGA Saturn", and "Homebrew Launcher" UIs
  - Creates `/_nds/TWiLightMenu/dsimenu.srldr`
- **romsel_r4theme**: menu for the "R4 Original" and "Gameboy Color" UIs
  - Creates `/_nds/TWiLightMenu/r4menu.srldr`
- **settings**: settings menu
  - Creates `/_nds/TWiLightMenu/settings.srldr`
- **slot1launch**:
  - Creates `/_nds/TWiLightMenu/slot1launch.srldr`
- **title**: boot splash screen (Nintendo logo by default)
  - Creates `/_nds/TWiLightMenu/main.srldr`

# Translating

You can help translate TWiLight Menu++ on the [Crowdin project](https://crowdin.com/project/TwilightMenu). If you'd like to request a new language be added then please ask on the [Discord server](https://ds-homebrew.com/discord).

# Credits
## Main Developers
- [Rocket Robz](https://github.com/RocketRobz): Lead Developer
- [chyyran](https://github.com/chyyran): Porting the akMenu/Wood UI to TWiLight Menu++ (before re-development) & adding the ability to load themes off the SD card for DSi/3DS UIs
- [Pk11](https://github.com/Epicpkmn11): Adding the ability to load custom themes off the SD card for the original R4 UI, implemented sorting & made manuals use PNG, improved font rendering, providing a custom background for Unlaunch, managing translations, and various bug fixes
## Secondary Developers
- [DieGo](https://github.com/DieGo367): Adding support for custom ROM/folder icons and improvements for custom themes
- [lifehackerhansol](https://github.com/lifehackerhansol): Improving support for flashcard autobooting and kernel loading
    - [Deletecat](https://github.com/Deletecat): Original R4SDHC autoboot file containing a [flashcard-bootstrap](https://github.com/lifehackerhansol/flashcard-bootstrap) binary
- [NightScript](https://github.com/NightScript370): Code cleanup, defining code standards, added flashcard functionality for Wood UI (before re-development), manual pages
- [asiekierka](https://github.com/asiekierka): Stargate 3DS autoboot file from [nds-miniboot](https://github.com/asiekierka/nds-miniboot)
## App Launchers
- [ahezard](https://github.com/ahezard), [shutterbug2000](https://github.com/shutterbug2000) and [Rocket Robz](https://github.com/RocketRobz): [nds-bootstrap](https://github.com/ahezard/nds-bootstrap) (used for launching Nintendo DS(i) ROMs off the SD card)
- [Alekmaul](https://github.com/alekmaul) & [wavemotion](https://github.com/wavemotion-dave): [StellaDS](https://github.com/wavemotion-dave/StellaDS), [A5200DS](https://github.com/wavemotion-dave/A5200DS), [A7800DS](https://github.com/wavemotion-dave/A7800DS), [A8DS](https://github.com/wavemotion-dave/A8DS) (used for launching Atari 2600/5200/7800/XEGS ROMs), and [ColecoDS](https://github.com/wavemotion-dave/ColecoDS) (used for launching ColecoVision, MSX, Sord M5, and Sega SG-1000/SC-3000 ROMs)
- archiede: [SNEmulDS](https://www.gamebrew.org/wiki/SNEmulDS) (used for launching SNES ROMs)
     - [Coto](https://github.com/cotodevel/): [SNEmulDS Revival](https://github.com/cotodevel/snemulds)
- [chishm](https://github.com/chishm): [tuna-viDS](https://github.com/chishm/tuna-vids) (used for playing Xvid videos)
- [Stewmath](https://github.com/Stewmath): [GameYob](https://github.com/Stewmath/GameYob) (used for launching Gameboy ROMs)
- [FluBBaOfWard](https://github.com/FluBBaOfWard): [S8DS](https://github.com/FluBBaOfWard/S8DS) (used for launching Sega Master System, Game Gear, SG-1000/SG-3000, and ColecoVision ROMs), [NitroGrafx](https://github.com/FluBBaOfWard/NitroGrafx) (used for launching PC Engine/TurboGrafx-16 ROMs), [NitroSwan](https://github.com/FluBBaOfWard/NitroSwan) (used for launching WonderSwan ROMs), and [NGPDS](https://github.com/FluBBaOfWard/NGPDS) (used for launching Neo Geo Pocket ROMs)
- [FluBBaOfWard](https://github.com/FluBBaOfWard), [Coto](https://coto88.bitbucket.io/), and Loopy: [nesDS](https://github.com/DS-Homebrew/NesDS) (used for launching NES ROMs)
     - [Apache Thunder](https://github.com/ApacheThunder): [TWL Edition](https://github.com/ApacheThunder/NesDS) of nesDS
- [Gericom](https://github.com/Gericom): [FastVideoDSPlayer](https://github.com/Gericom/FastVideoDSPlayer) (used for launching FastVideoDS videos) & [GBARunner2](https://github.com/Gericom/GBARunner2) (used for launching GameBoy Advance ROMs outside of the DS Phat/Lite's GBA mode)
     - [therealteamplayer](https://github.com/therealteamplayer): [Hicode+DSP](https://github.com/therealteamplayer/GBARunner2) merged builds of GBARunner2 included for DSi/3DS
     - [unresolvedsymbol](https://github.com/unresolvedsymbol): [rom3M+master "DSL-Enhanced"](https://github.com/unresolvedsymbol/GBARunner2-DSL-Enhanced) builds of GBARunner2 included for playing *Pokémon: Emerald Version* on flashcards
- Lordus: [jEnesisDS](https://gamebrew.org/wiki/JEnesisDS) (used for launching Sega Mega Drive/Genesis ROMs)
     - [xonn83](https://github.com/xonn83): GBMacro version of [jEnesisDS](https://github.com/xonn83/jEnesisDS_macro)
- Ryan FB, [Rocket Robz](https://github.com/RocketRobz), and [xonn83](https://github.com/xonn83): [PicoDriveTWL](https://github.com/DS-Homebrew/PicoDriveTWL) (used for launching large Sega Mega Drive/Genesis ROMs)
- [wavemotion](https://github.com/wavemotion-dave): [Nintellivision/NINTV-DS](https://github.com/wavemotion-dave/NINTV-DS) (used for launching Intellivision ROMs) and [SugarDS](https://github.com/wavemotion-dave/SugarDS) (used for launching Amstrad CPC ROMs)
## Graphics & Themes
- [Absent-Reality](http://pixeljoint.com/p/19283.htm): Intellivision icon
- davi: Border for GBC UI (originally for GameYob)
- [fail0verflow](https://github.com/fail0verflow/), Fluto, and Arkhandar: Homebrew Channel/Launcher graphics
- [FlameKat53](https://github.com/FlameKat53): Manual icon for DSi UI's `SELECT` menu
- [Mr. Start](https://github.com/Arthur-Start): Super Nintendo DS splash screen
- [PW5190](https://github.com/PW5190): Kirby-themed and Pokémon Day TWLMenu++ splash screens
- [spinal_cord](https://gbatemp.net/members/spinal_cord.90607/): [DSi4DS](https://gbatemp.net/threads/dsi4ds.173617/) and [DSision2](https://gbatemp.net/threads/dsision2.92740/) graphics
- [StarvingArtist](https://www.deviantart.com/starvingartist/): Game Console icons
## Music
- [BlastoiseVeteran](https://soundcloud.com/blastyveteran): Remastered version of Nintendo DSi Shop music
- IkaMusumeYiyaRoxie: General N64 MIDI Soundfont, used for the title splash fanfare on old versions
- [TeciorFILM](https://www.youtube.com/channel/UCEyPYQavt2g_tdmkd-pQgYw): The video ([10 Minutes of Nintendo DSi Menu Music (Authentic)](https://www.youtube.com/watch?v=LLc3-z8VZwc)) used for the better DSi Menu music add-on
## Sound
- [dbry](https://github.com/dbry): Xtreme Quality IMA-ADPCM decoder code from [adpcm-xq](https://github.com/dbry/adpcm-xq).
- [Firexploit](https://github.com/Firexploit): 3DS UI sounds, which some are also used in the DSi UI.
## Others
- [ahezard](https://github.com/ahezard): NDMA code from nds-bootstrap
- Another World & Yellow Wood Goblin: The original akMenu/Wood UI
    - [Rocket Robz](https://github.com/RocketRobz): Re-developed the Wood UI with pieces of the original code ported over
- [Arisotura](https://github.com/Arisotura): ROM list from melonDS, and BIOS dumper code from [dsibiosdumper](https://github.com/Arisotura/dsibiosdumper)
- [Dartz150](https://github.com/Dartz150): Provided fix for *Iridion II* & *Top Gun: Combat Zones* GBA games
- [devkitPro](https://github.com/devkitPro): Code used in nds-hb-menu, and the use of the bootloader, devkitARM, libnds, and libfat
- [edo9300](https://github.com/edo9300): DS Phat check code
- [Gericom](https://github.com/Gericom), TrolleyDave, and FAST6191: GBA SRAM-patching code, used in gbapatcher ([SRAM patching thread at GBAtemp](https://gbatemp.net/threads/reverse-engineering-gba-patching.60168/))
- [Gericom](https://github.com/Gericom) & [profi200](https://github.com/profi200): GBA color correction code, used in [GBARunner3](https://github.com/Gericom/GBARunner3)
- Nikokaro: Found no-tilt patches for *WarioWare: Twisted!*, and *Yoshi Topsy-Turvy*. ([GBAtemp thread](https://gbatemp.net/threads/gba-no-tilt-patches-for-ds-users.584128/))
- [profi200](https://github.com/profi200): Improved SD code from fastboot3DS
- RadDude McCoolguy, fintogive, KazoWAR, Vague Rant, [gamemasterplc](https://github.com/gamemasterplc), [Rocket Robz](https://github.com/RocketRobz), [ChampionLeake](https://github.com/ChampionLeake), [DeadSkullzJr](https://github.com/DeadSkullzJr), [spellboundtriangle](https://github.com/spellboundtriangle), [LedyBacer](https://github.com/LedyBacer): Widescreen cheats
- [SNBeast](https://github.com/SNBeast): Unlaunch patches
- Taiju Yamada: Code used to bypass R4i-SDHC boot file protection
## Translators
- Arabic: [Ken Brown](https://crowdin.com/profile/kenkenkenneth), [SLG3](https://crowdin.com/profile/slg3)
- Bulgarian: [Peter0x44](https://github.com/Peter0x44), [Tescu](https://crowdin.com/profile/tescu48)
- Catalan:
- Chinese Simplified: [cai_miao](https://crowdin.com/profile/cai_miao), [Chris](https://crowdin.com/profile/z0287yyy), [Forbidden](https://crowdin.com/profile/Origami), [James-Makoto](https://crowdin.com/profile/VCMOD55), [R-YaTian](https://github.com/R-YaTian), [Yukino Song](https://crowdin.com/profile/ClassicOldSong), [曾国立](https://crowdin.com/profile/notthingtosay), [天天地地人人](https://crowdin.com/profile/realworld), [百地 希留耶](https://crowdin.com/profile/FIve201)
- Chinese Traditional: [cai_miao](https://crowdin.com/profile/cai_miao), [ccccchoho](https://crowdin.com/profile/ccccchoho), [James-Makoto](https://crowdin.com/profile/VCMOD55), [Rintim](https://crowdin.com/profile/Rintim), [奇诺比奥](https://crowdin.com/profile/Counta6_233)
- Czech: [Štěpán Dolský](https://crowdin.com/profile/dolskystepa)
- Danish: [jonata](https://github.com/Jonatan6), [Michael Millet](https://crowdin.com/profile/duroluro), [Nadia Pedersen](https://crowdin.com/profile/nadiaholmquist)
- Dutch: [Arthur](https://crowdin.com/profile/arthurr2014.tl), [guusbuk](https://crowdin.com/profile/guusbuk), [Mikosu](https://crowdin.com/profile/miko303), [Minionguyjpro](https://crowdin.com/profile/minionguyjpro), [Xtremegamer007](https://crowdin.com/profile/xtremegamer007)
- French: [Arcky](https://github.com/ArckyTV), [cooolgamer](https://crowdin.com/profile/cooolgamer), [Dhalian](https://crowdin.com/profile/DHALiaN3630), [maximesharp](https://crowdin.com/profile/maximesharp), [Ghost0159](https://crowdin.com/profile/Ghost0159), [Léo](https://crowdin.com/profile/leeo97one), [LinuxCat](https://github.com/LinUwUxCat), [Martinez](https://github.com/flutterbrony), [NightScript](https://github.com/NightScript370), [SLG3](https://crowdin.com/profile/slg3), [SombrAbsol](https://crowdin.com/profile/sombrabsol), [TM-47](https://crowdin.com/profile/-tm-), [Yolopix](https://crowdin.com/profile/yolopix)
- German: [ariebe9115](https://crowdin.com/profile/ariebe9115), [Blurry Knight](https://crowdin.com/profile/blurryknight), [Christian Schuhmann](https://github.com/c-schuhmann), [Dubsenbert Reaches](https://crowdin.com/profile/Bierjunge), [Fırat Tay](https://crowdin.com/profile/paradox-), [hehe](https://crowdin.com/profile/znime), [İlke Hür Eyiol](https://crowdin.com/profile/ilkecan51), [Julian](https://crowdin.com/profile/nailujx86), [Kazuto](https://crowdin.com/profile/Marcmario), [malekairmaroc7](https://github.com/malekairmaroc7), [Michael Brey](https://crowdin.com/profile/xxmichibxx), [Oleh Hatsenko](https://github.com/IRONKAGE), [SkilLP](https://github.com/SkilLP), [SuperSaiyajinStackZ](https://github.com/SuperSaiyajinStackZ), [Tcm0](https://github.com/Tcm0), [TheDude](https://crowdin.com/profile/the6771), [TM-47](https://crowdin.com/profile/-tm-), [Uriki](https://github.com/Uriki)
- Greek: [Anestis1403](https://crowdin.com/profile/anestis1403)
- Hebrew: [Barawer](https://crowdin.com/profile/barawer), [Yaniv Levin](https://crowdin.com/profile/y4niv)
- Hungarian: [Viktor Varga](http://github.com/vargaviktor), [ハトヴィング -- ハット](https://crowdin.com/profile/hatoving)
- Indonesian: [Cyruz Wings](https://crowdin.com/profile/cyruzwings), [Farid Irwan](https://crowdin.com/profile/farid1991), [heydootdoot](https://crowdin.com/profile/heydootdoot), [Shiori](https://crowdin.com/profile/egoistamamono)
- Italian: [Alessandro Tavolieri](https://crowdin.com/profile/ale2197), [Leonardo Ledda](https://github.com/LeddaZ), [Mattia](https://crowdin.com/profile/mattiau59), [TM-47](https://crowdin.com/profile/-tm-), [Vendicatorealato](https://crowdin.com/profile/vendicatorealato), [xavimel](https://github.com/xavimel)
- Japanese: [Chromaryu](https://crowdin.com/profile/knight-ryu12), [inucat](https://crowdin.com/profile/inucat), [Pk11](https://github.com/Epicpkmn11), [kuragehime](https://crowdin.com/profile/kuragehimekurara1), [rinrinrin2002](https://crowdin.com/profile/rinrinrin2002), [Rintim](https://crowdin.com/profile/Rintim), [Ronny Chan](https://github.com/chyyran), [Uriki](https://github.com/Uriki)
- Korean: [DDinghoya](https://crowdin.com/profile/ddinghoya), [lifehackerhansol](https://github.com/lifehackerhansol), [I'm Not Cry](https://crowdin.com/profile/cryental), [Myebyeol_NOTE](https://crowdin.com/profile/groovy-mint), [Oleh Hatsenko](https://github.com/IRONKAGE), [그그기그](https://crowdin.com/profile/gigueguegue0803)
- Norwegian: [Nullified Block](https://crowdin.com/profile/elasderas123)
- Polish: [Avginike](https://crowdin.com/profile/avginike), [gierkowiec tv](https://crowdin.com/profile/krystianbederz), [Kipi000](https://crowdin.com/profile/kipi000), [Konrad Borowski](https://crowdin.com/profile/xfix), [MaksCROWDIN0](https://crowdin.com/profile/makscrowdin0), [Mateusz Tobiasz](https://crowdin.com/profile/tobiaszmateusz), [Michał Słonina](https://crowdin.com/profile/badis_), [RedstonekPL](https://crowdin.com/profile/redstonekpl), [TheCasachii](https://crowdin.com/profile/thecasachii)
- Portuguese (Brazil): [César Memère](https://crowdin.com/profile/blueo110), [Jeff Sousa](https://crowdin.com/profile/lordeilluminati), [themasterf](https://crowdin.com/profile/themasterf), [Victor Coronado](https://crowdin.com/profile/raulcoronado)
- Portuguese (Portugal): [bruwyvn](https://crowdin.com/profile/bruwyvn), [Christopher Rodrigues](https://crowdin.com/profile/chrismr197), [Gabz Almeida](https://crowdin.com/profile/connwcted), [jim](https://crowdin.com/profile/hnrwx), [joyrv](https://crowdin.com/profile/joyrv), [leteka 1234](https://crowdin.com/profile/Leaqua21), [Rodrigo Tavares](https://crowdin.com/profile/rodrigodst), [Tiago Silva](https://crowdin.com/profile/TheGameratorT), [Wodson de Andrade](https://crowdin.com/profile/CaptainCheep), [Wodson de Andrade](https://crowdin.com/profile/WodsonKun), [Zak](https://github.com/zekroman)
- Romanian: [Tescu](https://crowdin.com/profile/tescu48)
- Russian: [Alexey Barsukov](https://crowdin.com/profile/lps), [Ckau](https://crowdin.com/profile/Ckau), [manwithnoface](https://github.com/1upus), [mbhz](https://github.com/mbhz), [MMR Marler](https://crowdin.com/profile/bessmertnyi_mikhail), [Nikita](https://crowdin.com/profile/bacer), [Молодая Кукуруза](https://crowdin.com/profile/bessmertnyi_mikhail)
- Ryukyuan: [kuragehime](https://crowdin.com/profile/kuragehimekurara1)
- Spanish: [Adrin Ramen](https://crowdin.com/profile/adiiramen), [Adrian Rodriguez](https://crowdin.com/profile/ar9555997), [Allinxter](https://crowdin.com/profile/allinxter), [beta215](https://crowdin.com/profile/beta215), [ccccmark](https://github.com/ccccmark), [dimateos](https://crowdin.com/profile/dimateos), [Kaede159](https://crowdin.com/profile/daemo159), [KplyAsteroid](https://crowdin.com/profile/KplyAsteroid), [mschifino](https://crowdin.com/profile/mschifino), [Nicolás Herrera Concha](https://crowdin.com/profile/noname141203), [Nintendo R](https://crowdin.com/profile/nintendor), [nuxa17](https://twitter.com/TimeLordJean), [Radriant](https://crowdin.com/profile/radriant), [SofyUchiha](https://crowdin.com/profile/sofyuchiha), [TM-47](https://crowdin.com/profile/-tm-), [Uriki](https://github.com/Uriki), [XxPhoenix1996xX](https://github.com/XxPhoenix1996xX)
- Swedish: [Max Hambraeus](https://github.com/maxhambraeus), [Nullified Block](https://crowdin.com/profile/elasderas123), [TM-47](https://crowdin.com/profile/-tm-), [Victor Ahlin](https://crowdin.com/profile/VSwede), [Walter Lindell](https://crowdin.com/profile/walter.lindell)
- Turkish: [Alp](https://crowdin.com/profile/alpcinar), [Egehan.TWL](https://crowdin.com/profile/egehan.twl), [Emir](https://crowdin.com/profile/dirt3009), [GlideGuy06](https://crowdin.com/profile/glideguy06), [Grandmaquil](https://crowdin.com/profile/grandmaquil), [imbeegboi22](https://crowdin.com/profile/imbeegboi22), [İlke Hür Eyiol](https://crowdin.com/profile/ilkecan51), [rewix32](https://crowdin.com/profile/rewix32), [rewold20](https://crowdin.com/profile/rewold20), [Yağmur Celep](https://crowdin.com/profile/FixingCarp)
- Ukrainian: [Oleh Hatsenko](https://github.com/IRONKAGE), [Mykola Pukhalskyi](https://crowdin.com/profile/sensetivity), [TM-47](https://crowdin.com/profile/-tm-), [вухаста гітара](https://crowdin.com/profile/earedguitr)
- Valencian: [tsolo](https://crowdin.com/profile/tsolo)
- Vietnamese: [Chử Tiến Bình](https://crowdin.com/profile/okabe_zero-link), [daicahuyoi](https://crowdin.com/profile/daicahuyoi) [Đỗ Minh Hiếu](https://crowdin.com/profile/hieu2097), [hotungkhanh](https://crowdin.com/profile/hotungkhanh), [Trương Hồng Sơn](https://crowdin.com/profile/truonghongson2005)
