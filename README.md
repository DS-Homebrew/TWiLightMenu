# ![TWiLightMenu++](https://github.com/Iam2smart/TWiLightMenu/blob/master/Untitled.png)
TWiLight Menu++ is an open-source DSi Menu upgrade/replacement, and frontend for nds-bootstrap for DSi, and flashcards.

# Building

Building this app by yourself requires devkitPRO with devkitARM. Make sure that grit and mmutil are installed.

# Building with Docker

TWiLight Menu++ comes included with a Docker image for easy building without having to manually set up the required version of devkitARM.

## Building with Docker for Windows users.

You can compile TWiLight Menu++ with Docker using the provided PowerShell (`.ps1`) scripts. First, install docker at http://docker.com, and [configured Shared Drives](https://blogs.msdn.microsoft.com/stevelasker/2016/06/14/configuring-docker-for-windows-volumes/) for the drive where DSiMenu++ is cloned. 

Then run `compile_docker.ps1` in a PowerShell window. The script accepts `make` arguments as well, for example `.\compile_docker.ps1 clean`. Note that Docker compilation is not compatible with native Windows compilation. You should run `.\compile_docker.ps1 clean` to clean the artifacts before attempting to build with Docker.

To build all artifacts, run `.\compile_docker.ps1 package`.

# Credits

- ahezard: [nds-bootstrap](https://github.com/ahezard/nds-bootstrap)
- Apache Thunder: Providing the Miku theme (not made by him) for Acekard theme.
- Vulpes-Vulpeos: DSiMenu++ logo (v4.3.0-v6.2.1, and for the launcher, after v1.0.0), and MHGen theme for Acekard theme.
- Joom: Original TWLoader logo.
- Another World and Yellow Wood Goblin: The original akMenu/Wood UI.
- chyyran: Port of akMenu/Wood UI to TWLMenu++ as a theme.
- RocketRobz: Lead Developer, implementing the auto-reset power button function used in NTR-mode, and LED functions, to nds-bootstrap.
- shutterbug2000: For the muted sound/touchscreen fix for nds-bootstrap.
- spinal_cord: DSi4DS and DSision2 graphics.
- devkitPro: Some code used in nds-hb-menu.
