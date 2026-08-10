# Workflow: build, preview, deploy

## Scripts (raiz do projeto)

| Script | O que faz |
|--------|-----------|
| `./build.sh` | Compila `romsel_dsimenutheme` via docker e gera **`dist/dsimenu.srldr`**. `--preview` também copia pro SD de preview. |
| `./preview.sh` | Compila + copia pro SD de preview + lança o melonDS. `--no-build` (só copia+lança), `--launch` (só lança). |
| `./deploy_sd.sh` | Instala no **cartão real do DSi**: substitui `dsimenu.srldr` (com backup) e sincroniza a pasta de temas do preview pro cartão. Detecta `/Volumes/DSI`. |
| `tools/convert_bar.py` | Converte `assets/Botton_bar.png` → `gfx/botton_bar.bmp` + `.grit` (ver [assets-and-pipelines.md]). |
| `tools/rasterize_ui.py` | (legado, sem uso atual) rasterizava SVGs para o antigo pipeline "uikit". |

## Build

O build é **via docker** (imagem `twilightmenu`, mesma do `compile_docker.sh`):

```
docker run --rm -v "$(pwd):/data" twilightmenu make romsel_dsimenutheme
```

Gera `romsel_dsimenutheme/romsel_dsimenutheme.nds`. Copie para `dist/dsimenu.srldr`.

- O grit (conversão de imagens em `gfx/*.bmp` + `*.grit` → símbolos C) roda **dentro** do build.
  `GRAPHICS := ../gfx` no `arm9/Makefile`.
- Fontes compilados: `arm9/source` e subdirs são globbed (`source/graphics/*.cpp` etc.). Arquivo
  novo em `source/graphics/` é compilado automaticamente.
- Se um `.o` ficar em cache e não recompilar após editar (raro), delete
  `romsel_dsimenutheme/arm9/build/<arquivo>.o` e rebuilde.

## Preview (melonDS)

Roda em **melonDS 1.1** (`/Applications/melonDS.app/Contents/MacOS/melonDS`), modo **DSi**,
HLE do DS-bios. Setup e config em [melonds-preview-workflow] (memória) — resumo:

- Tudo em `.preview/` (untracked): `melonds/` (nand/bios/firmware dumpados), `sdcard/` (cópia do
  `_nds/` + temas + ROMs de teste).
- Rodar: `melonDS ".preview/sdcard/dsimenu.nds"`; esperar ~16-24s; capturar com `screencapture -x`.
- **Cache de folder-sync incha:** `~/Library/Preferences/melonDS/dsisd.bin` cresce e trava o boot.
  `preview.sh` já apaga a cada run; ao relançar manual, apague também.

### Relançar de forma confiável

O `pkill -x melonDS` às vezes não mata a instância a tempo (aí você vê a build antiga!). Use:

```
killall -9 melonDS 2>/dev/null; pkill -9 -f melonDS 2>/dev/null; sleep 2
```

Depois copie o `.nds` novo para `.preview/sdcard/dsimenu.nds` e lance. Para capturar a janela,
traga pra frente antes: `osascript -e 'tell application "System Events" to set frontmost of (first process whose name is "melonDS") to true'`.

Regiões úteis de `screencapture -R x,y,w,h` (janela melonDS ~fullscreen, telas centradas):
- Tela superior: `-R451,40,540,390`
- Tela inferior: `-R451,430,540,400`

(ajuste se a janela mover; se pegar a janela errada, é porque outra janela ficou na frente.)

### ⚠️ LIMITAÇÃO CRÍTICA do preview

**Boot de jogos NÃO funciona no preview** — só UI/render/navegação. Ao lançar um jogo, chega até o
nds-bootstrap e dá **tela branca** (melonDS crasha no argv-chainload / boot em `DSI_MODE=1` precisa
de donor ROM). **Confirmado que o baseline (sem nossas mudanças) também dá tela branca** → não é
bug nosso. **Boot só testa em hardware real** (via `deploy_sd.sh`, funciona 100%).

### Injeção de tecla é não-confiável

Eventos sintéticos de tecla (`osascript key code`) **não chegam** de forma confiável ao input do DS
no melonDS. Para testar navegação/telas que precisam de input (pop-ups X/Y etc.), peça ao usuário
testar no teclado físico, ou capture o estado que já estiver na tela.

## Deploy no cartão real

```
./build.sh          # gera dist/dsimenu.srldr
./deploy_sd.sh      # copia pro /Volumes/DSI + sincroniza temas; faz backup do srldr antigo
```

Alvos no cartão:
- `sd:/_nds/TWiLightMenu/dsimenu.srldr` (nosso binário)
- `sd:/_nds/TWiLightMenu/dsimenu/themes/` (temas, sincronizados do preview)
