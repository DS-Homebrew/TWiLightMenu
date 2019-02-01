# ![TWiLightMenu++](https://github.com/RocketRobz/TWiLightMenu/blob/master/logo.png)
TWiLight Menu++ is an open-source DSi Menu upgrade/replacement, and frontend for nds-bootstrap for DSi, 3DS, and flashcards.

# Building

Building this app by yourself requires devkitPRO with devkitARM and [RocketRobz libnds fork](https://github.com/RocketRobz/libnds). Make sure that grit and mmutil are installed.

# Building with Docker

TWiLight Menu++ comes included with a Docker image for easy building without having to manually set up the required version of devkitARM.

## Building with Docker for Windows users.

You can compile TWiLight Menu++ with Docker using the provided PowerShell (`.ps1`) scripts. First, install docker at http://docker.com, and [configured Shared Drives](https://blogs.msdn.microsoft.com/stevelasker/2016/06/14/configuring-docker-for-windows-volumes/) for the drive where DSiMenu++ is cloned. 

Then run `compile_docker.ps1` in a PowerShell window. The script accepts `make` arguments as well, for example `.\compile_docker.ps1 clean`. Note that Docker compilation is not compatible with native Windows compilation. You should run `.\compile_docker.ps1 clean` to clean the artifacts before attempting to build with Docker.

To build all artifacts, run `.\compile_docker.ps1 package`.

# Credits
## Main Developers
- RocketRobz: Lead Developer, implementing the auto-reset power button function used in NTR-mode, and LED functions, to nds-bootstrap.
- Another World and Yellow Wood Goblin: The original akMenu/Wood UI.
- chyyran: Port of akMenu/Wood UI to TWiLightMenu++ as a theme.
- Epicpkmn11: Adding new features & bug fixes to Acekard theme, and the Switch themes for the Acekard theme.
- shutterbug2000: NDMA SD read code, and the muted sound/touchscreen fix for nds-bootstrap.
## App Launchers
- [ahezard](https://github.com/ahezard): [nds-bootstrap](https://github.com/ahezard/nds-bootstrap), and improved NDMA SD read code.
- [Drenn](https://github.com/Drenn1): [GameYob](https://github.com/Drenn1/GameYob)
- Coto: [nesDS](https://sourceforge.net/projects/nesds/)
- Apache Thunder: nesDS TWL Edition, and the DSi splash from NTR Launcher.
- Lordus: [jEnesisDS](https://gamebrew.org/wiki/JEnesisDS)
- archeid (Loopy): [SNEmulDS](https://www.gamebrew.org/wiki/SNEmulDS)
## Graphics & Theme
- spinal_cord: DSi4DS and DSision2 graphics.
- [StarvingArtist](https://www.deviantart.com/starvingartist/): Some game console icons used.
- Vulpes-Vulpeos: MHGen and DS Menu themes for Acekard theme.
## Others
- profi200: Improved SD code from fastboot3DS.
- devkitPro, WinterMute: Code used in nds-hb-menu, and the use of the bootloader, devkitARM, libnds, and libfat.
- DeadSkullzjr: [Cheat Database](https://gbatemp.net/threads/deadskullzjrs-flashcart-cheat-databases.488711/)
