<?php
/**
 * scraper.php — baixa vídeos de gameplay (mp4) do screenscraper.fr via API v2.
 *
 * Uso:
 *   php scraper.php "Mario Kart DS (USA)"            # um jogo (por nome de rom)
 *   php scraper.php "Mario Kart DS" "New Super Mario Bros"   # vários
 *   php scraper.php --list games.txt                 # um por linha
 *   php scraper.php --system 15 "Mario Kart DS"      # força o systemeid
 *
 * Config/credenciais: scraper/config.php (ver config.example.php).
 */

error_reporting(E_ALL);
ini_set('display_errors', '1');

const SS_BASE = 'https://www.screenscraper.fr/api2/';

function fail(string $msg, int $code = 1): void {
    fwrite(STDERR, "!! $msg\n");
    exit($code);
}

function load_config(): array {
    $path = __DIR__ . '/config.php';
    if (!is_file($path)) {
        fail("config.php não encontrado. Copie config.example.php para config.php e preencha.");
    }
    $cfg = require $path;
    foreach (['devid', 'devpassword', 'softname', 'ssid', 'sspassword'] as $k) {
        if (empty($cfg[$k]) || in_array($cfg[$k], ['SEU_DEVID', 'SEU_DEVPASSWORD', 'SEU_USUARIO', 'SUA_SENHA'], true)) {
            fail("config.php: campo '$k' vazio/placeholder. Preencha (devid/devpassword exigem registro de dev no screenscraper.fr).");
        }
    }
    return $cfg;
}

/** GET numa endpoint da API v2, retorna array decodificado (ou null se não for JSON). */
function ss_request(array $cfg, string $endpoint, array $params): ?array {
    $auth = [
        'devid'       => $cfg['devid'],
        'devpassword' => $cfg['devpassword'],
        'softname'    => $cfg['softname'],
        'ssid'        => $cfg['ssid'],
        'sspassword'  => $cfg['sspassword'],
        'output'      => 'json',
    ];
    $url = SS_BASE . $endpoint . '?' . http_build_query(array_merge($auth, $params));

    $ch = curl_init($url);
    curl_setopt_array($ch, [
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_FOLLOWLOCATION => true,
        CURLOPT_CONNECTTIMEOUT => 15,
        CURLOPT_TIMEOUT        => 60,
        CURLOPT_USERAGENT      => $cfg['softname'],
    ]);
    $body = curl_exec($ch);
    $http = curl_getinfo($ch, CURLINFO_HTTP_CODE);
    $err  = curl_error($ch);

    if ($body === false) {
        fail("cURL falhou ($endpoint): $err");
    }
    $json = json_decode($body, true);
    if ($json === null) {
        // A API costuma responder erros em texto puro (créditos, credenciais, etc.)
        fwrite(STDERR, "!! Resposta não-JSON de $endpoint (HTTP $http): " . trim(substr($body, 0, 300)) . "\n");
        return null;
    }
    return $json;
}

/** Valida o login e mostra a quota. */
function check_auth(array $cfg): void {
    $r = ss_request($cfg, 'ssuserInfos.php', []);
    if ($r === null) {
        fail("Autenticação falhou (ver mensagem acima). Confira devid/devpassword e ssid/sspassword.");
    }
    $user = $r['response']['ssuser'] ?? null;
    if ($user) {
        $id  = $user['id'] ?? '?';
        $max = $user['maxrequestsperday'] ?? '?';
        $today = $user['requeststoday'] ?? '?';
        fwrite(STDERR, ">> Autenticado como '$id' — requisições hoje: $today/$max\n");
    }
}

/** Procura a URL do vídeo nas mídias do jogo, na ordem de preferência do config. */
function find_video_url(array $jeu, array $mediaTypes): ?string {
    $medias = $jeu['medias'] ?? [];
    foreach ($mediaTypes as $want) {
        // preferência de região: mundo, US, EU, JP, depois qualquer
        foreach (['wor', 'us', 'eu', 'jp', ''] as $region) {
            foreach ($medias as $m) {
                if (strtolower($m['type'] ?? '') !== strtolower($want)) continue;
                if ($region !== '' && strtolower($m['region'] ?? '') !== $region) continue;
                if (!empty($m['url'])) return $m['url'];
            }
        }
    }
    return null;
}

