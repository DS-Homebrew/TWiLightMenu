# Alterações na estrutura de assets — prompt para o lado-DS (TWiLightMenu modificado)

> Prompt pronto para entregar a quem for atualizar o leitor de assets no DS.
> Documenta a estrutura gerada pelo pipeline do host (GridFootage:
> `run_sdcard.sh` → `scan_and_bind.py` → `fetch_ds_media.sh` → `split_ds_video.py`).

---

## Contexto

O pipeline do host (GridFootage) mudou a estrutura de assets que a versão modificada do
TWiLightMenu consome. Atualize o leitor no DS para a estrutura abaixo.

## Localização (no SD card)

Tudo fica em: `<SD>/_nds/TWiLightMenu/dsimenu/`

```
<SD>/_nds/TWiLightMenu/dsimenu/
├── manifest.yml               # fonte de verdade: game_id -> identity + assets
├── assets_index.yml           # índice runtime: nome-da-ROM (sem ext) -> game_id
└── assets/
    └── <sha1>/                 # uma pasta por jogo; <sha1> = SHA1 do arquivo .nds inteiro
        ├── logo.png            # logo transparente / wheel
        ├── top.tgrv            # vídeo da TELA SUPERIOR
        └── bottom.tgrv         # vídeo da TELA INFERIOR
```

(Ao lado seguem existindo `logos/` + `logos.yml`, da feature antiga de logo por nome — não conflitam.)

## O que mudou vs. a estrutura anterior

- **ANTES:** assets tinham `{ logo, video }` e o vídeo era um mp4 empilhado (2 telas).
- **AGORA:** o vídeo empilhado foi **DIVIDIDO** em duas telas e convertido para o formato
  **TGRV** (frames crus, blit direto). O campo `video` (mp4) **NÃO é mais usado** (fica
  `null`) e o mp4 não é copiado para o SD. Use `video_top` e `video_bottom`.
- **NOVO:** `assets_index.yml` — índice por nome que o DS usa em runtime para casar a ROM
  em foco sem precisar hashear (nome-base sem extensão → game_id).

## Schema do `manifest.yml`

```yaml
version: 1
games:
  - game_id: "<sha1>"
    identity:                      # identidade por CONTEÚDO (prioridade sha1 > md5 > crc32+size)
      sha1: "<hex>"
      md5:  "<hex>"
      crc32: "<hex>"
      size: <bytes>
    rom_name: "<nome do arquivo>"  # só informativo / fallback humano
    assets:
      logo: "assets/<sha1>/logo.png"            # ou null
      video: null                               # LEGADO: sempre null (não usar)
      video_top: "assets/<sha1>/top.tgrv"       # ou null
      video_bottom: "assets/<sha1>/bottom.tgrv" # ou null
```

Caminhos são **relativos** à pasta do `manifest.yml`. Um game pode ter assets faltando
(`null`): degrade com placeholder, não quebre.

## Schema do `assets_index.yml` (índice runtime por nome)

```yaml
version: 1
roms:
  "<nome-base-da-ROM-sem-extensão>": "<game_id>"
  ...
```

- Uma linha por **arquivo** do SD, incluindo duplicatas: `"Jogo"` e `"Jogo - cópia"`
  apontam para o **mesmo** `game_id`.
- As chaves (nomes) estão em **Unicode NFC** (ex.: "ó" precomposto). Ao comparar com o
  nome lido do sistema de arquivos, normalize para NFC antes de casar.

## Formato TGR2 (arquivo de vídeo por tela) — little-endian

O `.tgrv` é vídeo **cru** (sem codec): o "bitrate" é fixo em
`width*height*bytes_por_pixel*fps` e a **banda de leitura do cartão SD** é o que
limita o FPS no DS. Por isso as telas são gravadas em **baixa resolução** (padrão
128x96, reescalada por hardware p/ 256x192) e, por padrão, em **8bpp indexado**
(metade dos dados do BGR555). O header carrega `width`/`height`/`fmt` — o leitor
deve **respeitá-los** (não assumir 256x192/98304 bytes fixos).

```
offset 0 : magic   'TGR2'  (4 bytes ASCII)
offset 4 : width   u16     (padrão 128)
offset 6 : height  u16     (padrão 96)
offset 8 : fps     u16     (padrão 12)
offset 10: nframes u32
offset 14: fmt     u8       (0 = BGR555 16bpp, 1 = PAL8 8bpp)
offset 15: flags   u8       (bit0 = pixels/paleta já têm bit15/opaco setado)
offset 16: pal_cnt u16      (0 no BGR555; 256 no PAL8)
offset 18: paleta  pal_cnt × u16 BGR555 (bit15=1 opaco)   [só quando fmt=PAL8]
offset ..: nframes × frame
  PAL8   (fmt=1): cada frame = width*height bytes (1 índice por pixel).
                  Carregue a paleta (256 × u16 BGR555) na palette RAM uma vez e
                  use o modo BG bitmap de 256 cores; o índice é blitado direto.
  BGR555 (fmt=0): cada frame = width*height*2 bytes, u16 BGR555
                  (bits 0-4=R, 5-9=G, 10-14=B, bit15=1 opaco). Layout idêntico ao
                  framebuffer 16-bit do DS -> fread direto p/ u16[width*height].
Reescale de width×height até 256×192 na tela (BG affine faz ×2 por hardware com
128×96). Sem áudio (preview mudo).
```

## Como casar a ROM ao asset (runtime)

1. Pegue o **nome-base** da ROM em foco (sem extensão) e normalize para **NFC**.
2. Busque em `assets_index.yml` → obtém o `game_id`.
3. Carregue `assets/<game_id>/logo.png`, `top.tgrv`, `bottom.tgrv` (tratando ausências).
   (Opcional: cruze com `manifest.yml` para metadados extras via `game_id`.)

**IMPORTANTE:** o índice já resolve duplicatas — ROMs renomeadas/duplicadas mapeiam para o
mesmo `game_id` e compartilham os mesmos assets. Não tente casar por conteúdo no DS
(o host já fez isso por SHA1); apenas use o índice por nome.

## Tarefas

- Ler `assets_index.yml` e `manifest.yml` de `<SD>/_nds/TWiLightMenu/dsimenu/`.
- Resolver, para o jogo em foco, `logo` + `top.tgrv` + `bottom.tgrv` (tratando `null`).
- Reproduzir `top.tgrv` na tela superior e `bottom.tgrv` na inferior, em loop, respeitando o
  `fps` e o `width`/`height`/`fmt` do header. PAL8: carregue a paleta uma vez e `fread`
  `width*height` bytes/frame p/ o BG bitmap 256 cores. BGR555: `fread` `width*height*2`
  bytes/frame -> `u16[width*height]`. Reescale até 256x192 (BG affine ×2 com 128x96).
- Tratar assets ausentes (`null` / arquivo inexistente) com fallback (ex.: usar só o logo).
