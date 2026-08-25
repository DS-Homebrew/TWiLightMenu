# Roadmap / estado

Snapshot do que já está feito e do que ainda dá pra fazer. **Atualize conforme avançar.**

## Feito ✅

- **Grid de 3 linhas** column-major, scroll horizontal, seleção center-locked.
- **Boxes do item** usando os assets do tema (`box.bmp`/`folder.bmp`).
- **Integer scaling** do zoom (ativo 64px / inativo 32px; ícone 32/16).
- **Navegação** por d-pad (`moveCursor3Row`) + **toque** (grid tap). L/R shoulder desabilitados.
- **Menu bar** no rodapé da tela inferior (`Botton_bar`, via grit), por cima dos items.
- **Fundo brick** nas duas telas (`quickmenu/topbg.png`).
- **Fonte do tema** (pixel, resolução DSi) forçada para o tema DSi.
- **Titlebox** ancorada no rodapé da tela superior, com título + desenvolvedor.
- **Startbox** ("START to play") substitui a titlebox ~1.5s após selecionar o item
  (`BOX_SWAP_DELAY`), **desacoplada** do vídeo de fundo.
- **Vídeo de fundo** de gameplay no topo (`dsiVideoBg`), com estilo de opacidade
  dithering/transparency (`dsiVideoFadeMode`).
- **Status bar** (topo, canto sup. dir.): hora + bateria, sem flicker (redraw-on-change).
- **Overlay de debug** opt-in (`dsiDebugMenu`): fps / polígonos / vértices / VRAM.
- **Menu de configs do frontend** (segurar `Y`): alterna os 3 toggles acima e salva em `options.ini`.
- **Pop-ups X/Y** com texto preto (paleta dialog).
- **Scripts**: `build.sh`, `preview.sh`, `deploy_sd.sh`, `tools/convert_bar.py`.
- **Docs** em `claude/`.

## Em aberto / próximos passos possíveis 🔜

- **Dialogbox dos pop-ups (X/Y):** o usuário estava iterando no asset `grf/dialogbox.bmp` para
  combinar com o tema. Verificar se ficou consistente (caixa clara + texto preto).
- **Elementos da tela superior:** já temos titlebox/startbox, logo, vídeo de fundo, status bar
  (hora+bateria) e overlay de debug. Possíveis adições: **data** na status bar, box art do jogo,
  contador de jogos, mais info (região, tamanho).
- **Layout do rodapé (bottom):** a menu bar e a linha de baixo do grid podem se aproximar/sobrepor.
  Conferir espaçamento com a barra.
- **Ícones de sistema:** pasta (folder) já ok; falta padronizar `icon_settings`, `icon_unk`, e os
  ícones de emulador (GBA/NES/...) se forem exibidos.
- **Conteúdo funcional da menu bar:** hoje é só arte. Se virar barra de menu de verdade, definir os
  botões/áreas e o input.

## Limpeza pendente 🧹

- Remover assets legados sem uso: `assets/*.svg`, `assets/active_border.bmp`,
  `assets/inactive_border.bmp`, e `tools/rasterize_ui.py` (pipeline "uikit" abandonado).
- `assets/Botton_bar.png` tem nome com typo ("Botton") — o `convert_bar.py` referencia esse nome;
  se renomear, atualizar o script.

## Cuidados permanentes ⚠️

- Boot de jogo só valida em **hardware real** (preview dá tela branca — limitação).
- **Flicker do topo só aparece no hardware** (não no melonDS): não escreva no `BG_GFX_SUB` por frame;
  componha no `drawTopTitle` e redesenhe só quando muda. Ver [gotchas.md].
- Campo novo em `TWLSettings` = default + load + **save** (senão não persiste). Ver [gotchas.md].
- Constantes de grid duplicadas (graphics.cpp ↔ fileBrowse.cpp) — manter em sincronia.
- Novas texturas gl2d: alocar **antes** de `iconManagerInit` (VRAM banco A).
- Ver [gotchas.md] antes de repetir tentativas de fonte/VRAM/PNG-runtime/flicker.
