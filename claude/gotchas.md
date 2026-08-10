# Armadilhas já resolvidas (leia antes de repetir)

## VRAM de textura (banco A) é o gargalo

- Só o **banco A (128KB)** é VRAM de textura (videoSetup em `ThemeTextures.cpp` põe B/C/D como BG).
- O debug antigo mostrava "512KB" — é o **range do alocador**, não o utilizável. Alocar além do
  banco A dá textura **inválida** (sprites brancos / invisíveis).
- Os 24 bancos de ícone (`NDS_ICON_LIST_BANKS=24`, cada 32×256 4bpp = 4KB) já usam ~100KB.
- **Ordem de alocação importa:** aloque texturas novas (menu bar) **ANTES** de `iconManagerInit()`
  em `main.cpp`, senão o `glLoadTileSet`/`glTexImage2D` falha por falta de espaço contíguo.
  Foi exatamente o bug do menu bar e do antigo uikit (sprites brancos).
- Orçamento: menu bar (256×64 8bpp) = 16KB. Cabe se alocada primeiro.

## Integer scaling limita o zoom

Fontes: box 64px, ícone 32px. Nearest-neighbor uniforme só com fatores **1.0 e 0.5** (e 2.0/0.25).
Por isso ativo=64/inativo=32 (salto 2×). Não existe intermediário "limpo" (0.75 causa sampling
irregular). Para zoom menos dramático **mantendo nitidez**, precisaria de arte numa resolução-fonte
diferente (ex.: box base 48px).

## Camadas: texto (BG) vs sprite (gl2d) não se misturam fácil

- Título/debug/pop-up = `FontGraphic` → buffer BG. Items/box/barra = gl2d (3D).
- Na tela inferior o 3D fica **na frente** do BG. Texto no BG some atrás dos items/barra.
- Por isso o **título vive na tela superior** (SUB, só BG). Se pedirem "título na tela inferior",
  lembre desse conflito (e da barra que já ocupa o rodapé) — provavelmente precisa desenhar texto
  como textura gl2d, que é trabalhoso.

## Fonte: não faça delete-e-recria do FontGraphic

Trocar a fonte com `delete smallFont; smallFont = new FontGraphic(...)` no meio do `fontInit`
**quebrou o render de texto** (o destrutor libera um buffer compartilhado → `print()` desenha
vazio). Solução: forçar `fontPath` para o diretório de fonte do tema **antes** da seleção normal
(mesmo caminho testado do `USE_THEME_FONT`), sem delete/recria. Ver `fontHandler.cpp`.

- `ds.nftr` (fonte do sistema DS embutida) **renderiza vazio** via `FontGraphic::print` nesse
  caminho — não use. A fonte do tema (`font/small.nftr`) funciona.

## Paleta de fonte dos pop-ups

A cor do texto dos pop-ups vem de `FontPaletteDialog1..4` (índices 12-15 no array de `fontInit`).
Forçamos **preto** (`RGB15(0,0,0)|BIT(15)`) para o tema DSi (o dialogbox ficou claro). O texto do
menu normal usa `FontPalette::regular` (índices 0-3).

## BMP: cuidado com bpp e offset da paleta

- Os `.bmp` do tema costumam ser **4bpp** (16 cores), não 8bpp — `sips` reporta "bitsPerSample 8"
  enganosamente. Cheque o header real (byte 28 = bpp).
- Paleta começa em `14 + infoHeaderSize` (normalmente 54, mas confira). `dataOffset` no byte 10.
- 4bpp: 2 pixels/byte (nibble alto primeiro). Rows padded a 4 bytes. BMP é **bottom-up**.
- O loader de titlebox (`ThemeTextures.cpp`) já trata 4bpp e 8bpp.

## Preview do melonDS

- Boot de jogos = tela branca (limitação, não bug). Só testa em hardware. Ver [workflow.md].
- `pkill -x melonDS` pode não matar a tempo → você vê a build **antiga**. Use `killall -9`.
  Sintoma clássico: mudou o código, buildou OK, mas a tela mostra o comportamento velho.
  Cheque com `strings dist/dsimenu.srldr | grep <string removida>` se a build tem/perdeu algo.
- Injeção de tecla não é confiável — peça teste manual para telas que precisam de input.

## Constantes de grid em dois arquivos

`colSpacing`/`rowCY` (render, graphics.cpp) têm que bater com `ROW3_COL_SPACING`/`rowCY` do handler
de toque (fileBrowse.cpp). Mudou um, muda o outro, senão o toque desalinha do visual.