function sanitize(string $name): string {
    $name = preg_replace('/\.[a-z0-9]{2,4}$/i', '', $name); // tira extensão
    $name = preg_replace('/[^\w\-\. ()]+/u', '_', $name);
    return trim($name);
}

/** Baixa uma URL para arquivo, com fallback clone. (como o sscraper). */
function download(array $cfg, string $url, string $dest): bool {
    foreach ([$url, str_replace('www.', 'clone.', $url)] as $try) {
        $fp = fopen($dest, 'wb');
        if (!$fp) fail("Não consegui escrever em $dest");
        $ch = curl_init($try);
        curl_setopt_array($ch, [
            CURLOPT_FILE           => $fp,
            CURLOPT_FOLLOWLOCATION => true,
            CURLOPT_CONNECTTIMEOUT => 15,
            CURLOPT_TIMEOUT        => 300,
            CURLOPT_USERAGENT      => $cfg['softname'],
        ]);
        $ok   = curl_exec($ch);
        $http = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        fclose($fp);
        if ($ok && $http >= 200 && $http < 300 && filesize($dest) > 1024) {
            return true;
        }
        @unlink($dest);
    }
    return false;
}

/** Baixa o vídeo de um jogo (por nome de rom). */
function scrape_game(array $cfg, string $romName, int $systemeid): void {
    fwrite(STDERR, ">> [$romName] consultando...\n");
    $r = ss_request($cfg, 'jeuInfos.php', [
        'systemeid' => $systemeid,
        'romtype'   => 'rom',
        'romnom'    => $romName,
    ]);
    if ($r === null) {
        fwrite(STDERR, "   x consulta falhou\n");
        return;
    }
    $jeu = $r['response']['jeu'] ?? null;
    if (!$jeu) {
        fwrite(STDERR, "   x jogo não encontrado\n");
        return;
    }
    $url = find_video_url($jeu, $cfg['media_types']);
    if (!$url) {
        fwrite(STDERR, "   x sem vídeo disponível\n");
        return;
    }
    $dest = rtrim($cfg['output_dir'], '/') . '/' . sanitize($romName) . '.mp4';
    fwrite(STDERR, "   baixando -> $dest\n");
    if (download($cfg, $url, $dest)) {
        fwrite(STDERR, "   ✓ ok (" . number_format(filesize($dest) / 1048576, 1) . " MB)\n");
    } else {
        fwrite(STDERR, "   x download falhou\n");
    }
}

// ---------------- CLI ----------------
function main(array $argv): void {
    $cfg = load_config();
    if (!is_dir($cfg['output_dir'])) @mkdir($cfg['output_dir'], 0777, true);

    $systemeid = (int)$cfg['systemeid'];
    $games = [];
    for ($i = 1; $i < count($argv); $i++) {
        $a = $argv[$i];
        if ($a === '--system' && isset($argv[$i + 1])) { $systemeid = (int)$argv[++$i]; continue; }
        if ($a === '--list' && isset($argv[$i + 1])) {
            $file = $argv[++$i];
            if (!is_file($file)) fail("lista não encontrada: $file");
            foreach (file($file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) as $ln) {
                $ln = trim($ln);
                if ($ln !== '' && $ln[0] !== '#') $games[] = $ln;
            }
            continue;
        }
        $games[] = $a;
    }

    if (!$games) {
        fwrite(STDERR, "Uso: php scraper.php \"Nome do Jogo\" [outro...] | --list games.txt [--system N]\n");
        exit(2);
    }

    check_auth($cfg);
    foreach ($games as $g) {
        scrape_game($cfg, $g, $systemeid);
    }
}

main($argv);
