# assetbind — bind ROM ↔ assets por HASH de conteúdo

Camada que vincula cada ROM aos seus assets (`logo`, `video`) através de um manifesto `.yml`,
usando a **identidade de conteúdo** da ROM — não o nome do arquivo.

## Por que hash e não nome (decisão de identificação)

O nome do arquivo é frágil: pode ser **renomeado**, **duplicado**, e duas ROMs diferentes podem
ter o **mesmo nome**. A identidade estável é o **conteúdo**. Por isso a chave é o hash, com
prioridade **`sha1` > `md5` > `crc32`+`size`**. O nome só é usado como *fallback* opcional
(`allow_name_match`, desligado por padrão). Para DS o hash é do **arquivo inteiro** (sem stripping
de header).

Consequências (esperadas, não são erros):
- ROM renomeada/duplicada → resolve para a mesma entrada (mesmo hash).
- N arquivos com o mesmo conteúdo → compartilham os mesmos assets.
- Arquivos com nomes iguais mas conteúdos diferentes → entradas diferentes, sem colisão.

## Estrutura de pastas

O host (gerador) organiza os assets numa pasta raiz, com **uma subpasta por jogo**:

```
<raiz>/
  manifest.yml
  assets/                       # pasta raiz dos assets
    <game_id>/                  # uma pasta por jogo (game_id = sha1 da ROM)
      logo.png
      video.<ext>
```

Os caminhos no `.yml` são **relativos à raiz do manifesto** (nunca absolutos da máquina).

## Schema do `.yml`

```yaml
version: 1
# allow_name_match: true        # opcional (padrão off): habilita fallback por nome no runtime
games:
  - game_id: "<id estável (sha1 da rom, ou id do ScreenScraper)>"
    identity:                   # CHAVE = conteúdo (sha1 > md5 > crc32+size)
      sha1: "<hex>"
      md5:  "<hex>"
      crc32: "<hex>"
      size: <bytes>
    rom_name: "Cool Game (USA).nds"   # apenas informativo / fallback humano
    assets:
      logo:  "assets/<game_id>/logo.png"       # ou null se ausente
      video: null                              # mp4 empilhado (não vai para o SD)
      video_top:    "assets/<game_id>/top.tgrv"    # tela superior TGR2 128x96 PAL8 (DS)
      video_bottom: "assets/<game_id>/bottom.tgrv" # tela inferior TGR2 128x96 PAL8 (DS)
```

O `fetch_ds_media.sh` já divide o vídeo empilhado do ScreenScraper em dois `.tgrv`
(via `../split_ds_video.py`): `top.tgrv` (tela superior) e `bottom.tgrv` (tela inferior),
cada um em TGR2 128x96 8bpp indexado (paleta de 256 cores por vídeo), prontos para a
versão modificada do TWiLightMenu. Veja o formato em `../docs/asset-structure-changes.md`.
O `run_sdcard.sh`/`scan_and_bind.py` movem esses `.tgrv` para `assets/<game_id>/` e os
registram no manifesto. O `mp4` intermediário não é copiado para o SD.

Ver `manifest.example.yml`.

## Componentes

- **`scan_and_bind.py`** — orquestrador do SD card. **Só LÊ o SD na varredura**; todo o
  download/processamento (baixar via `../fetch_ds_media.sh`, split de vídeo, downscale do logo)
  acontece numa **pasta de trabalho local** (`--cache`, padrão `../cache`). Gera `manifest.yml`
  + `assets_index.yml` no cache e, ao final, **monta as pastas no SD e move** o conteúdo para o
  destino (`--out`). Filtra apps de sistema (blocklist), dedup por hash, idempotente (pula quem
  já está no SD), e mostra **progresso em %**.
- **`system_blocklist.txt`** — arquivos `.nds` que NÃO são jogos (dsimenu, pictochat, nds-bootstrap,
  GBARunner2, …). Exclui por `sha1:` e/ou `name:` — o hash é robusto a renome.
- **`generate_manifest.py`** — o "aplicativo": varre as ROMs + a pasta de mídia do Skyscraper,
  calcula os hashes, **copia/organiza** os assets em `assets/<game_id>/`, e escreve o `manifest.yml`.
- **`rom_binder.py`** — bind em runtime: `load_manifest(path)` → `Binder`; `binder.bind(rom_path)`
  devolve `{game_id, logo, video, matched_by}` (caminhos absolutos, validados em disco) ou `None`.
- **`rom_hash.py`** — hash de conteúdo (sha1/md5/crc32/size) numa passada.
- **`yaml_io.py`** — I/O do `.yml` (usa PyYAML se instalado; senão, leitor/escritor próprio do schema).
- **`test_binder.py`** — testes (unittest).

## Uso

### Escanear o SD card, baixar e bindar (feature do SD)

Varre o SD, ignora apps de sistema (blocklist), baixa logo+vídeo de cada jogo, organiza em
`assets/<sha1>/` **dentro do SD** e escreve o `manifest.yml`. Duplicatas (`... - cópia.nds`)
colapsam por hash em uma entrada e compartilham os assets.

```bash
export SS_USER='...'; export SS_PASS='...'   # ou deixe o fetch_ds_media.sh perguntar/usar cache
python3 scan_and_bind.py --sd "/Volumes/SDCARD"
# opções:
#   --out <dir>          raiz de saída (padrão: a própria raiz do SD)
#   --no-download        só (re)organiza assets existentes e reescreve o manifesto
#   --force              rebaixa mesmo se assets/<sha1>/ já existir
#   --blocklist <file>   blocklist customizada (padrão: system_blocklist.txt)
```

### Gerar o manifesto (host)

O Skyscraper baixa, por ROM, `"<base>-logo.png"` e `"<base>-video.<ext>"`. Então:

```bash
python3 generate_manifest.py \
  --roms  "/caminho/roms/nds" \
  --media "/caminho/skyscraper/media" \
  --out   "/caminho/saida"
# opções: --rom-ext .nds  --logo-suffix -logo.png  --video-suffix -video  --allow-name-match
```

Gera `/caminho/saida/manifest.yml` + `/caminho/saida/assets/<game_id>/...`.

### Bind em runtime

```python
from rom_binder import load_manifest
binder = load_manifest("/caminho/saida/manifest.yml")
res = binder.bind("/sd/roms/Alguma ROM.nds")
if res:
    print(res.logo, res.video, "via", res.matched_by)
else:
    print("sem arte (scrapear depois)")
```

### Testes

```bash
python3 test_binder.py          # ou: python3 -m unittest -v
```
Cobre: (a) renomeado resolve; (b) duplicatas compartilham arte; (c) nomes iguais + hashes
diferentes não colidem; (d) asset faltante degrada com aviso; + hash duplicado = manifesto inválido;
+ round-trip do YAML.

## Tratamento de erros
- Asset ausente em disco (mas referenciado): vincula o que existir, loga aviso, não quebra.
- ROM sem entrada: retorna `None` e loga o `sha1` (para scrapear depois).
- Mesmo hash em duas entradas do `.yml`: `ManifestError` apontando os `game_id` conflitantes.

## Nota de integração com o DS (ponte)

O DS **não calcula hash em runtime** (ARM9 a 67 MHz não hasheia ROMs de dezenas/centenas de MB a
tempo). Este manifesto é do **host**: ele é a fonte de verdade robusta. Para o frontend do DS
consumir, o host resolve cada ROM do cartão por hash e emite um índice leve que o DS casa barato
(hoje o frontend usa `logos.yml` por nome — ver a feature de logo). Essa ponte (emitir o índice do
DS a partir deste manifesto) é o próximo passo natural.
