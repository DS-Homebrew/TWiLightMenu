This subfolder builds bootstrap code for some popular DS flash cards and should be reasonably easy to modify for others. Essentially it's just a matter of patching the bootstrap code with the appropriate dldi.

The main issues users will have here is where the card menu is encrypted in some way or if the supplied dldi has been built with arm9 code. Hbmenu uses the arm7 to load code in order to allow the maximum use of memory.

The encoder/decoder for the R4 menu is available in devkitPro svn.

svn co https://devkitpro.svn.sourceforge.net/svnroot/devkitpro/trunk/projects/tools/r4denc r4denc

compile this and place it somewhere in your path.