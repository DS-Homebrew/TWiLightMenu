# fileQR

Transferência de arquivos para o **Nintendo DSi** via **QR codes de
múltiplos frames**, usando a **câmera** do console.

Um emissor (PC, celular, tablet — qualquer navegador) transforma um
arquivo em uma sequência animada de QR codes. O DSi filma a tela com a
câmera, decodifica cada frame, remonta o arquivo e grava no cartão SD.

```
   [ arquivo ]                                   [ DSi ]
       │                                            ▲
       ▼                                            │ grava no SD
  sender (web)  ──►  ░▓ QR ▓░  ──►  câmera do DSi ──► quirc ──► remonta
   FQR1 encode      loop de frames    captura       decode    + CRC32
```

## Estrutura

```
fileQR/
├── shared/PROTOCOL.md     Especificação do protocolo FQR1 (a fonte da verdade)
├── sender/                UI de teste multiplataforma (HTML/JS self-contained)
│   ├── index.html
│   ├── app.js             encode FQR1 + CRC32 + geração/animação dos QR
│   ├── style.css
│   └── vendor-qrcode.js   qrcode-generator (Kazuhiko Arase, MIT)
└── nds/                   Receptor Nintendo DS/DSi (devkitARM + libnds)
    ├── Makefile
    └── source/
        ├── main.c         câmera do DSi → quirc → remontador → FAT/SD
        ├── protocol.c/.h  decode base64 + remontagem FQR1 (testado no host)
        ├── crc32.c/.h     CRC32 zlib (idêntico ao do sender)
        └── quirc/         decodificador de QR vendorizado (dlbeer/quirc, ISC)
```

## Usar o sender

Não precisa de build. Abra `sender/index.html` no navegador
(`open sender/index.html` no macOS), escolha um arquivo, clique
**Gerar frames** e **Play**. Aponte a câmera do DSi para a tela.

Controles: tamanho do chunk, nível de correção de erro, FPS e tamanho
do módulo em px — ajuste para equilibrar densidade × confiabilidade de
leitura da câmera.

O receptor é um projeto **dual-CPU**: o ARM7 faz o init I2C do sensor
Aptina da câmera; o ARM9 captura o frame (256×192 RGB555), converte para
luma, roda o `quirc`, remonta e grava.

## Compilar o receptor NDS

Requer devkitPro com `nds-dev` (devkitARM + libnds 2.x + tools):

```sh
export DEVKITPRO=/opt/devkitpro
export DEVKITARM=/opt/devkitpro/devkitARM
export PATH=$PATH:$DEVKITARM/bin:$DEVKITPRO/tools/bin

cd nds
make          # gera fileQR.nds
```

## Testar

- **Câmera real:** DSi com CFW (rodar o `.nds`), ou **melonDS em modo
  DSi** com BIOS/firmware/NAND do DSi configurados e uma fonte de imagem
  para a câmera. O menu `nds/` usa a câmera **externa** (aponte para a
  tela que exibe os QR do sender). `A` troca de câmera, `START` sai.
- **Pipeline (sem câmera):** o decode+remontagem foi validado no host
  rasterizando os QR reais do sender e passando pelo `quirc` — arquivo
  reconstruído byte-a-byte idêntico. Veja "Estado" abaixo.

> libnds 2.x (calico) ainda **não** traz API de câmera pronta; o driver
> aqui é integrado do projeto `dsi-camera` (ver Créditos).

## Protocolo (resumo)

Cada QR carrega um pacote binário `FQR1` cru (Byte mode, sem base64).
Frame 0 = manifest (nome, tamanho, nº de frames, CRC32); frames 1..N =
chunks de dados. O receptor é idempotente e independe de ordem — por
isso o emissor fica em loop e reexibe o manifest periodicamente.
Detalhes em `shared/PROTOCOL.md`.

## Deploy no console

```sh
./deploy_sd.sh            # compila e copia p/ /Volumes/DSI/fileQR.nds
./deploy_sd.sh --no-build # só copia o .nds atual
```
Ejete o cartão, insira no DSi e rode **fileQR** pelo TWiLightMenu.

## Estado

- [x] Protocolo FQR1 (binário puro, sem base64 — ~33% menos frames)
- [x] Sender web (encode, CRC32, QR binário, estimativa de tempo/taxa,
      reexibição do manifest) — testado
- [x] quirc vendorizado + driver de câmera do DSi (arm7 I2C + arm9)
- [x] Receptor com **seletor de pasta** (A entra, B volta, START confirma)
- [x] **Gravação em streaming p/ SD** (grava por offset, sem limite de
      RAM; CRC relendo o arquivo) — testado no host byte-a-byte com
      frames fora de ordem, duplicatas e lacunas
- [x] `.nds` compilado (`nds/fileQR.nds`, DSi-enhanced) + `deploy_sd.sh`
- [x] Câmera validada em **console real** (DSi)
- [ ] Transferência completa ponta-a-ponta no console (sender → câmera →
      arquivo no SD) — próximo teste de hardware
- [ ] (roadmap) fountain codes p/ robustez contra frames perdidos

## Créditos

- **quirc** — decodificador de QR, Daniel Beer (licença ISC),
  vendorizado em `nds/source/quirc/` (via `arm9/source/quirc/`).
- **dsi-camera** — driver de câmera do DSi, Epicpkmn11 (Pk11);
  pesquisa do problema de captura por Arisotura (melonDS). Base do
  ARM7 (I2C/Aptina) e do `camera.c`/`camera.h` do ARM9.
- **qrcode-generator** — Kazuhiko Arase (MIT), usado no sender.
