# Frontend do `romsel_dsimenutheme` — carrossel, cursor e render

Documentação da mecânica do ROM selector (tema DSi) do TWiLightMenu, para servir de base ao
rework. Arquivos-chave: `arm9/source/fileBrowse.cpp` (dados + input + launch), `arm9/source/graphics/graphics.cpp`
(render, `vBlankHandler`/`frameRateHandler`), `arm9/source/iconTitle.cpp` + `graphics/iconHandler.*`
(ícones), `arm9/source/graphics/ThemeTextures.cpp` (backgrounds/sprites), `arm9/source/main.cpp`
(boot dos jogos, **fora** do frontend).

## 1. Telas e engines

- **Tela de cima = engine SUB** (`BG_GFX_SUB`), background via `tex().drawTopBg()` (textura de fundo índice 0), buffer `_bgSubBuffer`.
- **Tela de baixo = engine MAIN** (`BG_GFX`), background via `tex().drawBottomBg(index)` (índices 1/2/3), buffer `_bgMainBuffer`.
- O **3D (gl2d)** roda na MAIN e é exibido na **tela de baixo** (via lcd swap). Os **ícones/cursor** são sprites gl2d desenhados sobre o BG da tela de baixo.
- Camadas da tela de baixo (de trás pra frente): BG (wallpaper) → 3D gl2d (ícones, boxes, brace) → camada de fonte (texto, BG2).
- Backgrounds hoje: **cor sólida** (`SOLID_BG_COLOR` em `ThemeTextures.cpp`) em vez do bitmap do tema, tanto em `drawTopBg` quanto em `drawBottomBg`.

## 2. Modelo de dados da listagem

- **`dirContents[scrn]`** (`vector<DirEntry>`): itens do diretório atual (`name`, `isDirectory`). `scrn` = `SwitchState` (dispositivo).
- **`file_count`** = `dirContents[scrn].size()`.
- **Paginação de 40** via **`PAGENUM`** (`ms().pagenum[secondaryDevice]`): índice global = **`CURPOS + PAGENUM*40`**.
- **`CURPOS`** (`ms().cursorPosition[secondaryDevice]`, macro em `fileBrowse.h`): seleção **dentro da página** (0..`last_used_box`). É a **fonte da verdade** para launch e config.
- **`last_used_box`** = `clamp(file_count-1-PAGENUM*40, 0, 39)`.
- **`spawnedtitleboxes`**: nº de boxes materializados na página.
- **Arrays paralelos por item** (índice 0..40; 40 = slot do "moving app"): `isDirectory`, `bnrRomType`, `isDSiWare`, `unitCode`, `gameTid`, `isValid`, `isHomebrew`, `isTwlm`, `customIcon`, `bnrSysSettings`, `bnriconframenumY`, `bannerFlip`, `cachedTitle`, etc. Preenchidos por `getFileInfo`→`getGameInfo` (metadados) na carga da página.

## 3. Geometria do carrossel e centralização

Constantes em `graphics.cpp`: `titleboxXspacing = 58` (passo horizontal entre itens), `titleboxYpos = 85`.

- Cada item `pos` é desenhado em **`iconXpos = 112 + pos*titleboxXspacing`** (box em `96 + pos*spacing`), deslocado por **`titleboxXpos[secondaryDevice]`** (posição de scroll animada).
- O item **selecionado fica sempre centralizado**: o scroll `titleboxXpos` desliza pra pôr o `CURPOS` no centro.
- **`realCurPos = (titleboxXpos + 32) / titleboxXspacing`**: item atualmente centralizado (derivado do scroll). O render usa `realCurPos` pra efeitos de "leque"/aproximação dos vizinhos.
- O render desenha só a janela `pos = CURPOS-3 … CURPOS+3` (`maxIconNumber = 3`; Saturn = 0).

## 4. Cursor / indicadores de seleção

- Não há um "sprite de cursor" que se move; o **item selecionado é o do centro** (posição fixa na tela), e o carrossel rola por baixo.
- **Brace** (`tex().braceImage()`): colchetes desenhados nas bordas do box central (esquerda em ~`66-titleboxXpos`, direita espelhada) — moldura de seleção do tema DSi.
- **Bubble** (`drawBubble(tex().bubbleImage())`, `currentBg==1`): "balão" de fundo do item selecionado.
- **Dbox / info** (`drawDbox`): caixa com nome/desenvolvedora do jogo selecionado + box art.
- Como tudo é centralizado, o `CURPOS` "some" visualmente no centro — o feedback de seleção é o item central + brace + dbox, não uma caixa que percorre a lista.

## 5. Navegação e input (loop principal `browseForFile`)

Loop de espera: `do { scanKeys(); pressed=keysDown(); held=keysDownRepeat(); … updateText(false); } while(!held);`
A cada frame atualiza a info do item central (`titleUpdate(CURPOS)` quando `!bannerTextShown`) e roda checagens de compat (`checkDsiBinaries`, `checkRomAP`) num timer de 30 frames.

| Tecla | Ação |
|-------|------|
| `KEY_LEFT`/`KEY_RIGHT` (ou setas touch no tema DSi) | `moveCursor(false/true)` → CURPOS ∓/± 1 |
| `KEY_UP` (só `sortMethod==4`) | modo "mover app" (reordenar) |
| `KEY_L`/`KEY_R` | `previousPage`/`nextPage` (PAGENUM ∓1, CURPOS=0, recarrega dir/ícones) |
| `KEY_A`/`START` | lança o item central (requer `bannerTextShown && showSTARTborder`) |
| `KEY_Y` | `perGameSettings(...)` (config por-jogo) |
| `KEY_B` | sobe um diretório |
| Touch na scrollbar / arrastar | rola o carrossel; `CURPOS` re-sincronizado de `titleboxXpos` |

