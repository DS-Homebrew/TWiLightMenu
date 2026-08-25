# fileQR — Protocolo FQR1

Transferência de arquivos por sequência de QR codes ("multi-frame").
O emissor (PC/celular/tablet) exibe uma sequência animada de QR codes.
O receptor (Nintendo DSi) filma a tela com a câmera, decodifica cada
frame, remonta o arquivo e grava no cartão SD.

## Codificação de um frame

Cada QR carrega o **pacote binário cru** no modo **Byte** do QR (8 bits
por caractere, sem base64). O emissor escreve os bytes via string
Latin-1 (`charCodeAt & 0xFF`) e o `quirc` os devolve idênticos
(`QUIRC_DATA_TYPE_BYTE`). Sem base64 = ~33% menos frames.

### Pacote binário (little-endian)

```
offset  tam  campo
0       4    magic  = "FQR1"
4       1    type   = 0x00 manifest | 0x01 data
5       2    index  (uint16)  frame: 0 no manifest, 1..N nos dados
7       2    total  (uint16)  N = número de frames de dados
9       ..   payload
```

### Payload do manifest (type = 0x00)

```
0    4   crc32    (uint32)  CRC32 do arquivo original inteiro
4    4   size     (uint32)  tamanho do arquivo em bytes
8    2   chunk    (uint16)  bytes de dados por frame
10   1   nameLen  (uint8)
11   ..  name     nameLen bytes, nome do arquivo em UTF-8
```

### Payload de dados (type = 0x01)

```
0  ..  bytes do chunk (até `chunk` bytes)
```

O frame de dados de `index = i` (1-based) cobre os bytes do arquivo no
intervalo `[(i-1)*chunk , i*chunk)`. O último frame pode ser menor.

## Remontagem (receptor)

1. Aguarda o manifest (type 0) → conhece nome, tamanho, N, chunk, crc32.
2. Aloca buffer de `size` bytes e um bitmap de `N` frames recebidos.
3. A cada QR de dados novo, copia o chunk para a posição correta e marca
   o bitmap. Frames repetidos são ignorados (idempotente) — o emissor
   fica em loop, então o receptor pode capturar em qualquer ordem.
4. Quando todos os N frames + manifest chegaram, calcula CRC32 do buffer
   e compara com o do manifest. Se bater, grava `name` no SD.

## CRC32

Polinômio padrão `0xEDB88320` (zlib/PNG), valor inicial `0xFFFFFFFF`,
XOR final `0xFFFFFFFF`. Implementado igual em JS (`crc32.js`) e C
(`crc32.c`) — os dois devem produzir o mesmo valor.

## Parâmetros recomendados

| Parâmetro       | Default | Observação                                       |
|-----------------|---------|--------------------------------------------------|
| chunk           | 512 B   | pacote ~521 B cru → QR ~v18; ajuste p/ a câmera   |
| ECC             | M       | equilíbrio densidade × correção                  |
| FPS             | 5       | limite ~ taxa de decode do quirc no ARM9         |
| manifest a cada | 40      | reexibe o manifest a cada N frames de dados      |

Aumente o chunk para arquivos grandes se a câmera/tela permitirem
escanear QRs mais densos; reduza se houver falhas de leitura. O
manifest é reexibido periodicamente para que a câmera, ao entrar no
meio do loop, obtenha o cabeçalho sem esperar a volta inteira.

> **.nds grandes:** o receptor grava em **streaming** — cada chunk vai
> direto ao arquivo no SD por offset (`fseek`+`fwrite`), sem bufferizar
> o arquivo em RAM. Só um bitmap de frames (1 bit/frame) fica na RAM. O
> CRC32 é conferido relendo o arquivo do SD ao final. Tamanho limitado
> pelo FAT32 (4 GB), não pela RAM.
