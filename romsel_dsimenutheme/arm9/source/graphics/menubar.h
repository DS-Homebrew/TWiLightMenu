#pragma once

// Barra de menu inferior (assets/Botton_bar.png embutido em nitrofiles).
// Desenhada por cima dos items do grid (camada superior).

// Carrega a textura da barra. Chamar antes de iconManagerInit() para garantir
// VRAM de textura no banco A (senão o glTexImage2D pode falhar).
void menuBarInit();

// Desenha a barra no rodapé da tela inferior. Chamar no render DEPOIS dos items.
void menuBarDraw();
