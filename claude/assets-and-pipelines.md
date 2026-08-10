# Assets e pipelines

Há **três** formas de asset chegar na tela. Saber qual usar evita retrabalho.

## 1. Assets do tema, via SD em runtime (preferido p/ o que o tema já tem)

O tema ativo fica em `sd:/_nds/TWiLightMenu/dsimenu/themes/<DSI_THEME>/`. Tema atual:
**`DS Menu V2 test`** (setado em `settings.ini` → `DSI_THEME`). No preview:
`/Users/biaenico/github/TWiLightMenuGRID/.preview/sdcard/_nds/TWiLightMenu/dsimenu/themes/DS Menu V2 test/`.

Carregados pela classe `Texture` (via `loadDSiTheme()` em `ThemeTextures.cpp`) como texturas gl2d:
- `grf/box.bmp` (64×128 = 2 tiles 64×64) → `tex().boxfullImage()` — **o box do item** (tile 0 = box
  com o ícone; tile 1 = "DS CARD" vazio).
- `grf/folder.bmp` (64×64) → `tex().folderImage()` — pasta.
- `grf/dialogbox.bmp` (256×192) → dialogbox dos pop-ups.
- `font/*.nftr` → fontes do tema.
- `quickmenu/topbg.png` (256×192) → fundo brick (ver abaixo).

**Editar esses assets NÃO exige recompilar** — só relançar o preview (limpando o cache dsisd.bin).
Formato: BMP paletado (4bpp/8bpp) com **magenta `#FF00FF` = transparente** (cor-chave do grit).

## 2. Grit (asset embutido no binário, textura gl2d)

Para arte nossa que vira **textura gl2d** (tela inferior). Fluxo do `Botton_bar`:

1. Arte em `assets/Botton_bar.png` (256×35 RGBA com transparência).
2. `python3 tools/convert_bar.py` → decodifica o PNG (puro Python, sem libs) e escreve
   `romsel_dsimenutheme/gfx/botton_bar.bmp` (256×64, fundo magenta transparente) + `.grit` (`-gb -gB8 -gT FF00FF`).
3. O **grit do build** converte `gfx/botton_bar.bmp` → símbolos C `botton_barBitmap`/`botton_barPal`
   (header `botton_bar.h`).
4. `menubar.cpp` carrega via `glLoadTileSet(... GL_RGB256, TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT ...)`
   e desenha com `glSprite`.

Para atualizar a barra: troque `assets/Botton_bar.png`, rode `convert_bar.py`, rebuilde.

> **Por que grit e não PNG em runtime?** Tentamos carregar PNG via lodepng + textura `GL_RGBA` e
> falhou (textura invisível). O caminho paletado do grit + `COLOR0_TRANSPARENT` é o comprovado
> neste projeto (é o mesmo do `bubbles` e dos assets do tema).

## 3. BMP loader em runtime (bitmap na tela superior — SUB BG)

Para arte na **tela superior** (que não tem gl2d), lemos o BMP do tema e blittamos os pixels no
`BG_GFX_SUB`. Caso do **titlebox** (`ThemeTextures.cpp`):

- `loadTitlebox()` lê `<tema>/grf/topscreen_titlebox.bmp` (parser de BMP 4bpp/8bpp próprio),
  converte para formato do BG, trata magenta como transparente, e **detecta o bounding box** da
  caixa opaca dentro do canvas (o asset pode ser 256×192 com a caixa em qualquer lugar).
- `drawTopTitle()` blita a caixa **ancorada no rodapé da top screen** (centralizada) e desenha o
  título + desenvolvedor (texto preto, `FontGraphic`) centrado dentro dela.
- Constantes de posição no `drawTopTitle`: `margin` (folga do rodapé). Texto centra no bbox.

Editar `topscreen_titlebox.bmp` **não exige recompilar** (lido do SD). Mudar posição/margem sim.

## Fundos das telas (topbg.png)

`ThemeTextures.cpp` → `loadMenuBg()` decodifica `<tema>/quickmenu/topbg.png` (via lodepng) para
`_menuBgBuffer` (formato BG). `drawTopBg` e `drawBottomBg` copiam esse buffer para as duas telas
(brick em ambas). Fallback = cor sólida se o PNG faltar.

## Assets: onde cada um mora

| Asset | Origem | Como carrega | Recompila? |
|-------|--------|--------------|-----------|
| box/folder do item | tema `grf/*.bmp` | `Texture`/gl2d | não |
| dialogbox (X/Y) | tema `grf/dialogbox.bmp` | `Texture`/gl2d | não |
| fundo brick | tema `quickmenu/topbg.png` | lodepng → BG | não |
| titlebox (top) | tema `grf/topscreen_titlebox.bmp` | BMP loader → SUB BG | não (código sim) |
| menu bar (bottom) | `assets/Botton_bar.png` | convert_bar.py → grit → gl2d | **sim** |
| fonte | tema `font/*.nftr` | `fontHandler` | não |

## Histórico: pipeline "uikit" (abandonado)

Houve uma tentativa de border/background custom via SVG (`assets/*.svg`) → `tools/rasterize_ui.py`.
Foi **substituída** pelo uso do `box.bmp` do tema. Arquivos remanescentes sem uso:
`assets/*.svg`, `assets/active_border.bmp`, `assets/inactive_border.bmp`, `tools/rasterize_ui.py`.
Podem ser removidos.
