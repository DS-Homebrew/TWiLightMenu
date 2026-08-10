#include "menubar.h"
#include <gl2d.h>
#include <nds.h>

// Bitmap + paleta gerados pelo grit a partir de gfx/botton_bar.bmp
// (tools/convert_bar.py). Cor-chave FF00FF => índice 0 => transparente.
#include "botton_bar.h"

#define BAR_W 256
#define BAR_H 64          // textura pot-de-2; a barra ocupa as 35 linhas do topo
#define BAR_ART_H 35      // altura real da arte
#define BAR_Y (192 - BAR_ART_H)

static int _barTexID = -1;
static glImage _barImg[1];
static bool _barOk = false;

void menuBarInit() {
	_barTexID = glLoadTileSet(_barImg,
				  BAR_W, BAR_H,   // sprite (tile) size
				  BAR_W, BAR_H,   // bitmap size
				  GL_RGB256,      // 8bpp paletado (gradiente cinza)
				  TEXTURE_SIZE_256, TEXTURE_SIZE_64,
				  TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				  256,            // 256 cores (índice 0 transparente)
				  (u16 *)botton_barPal,
				  (u8 *)botton_barBitmap);
	_barOk = (_barTexID >= 0);
}

void menuBarDraw() {
	if (!_barOk)
		return;
	// Rodapé da tela inferior; arte de 35px no topo da textura de 64px.
	glSprite(0, BAR_Y, GL_FLIP_NONE, &_barImg[0]);
}