### `moveCursor(right, dirContents, maxEntry)`
`do { CURPOS±1; titleUpdate(CURPOS); iconUpdate(CURPOS±2) [carrega o ícone que entra]; anima titleboxXdest em 8 passos de titleboxXspacing/8; } while (tecla ainda held);`
- Ao fim de cada passo: `titleboxXdest = CURPOS*titleboxXspacing`.
- Nos limites (`CURPOS<=0` ou `>=last_used_box`): som de "edge bump", sem mover.
- A **animação real** de `titleboxXpos → titleboxXdest` acontece no `frameRateHandler` (IRQ de VCOUNT), não em `moveCursor`.

## 6. Sistema de ícones

- **6 bancos de textura** (`NDS_ICON_BANK_COUNT = 7`; banco 6 = moving app). **`banco = num % 6`** (`iconTitle.cpp`: `getIcon`, `iconUpdate`).
- **Carregamento sob demanda**: só ~5–6 ícones ao redor do `CURPOS` ficam residentes. Ao rolar, `moveCursor` chama `iconUpdate(CURPOS±2)` pra carregar o ícone que entra na janela; na carga da página, um laço carrega a janela inicial.
- `iconUpdate(isDir, name, num)` decodifica o banner e carrega no banco `num%6` (pastas → `clearIcon`). `drawIcon(x, y, num)` desenha o ícone 32×32 do banco com o frame de animação atual.
- **Metadados** (`getFileInfo`→`getGameInfo`) são carregados pra **todos os 40 itens** da página (não é lazy) — por isso os flags de boot (`isDSiWare`, `unitCode`, `bnrRomType`…) estão sempre válidos para qualquer `CURPOS`.

## 7. Título / texto

- **`titleUpdate(isDir, name, num)`** (`iconTitle.cpp`): só escreve o texto (nome/desenvolvedora) via `writeBannerText`/`writeDialogTitle`, usando `cachedTitle[num]`/`infoFound[num]`. **Não** carrega metadados de boot.
- **`updateText(top)`** (`fontHandler.cpp`): comita a fila de texto na camada de fonte (BG2 = baixo, BG6 = cima).
- Box art: carregada pro `CURPOS` de `_nds/TWiLightMenu/boxart/<TID>.png`.

## 8. Render (`vBlankHandler`, IRQ de VBLANK)

`if (updateFrame) { glBegin2D(); … glEnd2D(); GFX_FLUSH=0; }` — desenha a tela de baixo (3D). Ordem: wallpaper/chrome (bips/scrollwindow/brace), loop de ícones do carrossel (`pos = CURPOS±3`), moving app, dbox, box art, borda START.
- **`frameRateHandler`** (IRQ de VCOUNT): anima `titleboxXpos→titleboxXdest`, faz o **fade** (`screenBrightness` via `SetBrightness`, `fadeType`), e sinaliza `updateFrame`.
- **Fade**: `screenFadedIn()` = brightness==0; `screenFadedOut()` = brightness>24. O launch espera esses estados (`while(!screenFaded…())`).
- `bottomBgRefresh()` (fim do vblank) redesenha o BG da tela de baixo todo frame.

## 9. Backgrounds (agora cor sólida)

- `drawTopBg()` (SUB, buffer `_bgSubBuffer`) e `drawBottomBg(index)` (MAIN, `_bgMainBuffer`) — em vez de `_backgroundTextures[i].copy(...)`, preenchem o buffer com **`SOLID_BG_COLOR`** (`RGB15(20,24,31)|BIT(15)`, defina em `ThemeTextures.cpp`).
- `clearTopScreen()` preenche a tela de cima de branco (usado no launch).
- `bottomBgLoad(int, init)` escolhe a variante (1 base / 2 selecionado / 3 mover-app) e chama `drawBottomBg`.

## 10. Costuras de boot e config (mantidas)

- **Boot**: `browseForFile()` retorna o `entry->name` e seta `applaunch=true` (KEY_A). O **boot real fica em `main.cpp`** após o retorno (`if (applaunch)`), via `runNdsFile`/nds-bootstrap (soft-reset "passme" → carrega `nds-bootstrap-*.nds` → jogo). Decide DS/DSi mode pelos flags por-item.
- **Config**: `perGameSettings(name, …)` (KEY_Y) e o menu de settings.
- Lista de diretório: `getDirectoryContents`, `getFileInfo`, `DirEntry`.

## 11. Pontos de atenção (para o rework)

- O acoplamento **CURPOS (macro) × titleboxXpos (scroll)**: a seleção "centralizada" é derivada do scroll (`realCurPos`), enquanto launch/config usam `CURPOS`. Ao trocar o layout (ex.: grid/lista), garanta que o índice de seleção usado no desenho seja **o mesmo** que o launch (`CURPOS`), lido no **mesmo contexto** (o render roda no IRQ; publicar um snapshot do loop principal evita divergência).
- Só ~6 ícones residentes por vez (banco `num%6`): mostrar mais ícones simultâneos exige aumentar bancos e carregar a janela correspondente.
- `getFileInfo` carrega metadados de toda a página → boot não depende de lazy-load do ícone.
