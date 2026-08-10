<?php
// Copie para config.php e preencha. config.php é gitignored (não versionar credenciais).
//
// A API v2 do ScreenScraper exige credenciais de DESENVOLVEDOR (devid/devpassword) +
// um softname registrado, ALÉM do seu login de usuário. As de dev são obtidas
// registrando-se como dev no fórum do screenscraper.fr. Sem elas a API retorna erro.

return [
    // --- credenciais de desenvolvedor (obrigatórias) ---
    'devid'       => 'SEU_DEVID',
    'devpassword' => 'SEU_DEVPASSWORD',
    'softname'    => 'TWiLightMenuGRID-scraper',

    // --- seu login de usuário do screenscraper.fr ---
    // Atenção: 'ssid' costuma ser o PSEUDO (username), não o e-mail.
    'ssid'        => 'SEU_USUARIO',
    'sspassword'  => 'SUA_SENHA',

    // --- padrões ---
    'systemeid'   => 15,          // 15 = Nintendo DS
    'output_dir'  => __DIR__ . '/videos',
    'media_types' => ['video-normalized', 'video'], // ordem de preferência
];
