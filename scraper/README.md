# scraper — baixa vídeos de gameplay do screenscraper.fr

Scraper em PHP que usa a **API v2 do ScreenScraper** para baixar os vídeos de gameplay (mp4) dos
jogos. Referência: https://github.com/zayamatias/sscraper

## Setup

1. `cp config.example.php config.php` e preencha `config.php` (é gitignored).
2. **Credenciais de desenvolvedor** (`devid` / `devpassword`): a API v2 **exige** isso, além do seu
   login de usuário. Elas são obtidas registrando-se como dev no fórum do screenscraper.fr. Sem
   elas, a API retorna erro (mensagem de créditos/credenciais).
3. `ssid` / `sspassword`: seu login de usuário. `ssid` costuma ser o **pseudo** (username), não o
   e-mail — se a autenticação falhar, troque.

## Uso

```bash
php scraper.php "Mario Kart DS (USA)"                 # um jogo
php scraper.php "Mario Kart DS" "Nintendogs"          # vários
php scraper.php --list games.txt                      # um por linha (# = comentário)
php scraper.php --system 15 "Mario Kart DS"           # 15 = Nintendo DS (padrão do config)
```

Os vídeos vão para `scraper/videos/<nome>.mp4`.

## Como funciona

- `ssuserInfos.php` — valida o login e mostra a quota diária.
- `jeuInfos.php?systemeid=..&romnom=..` — busca o jogo e suas mídias.
- Procura a mídia de tipo `video-normalized` (fallback `video`), com preferência de região
  (mundo → US → EU → JP → qualquer), e baixa a URL (com fallback `www.`→`clone.`).

## Notas de segurança

- `config.php` e `videos/` estão no `.gitignore` — credenciais e mídia não são versionadas.
- Se a senha foi compartilhada em texto, considere trocá-la depois.
- Este scraper baixa mídia da **sua própria conta** via API oficial; respeite a quota diária.
