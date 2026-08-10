# Fluxo do `browseForFile` (o loop do menu)

`browseForFile` (`fileBrowse.cpp:3204`) é o loop principal do menu: mostra o grid, processa input,
e **retorna qual jogo lançar**. O boot em si é feito depois, no `main.cpp`. Entender o **contrato de
retorno** é o que permite mexer no frontend sem quebrar o launch.

## Contrato de retorno (a "costura" com o main)

```
main.cpp (~1272):
    filename = browseForFile(extensionList);
    if (applaunch) {           // browseForFile setou essa flag global
        ... runNdsFile / nds-bootstrap / DSiWare ...   // BOOT (intocado)
    }
```

`browseForFile` retorna:
- **`entry->name`** (o filename do item) **e seta `applaunch = true`** → o main boota esse arquivo.
  (arquivo) ou entra na pasta se for diretório.
- **`"null"`** → nenhuma ação de launch; o main faz outra coisa (reabrir menu, config, etc.).

Ou seja: para lançar um jogo, o frontend só precisa setar `CURPOS` no item certo, `applaunch=true`
e `return entry->name`. Tudo que fazemos de visual é em volta disso.

## Anatomia do loop (alto nível)

```
browseForFile:
  setup (getDirectoryContents, getFileInfo, loadRow3WindowIcons na entrada)
  while (1):
    // --- polling de input (uma vez por frame) ---
    do {
      scanKeys(); pressed = keysDown(); held = keysDownRepeat(); touchRead(&touch);
      // (tema DSi) centraliza a coluna: titleboxXdest = (CURPOS/3)*ROW3_COL_SPACING
      bgOperations(true);   // avança animações/bg
    } while (!held && ...);

    // --- D-pad / navegação (gated EThemeDSi) ---
    LEFT/RIGHT -> moveCursor3Row(±1, 0)
    UP/DOWN    -> moveCursor3Row(0, ±1)
    // L/R shoulder: DESABILITADOS no tema DSi

    // --- toque ---
    "Grid tap": mapeia touch -> célula (mesma matemática do render);
                seleciona o item, ou lança se já selecionado (gameTapped)

    // --- ações ---
    A / START / gameTapped:  se diretório -> entra; senão applaunch=true; return entry->name
    Y:  perGameSettings(...)      // pop-up de config por jogo (dialogbox)
    X:  dialog de deletar/ocultar // dialogbox
    B:  sobe um diretório

    // --- atualização visual por seleção ---
    titleUpdate(...) -> (tema DSi) tex().drawTopTitle(titulo)  // titlebox na top screen
    updateBoxArt()   -> desligado no tema DSi (early return)
```

## Estado global relevante

- `CURPOS` = `ms().cursorPosition[secondaryDevice]` — índice selecionado na página (0..39).
- `PAGENUM` = `ms().pagenum[secondaryDevice]` — paginação em blocos de 40. Índice global =
  `CURPOS + PAGENUM*40`.
- `titleboxXpos`/`titleboxXdest` — scroll horizontal (o render lê `titleboxXpos`; a navegação seta
  `titleboxXdest`, o `frameRateHandler` anima entre eles).
- `displayGameIcons` — se o grid está visível (fica `true` no tema DSi mesmo com pop-up aberto).
- `spawnedtitleboxes` — quantos items existem na página (limite do loop de render).

## Onde entram os pop-ups

- **Y → `perGameSettings()`** (`perGameSettings.cpp`): seta `dbox_showIcon`/`showdialogbox`, desliza
  o `dialogbox` do tema e desenha as opções com `printSmall(..., FontPalette::dialog)`.
- **X →** dialog de deletar/ocultar arquivo (mesma mecânica de dbox).
- Ambos usam `drawDbox()` (`graphics.cpp`) que blita `tex().dialogboxImage()` deslizando de baixo.
- Texto dos pop-ups é **preto** no tema DSi (paleta forçada em `fontHandler.cpp`).

## Regra ao mexer aqui

- Não toque no bloco `if (applaunch) { ... }` do `main.cpp` nem nas chamadas `runNdsFile`.
- Se adicionar input novo, respeite o gate `ms().theme == EThemeDSi` para não afetar outros temas.
- Qualquer mudança de layout do grid precisa refletir no **handler de toque** (mesma matemática).
