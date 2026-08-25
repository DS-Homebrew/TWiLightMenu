# Arquitetura do frontend

Tudo abaixo é do subprojeto `romsel_dsimenutheme/` (o "romsel" do tema DSi). É o `.nds` que vira
`dsimenu.srldr` e roda como o menu.

## Mapa mental das telas (crucial)

O DS tem duas telas e o menu usa **engines diferentes** em cada uma:

| Tela | Engine | O que desenha | Como desenhamos |
|------|--------|---------------|-----------------|
| **Inferior (bottom)** | MAIN | grid de items + barra | BG bitmap (`BG_GFX` / `_bgMainBuffer`) para o fundo + **gl2d (3D)** para items/barra/box |
| **Superior (top)** | SUB | fundo + titlebox + texto | BG bitmap (`BG_GFX_SUB` / `_bgSubBuffer`) — **não tem gl2d aqui** |

Implicações práticas:
- Items, box do tema e a menu bar são **sprites gl2d** (3D) na tela inferior. Desenhar "por cima
  dos items" = desenhar depois no loop gl2d (a ordem de desenho define a profundidade).
- O **texto** (título, debug, pop-ups) é `FontGraphic` renderizado num buffer e copiado para o BG
  bitmap. Por isso o título fica na tela **superior** (BG_GFX_SUB): lá não competimos com o 3D.
- Não dá pra "misturar" facilmente texto (BG) e sprite (gl2d) na mesma camada. Ver [gotchas.md].

## Onde o render acontece

`arm9/source/graphics/graphics.cpp`:
- **`vBlankHandler()`** (IRQ de vblank): faz o render gl2d dentro de `if (updateFrame) { glBegin2D(); ... glEnd2D(); }`.
  Dentro há `if (displayGameIcons)` e, para o tema DSi, o **bloco do grid de 3 linhas**.
- **`frameRateHandler()`** (IRQ de vcount): anima scroll (`titleboxXpos`→`titleboxXdest`) e fade.

### O bloco do grid (graphics.cpp, dentro de `if (ms().theme == EThemeDSi)`)

Layout column-major: coluna `c` tem os items `{3c, 3c+1, 3c+2}` nas 3 linhas. O scroll horizontal
(`titleboxXpos`) centraliza a coluna selecionada; up/down escolhem a linha. Constantes atuais:

```
NROWS = 3
ACTIVE_BOX = 64      // box do item selecionado (escala 1.0 — integer scaling)
INACTIVE_BOX = 32    // box dos não selecionados (escala 0.5)
ICON_NUM/ICON_DEN = 32/64   // ícone = box/2 (32 ativo / 16 inativo)
colSpacing = 48      // distância horizontal entre colunas (== ROW3_COL_SPACING em fileBrowse)
rowCY[3] = {36, 88, 140}    // centros das linhas (pitch 52, subido 8px)
```

Por item, desenha em ordem de camada: **box do tema** (`tex().boxfullImage()[0]` para arquivos,
`tex().folderImage()[0]` para pastas) → **ícone** (`drawIconScaled`, centrado). Depois do loop,
**`menuBarDraw()`** desenha a barra por cima.

> Integer scaling: os únicos fatores "limpos" (nearest uniforme) para fontes 64px (box) e 32px
> (ícone) são **1.0 e 0.5**. Por isso o salto de zoom é exatamente 2×. Ver [gotchas.md].

## Composição da tela superior (top compose)

A tela superior (SUB) não tem gl2d: **tudo é bitmap** composto em `ThemeTextures.cpp`. O coração é
`drawTopTitle(text)`, que monta o quadro inteiro num buffer off-screen `_topCompose` e só então faz
**uma cópia contígua** (`tonccpy`) para `BG_GFX_SUB` — nunca se vê meia-tela (evita flicker). Ordem
de composição (trás → frente):

1. **Fundo**: brick sólido (`_menuBgBuffer`) OU vídeo de gameplay (quando ativo), com o estilo de
   opacidade escolhido por `dsiVideoFadeMode` (0 = checker/brick dithered barato; 1 = alphablend a ~40%).
2. **Logo do jogo** (centralizado, com zoom-in/out e drop shadow).
3. **Titlebox** (título + desenvolvedor) **ou startbox** ("START to play"), ancorada no rodapé.
4. **Status bar** (`composeStatusBar(dst)`) — desenhada por último, então fica **por cima de tudo**.

`drawTopTitle` só roda quando algo muda (o loop ocioso chama `tickLogoLoad()` 1x/frame, que só dispara
redraw em `needRedraw`: animação de logo, frame de vídeo, slide de caixa). Implicações:

- **Vídeo de fundo** (`dsiVideoBg`): agendado em `loadGameLogo` (deferido `VIDEO_START_DELAY` frames p/
  não travar a navegação), avança em `tickLogoLoad`. Gated pelo toggle.
- **Titlebox ↔ startbox**: `_boxSwapTimer` conta frames **desde a seleção do item** (resetado em
  `loadGameLogo`, **não** no início do vídeo); ao passar `BOX_SWAP_DELAY` (~90 frames), a caixa atual
  "cai" e a outra "sobe". Desacoplado do vídeo de propósito.
- **Status bar** (hora + bateria, canto sup. dir.): `composeStatusBar` é chamado no fim de `drawTopTitle`
  (camada de cima). Para **não** escrever no framebuffer todo frame (causava flicker no hardware),
  `tickStatusBar()` (1x/frame, via `bgOperations`) só chama `redrawTop()` quando a **hora ou a bateria
  mudam**. Ver [gotchas.md]. `redrawTop()` = `drawTopTitle(_topTitleText)`.
