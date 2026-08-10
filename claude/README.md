# TWiLightMenuGRID — guia do projeto (para o Claude)

Documentação viva do que já foi aprendido/alterado neste fork. Objetivo: numa sessão nova,
conseguir se localizar rápido na estrutura e saber **onde mexer**.

## O que é este projeto

Fork do **TWiLightMenu** (launcher homebrew de ROMs para Nintendo DS/DSi) com um **frontend
próprio em grid/carrossel** substituindo o menu do tema DSi. Só o **render do frontend** foi
reescrito; o **boot de jogos/ROMs** (nds-bootstrap, DSiWare, chainload) e o **select de configs**
foram mantidos intactos.

- Fork: `/Users/biaenico/github/TWiLightMenuGRID`
- Projeto original de referência: `/Users/biaenico/github/TWiLightMenu`
- Todas as nossas mudanças ficam gated para o tema DSi: `ms().theme == TWLSettings::EThemeDSi`.
  Outros temas (3DS/Saturn/HBL) usam o código original.

## Estado atual do frontend (resumo visual)

- **Tela inferior (bottom, MAIN engine):** grid de 3 linhas × N colunas. Cada item = box do tema
  (`box.bmp`/`folder.bmp`) + ícone do jogo por cima. Item selecionado é center-locked e ampliado
  (integer scaling 1.0; inativos 0.5). Barra de menu (`Botton_bar`) no rodapé, por cima dos items.
- **Tela superior (top, SUB engine):** fundo brick + **titlebox** ancorada no rodapé contendo o
  título + desenvolvedor do jogo (texto preto).
- **Fundo das duas telas:** `quickmenu/topbg.png` do tema (padrão de tijolinhos).
- **Fonte do menu:** a fonte do tema (pixel, resolução DSi), forçada para o tema DSi.
- **Pop-ups (X/Y):** dialogbox do tema, texto preto.

## Índice da documentação

- [architecture.md](architecture.md) — como o render funciona, camadas de tela, arquivos-chave,
  navegação, e as constantes que você provavelmente vai querer ajustar.
- [browseforfile-flow.md](browseforfile-flow.md) — o loop do menu (`browseForFile`), o contrato de
  retorno com o boot, input, e onde entram os pop-ups.
- [workflow.md](workflow.md) — build (docker), preview (melonDS), deploy no cartão, e as
  limitações do preview.
- [assets-and-pipelines.md](assets-and-pipelines.md) — de onde vem cada asset, os pipelines de
  conversão (grit, BMP loader), e como adicionar/trocar arte.
- [gotchas.md](gotchas.md) — armadilhas já resolvidas (VRAM, integer scaling, fonte, melonDS).
- [roadmap.md](roadmap.md) — o que já está feito, o que falta, e limpeza pendente.

## Regra de ouro ao mexer

1. Quase tudo do nosso frontend está gated com `ms().theme == TWLSettings::EThemeDSi`.
2. Constantes de layout do grid vivem em **dois lugares** que precisam bater:
   `graphics.cpp` (render) e `fileBrowse.cpp` (navegação/toque). Ver [architecture.md](architecture.md).
3. Depois de mexer em código: `./build.sh` gera `dist/dsimenu.srldr`. Assets do tema (SD) não
   exigem recompilar, só relançar o preview.
