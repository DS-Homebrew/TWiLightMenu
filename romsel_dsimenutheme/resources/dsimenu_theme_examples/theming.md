# Creating Custom Themes (instructions for end users)

The easiest way of customizing a theme is by editing the BMP textures in a theme's *ui*, *battery*, or *volume* folder. These BMP textures must be in RGB565 format, and use `#FF00FF` as a transparent key color. BMP textures are allowed to vary in size, but may require tweaking of the theme configuration to render correctly (see below).

Changes to paletted textures are more involved. Within the *grit* folder of a theme, the various image files may be edited. You will also require [devkitPro](https://devkitpro.org) with GRIT installed. Once you have finished editing your files, you must run

```bash
$ make
```

In order to compile your themes into Grit RIFF Format. This will compile your paletted textures into **.grf** format in the *grf* folder.

Be aware the paletted textures come with more restrictions than BMP textures, the primary being an absolute maximum of 16 colors per texture. However, some textures may have even tighter default palette restrictions, which can be modified at the risk of running out of palette memory (see below).

## Theme File Descriptions

*volume* and *battery* textures are self explanatory.

### UI Textures
| Texture          | Description                                                                                       |
| ---------------- | ------------------------------------------------------------------------------------------------- |
| bottom           | The bottom background texture when not hovering over an icon                                      |
| bottom_bubble    | The bottom background texture when hovering over an icon                                          |
| bottom_bubble_ds | For the 3DS theme, the bottom background texture when hovering over an icon when on a DS lite     |
| bottom_ds        | For the 3DS theme, the bottom background texture when not hovering over an icon when on a DS lite |
| date_time_font   | The font to display the date and time                                                             |
| Lshoulder        | The left shoulder                                                                                 |
| Lshoulder_greyed | The left shoulder when there are no pages to the left                                             |
| Rshoulder        | The right shoulder                                                                                |
| Rshoulder_greyed | The right shoulder when there are no pages to the right                                           |
| top              | The top background                                                                                |

### Paletted Textures
| Texture       | Description                                                                                          | Palette Restrictions (if less than 16)                          |
| ------------- | ---------------------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| bips          | The bips displayed on the bottom of the scrollbar (DSi Theme)                                        |                                                                 |
| launch_dot    | The dots displayed when a game is launched (DSi Theme)                                               |                                                                 |
| moving_arrow  | The arrow displayed when a game is being moved (DSi Theme)                                           |                                                                 |
| bubble        | The bottom bit of the bubble that draws over the start border or icon box                            | 8 Colors                                                        |
| box_empty     | The texture displayed on an empty box (3DS Theme)                                                    | On the 3DS theme, the transparent color is `#E0DAD9` by default |
| box_full      | The texture displayed on an empty box (3DS Theme)                                                    | On the 3DS theme, the transparent color is `#E0DAD9` by default |
| icon_gba      | The icon displayed for GBARunner                                                                     | The default transparent color is `#00FF00`                      |
| progress      | The progress loading animation with 8 frames                                                         | 9 Colors                                                        |
| icon_gbamode  | The icon displayed for native GBA Mode                                                               |                                                                 |
| icon_md       | The icon displayed for Mega Drive games                                                              |                                                                 |
| wirelessicons | The icon displayed to indicate a game has wireless support                                           | 7 Colors                                                        |
| box           | The box texture, containing both full and empty textures (for DSi Theme)                             |                                                                 |
| start_text    | The text displayed for the start border on the DSi Theme                                             | 4 Colors                                                        |
| icon_unk      | The icon displayed when a ROM is missing an icon                                                     |                                                                 |
| brace         | The brace texture for the DSi theme shown past the first and last icon                               | 4 Colors                                                        |
| start_border  | The border with animation frames that indicates the selected icon (DSi Theme)                        |
| icon_gb       | The icon displayed for Game Boy games                                                                |                                                                 |
| cursor        | The border with animation frames that indicate the selected icon (3DS Theme)                         |                                                                 |
| icon_snes     | The icon displayed for SNES ROMs                                                                     |                                                                 |
| icon_nes      | The icon displayed for NES ROMs                                                                      |                                                                 |
| icon_gg       | The icon that is displayed for Game Gear ROMs                                                        |                                                                 |
| icon_settings | The icon that is displayed for Nintendo DSi Settings                                                 |
| icon_sms      | The icon that is displayed for Sega Master System ROMs                                               |                                                                 |
| scroll_window | The part of the scrollbar that indicates the icons that are in view                                  | 7 Colors                                                        |
| cornerbutton  | The buttons that are displayed on the SELECT menu (DSi Menu) The name of this texture is a misnomer. |                                                                 |
| button_arrow  | The textures for the arrows on either side of bottom scrollbar (DSi Theme)                           |
| small_cart    | Small icon versions of the icons listed above                                                        |                                                                 |
| dialogbox     | The background of the dialog box that slides down                                                    |                                                                 |
| folder        | The icon for folders                                                                                 |                                                                 |


#### Theme Configuration

You may configure various options on how the theme is drawn in the `theme.ini`, to accommodate larger sprites or textures.

| Value                    | Description                                                                                                  | Default (3DS) | Default (DSi) |
| ------------------------ | ------------------------------------------------------------------------------------------------------------ | ------------- | ------------- |
| `StartBorderRenderY`     | The initial Y position of the Start Border                                                                   | 92            | 81            |
| `StartBorderSpriteW`     | The width of the start border sprite. Note that the start border texture is exactly half of the full border. | 32            | 32            |
| `StartBorderSpriteH`     | The height of the start border sprite                                                                        | 64            | 80            |
| `TitleboxRenderY`        | The initial Y position of the title text drawn                                                               | 96            | 85            |
| `BubbleTipRenderY`       | The Y position of the tip of the bubble that is drawn over the start border                                  | 98            | 80            |
| `BubbleTipRenderX`       | The X position of the tip of the bubble that is drawn over the start border                                  | 125           | 22            |
| `BubbleTipSpriteH`       | The height of the bubble tip sprite                                                                          | 7             | 8             |
| `BubbleTipSpriteW`       | The width of the bubble tip sprite                                                                           | 7             | 11            |
| `RotatingCubesRenderY`   | The Y position on the top screen to draw the rotating cubes                                                  | 78            | N/A           |
| `ShoulderLRenderY`       | The Y position on the top screen to draw the left shoulder                                                   | 172           | 172           |
| `ShoulderLRenderX`       | The X position on the top screen to draw the left shoulder                                                   | 0             | 0             |
| `ShoulderRRenderY`       | The Y position on the top screen to draw the right shoulder                                                  | 172           | 172           |
| `ShoulderRRenderX`       | The X position on the top screen to draw the right shoulder                                                  | 178           | 178           |
| `VolumeRenderX`          | The X position on the top screen to draw the right shoulder                                                  | 4             | 4             |
| `VolumeRenderY`          | The Y position on the top screen to draw the volume icon                                                     | 16            | 16            |
| `BatteryRenderY`         | The Y position on the top screen to draw the battery icon                                                    | 5             | 5             |
| `BatteryRenderX`         | The X position on the top screen to draw the battery icon                                                    | 235           | 235           |
| `RenderPhoto`            | Whether or not to draw a photo on the top screen                                                             | 0             | 1             |
| `StartTextUserPalette`   | Assign the DSi Profile Theme to the palette of the start text                                                | N/A           | 1             |
| `StartBorderUserPalette` | Assign the DSi Profile Theme Palette to the palette of the start border                                      | N/A           | 1             |
| `ButtonArrowUserPalette` | Assign the DSi Profile Theme Palette to the palette of the arrow buttons on the bottom of the screen         | N/A           | 1             |
| `MovingArrowUserPalette` | Assign the DSi Profile Theme Palette to the palette of the arrow shown when moving icons                     | N/A           | 1             |
| `LaunchDotsUserPalette`  | Assign the DSi Profile Theme Palette to the palette of the launch dots                                       | N/A           | 1             |
| `DialogBoxUserPalette`   | Assign the DSi Profile Theme Palette to the palette of the dialog box                                        | N/A           | 1             |
| `PurpleBatteryAvailable` | Set this to 1 if you have purple-colored battery icons alongside the blue/red ones                           | 0             | 0             |
#### Advanced Theming

Occasionally, you may require more than the default number of colors for some paletted textures. In such cases, you may modify the `.grit` compilation file for the texture to increase the size of the palette.

For example, in `scroll_window.grit`, you may edit `-pn7` and change `7` to `16` for 16 colors. Be aware that if you remove the entire `-pn` line, you may encounter unexpected results.

Also note that the absolute maximum of 16 colors per texture is enforced in code and can not be modified. Even if you increase the number of palettes to above 16, no more than 16 colors worth of palette data will be loaded. With the amount of textures loaded in, there may not be enough palette memory to hold 16 colors worth of palettes for every texture. Keep this in mind when adjusting palette sizes.

Additionally, paletted textures must have dimensions that are a multiple of 2. Paletted textures sizes can not be changed except for `bubble` and `start_border`, which can have configurable sprite dimensions in `theme.ini`. However, note that doing so may have unexpected consequences.

Paletted textures are not checked for validity. An invalid texture should be rare if created with the provided makefile, but in certain cases a corrupted texture will cause the menu to not load at all.

The 3DS theme loads its rotating cubes animation from the file located at `/video/3dsRotatingCubes.rvid` in RocketVideo format. If you wish for this not to draw for theme purposes, the file can simply be deleted.

The easiest way of customizing a theme is by editing the BMP textures in a theme's *ui*, *battery*, or *volume* folder. These BMP textures must be in RGB565 format, and use `#FF00FF` as a transparent key color. BMP textures are allowed to vary in size, but may require tweaking of the theme configuration to render correctly (see below).

Changes to paletted textures, or backgrounds are more involved. Within the *grit* or *background_grit* folder of a theme, the various image files may be edited. You will also require [devkitPro](https://devkitpro.org) with GRIT installed. Once you have finished editing your files, you must run

```bash
$ make
```

Do not make any changes to the *.grit* file until you have read the section below on advanced theming.

In order to compile your themes into Grit RIFF Format. This will compile your paletted textures into **.grf** format in the *grf* folder.

Be aware the paletted textures come with more restrictions than BMP textures, the primary being an absolute maximum of 16 colors per texture. However, some textures may have even tighter default palette restrictions, which can be modified at the risk of running out of palette memory (see below).

## Theme File Descriptions

*volume* and *battery* textures are self explanatory.

### UI Textures
| Texture          | Description                                             |
| ---------------- | ------------------------------------------------------- |
| date_time_font   | The font to display the date and time                   |
| Lshoulder        | The left shoulder                                       |
| Lshoulder_greyed | The left shoulder when there are no pages to the left   |
| Rshoulder        | The right shoulder                                      |
| Rshoulder_greyed | The right shoulder when there are no pages to the right |
| top              | The top background                                      |

#### Background Textures
| Texture          | Description                                                                                       |
| ---------------- | ------------------------------------------------------------------------------------------------- |
| bottom           | The bottom background texture when not hovering over an icon                                      |
| bottom_bubble    | The bottom background texture when hovering over an icon                                          |
| bottom_bubble_ds | For the 3DS theme, the bottom background texture when hovering over an icon when on a DS lite     |
| bottom_ds        | For the 3DS theme, the bottom background texture when not hovering over an icon when on a DS lite |
| bottom_moving    | The bottom background texture when moving an icon on the DSi Theme                                |

### Paletted Textures
| Texture       | Description                                                                                          | Palette Restrictions (if less than 16)                          |
| ------------- | ---------------------------------------------------------------------------------------------------- | --------------------------------------------------------------- |
| bips          | The bips displayed on the bottom of the scrollbar (DSi Theme)                                        |                                                                 |
| launch_dot    | The dots displayed when a game is launched (DSi Theme)                                               |                                                                 |
| moving_arrow  | The arrow displayed when a game is being moved (DSi Theme)                                           |                                                                 |
| bubble        | The bottom bit of the bubble that draws over the start border or icon box                            | 8 Colors                                                        |
| box_empty     | The texture displayed on an empty box (3DS Theme)                                                    | On the 3DS theme, the transparent color is `#E0DAD9` by default |
| box_full      | The texture displayed on an empty box (3DS Theme)                                                    | On the 3DS theme, the transparent color is `#E0DAD9` by default |
| icon_gba      | The icon displayed for GBARunner                                                                     | The default transparent color is `#00FF00`                      |
| progress      | The progress loading animation with 8 frames                                                         | 9 Colors                                                        |
| icon_gbamode  | The icon displayed for native GBA Mode                                                               |                                                                 |
| icon_md       | The icon displayed for Mega Drive games                                                              |                                                                 |
| wirelessicons | The icon displayed to indicate a game has wireless support                                           | 7 Colors                                                        |
| box           | The box texture, containing both full and empty textures (for DSi Theme)                             |                                                                 |
| start_text    | The text displayed for the start border on the DSi Theme                                             | 4 Colors                                                        |
| icon_unk      | The icon displayed when a ROM is missing an icon                                                     |                                                                 |
| brace         | The brace texture for the DSi theme shown past the first and last icon                               | 4 Colors                                                        |
| start_border  | The border with animation frames that indicates the selected icon (DSi Theme)                        |
| icon_gb       | The icon displayed for Game Boy games                                                                |                                                                 |
| cursor        | The border with animation frames that indicate the selected icon (3DS Theme)                         |                                                                 |
| icon_snes     | The icon displayed for SNES ROMs                                                                     |                                                                 |
| icon_nes      | The icon displayed for NES ROMs                                                                      |                                                                 |
| icon_gg       | The icon that is displayed for Game Gear ROMs                                                        |                                                                 |
| icon_settings | The icon that is displayed for Nintendo DSi Settings                                                 |
| icon_sms      | The icon that is displayed for Sega Master System ROMs                                               |                                                                 |
| scroll_window | The part of the scrollbar that indicates the icons that are in view                                  | 7 Colors                                                        |
| cornerbutton  | The buttons that are displayed on the SELECT menu (DSi Menu) The name of this texture is a misnomer. |                                                                 |
| button_arrow  | The textures for the arrows on either side of bottom scrollbar (DSi Theme)                           |
| small_cart    | Small icon versions of the icons listed above                                                        |                                                                 |
| dialogbox     | The background of the dialog box that slides down                                                    |                                                                 |
| folder        | The icon for folders                                                                                 |                                                                 |


#### Theme Configuration

You may configure various options on how the theme is drawn in the `theme.ini`, to accommodate larger sprites or textures.

| Value                    | Description                                                                                                  | Default (3DS) | Default (DSi) |
| ------------------------ | ------------------------------------------------------------------------------------------------------------ | ------------- | ------------- |
| `StartBorderRenderY`     | The initial Y position of the Start Border                                                                   | 92            | 81            |
| `StartBorderSpriteW`     | The width of the start border sprite. Note that the start border texture is exactly half of the full border. | 32            | 32            |
| `StartBorderSpriteH`     | The height of the start border sprite                                                                        | 64            | 80            |
| `TitleboxRenderY`        | The initial Y position of the title text drawn                                                               | 96            | 85            |
| `BubbleTipRenderY`       | The Y position of the tip of the bubble that is drawn over the start border                                  | 98            | 80            |
| `BubbleTipRenderX`       | The X position of the tip of the bubble that is drawn over the start border                                  | 125           | 22            |
| `BubbleTipSpriteH`       | The height of the bubble tip sprite                                                                          | 7             | 8             |
| `BubbleTipSpriteW`       | The width of the bubble tip sprite                                                                           | 7             | 11            |
| `RotatingCubesRenderY`   | The Y position on the top screen to draw the rotating cubes                                                  | 78            | N/A           |
| `ShoulderLRenderY`       | The Y position on the top screen to draw the left shoulder                                                   | 172           | 172           |
| `ShoulderLRenderX`       | The X position on the top screen to draw the left shoulder                                                   | 0             | 0             |
| `ShoulderRRenderY`       | The Y position on the top screen to draw the right shoulder                                                  | 172           | 172           |
| `ShoulderRRenderX`       | The X position on the top screen to draw the right shoulder                                                  | 178           | 178           |
| `VolumeRenderX`          | The X position on the top screen to draw the right shoulder                                                  | 4             | 4             |
| `VolumeRenderY`          | The Y position on the top screen to draw the volume icon                                                     | 16            | 16            |
| `BatteryRenderY`         | The Y position on the top screen to draw the battery icon                                                    | 15            | 15            |
| `BatteryRenderX`         | The X position on the top screen to draw the battery icon                                                    | 235           | 235           |
| `RenderPhoto`            | Whether or not to draw a photo on the top screen                                                             | 0             | 1             |
| `StartTextUserPalette`   | Assign the DSi Profile Theme to the palette of the start text                                                | N/A           | 1             |
| `StartBorderUserPalette` | Assign the DSi Profile Theme Palette to the palette of the start border                                      | N/A           | 1             |
| `ButtonArrowUserPalette` | Assign the DSi Profile Theme Palette to the palette of the arrow buttons on the bottom of the screen         | N/A           | 1             |
| `MovingArrowUserPalette` | Assign the DSi Profile Theme Palette to the palette of the arrow shown when moving icons                     | N/A           | 1             |
| `LaunchDotsUserPalette`  | Assign the DSi Profile Theme Palette to the palette of the launch dots                                       | N/A           | 1             |
| `DialogBoxUserPalette`   | Assign the DSi Profile Theme Palette to the palette of the dialog box                                        | N/A           | 1             |
| `PurpleBatteryAvailable` | Set this to 1 if you have purple-colored battery icons alongside the blue/red ones                           | 0             | 0             |
#### Advanced Theming

Occasionally, you may require more than the default number of colors for some paletted textures. In such cases, you may modify the `.grit` compilation file for the texture to increase the size of the palette. Note that the `.grit` files for backgrounds **should not** be changed. The background files are not paletted, but losslessly compressed, and you are allowed full 16-bit full color in backgrounds.

For example, in `scroll_window.grit`, you may edit `-pn7` and change `7` to `16` for 16 colors. Be aware that if you remove the entire `-pn` line, you may encounter unexpected results.

Also note that the absolute maximum of 16 colors per texture is enforced in code and can not be modified. Even if you increase the number of palettes to above 16, no more than 16 colors worth of palette data will be loaded. With the amount of textures loaded in, there may not be enough palette memory to hold 16 colors worth of palettes for every texture. Keep this in mind when adjusting palette sizes.

Additionally, paletted textures must have dimensions that are a multiple of 2. Paletted textures sizes can not be changed except for `bubble` and `start_border`, which can have configurable sprite dimensions in `theme.ini`. However, note that doing so may have unexpected consequences.

Paletted textures are not checked for validity. An invalid texture should be rare if created with the provided makefile, but in certain cases a corrupted texture will cause the menu to not load at all.

The 3DS theme loads its rotating cubes animation from the file located at `/video/3dsRotatingCubes.rvid` in RocketVideo format. If you wish for this not to draw for theme purposes, the file can simply be deleted.)

#### Custom Background Music and Sound fonts.

The DSi Menu and 3DS themes also support custom music. See [sound.md](https://github.com/DS-Homebrew/TWiLightMenu/tree/master/romsel_dsimenutheme/resources/sound/sound.md) for more details.