- **Overlay de debug** (`dsiDebugMenu`): `drawTopDebug()` escreve fps/polígonos/vértices/VRAM direto no
  `BG_GFX_SUB` no loop ocioso (após `tickLogoLoad`); ao desligar, `redrawTop()` limpa a box residual.

## Navegação e toque

`arm9/source/fileBrowse.cpp` (função `browseForFile`, o loop principal do menu):
- **`moveCursor3Row(dCol, dRow, dc)`** (perto da linha ~518): move a seleção no grid. `CURPOS`
  continua sendo o índice único (usado por launch/config). Atualiza
  `titleboxXdest = (CURPOS/3) * ROW3_COL_SPACING` para centralizar a coluna.
- D-pad: LEFT→`moveCursor3Row(-1,0)`, RIGHT→`(1,0)`, UP→`(0,-1)`, DOWN→`(0,1)` (gated EThemeDSi).
- **L/R (shoulder) desabilitados** no tema DSi (troca de página removida).
- **Toque:** handler de "grid tap" (procure `Grid tap (DSi theme)` em fileBrowse.cpp): mapeia o
  toque para a célula usando a MESMA matemática do render; tocar num item seleciona, tocar no já
  selecionado lança. Os `rowCY` do toque **precisam bater** com os do render.
- **Carregamento de ícones:** `loadRow3WindowIcons` / `loadRow3Column` carregam a janela de
  colunas visíveis (`selCol-4 .. selCol+3`) nos bancos de ícone.

### ⚠️ Constantes duplicadas (mantenha em sincronia)

| Constante | graphics.cpp (render) | fileBrowse.cpp (nav/toque) |
|-----------|----------------------|----------------------------|
| espaçamento de coluna | `colSpacing` | `#define ROW3_COL_SPACING` |
| centros das linhas | `rowCY[]` | `rowCY[]` no handler de toque |

Se mudar uma, mude a outra.

## Arquivos-chave e o que fazem

- `arm9/source/graphics/graphics.cpp` — **render do grid**, `vBlankHandler`, `drawDbox`, gates do
  tema. É o coração visual da tela inferior.
- `arm9/source/fileBrowse.cpp` — **loop do menu**, navegação, toque, e o boot/launch (intocado na
  parte de boot). Arquivo gigante.
- `arm9/source/graphics/ThemeTextures.cpp` — **fundos** (`drawTopBg`/`drawBottomBg`), **composição do
  topo** (`drawTopTitle` + `composeStatusBar`), loaders de BMP (`loadBoxBmp` c/ param `maxH`;
  `loadTitlebox`/`loadStartbox`/`loadStatusBar`), logo/vídeo de fundo (`tickLogoLoad`, `loadGameLogo`),
  **status bar** (`tickStatusBar`/`composeStatusBar`, bateria via `loadBattery`), **overlay de debug**
  (`drawTopDebug`), `redrawTop`, `loadDSiTheme`, dialogbox. Constantes ajustáveis no topo do arquivo:
  `BOX_SWAP_DELAY`, `SB_TIME_NUM/DEN` (escala da fonte da hora), `SB_TIME_TRACKING` (espaço entre
  caracteres), `SB_BATT_RIGHT_INSET` (recuo fixo da bateria).
- `universal/include/common/twlmenusettings.h` + `universal/source/common/twlmenusettings.cpp` —
  classe `TWLSettings` (`ms()`), lê/grava `options.ini`. Toggles do frontend DSi: `dsiVideoBg`,
  `dsiVideoFadeMode`, `dsiDebugMenu` (chaves `DSI_VIDEO_BG`/`DSI_VIDEO_FADE_MODE`/`DSI_DEBUG_MENU` na
  seção `SRLOADER`). **Todo campo novo precisa de load E save** — ver [gotchas.md].
- `arm9/source/graphics/menubar.cpp/.h` — **barra de menu** (nosso módulo novo). `menuBarInit()`
  (chamado em `main.cpp` **antes** de `iconManagerInit`) e `menuBarDraw()`.
- `arm9/source/graphics/iconHandler.cpp/.h` — bancos de textura de ícones (VRAM). `NDS_ICON_LIST_BANKS=24`.
- `arm9/source/iconTitle.cpp` — decodifica/desenha ícones (`drawIconScaled`), e `titleUpdate`
  redireciona o título para a top screen no tema DSi (`tex().drawTopTitle(...)`).
- `arm9/source/graphics/fontHandler.cpp` — `fontInit()`: escolhe a fonte (forçamos a do tema para
  DSi) e monta as paletas de fonte (forçamos a paleta `dialog` para preto no DSi).
- `arm9/source/main.cpp` — sequência de init: `graphicsInit()` → `menuBarInit()` →
  `iconManagerInit()` → `fontInit()`.
- `arm9/source/perGameSettings.cpp` — pop-up de configs por jogo (tecla Y).

## Boot de jogos (NÃO MEXER sem necessidade)

`main.cpp` cuida do launch após `browseForFile` retornar o filename e setar `applaunch`. Fluxo:
`runNdsFile` → nds-bootstrap (passme/`swiSoftReset`) → chainload. Isso foi mantido do original.
No hardware real funciona 100%. No preview (melonDS DSi-mode) **dá tela branca** — é limitação do
preview, não bug nosso (ver [workflow.md]).
