
#include "ThemeTextures.h"
#include "ThemeConfig.h"

#include <nds.h>
#include <nds/arm9/dldi.h>
#include <stdio.h>
#include <time.h>
#include "common/twlmenusettings.h"
#include "common/systemdetails.h"
#include "common/logging.h"
#include "myDSiMode.h"

#include "paletteEffects.h"
#include "themefilenames.h"
#include "tool/colortool.h"
// Graphic files
#include "../include/startborderpal.h"

// #include "common/ColorLut.h"
#include "color.h"
#include "errorScreen.h"
#include "fileBrowse.h"
#include "fileCopy.h"
#include "common/lzss.h"
#include "common/tonccpy.h"
#include "common/lodepng.h"
#include "language.h"
#include "ndsheaderbanner.h"
#include "ndma.h"


extern bool useTwlCfg;

// Solid background colour used for both screens instead of the theme's background bitmap.
// Change these RGB (0-31) values to recolour the background. BIT(15) = opaque.
#define SOLID_BG_COLOR (RGB15(20, 24, 31) | BIT(15))

//extern bool widescreenEffects;

extern u16* colorTable;
extern bool invertedColors;
extern bool noWhiteFade;
extern u32 rotatingCubesLoaded;
extern bool rocketVideo_playVideo;
extern u8 *rotatingCubesLocation;

// #include <nds/arm9/decompress.h>
extern bool showColon;

static u16 _bgMainBuffer[256 * 192] = {0};
static u16 _bgSubBuffer[256 * 192] = {0};
static u16* _photoBuffer = NULL;
static u16 _topBorderBuffer[256 * 192] = {0};
static u16* _bgSubBuffer2 = (u16*)_bgSubBuffer;
static u16* _photoBuffer2 = (u16*)_photoBuffer;

// Menu background (quickmenu/topbg.png) loaded from the active theme, converted to BG format.
static u16 _menuBgBuffer[256 * 192];
static bool _menuBgLoaded = false;

// Off-screen compose buffer for the top screen. drawTopTitle builds the whole frame here
// (brick + logo + titlebox + text) and copies it to BG_GFX_SUB in one shot, so the live
// framebuffer never shows a half-drawn state (which flickered the titlebox during the
// per-frame logo zoom redraws).
static u16 _topCompose[256 * 192];

static void loadMenuBg() {
	_menuBgLoaded = true;
	std::vector<unsigned char> image;
	unsigned w = 0, h = 0;
	std::string path = tfn().uiDirectory() + "/quickmenu/topbg.png";
	if (lodepng::decode(image, w, h, path) == 0 && w == 256 && h == 192) {
		for (int i = 0; i < 256 * 192; i++) {
			u8 r = image[i * 4], g = image[i * 4 + 1], b = image[i * 4 + 2];
			_menuBgBuffer[i] = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | BIT(15);
		}
	} else {
		for (int i = 0; i < 256 * 192; i++)
			_menuBgBuffer[i] = SOLID_BG_COLOR;
	}
}

// Dithered version of the top background (quickmenu/topbg_dither.png), gerado pelo host: os pixels
// opacos do brick mantêm a cor (BIT15 setado); os pixels "buraco" (transparentes no PNG) viram 0
// para o vídeo aparecer por trás. Usado como overlay barato sobre o vídeo (sem alpha blend por
// pixel). _menuBgDitherHas = false quando o tema não tem o asset (aí cai no blend por software).
static u16 _menuBgDither[256 * 192];
static bool _menuBgDitherLoaded = false;
static bool _menuBgDitherHas = false;

static void loadMenuBgDither() {
	_menuBgDitherLoaded = true;
	_menuBgDitherHas = false;
	std::vector<unsigned char> image;
	unsigned w = 0, h = 0;
	std::string path = tfn().uiDirectory() + "/quickmenu/topbg_dither.png";
	if (lodepng::decode(image, w, h, path) == 0 && w == 256 && h == 192) {
		for (int i = 0; i < 256 * 192; i++) {
			u8 r = image[i * 4], g = image[i * 4 + 1], b = image[i * 4 + 2], a = image[i * 4 + 3];
			_menuBgDither[i] = (a >= 128) ? ((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | BIT(15)) : 0;
		}
		_menuBgDitherHas = true;
	}
}

// Top-screen title box (grf/topscreen_titlebox.bmp): the game title/developer text is drawn
// inside it. Loaded from the active theme; pixels are BG-format, 0 = transparent (magenta key).
#define TITLEBOX_MAXW 256
#define TITLEBOX_MAXH 192
static u16 _titleboxPix[TITLEBOX_MAXW * TITLEBOX_MAXH];
static int _titleboxW = 0, _titleboxH = 0;
// Opaque box bounding box within the (possibly full-screen) asset; used to place the box.
static int _tbBoxX = 0, _tbBoxY = 0, _tbBoxW = 0, _tbBoxH = 0;
static bool _titleboxLoaded = false;

// Start box (grf/topscreen_startbox.bmp): substitui a titlebox+texto enquanto o vídeo toca.
static u16 _startboxPix[TITLEBOX_MAXW * TITLEBOX_MAXH];
static int _startboxW = 0, _startboxH = 0;
static int _sbBoxX = 0, _sbBoxY = 0, _sbBoxW = 0, _sbBoxH = 0;
static bool _startboxLoaded = false, _startboxHas = false;

// Slide vertical da caixa inferior (titlebox/startbox): a que sai "cai" (desce e some), a que
// entra "sobe" (vem de baixo até o lugar). A troca é disparada pelo início/fim do vídeo.
#define TOPBOX_MARGIN  2       // folga do rodapé
#define BOX_SLIDE_STEP 8       // px por frame do slide
#define BOX_SWAP_DELAY 90      // frames após o vídeo iniciar até trocar titlebox->startbox (~1.5s)
static int _boxKind = 0;       // caixa exibida agora: 0 = titlebox(+texto), 1 = startbox
static int _boxSlide = 0;      // deslocamento vertical (0 = no lugar; >0 = descida/escondida)
static int _boxSwapTimer = 0;  // frames desde que o vídeo começou (conta até BOX_SWAP_DELAY)

// Per-game logo (top screen, drawn above/behind the titlebox). Mapped via logos.yml.
static u16 _logoPix[256 * 128];
static int _logoW = 0, _logoH = 0;
static bool _logoPresent = false;
static std::string _logoKey; // rom base name já resolvido (cache p/ não recarregar toda seleção)
static std::string _gameId;  // game_id (sha1) do jogo em foco, resolvido pelo índice do host

// Carregamento assíncrono/deferido do logo: a troca de item só AGENDA o decode (barato);
// o decode (lodepng, custoso) roda no loop ocioso após o item estabilizar por alguns frames.
// Trocar de item substitui o pendente = cancelamento do anterior. Assim, rolar não trava a UI.
#define LOGO_LOAD_DELAY 8              // frames de estabilidade antes de decodar (debounce)
static std::string _pendingLogoPath;  // caminho do logo a decodar ("" = nada pendente)
static int _pendingLogoDelay = 0;     // frames restantes até decodar
static std::u16string _topTitleText;  // título atual (para redesenhar o topo após o decode)

// Animação de zoom do logo (blit manual no BG da tela superior, sem hardware scaling):
// escala atual anima até o alvo. Ao aparecer (decode pronto) -> zoom-in (0 -> 1).
// Ao trocar de item -> zoom-out do logo anterior (1 -> 0). Zoom-out é mais rápido que
// o debounce do decode, evitando o logo antigo virar o novo no meio da animação.
#define LOGO_ZOOM_IN_STEP  0.14f      // velocidade do zoom-in (aparecer)
#define LOGO_ZOOM_OUT_STEP 0.22f      // velocidade do zoom-out (sair na troca de item)
// Drop shadow do logo (software, na composição): silhueta preta deslocada e alpha-blendada,
// desenhada ANTES do logo. Dá profundidade sobre o brick/vídeo. (O DS não tem shaders.)
#define LOGO_SHADOW_DX    2           // deslocamento horizontal da sombra (px)
#define LOGO_SHADOW_DY    2           // deslocamento vertical da sombra (px)
#define LOGO_SHADOW_ALPHA 128         // opacidade da sombra (0..255; 128 = ~50% preto)
static float _logoScale = 0.0f;       // escala renderizada agora (0..1)
static float _logoScaleDest = 0.0f;   // alvo da animação (0 = escondido, 1 = tamanho cheio)

// ---- Vídeo de gameplay por jogo (fundo da tela superior, .tgrv streamado do SD) ----
// Formato .tgrv: header de 14 bytes ["TGRV", u16 w, u16 h, u16 fps, u32 frameCount] seguido de
// frameCount frames raw RGB15 (w*h*2 bytes cada). Cada jogo tem top.tgrv + bottom.tgrv (as duas
// metades da captura de gameplay); reproduzimos na tela SUPERIOR, em loop, alternando
// top->bottom->top... O vídeo fica ATRÁS do brick, que cai p/ VIDEO_BG_ALPHA de opacidade para
// deixá-lo aparecer. Streamado um frame por vez (arquivos ~40MB não cabem na RAM). Composição por
// software (a SUB engine só tem 1 camada de BG bitmap; não há VRAM p/ blend por hardware).
#define VIDEO_START_DELAY 90   // frames parado no item antes de começar a carregar o vídeo (~1.5s)
#define VIDEO_BG_ALPHA    102  // opacidade do brick por cima do vídeo (~40% de 255)
#define VIDEO_FADE_STEP   12   // velocidade do fade do brick (alpha por frame)
static u16  _videoFrame[256 * 192];   // frame atual decodificado (RGB15), em RAM
static bool _videoActive = false;     // reprodução em andamento (há frame válido)
static FILE *_videoFile = NULL;       // arquivo .tgrv aberto no momento
static int  _videoW = 0, _videoH = 0, _videoFps = 15, _videoFrameCount = 0, _videoFrameIdx = 0;
static int  _videoFmt = 0;            // 0 = BGR555 (16bpp), 1 = PAL8 (8bpp paletado)
static int  _videoFlags = 0;          // bit0 = pixels já têm bit15 (opaco)
static u16  _videoPal[256];           // paleta (PAL8): u16 BGR555 com bit15
static u8   _videoIdxBuf[256 * 192];  // índices do frame (PAL8) antes de expandir p/ _videoFrame
static int  _videoTickAccum = 0;      // acumulador de pacing (loop ~60fps -> vídeo a _videoFps)
static int  _videoWhich = 0;          // 0 = top.tgrv, 1 = bottom.tgrv (alterna ao terminar)
static int  _videoBgAlpha = 255;      // opacidade atual do brick (255 = opaco; anima p/ VIDEO_BG_ALPHA)
static std::string _videoTopPath, _videoBotPath; // caminhos resolvidos p/ o jogo atual
static std::string _pendingVideoBase; // base assets/<id> pendente ("" = nada); arma o start
static int  _pendingVideoDelay = 0;   // frames restantes até começar a abrir o vídeo
// Upscale por nearest-neighbor: o vídeo pode ser menor que a tela (256x192) para poupar leitura
// do SD. Estas LUTs mapeiam cada pixel da tela para o pixel-fonte do vídeo (sem divisão por pixel).
static u8   _vidColMap[256];          // x-tela -> x-vídeo
static u8   _vidRowMap[192];          // y-tela -> y-vídeo

// Carrega um BMP 4/8bpp do tema (magenta #FF00FF = transparente) para `pix`, calculando o
// bounding box opaco (bx,by,bw,bh). Retorna false se não abrir/for inválido. Usado p/ titlebox
// e startbox.
static bool loadBoxBmp(const std::string &path, u16 *pix, int &outW, int &outH,
                       int &bx, int &by, int &bw, int &bh) {
	FILE *f = fopen(path.c_str(), "rb");
	if (!f)
		return false;
	u8 hdr[54];
	if (fread(hdr, 1, 54, f) != 54) { fclose(f); return false; }
	u32 dataOff = hdr[10] | (hdr[11] << 8) | (hdr[12] << 16) | (hdr[13] << 24);
	int w = hdr[18] | (hdr[19] << 8) | (hdr[20] << 16) | (hdr[21] << 24);
	int h = hdr[22] | (hdr[23] << 8) | (hdr[24] << 16) | (hdr[25] << 24);
	int bpp = hdr[28] | (hdr[29] << 8);
	if ((bpp != 4 && bpp != 8) || w <= 0 || w > TITLEBOX_MAXW || h <= 0 || h > TITLEBOX_MAXH) { fclose(f); return false; }

	u16 pal[256];
	bool trans[256] = {false};
	int ncol = (int)(dataOff - 54) / 4;
	fseek(f, 54, SEEK_SET);
	for (int i = 0; i < ncol && i < 256; i++) {
		u8 pe[4];
		if (fread(pe, 1, 4, f) != 4) break;
		u8 b = pe[0], g = pe[1], r = pe[2];
		trans[i] = (r >= 248 && g <= 8 && b >= 248); // magenta #FF00FF => transparent
		pal[i] = (r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | BIT(15);
	}

	// BMP rows padded to 4 bytes (4bpp = 2 pixels/byte, high nibble first).
	int rowsz = (bpp == 4) ? ((((w + 1) / 2) + 3) & ~3) : ((w + 3) & ~3);
	u8 rowbuf[TITLEBOX_MAXW + 4];
	fseek(f, dataOff, SEEK_SET);
	for (int yy = 0; yy < h; yy++) {
		if (fread(rowbuf, 1, rowsz, f) != (size_t)rowsz) break;
		int y = h - 1 - yy; // BMP is bottom-up
		for (int x = 0; x < w; x++) {
			u8 idx = (bpp == 4) ? ((x & 1) ? (rowbuf[x / 2] & 0xF) : (rowbuf[x / 2] >> 4)) : rowbuf[x];
			pix[y * w + x] = trans[idx] ? 0 : pal[idx];
		}
	}
	fclose(f);
	outW = w;
	outH = h;

	// Locate the opaque box within the canvas. If nothing opaque, fall back to the whole asset.
	int minX = w, minY = h, maxX = -1, maxY = -1;
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			if (pix[y * w + x]) {
				if (x < minX) minX = x;
				if (x > maxX) maxX = x;
				if (y < minY) minY = y;
				if (y > maxY) maxY = y;
			}
	if (maxX < 0) { minX = minY = 0; maxX = w - 1; maxY = h - 1; }
	bx = minX; by = minY; bw = maxX - minX + 1; bh = maxY - minY + 1;
	return true;
}

static void loadTitlebox() {
	_titleboxLoaded = true;
	loadBoxBmp(tfn().uiDirectory() + "/grf/topscreen_titlebox.bmp",
	           _titleboxPix, _titleboxW, _titleboxH, _tbBoxX, _tbBoxY, _tbBoxW, _tbBoxH);
}

static void loadStartbox() {
	_startboxLoaded = true;
	_startboxHas = loadBoxBmp(tfn().uiDirectory() + "/grf/topscreen_startbox.bmp",
	           _startboxPix, _startboxW, _startboxH, _sbBoxX, _sbBoxY, _sbBoxW, _sbBoxH);
}

// Remove aspas YAML e des-escapa \" \\; também faz trim.
static std::string ymlUnquote(std::string s) {
	size_t a = s.find_first_not_of(" \t");
	if (a == std::string::npos)
		return "";
	size_t b = s.find_last_not_of(" \t");
	s = s.substr(a, b - a + 1);
	if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
		std::string o;
		for (size_t i = 1; i + 1 < s.size(); i++) {
			if (s[i] == '\\' && i + 2 < s.size()) { o += s[++i]; continue; }
			o += s[i];
		}
		return o;
	}
	return s;
}

// Procura, num .yml simples (chave: valor, chave podendo estar entre aspas), o valor da `key`.
// Suporta chaves com ':' se entre aspas. Retorna "" se não achar.
static std::string ymlLookup(const std::string &path, const std::string &key) {
	FILE *f = fopen(path.c_str(), "rb");
	if (!f)
		return "";
	char line[512];
	std::string result;
	while (fgets(line, sizeof(line), f)) {
		std::string s(line);
		while (!s.empty() && (s.back() == '\n' || s.back() == '\r'))
			s.pop_back();
		size_t a = s.find_first_not_of(" \t");
		if (a == std::string::npos || s[a] == '#')
			continue;
		std::string k;
		size_t sep;
		if (s[a] == '"') {
			size_t i = a + 1;
			for (; i < s.size(); i++) {
				if (s[i] == '\\' && i + 1 < s.size()) { k += s[++i]; continue; }
				if (s[i] == '"') break;
				k += s[i];
			}
			if (i >= s.size()) continue;
			sep = s.find(':', i);
		} else {
			sep = s.find(':');
			if (sep == std::string::npos) continue;
			k = s.substr(a, sep - a);
			size_t ke = k.find_last_not_of(" \t");
			k = (ke == std::string::npos) ? "" : k.substr(0, ke + 1);
		}
		if (sep == std::string::npos)
			continue;
		if (k == key) { result = ymlUnquote(s.substr(sep + 1)); break; }
	}
	fclose(f);
	return result;
}

// Diretório-base dos assets do menu (SD ou FAT).
static std::string dsimenuDir() {
	return std::string(sys().isRunFromSD() ? "sd:" : "fat:") + "/_nds/TWiLightMenu/dsimenu";
}

// Resolve o game_id do jogo `romName` via índice leve do host (assets_index.yml).
// Guarda em _gameId. Retorna a pasta de assets do jogo (assets/<game_id>) ou "".
static std::string resolveGameAssetsDir(const std::string &romName) {
	_gameId = ymlLookup(dsimenuDir() + "/assets_index.yml", romName);
	if (_gameId.empty())
		return "";
	return dsimenuDir() + "/assets/" + _gameId;
}

// Decodifica (lodepng) + escala o PNG do logo para _logoPix. CUSTOSO — só rodar em background.
static void decodeLogoFile(const std::string &logoPath) {
	_logoPresent = false;
	std::vector<unsigned char> img;
	unsigned w = 0, h = 0;
	if (lodepng::decode(img, w, h, logoPath) != 0 || w == 0 || h == 0)
		return;

	// Integer scaling: escolhe o menor fator inteiro 1/N que cabe (amostragem uniforme de N em N).
	const int maxW = 240, maxH = 120;
	int N = 1;
	while ((int)w / N > maxW || (int)h / N > maxH)
		N++;
	int tw = (int)w / N, th = (int)h / N;
	if (tw < 1) tw = 1;
	if (th < 1) th = 1;

	for (int y = 0; y < th; y++) {
		int syi = y * N; // passo exato N = downscale 1/N uniforme
		for (int x = 0; x < tw; x++) {
			int sxi = x * N;
			int i = (syi * (int)w + sxi) * 4;
			u8 r = img[i], g = img[i + 1], b = img[i + 2], a = img[i + 3];
			_logoPix[y * 256 + x] = (a >= 128) ? ((r >> 3) | ((g >> 3) << 5) | ((b >> 3) << 10) | BIT(15)) : 0;
		}
	}
	_logoW = tw;
	_logoH = th;
	_logoPresent = true;
}

// Abre um dos vídeos (0=top,1=bottom), lê+valida o header TGR2 (18 bytes) e a paleta (PAL8),
// posicionando no início dos frames.
// TGR2: magic"TGR2", u16 w, u16 h, u16 fps, u32 nframes, u8 fmt(0=BGR555/1=PAL8), u8 flags,
//       u16 palCnt, [paleta palCnt*u16 se PAL8], depois nframes frames.
static bool videoOpen(int which) {
	const std::string &p = which ? _videoBotPath : _videoTopPath;
	if (p.empty())
		return false;
	_videoFile = fopen(p.c_str(), "rb");
	if (!_videoFile)
		return false;
	u8 h[18];
	if (fread(h, 1, 18, _videoFile) != 18 || h[0] != 'T' || h[1] != 'G' || h[2] != 'R' || h[3] != '2') {
		fclose(_videoFile); _videoFile = NULL; return false;
	}
	_videoW = h[4] | (h[5] << 8);
	_videoH = h[6] | (h[7] << 8);
	_videoFps = h[8] | (h[9] << 8);
	_videoFrameCount = h[10] | (h[11] << 8) | (h[12] << 16) | (h[13] << 24);
	_videoFmt = h[14];
	_videoFlags = h[15];
	int palCnt = h[16] | (h[17] << 8);
	// Aceita qualquer resolução até a tela (upscale no display); precisa caber no buffer.
	if (_videoW < 1 || _videoW > 256 || _videoH < 1 || _videoH > 192 ||
	    _videoW * _videoH > 256 * 192 || _videoFps < 1 || _videoFrameCount < 1 ||
	    (_videoFmt != 0 && _videoFmt != 1)) {
		fclose(_videoFile); _videoFile = NULL; return false;
	}
	if (_videoFmt == 1) { // PAL8: carrega a paleta (u16 BGR555, bit15 já setado no gerador)
		if (palCnt < 1 || palCnt > 256) { fclose(_videoFile); _videoFile = NULL; return false; }
		u8 palbuf[512];
		if (fread(palbuf, 1, (size_t)palCnt * 2, _videoFile) != (size_t)(palCnt * 2)) {
			fclose(_videoFile); _videoFile = NULL; return false;
		}
		for (int i = 0; i < palCnt; i++)
			_videoPal[i] = palbuf[i * 2] | (palbuf[i * 2 + 1] << 8);
		for (int i = palCnt; i < 256; i++)
			_videoPal[i] = 0;
	}
	// (Re)constrói as LUTs de upscale nearest-neighbor para esta resolução.
	for (int x = 0; x < 256; x++) _vidColMap[x] = (u8)(x * _videoW / 256);
	for (int y = 0; y < 192; y++) _vidRowMap[y] = (u8)(y * _videoH / 192);
	_videoFrameIdx = 0;
	return true;
}

// Encerra a reprodução e fecha o arquivo. O brick volta a opaco (fade) no tick.
static void videoStop() {
	if (_videoFile) { fclose(_videoFile); _videoFile = NULL; }
	_videoActive = false;
	_videoWhich = 0;
	_videoFrameIdx = 0;
	_videoTickAccum = 0;
	_boxSwapTimer = 0;
	_pendingVideoBase.clear();
	_pendingVideoDelay = 0;
}

// Lê 1 frame do arquivo aberto para _videoFrame (expande a paleta se PAL8). false = fim/erro.
static bool videoReadOne() {
	if (!_videoFile)
		return false;
	size_t n = (size_t)_videoW * _videoH;
	if (_videoFmt == 1) { // PAL8: lê índices e expande pela paleta
		if (fread(_videoIdxBuf, 1, n, _videoFile) != n)
			return false;
		for (size_t i = 0; i < n; i++)
			_videoFrame[i] = _videoPal[_videoIdxBuf[i]];
	} else { // BGR555: lê direto
		if (fread(_videoFrame, 1, n * 2, _videoFile) != n * 2)
			return false;
		if (!(_videoFlags & 1)) // sem bit de alpha no arquivo -> força opaco
			for (size_t i = 0; i < n; i++)
				_videoFrame[i] |= BIT(15);
	}
	return true;
}

// Avança um frame; ao terminar o arquivo, alterna top<->bottom (loop).
static bool videoAdvance() {
	if (!_videoFile)
		return false;
	if (_videoFrameIdx >= _videoFrameCount) { // acabou este vídeo -> troca para o outro
		fclose(_videoFile); _videoFile = NULL;
		_videoWhich ^= 1;
		if (!videoOpen(_videoWhich))
			return false;
	}
	if (!videoReadOne()) { // leitura curta -> trata como fim e alterna
		fclose(_videoFile); _videoFile = NULL;
		_videoWhich ^= 1;
		if (!videoOpen(_videoWhich) || !videoReadOne())
			return false;
	}
	_videoFrameIdx++;
	return true;
}

// REQUEST (barato): na troca de item, resolve só o CAMINHO do logo e AGENDA o decode em
// background. Não decodifica aqui (senão trava a UI). O item anterior pendente é cancelado.
// Prioridade: índice do host (assets/<game_id>/logo.png). Fallback: logos.yml antigo.
// Também para o vídeo atual e agenda o vídeo do novo jogo (se houver assets).
void ThemeTextures::loadGameLogo(const std::string &romName) {
	if (romName == _logoKey)
		return; // já resolvido p/ este jogo
	_logoKey = romName;
	_logoScaleDest = 0.0f;  // zoom-out do logo anterior (pixels mantidos p/ encolher)
	_gameId.clear();
	_pendingLogoPath.clear(); // cancela qualquer decode agendado do item anterior
	videoStop();              // para o vídeo do item anterior (brick volta a opaco no tick)
	if (romName.empty())
		return;

	std::string base = dsimenuDir();

	std::string logoPath;
	std::string assetsDir = resolveGameAssetsDir(romName);
	if (!assetsDir.empty() && ms().dsiVideoBg) {
		// Agenda o vídeo do jogo (abre/começa só após estabilizar por VIDEO_START_DELAY frames).
		// Gated pelo toggle DSI_VIDEO_BG.
		_videoTopPath = assetsDir + "/top.tgrv";
		_videoBotPath = assetsDir + "/bottom.tgrv";
		_pendingVideoBase = assetsDir;
		_pendingVideoDelay = VIDEO_START_DELAY;
	}
	if (!assetsDir.empty())
		logoPath = assetsDir + "/logo.png";
	if (logoPath.empty()) {
		std::string logoFile = ymlLookup(base + "/logos.yml", romName);
		if (!logoFile.empty())
			logoPath = base + "/logos/" + logoFile;
	}
	if (logoPath.empty())
		return;

	_pendingLogoPath = logoPath;         // agenda o decode
	_pendingLogoDelay = LOGO_LOAD_DELAY;  // após estabilizar N frames
}

// Roda no loop ocioso (uma vez por frame). Debounce: só decodifica após o item ficar estável.
// Se o item mudou nesse meio-tempo, _pendingLogoPath já foi trocado -> o anterior não roda.
void ThemeTextures::tickLogoLoad() {
	bool needRedraw = false;

	// 1) Decode deferido do logo: quando o item estabiliza, decodifica e arma o zoom-in.
	if (!_pendingLogoPath.empty() && --_pendingLogoDelay <= 0) {
		std::string path = _pendingLogoPath;
		_pendingLogoPath.clear();
		decodeLogoFile(path);          // custoso, mas só com o item estável (usuário parado)
		if (_logoPresent) {
			_logoScale = 0.0f;         // começa minúsculo...
			_logoScaleDest = 1.0f;     // ...e cresce (zoom-in ao aparecer)
		}
	}

	// 2) Anima a escala do logo rumo ao alvo.
	if (_logoScale != _logoScaleDest) {
		if (_logoScale < _logoScaleDest) {
			_logoScale += LOGO_ZOOM_IN_STEP;
			if (_logoScale > _logoScaleDest) _logoScale = _logoScaleDest;
		} else {
			_logoScale -= LOGO_ZOOM_OUT_STEP;
			if (_logoScale < _logoScaleDest) _logoScale = _logoScaleDest;
		}
		needRedraw = true;
	}

	// 3) Início deferido do vídeo: após o item ficar parado, abre e lê o 1º frame (custoso).
	if (!_pendingVideoBase.empty() && --_pendingVideoDelay <= 0) {
		_pendingVideoBase.clear();
		_videoWhich = 0;
		if (videoOpen(0) && videoAdvance()) { // começa pelo top.tgrv; lê o primeiro frame
			_videoActive = true;
			_videoTickAccum = 0;
			needRedraw = true;
		} else {
			videoStop();
		}
	}

	// 4) Pacing do vídeo: avança 1 frame a cada (~60/fps) ticks; alterna top<->bottom em loop.
	if (_videoActive && ++_videoTickAccum >= std::max(1, 60 / _videoFps)) {
		_videoTickAccum = 0;
		if (videoAdvance())
			needRedraw = true;
		else
			videoStop();
	}

	// 5) Fade do brick: cai p/ VIDEO_BG_ALPHA quando o vídeo toca, volta a 255 quando para.
	int alphaTarget = _videoActive ? VIDEO_BG_ALPHA : 255;
	if (_videoBgAlpha != alphaTarget) {
		if (_videoBgAlpha > alphaTarget)
			_videoBgAlpha = std::max(_videoBgAlpha - VIDEO_FADE_STEP, alphaTarget);
		else
			_videoBgAlpha = std::min(_videoBgAlpha + VIDEO_FADE_STEP, alphaTarget);
		needRedraw = true;
	}

	// 6) Slide da caixa inferior: alterna titlebox<->startbox. A troca só ocorre BOX_SWAP_DELAY
	//    frames APÓS o vídeo iniciar (desacoplado do start do vídeo). A caixa que sai "cai" (desce
	//    até sumir); então troca; a que entra "sobe" (vem de baixo até o lugar).
	if (_videoActive) {
		if (_boxSwapTimer < BOX_SWAP_DELAY) _boxSwapTimer++;
	} else {
		_boxSwapTimer = 0;
	}
	int wantKind = (_videoActive && _startboxHas && _boxSwapTimer >= BOX_SWAP_DELAY) ? 1 : 0;
	int curBoxH = (_boxKind == 1) ? _sbBoxH : _tbBoxH;
	if (_boxKind != wantKind) {
		_boxSlide += BOX_SLIDE_STEP;                 // desce a caixa atual
		if (_boxSlide >= curBoxH + TOPBOX_MARGIN + 1) {
			_boxKind = wantKind;                     // sumiu -> troca de caixa
			int newBoxH = (_boxKind == 1) ? _sbBoxH : _tbBoxH;
			_boxSlide = newBoxH + TOPBOX_MARGIN + 1; // posiciona a nova totalmente abaixo p/ subir
		}
		needRedraw = true;
	} else if (_boxSlide > 0) {
		_boxSlide -= BOX_SLIDE_STEP;                 // sobe a caixa nova até o lugar
		if (_boxSlide < 0) _boxSlide = 0;
		needRedraw = true;
	}

	if (needRedraw)
		drawTopTitle(_topTitleText);   // recompõe o topo (logo animado e/ou vídeo/fade)
}
// DSi mode double-frame buffers
//static u16* _frameBuffer[2] = {(u16*)0x02F80000, (u16*)0x02F98000};
static u16* _frameBufferBot[2] = {NULL};

static bool topBorderBufferLoaded = false;
bool boxArtColorDeband = false;

static u8* boxArtCache = NULL;	// Size: 0x1B8000
static bool boxArtFound[40] = {false};
uint boxArtWidth = 0, boxArtHeight = 0;

ThemeTextures::ThemeTextures()
    : bubbleTexID(0), bipsTexID(0), scrollwindowTexID(0), buttonarrowTexID(0),
      movingarrowTexID(0), launchdotTexID(0), startTexID(0), startbrdTexID(0), settingsTexID(0), manualTexID(0), braceTexID(0),
      boxfullTexID(0), boxemptyTexID(0), folderTexID(0), cornerButtonTexID(0), smallCartTexID(0), progressTexID(0),
      dialogboxTexID(0), wirelessiconTexID(0), _cachedVolumeLevel(-1), _cachedBatteryLevel(-1), _profileNameLoaded(false) {
	// Overallocation, but thats fine,
	// 0: Top, 1: Bottom, 2: Bottom Bubble, 3: Moving, 4: MovingLeft, 5: MovingRight
	_backgroundTextures.reserve(6);
}

void ThemeTextures::loadBubbleImage(const Texture &tex, int sprW, int sprH) {
	_bubbleImage = std::move(loadTexture(&bubbleTexID, tex, 1, sprW, sprH, GL_RGB16));
}

void ThemeTextures::loadProgressImage(const Texture &tex) {
	// todo: 9 palette
	_progressImage = std::move(loadTexture(&progressTexID, tex, (16 / 16) * (128 / 16), 16, 16, GL_RGB16));
}

void ThemeTextures::loadDialogboxImage(const Texture &tex) {
	_dialogboxImage = std::move(loadTexture(&dialogboxTexID, tex, (256 / 16) * (256 / 16), 16, 16, GL_RGB16));
}

void ThemeTextures::loadBipsImage(const Texture &tex) {
	_bipsImage = std::move(loadTexture(&bipsTexID, tex, (8 / 8) * (32 / 8), 8, 8, GL_RGB16));
}

void ThemeTextures::loadScrollwindowImage(const Texture &tex) {
	_scrollwindowImage = std::move(loadTexture(&scrollwindowTexID, tex, (32 / 16) * (32 / 16), 32, 32, GL_RGB16));
}

void ThemeTextures::loadButtonarrowImage(const Texture &tex) {
	_buttonarrowImage = std::move(loadTexture(&buttonarrowTexID, tex, (32 / 32) * (128 / 32), 32, 32, GL_RGB16));
}

void ThemeTextures::loadMovingarrowImage(const Texture &tex) {
	_movingarrowImage = std::move(loadTexture(&movingarrowTexID, tex, (32 / 32) * (32 / 32), 32, 32, GL_RGB16));
}

void ThemeTextures::loadLaunchdotImage(const Texture &tex) {
	_launchdotImage = std::move(loadTexture(&launchdotTexID, tex, (16 / 16) * (96 / 16), 16, 16, GL_RGB16));
}

void ThemeTextures::loadStartImage(const Texture &tex) {
	_startImage = std::move(loadTexture(&startTexID, tex, (64 / 16) * (128 / 16), 64, 16, GL_RGB16));
}

void ThemeTextures::loadStartbrdImage(const Texture &tex, int sprH) {
	int arraysize = (tex.texWidth() / tc().startBorderSpriteW()) * (tex.texHeight() / sprH);
	_startbrdImage = std::move(loadTexture(&startbrdTexID, tex, arraysize, tc().startBorderSpriteW(), sprH, GL_RGB16));
}
void ThemeTextures::loadBraceImage(const Texture &tex) {
	// todo: confirm 4 palette
	_braceImage = std::move(loadTexture(&braceTexID, tex, (16 / 16) * (128 / 16), 16, 128, GL_RGB16));
}

void ThemeTextures::loadSettingsImage(const Texture &tex) {
	_settingsImage = std::move(loadTexture(&settingsTexID, tex, (64 / 16) * (128 / 64), 64, 64, GL_RGB16));
}

void ThemeTextures::loadManualImage(const Texture &tex) {
	_manualImage = std::move(loadTexture(&manualTexID, tex, (32 / 32) * (32 / 32), 32, 32, GL_RGB16));
}

void ThemeTextures::loadBoxfullImage(const Texture &tex) {
	//_boxfullImage = std::move(loadTexture(&boxfullTexID, tex, (64 / 16) * (128 / 64), 64, 64, (ms().theme==4 ? GL_RGB256 : GL_RGB16)));
	_boxfullImage = std::move(loadTexture(&boxfullTexID, tex, (64 / 16) * (128 / 64), 64, 64, GL_RGB16));
}

void ThemeTextures::loadBoxemptyImage(const Texture &tex) {
	//_boxemptyImage = std::move(loadTexture(&boxemptyTexID, tex, (64 / 16) * (64 / 16), 64, 64, (ms().theme==4 ? GL_RGB256 : GL_RGB16)));
	_boxemptyImage = std::move(loadTexture(&boxemptyTexID, tex, (64 / 16) * (64 / 16), 64, 64, GL_RGB16));
}

void ThemeTextures::loadFolderImage(const Texture &tex) {
	_folderImage = std::move(loadTexture(&folderTexID, tex, (64 / 16) * (64 / 16), 64, 64, GL_RGB16));
}

void ThemeTextures::loadCornerButtonImage(const Texture &tex, int arraysize, int sprW, int sprH) {
	_cornerButtonImage = std::move(loadTexture(&cornerButtonTexID, tex, arraysize, sprW, sprH, GL_RGB16));
}

void ThemeTextures::loadSmallCartImage(const Texture &tex) {
	_smallCartImage = std::move(loadTexture(&smallCartTexID, tex, (32 / 16) * (256 / 32), 32, 32, GL_RGB16));
}

void ThemeTextures::loadWirelessIcons(const Texture &tex) {
	_wirelessIcons = std::move(loadTexture(&wirelessiconTexID, tex, (32 / 32) * (64 / 32), 32, 32, GL_RGB16));
}

inline GL_TEXTURE_SIZE_ENUM get_tex_size(int texSize) {
	if (texSize <= 8)
		return TEXTURE_SIZE_8;
	if (texSize <= 16)
		return TEXTURE_SIZE_16;
	if (texSize <= 32)
		return TEXTURE_SIZE_32;
	if (texSize <= 64)
		return TEXTURE_SIZE_64;
	if (texSize <= 128)
		return TEXTURE_SIZE_128;
	if (texSize <= 256)
		return TEXTURE_SIZE_256;
	if (texSize <= 512)
		return TEXTURE_SIZE_512;
	return TEXTURE_SIZE_1024;
}

inline const unsigned short *apply_personal_theme(const unsigned short *palette) {
	return palette + (getFavoriteColor() * 16);
}

unique_ptr<glImage[]> ThemeTextures::loadTexture(int *textureId, const Texture &texture, unsigned int arraySize,
						 int sprW, int sprH, GL_TEXTURE_TYPE_ENUM texType) {

	// We need to delete the texture since the resource held by the unique pointer will be
	// immediately dropped when we assign it to the pointer.

	u32 texW = texture.texWidth();
	u32 texH = texture.texHeight();
	u8 paletteLength = texture.paletteLength();

	if (*textureId != 0) {
		nocashMessage("Existing texture found!?");
		glDeleteTextures(1, textureId);
	}

	// Do a heap allocation of arraySize glImage
	unique_ptr<glImage[]> texturePtr = std::make_unique<glImage[]>(arraySize);

	// Load the texture here.
	*textureId = glLoadTileSet(texturePtr.get(),   // pointer to glImage array
				   sprW,	       // sprite width
				   sprH,	       // sprite height
				   texW,	       // bitmap width
				   texH,	       // bitmap height
				   texType,	   // texture type for glTexImage2D() in videoGL.h
				   get_tex_size(texW), // sizeX for glTexImage2D() in videoGL.h
				   get_tex_size(texH), // sizeY for glTexImage2D() in videoGL.h
				   TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				   paletteLength,	    // Length of the palette to use (16 colors)
				   (u16 *)texture.palette(), // Load our 16 color tiles palette
				   (u8 *)texture.texture()   // image data generated by GRIT
	);
	return texturePtr;
}

void ThemeTextures::reloadPalDialogBox() {
	if (ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) return;
	glBindTexture(0, dialogboxTexID);
	glColorSubTableEXT(0, 0, _dialogBoxTexture->paletteLength(), 0, 0, _dialogBoxTexture->palette());
	if (ms().theme != TWLSettings::ETheme3DS) {
		glBindTexture(0, cornerButtonTexID);
		glColorSubTableEXT(0, 0, 16, 0, 0, _cornerButtonTexture->palette());
	}
}

void ThemeTextures::loadBackgrounds() {
	// 0: Top, 1: Bottom, 2: Bottom Bubble, 3: Moving, 4: MovingLeft, 5: MovingRight

	if (ms().showPhoto && tc().renderPhoto()) {
		_backgroundTextures.emplace_back(TFN_BG_TOPPHOTOBG, TFN_BG_TOPBG, ms().theme == TWLSettings::EThemeDSi ? TFN_FALLBACK_BG_TOPPHOTOBG : TFN_FALLBACK_BG_TOPBG);
	} else {
		_backgroundTextures.emplace_back(TFN_BG_TOPBG, TFN_FALLBACK_BG_TOPBG);
	}
		
	
	if (ms().theme == TWLSettings::ETheme3DS && !sys().isRegularDS()) {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG);
		return;
	}

	if (ms().theme == TWLSettings::ETheme3DS && sys().isRegularDS()) {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG_DS, TFN_FALLBACK_BG_BOTTOMBG_DS);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG_DS, TFN_FALLBACK_BG_BOTTOMBUBBLEBG_DS);
		return;
	}
	// DSi Theme
	if (ms().macroMode) {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG_MACRO, TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG_MACRO, TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG_MACRO);
		if (ms().theme == TWLSettings::EThemeDSi) _backgroundTextures.emplace_back(TFN_BG_BOTTOMMOVINGBG_MACRO, TFN_BG_BOTTOMMOVINGBG, TFN_FALLBACK_BG_BOTTOMMOVINGBG);
	} else {
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBG, TFN_FALLBACK_BG_BOTTOMBG);
		_backgroundTextures.emplace_back(TFN_BG_BOTTOMBUBBLEBG, TFN_FALLBACK_BG_BOTTOMBUBBLEBG);
		if (ms().theme == TWLSettings::EThemeDSi) _backgroundTextures.emplace_back(TFN_BG_BOTTOMMOVINGBG, TFN_FALLBACK_BG_BOTTOMMOVINGBG);
	}
	
}

void ThemeTextures::loadHBTheme() {	
	logPrint("tex().loadHBTheme()\n");

	// iprintf("tex().loadBackgrounds()\n");
	loadBackgrounds();
	// iprintf("tex().loadUITextures()\n");
	loadUITextures();

	// iprintf("tex().loadVolumeTextures()\n");
	loadVolumeTextures();
	// iprintf("tex().loadBatteryTextures()\n");
	loadBatteryTextures();

	_boxFullTexture = std::make_unique<Texture>(TFN_GRF_BOX_FULL, TFN_FALLBACK_GRF_BOX_FULL);
	_boxEmptyTexture = std::make_unique<Texture>(TFN_GRF_BOX_EMPTY, TFN_FALLBACK_GRF_BOX_EMPTY);
	_braceTexture = std::make_unique<Texture>(TFN_GRF_BRACE, TFN_FALLBACK_GRF_BRACE);
	_cornerButtonTexture = std::make_unique<Texture>(TFN_GRF_CORNERBUTTON, TFN_FALLBACK_GRF_CORNERBUTTON);

	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);

	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);
	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_wirelessIconsTexture = std::make_unique<Texture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);
	_manualIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_MANUAL, TFN_FALLBACK_GRF_ICON_MANUAL);

	
	// iprintf("tex().loadSettingsImage(*_settingsIconTexture)\n");
	loadSettingsImage(*_settingsIconTexture);
	// iprintf("tex().loadBraceImage(*_braceTexture)\n");
	loadBraceImage(*_braceTexture);

	// iprintf("tex().loadBoxfullImage(*_boxFullTexture)\n");
	loadBoxfullImage(*_boxFullTexture);
	// iprintf("tex().loadBoxEmptyImage(*_boxFullTexture)\n");
	loadBoxemptyImage(*_boxEmptyTexture);

	// iprintf("tex().loadManualImage(*_manualIconTexture)\n");
	loadManualImage(*_manualIconTexture);
	// iprintf("tex().loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32)\n");
	loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32);
	// iprintf("tex().loadSmallCartImage(*_smallCartTexture)\n");
	loadSmallCartImage(*_smallCartTexture);
	// iprintf("tex().loadFolderImage(*_folderTexture)\n");
	loadFolderImage(*_folderTexture);
	
	// iprintf("tex().loadProgressImage(*_progressTexture)\n");
	loadProgressImage(*_progressTexture);
	// iprintf("tex().loadWirelessIcons(*_wirelessIconsTexture)\n");
	loadWirelessIcons(*_wirelessIconsTexture);
	
}

void ThemeTextures::loadSaturnTheme() {	
	logPrint("tex().loadSaturnTheme()\n");

	loadBackgrounds();
	loadUITextures();

	loadVolumeTextures();
	loadBatteryTextures();

	_boxFullTexture = std::make_unique<Texture>(TFN_GRF_BOX_FULL, TFN_FALLBACK_GRF_BOX_FULL);
	_boxEmptyTexture = std::make_unique<Texture>(TFN_GRF_BOX_EMPTY, TFN_FALLBACK_GRF_BOX_EMPTY);
	_braceTexture = std::make_unique<Texture>(TFN_GRF_BRACE, TFN_FALLBACK_GRF_BRACE);
	_cornerButtonTexture = std::make_unique<Texture>(TFN_GRF_CORNERBUTTON, TFN_FALLBACK_GRF_CORNERBUTTON);

	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);

	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);
	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_wirelessIconsTexture = std::make_unique<Texture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);
	_manualIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_MANUAL, TFN_FALLBACK_GRF_ICON_MANUAL);

	loadSettingsImage(*_settingsIconTexture);
	loadBraceImage(*_braceTexture);

	loadBoxfullImage(*_boxFullTexture);
	loadBoxemptyImage(*_boxEmptyTexture);

	loadManualImage(*_manualIconTexture);
	loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32);
	loadSmallCartImage(*_smallCartTexture);
	loadFolderImage(*_folderTexture);

	loadProgressImage(*_progressTexture);
	loadWirelessIcons(*_wirelessIconsTexture);
}

void ThemeTextures::load3DSTheme() {
	logPrint("tex().load3DSTheme()\n");

	loadBackgrounds();
	loadUITextures();

	loadVolumeTextures();
	loadBatteryTextures();

	_bubbleTexture = std::make_unique<Texture>(TFN_GRF_BUBBLE, TFN_FALLBACK_GRF_BUBBLE);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);

	_boxFullTexture = std::make_unique<Texture>(TFN_GRF_BOX_FULL, TFN_FALLBACK_GRF_BOX_FULL);
	_boxEmptyTexture = std::make_unique<Texture>(TFN_GRF_BOX_EMPTY, TFN_FALLBACK_GRF_BOX_EMPTY);
	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);

	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_wirelessIconsTexture = std::make_unique<Texture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_startBorderTexture = std::make_unique<Texture>(TFN_GRF_CURSOR, TFN_FALLBACK_GRF_CURSOR);
	_dialogBoxTexture = std::make_unique<Texture>(TFN_GRF_DIALOGBOX, TFN_FALLBACK_GRF_DIALOGBOX);

	applyUserPaletteToAllGrfTextures();

	loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH());
	loadSettingsImage(*_settingsIconTexture);

	loadBoxfullImage(*_boxFullTexture);
	loadBoxemptyImage(*_boxEmptyTexture);
	loadFolderImage(*_folderTexture);

	loadSmallCartImage(*_smallCartTexture);
	loadStartbrdImage(*_startBorderTexture, tc().startBorderSpriteH());
	loadDialogboxImage(*_dialogBoxTexture);
	loadProgressImage(*_progressTexture);
	loadWirelessIcons(*_wirelessIconsTexture);
}

void ThemeTextures::loadDSiTheme() {	
	logPrint("tex().loadDSiTheme()\n");

	//iprintf("loadBackgrounds()\n");
	loadBackgrounds();
	//iprintf("loadUITextures()\n");
	loadUITextures();

	//iprintf("loadVolumeTextures()\n");
	loadVolumeTextures();
	//iprintf("loadBatteryTextures()\n");
	loadBatteryTextures();

	_bipsTexture = std::make_unique<Texture>(TFN_GRF_BIPS, TFN_FALLBACK_GRF_BIPS);
	_boxTexture = std::make_unique<Texture>(TFN_GRF_BOX, TFN_FALLBACK_GRF_BOX);
	_braceTexture = std::make_unique<Texture>(TFN_GRF_BRACE, TFN_FALLBACK_GRF_BRACE);
	_bubbleTexture = std::make_unique<Texture>(TFN_GRF_BUBBLE, TFN_FALLBACK_GRF_BUBBLE);
	_buttonArrowTexture = std::make_unique<Texture>(TFN_GRF_BUTTON_ARROW, TFN_FALLBACK_GRF_BUTTON_ARROW);
	_cornerButtonTexture = std::make_unique<Texture>(TFN_GRF_CORNERBUTTON, TFN_FALLBACK_GRF_CORNERBUTTON);

	_dialogBoxTexture = std::make_unique<Texture>(TFN_GRF_DIALOGBOX, TFN_FALLBACK_GRF_DIALOGBOX);

	_folderTexture = std::make_unique<Texture>(TFN_GRF_FOLDER, TFN_FALLBACK_GRF_FOLDER);
	_launchDotTexture = std::make_unique<Texture>(TFN_GRF_LAUNCH_DOT, TFN_FALLBACK_GRF_LAUNCH_DOT);
	_movingArrowTexture = std::make_unique<Texture>(TFN_GRF_MOVING_ARROW, TFN_FALLBACK_GRF_MOVING_ARROW);

	_progressTexture = std::make_unique<Texture>(TFN_GRF_PROGRESS, TFN_FALLBACK_GRF_PROGRESS);
	_scrollWindowTexture = std::make_unique<Texture>(TFN_GRF_SCROLL_WINDOW, TFN_FALLBACK_GRF_SCROLL_WINDOW);
	_smallCartTexture = std::make_unique<Texture>(TFN_GRF_SMALL_CART, TFN_FALLBACK_GRF_SMALL_CART);
	_startBorderTexture = std::make_unique<Texture>(TFN_GRF_START_BORDER, TFN_FALLBACK_GRF_START_BORDER);
	_startTextTexture = std::make_unique<Texture>(TFN_GRF_START_TEXT, TFN_FALLBACK_GRF_START_TEXT);
	_wirelessIconsTexture = std::make_unique<Texture>(TFN_GRF_WIRELESSICONS, TFN_FALLBACK_GRF_WIRELESSICONS);
	_settingsIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_SETTINGS, TFN_FALLBACK_GRF_ICON_SETTINGS);
	_manualIconTexture = std::make_unique<Texture>(TFN_GRF_ICON_MANUAL, TFN_FALLBACK_GRF_ICON_MANUAL);

	// Apply the DSi palette shifts
	applyUserPaletteToAllGrfTextures();

	//iprintf("loadBipsImage(*_bipsTexture)\n");
	loadBipsImage(*_bipsTexture);

	//iprintf("loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH())\n");
	loadBubbleImage(*_bubbleTexture, tc().bubbleTipSpriteW(), tc().bubbleTipSpriteH());
	//iprintf("loadScrollwindowImage(*_scrollWindowTexture)\n");
	loadScrollwindowImage(*_scrollWindowTexture);
	//iprintf("loadSettingsImage(*_settingsIconTexture)\n");
	loadSettingsImage(*_settingsIconTexture);
	//iprintf("loadBraceImage(*_braceTexture)\n");
	loadBraceImage(*_braceTexture);

	//iprintf("loadStartImage(*_startTextTexture)\n");
	loadStartImage(*_startTextTexture);
	//iprintf("loadStartbrdImage(*_startBorderTexture, tc().startBorderSpriteH())\n");
	loadStartbrdImage(*_startBorderTexture, tc().startBorderSpriteH());

	//iprintf("loadButtonarrowImage(*_buttonArrowTexture)\n");
	loadButtonarrowImage(*_buttonArrowTexture);
	//iprintf("loadMovingarrowImage(*_movingArrowTexture)\n");
	loadMovingarrowImage(*_movingArrowTexture);
	//iprintf("loadLaunchdotImage(*_launchDotTexture)\n");
	loadLaunchdotImage(*_launchDotTexture);
	//iprintf("loadDialogboxImage(*_dialogBoxTexture)\n");
	loadDialogboxImage(*_dialogBoxTexture);

	// careful here, it's boxTexture, not boxFulltexture.
	//iprintf("loadBoxfullImage(*_boxTexture)\n");
	loadBoxfullImage(*_boxTexture);

	//iprintf("loadManualImage(*_manualIconTexture)\n");
	loadManualImage(*_manualIconTexture);
	//iprintf("loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32)\n");
	loadCornerButtonImage(*_cornerButtonTexture, (32 / 16) * (32 / 32), 32, 32);
	//iprintf("loadSmallCartImage(*_smallCartTexture)\n");
	loadSmallCartImage(*_smallCartTexture);
	//iprintf("loadFolderImage(*_folderTexture)\n");
	loadFolderImage(*_folderTexture);

	//iprintf("loadProgressImage(*_progressTexture)\n");
	loadProgressImage(*_progressTexture);
	//iprintf("loadWirelessIcons(*_wirelessIconsTexture)\n");
	loadWirelessIcons(*_wirelessIconsTexture);
}

void ThemeTextures::loadVolumeTextures() {
	if (dsiFeatures() && !sys().i2cBricked()) {
		_volume0Texture = std::make_unique<Texture>(TFN_VOLUME0, TFN_FALLBACK_VOLUME0);
		_volume1Texture = std::make_unique<Texture>(TFN_VOLUME1, TFN_FALLBACK_VOLUME1);
		_volume2Texture = std::make_unique<Texture>(TFN_VOLUME2, TFN_FALLBACK_VOLUME2);
		_volume3Texture = std::make_unique<Texture>(TFN_VOLUME3, TFN_FALLBACK_VOLUME3);
		_volume4Texture = std::make_unique<Texture>(TFN_VOLUME4, TFN_FALLBACK_VOLUME4);
	}
}

void ThemeTextures::loadBatteryTextures() {
	if (dsiFeatures() && !sys().i2cBricked()) {
		_batterychargeTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE, TFN_FALLBACK_BATTERY_CHARGE);
		_batterychargeblinkTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE_BLINK, TFN_FALLBACK_BATTERY_CHARGE_BLINK);
		_battery0Texture = std::make_unique<Texture>(TFN_BATTERY0, TFN_FALLBACK_BATTERY0);
		if (ms().consoleModel < 2 && ms().powerLedColor && tc().purpleBatteryAvailable()) {
			_battery1Texture = std::make_unique<Texture>(TFN_BATTERY1_PURPLE, TFN_FALLBACK_BATTERY1_PURPLE);
			_battery2Texture = std::make_unique<Texture>(TFN_BATTERY2_PURPLE, TFN_FALLBACK_BATTERY2_PURPLE);
			_battery3Texture = std::make_unique<Texture>(TFN_BATTERY3_PURPLE, TFN_FALLBACK_BATTERY3_PURPLE);
			_battery4Texture = std::make_unique<Texture>(TFN_BATTERY4_PURPLE, TFN_FALLBACK_BATTERY4_PURPLE);
		} else {
			_battery1Texture = std::make_unique<Texture>(TFN_BATTERY1, TFN_FALLBACK_BATTERY1);
			_battery2Texture = std::make_unique<Texture>(TFN_BATTERY2, TFN_FALLBACK_BATTERY2);
			_battery3Texture = std::make_unique<Texture>(TFN_BATTERY3, TFN_FALLBACK_BATTERY3);
			_battery4Texture = std::make_unique<Texture>(TFN_BATTERY4, TFN_FALLBACK_BATTERY4);
		}
	} else {
		if (sys().hasRegulableBacklight()) {
			_batterychargeTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE, TFN_FALLBACK_BATTERY_CHARGE);
			_batterychargeblinkTexture = std::make_unique<Texture>(TFN_BATTERY_CHARGE_BLINK, TFN_FALLBACK_BATTERY_CHARGE_BLINK);
		}
		_batteryfullTexture = std::make_unique<Texture>(TFN_BATTERY_FULL, TFN_FALLBACK_BATTERY_FULL);
		_batteryfullDSTexture = std::make_unique<Texture>(TFN_BATTERY_FULLDS, TFN_FALLBACK_BATTERY_FULLDS);
		_batterylowTexture = std::make_unique<Texture>(TFN_BATTERY_LOW, TFN_FALLBACK_BATTERY_LOW);
	}
}

void ThemeTextures::loadUITextures() {
	_dateTimeFont = std::make_unique<FontGraphic>(((access((TFN_FONT_DATE_TIME).c_str(), F_OK) == 0) ? TFN_FONT_DATE_TIME : TFN_FALLBACK_FONT_DATE_TIME).c_str(), false);
	if (access((TFN_FONT_USERNAME).c_str(), F_OK) == 0) {
		_usernameFont = std::make_unique<FontGraphic>((TFN_FONT_USERNAME).c_str(), false);
	}

	if (ms().theme != TWLSettings::EThemeHBL) {
		if (ms().showPhoto && tc().renderPhoto()) {
			_leftShoulderTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER_PHOTO, TFN_UI_LSHOULDER, TFN_FALLBACK_UI_LSHOULDER);
			_rightShoulderTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER_PHOTO, TFN_UI_RSHOULDER, TFN_FALLBACK_UI_RSHOULDER);
			_leftShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER_PHOTO_GREYED, TFN_UI_LSHOULDER_GREYED, TFN_FALLBACK_UI_LSHOULDER_GREYED);
			_rightShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER_PHOTO_GREYED, TFN_UI_RSHOULDER_GREYED, TFN_FALLBACK_UI_RSHOULDER_GREYED);
		} else {
			_leftShoulderTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER, TFN_FALLBACK_UI_LSHOULDER);
			_rightShoulderTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER, TFN_FALLBACK_UI_RSHOULDER);
			_leftShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_LSHOULDER_GREYED, TFN_FALLBACK_UI_LSHOULDER_GREYED);
			_rightShoulderGreyedTexture = std::make_unique<Texture>(TFN_UI_RSHOULDER_GREYED, TFN_FALLBACK_UI_RSHOULDER_GREYED);
		}
	}
}

void ThemeTextures::loadIconGBTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconGBTexture = std::make_unique<Texture>(TFN_GRF_ICON_GB, TFN_FALLBACK_GRF_ICON_GB);
	if (_iconGBTexture && tc().iconGBUserPalette()) {
		_iconGBTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GB, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconGBTexture\n");
}
void ThemeTextures::loadIconGBATexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconGBATexture = std::make_unique<Texture>(TFN_GRF_ICON_GBA, TFN_FALLBACK_GRF_ICON_GBA);
	if (_iconGBATexture && tc().iconGBAUserPalette()) {
		_iconGBATexture->applyUserPaletteFile(TFN_PALETTE_ICON_GBA, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconGBATexture\n");
	/* _iconGBAModeTexture = std::make_unique<Texture>(TFN_GRF_ICON_GBAMODE, TFN_FALLBACK_GRF_ICON_GBAMODE);
	if (_iconGBAModeTexture && tc().iconGBAModeUserPalette()) {
		_iconGBAModeTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GBAMODE, effectDSiArrowButtonPalettes);
	} */
}
void ThemeTextures::loadIconGGTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconGGTexture = std::make_unique<Texture>(TFN_GRF_ICON_GG, TFN_FALLBACK_GRF_ICON_GG);
	if (_iconGGTexture && tc().iconGGUserPalette()) {
		_iconGGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_GG, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconGGTexture\n");
}
void ThemeTextures::loadIconMDTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconMDTexture = std::make_unique<Texture>(TFN_GRF_ICON_MD, TFN_FALLBACK_GRF_ICON_MD);
	if (_iconMDTexture && tc().iconMDUserPalette()) {
		_iconMDTexture->applyUserPaletteFile(TFN_PALETTE_ICON_MD, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconMDTexture\n");
}
void ThemeTextures::loadIconNESTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconNESTexture = std::make_unique<Texture>(TFN_GRF_ICON_NES, TFN_FALLBACK_GRF_ICON_NES);
	if (_iconNESTexture && tc().iconNESUserPalette()) {
		_iconNESTexture->applyUserPaletteFile(TFN_PALETTE_ICON_NES, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconNESTexture\n");
}
void ThemeTextures::loadIconSGTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconSGTexture = std::make_unique<Texture>(TFN_GRF_ICON_SG, TFN_FALLBACK_GRF_ICON_SG);
	if (_iconSGTexture && tc().iconSGUserPalette()) {
		_iconSGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SG, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconSGTexture\n");
}
void ThemeTextures::loadIconSMSTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconSMSTexture = std::make_unique<Texture>(TFN_GRF_ICON_SMS, TFN_FALLBACK_GRF_ICON_SMS);
	if (_iconSMSTexture && tc().iconSMSUserPalette()) {
		_iconSMSTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SMS, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconSMSTexture\n");
}
void ThemeTextures::loadIconSNESTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconSNESTexture = std::make_unique<Texture>(TFN_GRF_ICON_SNES, TFN_FALLBACK_GRF_ICON_SNES);
	if (_iconSNESTexture && tc().iconSNESUserPalette()) {
		_iconSNESTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SNES, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconSNESTexture\n");
}
void ThemeTextures::loadIconPLGTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconPLGTexture = std::make_unique<Texture>(TFN_GRF_ICON_PLG, TFN_FALLBACK_GRF_ICON_PLG);
	if (_iconPLGTexture && tc().iconPLGUserPalette()) {
		_iconPLGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_PLG, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconPLGTexture\n");
}
void ThemeTextures::loadIconA26Texture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconA26Texture = std::make_unique<Texture>(TFN_GRF_ICON_A26, TFN_FALLBACK_GRF_ICON_A26);
	if (_iconA26Texture && tc().iconA26UserPalette()) {
		_iconA26Texture->applyUserPaletteFile(TFN_PALETTE_ICON_A26, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconA26Texture\n");
}
void ThemeTextures::loadIconCOLTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconCOLTexture = std::make_unique<Texture>(TFN_GRF_ICON_COL, TFN_FALLBACK_GRF_ICON_COL);
	if (_iconCOLTexture && tc().iconCOLUserPalette()) {
		_iconCOLTexture->applyUserPaletteFile(TFN_PALETTE_ICON_COL, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconCOLTexture\n");
}
void ThemeTextures::loadIconM5Texture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconM5Texture = std::make_unique<Texture>(TFN_GRF_ICON_M5, TFN_FALLBACK_GRF_ICON_M5);
	if (_iconM5Texture && tc().iconM5UserPalette()) {
		_iconM5Texture->applyUserPaletteFile(TFN_PALETTE_ICON_M5, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconM5Texture\n");
}
void ThemeTextures::loadIconINTTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconINTTexture = std::make_unique<Texture>(TFN_GRF_ICON_INT, TFN_FALLBACK_GRF_ICON_INT);
	if (_iconINTTexture && tc().iconINTUserPalette()) {
		_iconINTTexture->applyUserPaletteFile(TFN_PALETTE_ICON_INT, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconINTTexture\n");
}
void ThemeTextures::loadIconPCETexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconPCETexture = std::make_unique<Texture>(TFN_GRF_ICON_PCE, TFN_FALLBACK_GRF_ICON_PCE);
	if (_iconPCETexture && tc().iconPCEUserPalette()) {
		_iconPCETexture->applyUserPaletteFile(TFN_PALETTE_ICON_PCE, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconPCETexture\n");
}
void ThemeTextures::loadIconWSTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconWSTexture = std::make_unique<Texture>(TFN_GRF_ICON_WS, TFN_FALLBACK_GRF_ICON_WS);
	if (_iconWSTexture && tc().iconWSUserPalette()) {
		_iconWSTexture->applyUserPaletteFile(TFN_PALETTE_ICON_WS, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconWSTexture\n");
}
void ThemeTextures::loadIconNGPTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconNGPTexture = std::make_unique<Texture>(TFN_GRF_ICON_NGP, TFN_FALLBACK_GRF_ICON_NGP);
	if (_iconNGPTexture && tc().iconNGPUserPalette()) {
		_iconNGPTexture->applyUserPaletteFile(TFN_PALETTE_ICON_NGP, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconNGPTexture\n");
}
void ThemeTextures::loadIconCPCTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconCPCTexture = std::make_unique<Texture>(TFN_GRF_ICON_CPC, TFN_FALLBACK_GRF_ICON_CPC);
	if (_iconCPCTexture && tc().iconCPCUserPalette()) {
		_iconCPCTexture->applyUserPaletteFile(TFN_PALETTE_ICON_CPC, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconCPCTexture\n");
}
void ThemeTextures::loadIconVIDTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconVIDTexture = std::make_unique<Texture>(TFN_GRF_ICON_VID, TFN_FALLBACK_GRF_ICON_VID);
	if (_iconVIDTexture && tc().iconVIDUserPalette()) {
		_iconVIDTexture->applyUserPaletteFile(TFN_PALETTE_ICON_VID, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconVIDTexture\n");
}
void ThemeTextures::loadIconIMGTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconIMGTexture = std::make_unique<Texture>(TFN_GRF_ICON_IMG, TFN_FALLBACK_GRF_ICON_IMG);
	if (_iconIMGTexture && tc().iconIMGUserPalette()) {
		_iconIMGTexture->applyUserPaletteFile(TFN_PALETTE_ICON_IMG, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconIMGTexture\n");
}
void ThemeTextures::loadIconMSXTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconMSXTexture = std::make_unique<Texture>(TFN_GRF_ICON_MSX, TFN_FALLBACK_GRF_ICON_MSX);
	if (_iconMSXTexture && tc().iconMSXUserPalette()) {
		_iconMSXTexture->applyUserPaletteFile(TFN_PALETTE_ICON_MSX, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconMSXTexture\n");
}
void ThemeTextures::loadIconMINITexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconMINITexture = std::make_unique<Texture>(TFN_GRF_ICON_MINI, TFN_FALLBACK_GRF_ICON_MINI);
	if (_iconMINITexture && tc().iconMINIUserPalette()) {
		_iconMINITexture->applyUserPaletteFile(TFN_PALETTE_ICON_MINI, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconMINITexture\n");
}
void ThemeTextures::loadIconHBTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconHBTexture = std::make_unique<Texture>(TFN_GRF_ICON_HB, TFN_FALLBACK_GRF_ICON_HB);
	if (_iconHBTexture && tc().iconHBUserPalette()) {
		_iconHBTexture->applyUserPaletteFile(TFN_PALETTE_ICON_HB, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconHBTexture\n");
}
void ThemeTextures::loadIconUnknownTexture() {
	static bool loaded = false;
	if (loaded) return;
	loaded = true;

	_iconUnknownTexture = std::make_unique<Texture>(TFN_GRF_ICON_UNK, TFN_FALLBACK_GRF_ICON_UNK);
	if (_iconUnknownTexture && tc().iconUnknownUserPalette()) {
		_iconUnknownTexture->applyUserPaletteFile(TFN_PALETTE_ICON_UNK, effectDSiArrowButtonPalettes);
	}
	logPrint("Loaded iconUnknownTexture\n");
}
u16 *ThemeTextures::beginBgSubModify() {
	if (ms().macroMode)
		return _bgSubBuffer;

	u16* bgLoc = BG_GFX_SUB;
	if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}
	dmaCopyWords(0, bgLoc, _bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		dmaCopyWords(0, _frameBufferBot[1], _bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	return _bgSubBuffer;
}

void ThemeTextures::commitBgSubModify() {
	if (ms().macroMode)
		return;

	u16* bgLoc = BG_GFX_SUB;
	if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}
	DC_FlushRange(_bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		DC_FlushRange(_bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	while (REG_VCOUNT != 191); // Fix screen tearing
	dmaCopyWords(2, _bgSubBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		dmaCopyWords(2, _bgSubBuffer2, _frameBufferBot[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
}

void ThemeTextures::commitBgSubModifyAsync() {
	if (ms().macroMode)
		return;

	u16* bgLoc = BG_GFX_SUB;
	if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}
	DC_FlushRange(_bgSubBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband && ndmaEnabled()) {
		DC_FlushRange(_bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}
	while (REG_VCOUNT != 191); // Fix screen tearing
	dmaCopyWordsAsynch(2, _bgSubBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	if (boxArtColorDeband) {
		if (ndmaEnabled()) {
			ndmaCopyWordsAsynch(2, _bgSubBuffer2, _frameBufferBot[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
		} else {
			tonccpy(_frameBufferBot[1], _bgSubBuffer2, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
		}
	}
}

u16 *ThemeTextures::beginBgMainModify() {
	u16* bgLoc = BG_GFX;
	/*if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}*/
	dmaCopyWords(0, bgLoc, _bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	/*if (ndmaEnabled()) {
		dmaCopyWords(0, _frameBuffer[1], _bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}*/
	return _bgMainBuffer;
}

void ThemeTextures::commitBgMainModify() {
	u16* bgLoc = BG_GFX;
	/*if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}*/
	DC_FlushRange(_bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWords(2, _bgMainBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	/*if (ndmaEnabled()) {
		dmaCopyWords(2, _bgMainBuffer, _frameBuffer[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}*/
}

void ThemeTextures::commitBgMainModifyAsync() {
	u16* bgLoc = BG_GFX;
	/*if (boxArtColorDeband) {
		bgLoc = _frameBufferBot[0];
	}*/
	DC_FlushRange(_bgMainBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	dmaCopyWordsAsynch(2, _bgMainBuffer, bgLoc, sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	/*if (boxArtColorDeband) {
		ndmaCopyWordsAsynch(2, _bgMainBuffer, _frameBuffer[1], sizeof(u16) * BG_BUFFER_PIXELCOUNT);
	}*/
}

void ThemeTextures::drawTopBg() {
	beginBgSubModify();

	// Menu background (quickmenu/topbg.png) on the top screen (sub engine).
	if (!_menuBgLoaded)
		loadMenuBg();
	tonccpy(_bgSubBuffer, _menuBgBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);

	if (boxArtColorDeband) {
		tonccpy((u8*)_bgSubBuffer2, (u8*)_bgSubBuffer, 0x18000);
	}
	commitBgSubModify();
}

void ThemeTextures::drawBottomBg(int index) {

	// clamp index
	if (index < 1)
		index = 1;
	if (index > 3)
		index = 3;
	if (index > 2 && ms().theme == TWLSettings::ETheme3DS)
		index = 2;
	beginBgMainModify();

	// Menu background (quickmenu/topbg.png) on the bottom screen (main engine).
	(void)index;
	if (!_menuBgLoaded)
		loadMenuBg();
	tonccpy(_bgMainBuffer, _menuBgBuffer, sizeof(u16) * BG_BUFFER_PIXELCOUNT);

	commitBgMainModify();
}

void ThemeTextures::clearTopScreen() {
	beginBgSubModify();
	const u16 val = colorTable ? (colorTable[0x7FFF] | BIT(15)) : 0xFFFF;
	for (int i = 0; i < BG_BUFFER_PIXELCOUNT; i++) {
		_bgSubBuffer[i] = val;
		if (boxArtColorDeband) {
			_bgSubBuffer2[i] = val;
		}
	}
	commitBgSubModify();
}

void ThemeTextures::drawProfileName() {
	if (ms().theme == TWLSettings::EThemeDSi) return; // fork: no theme chrome (icons + top title only)
	if (_profileNameLoaded || ms().theme == TWLSettings::EThemeSaturn || ms().theme == TWLSettings::EThemeHBL) return;

	if (!topBorderBufferLoaded) {
		_backgroundTextures[ms().macroMode].copy(_topBorderBuffer, false);
		topBorderBufferLoaded = true;
	}

	// Load username
	int xPos = ((dsiFeatures() && !sys().i2cBricked()) ? tc().usernameRenderX() : tc().usernameRenderXDS());
	int yPos = tc().usernameRenderY();
	char16_t username[11] = {0};
	tonccpy(username, useTwlCfg ? (s16 *)0x02000448 : PersonalData->name, 10 * sizeof(char16_t));

	toncset16(FontGraphic::textBuf[1], 0, 256 * usernameFont()->height());
	usernameFont()->print(0, 0, true, username, Alignment::left, FontPalette::name);
	int width = usernameFont()->calcWidth(username);

	// Copy to background
	for (int y = 0; y < usernameFont()->height() && yPos + y < SCREEN_HEIGHT; y++) {
		if (yPos + y < 0) continue;
		for (int x = 0; x < width && xPos + x < SCREEN_WIDTH; x++) {
			if (xPos + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(yPos + y) * 256 + (xPos + x)];
			u16 val = 0;
			if (tc().usernameEdgeAlpha()) {
				val = px ? themealphablend(BG_PALETTE_SUB[px], bg, (px % 4) < 2 ? 128 : 224) : bg;
			} else {
				val = px ? (BG_PALETTE_SUB[px] | BIT(15)) : bg;
			}

			if (ms().macroMode) {
				_bgMainBuffer[(yPos + y) * 256 + (xPos + x)] = val;
			} else {
				_bgSubBuffer[(yPos + y) * 256 + (xPos + x)] = val;
				if (boxArtColorDeband) {
					_bgSubBuffer2[(yPos + y) * 256 + (xPos + x)] = val;
				}
			}
		}
	}

	ms().macroMode ? commitBgMainModify() : commitBgSubModify();
	_profileNameLoaded = true;
}


ITCM_CODE void ThemeTextures::resetProfileName() {
	_profileNameLoaded = false;
}

void ThemeTextures::loadBoxArtToMem(const char *filename, int num) {
	if (num < 0 || num > 39) {
		return;
	}

	extern off_t getFileSize(const char *fileName);
	off_t filesize = getFileSize(filename);

	if (filesize == 0 || filesize > 0xB000) {
		boxArtFound[num] = false;
		//filename = "nitro:/graphics/boxart_unknown.bmp";
		//file = fopen(filename, "rb");
		return;
	}

	boxArtFound[num] = true;

	FILE *file = fopen(filename, "rb");
	fread(boxArtCache+(num*0xB000), 1, 0xB000, file);
	fclose(file);
}

bool ThemeTextures::drawBoxArt(const char *filename, bool inMem) {
	if (inMem ? !boxArtFound[CURPOS] : access(filename, F_OK) != 0) return false;

	std::vector<unsigned char> image;
	uint imageXpos, imageYpos;
	if (inMem) {
		lodepng::decode(image, boxArtWidth, boxArtHeight, (unsigned char*)boxArtCache+(CURPOS*0xB000), 0xB000);
	} else {
		lodepng::decode(image, boxArtWidth, boxArtHeight, filename);
	}
	bool alternatePixel = false;
	if (boxArtWidth > 256 || boxArtHeight > 192) return false;

	if (ms().theme == TWLSettings::ETheme3DS && rocketVideo_playVideo) {
		rocketVideo_playVideo = false;
		while (dmaBusy(1)); // Wait for frame to finish rendering
		drawOverRotatingCubes(); // Clear top screen cubes for 3DS theme
	}

	if (ms().theme == TWLSettings::ETheme3DS) {
		extern uint photoWidth, photoHeight;
		tex().drawOverBoxArt(photoWidth, photoHeight);
	}

	beginBgSubModify();

	u16* bmpImageBuffer = new u16[256 * 192];
	u16* bmpImageBuffer2 = boxArtColorDeband ? new u16[256 * 192] : NULL;

	imageXpos = (256-boxArtWidth)/2;
	imageYpos = (192-boxArtHeight)/2;

	int photoXstart = imageXpos;
	int photoXend = imageXpos+boxArtWidth;
	int photoX = photoXstart;
	int photoY = imageYpos;

	for (uint i=0;i<image.size()/4;i++) {
		u8 pixelAdjustInfo = 0;
		if (boxArtColorDeband) {
			if (alternatePixel) {
				if (image[(i*4)] >= 0x4 && image[(i*4)] < 0xFC) {
					image[(i*4)] += 0x4;
					pixelAdjustInfo |= BIT(0);
				}
				if (image[(i*4)+1] >= 0x4 && image[(i*4)+1] < 0xFC) {
					image[(i*4)+1] += 0x4;
					pixelAdjustInfo |= BIT(1);
				}
				if (image[(i*4)+2] >= 0x4 && image[(i*4)+2] < 0xFC) {
					image[(i*4)+2] += 0x4;
					pixelAdjustInfo |= BIT(2);
				}
				if (image[(i*4)+3] >= 0x4 && image[(i*4)+3] < 0xFC) {
					image[(i*4)+3] += 0x4;
					pixelAdjustInfo |= BIT(3);
				}
			}
		}
		u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (colorTable) {
			color = colorTable[color % 0x8000] | BIT(15);
		}
		if (image[(i*4)+3] == 255) {
			bmpImageBuffer[i] = color;
		} else {
			bmpImageBuffer[i] = alphablend(color, _bgSubBuffer[(photoY*256)+photoX], image[(i*4)+3]);
		}
		if (boxArtColorDeband) {
			if (alternatePixel) {
				if (pixelAdjustInfo & BIT(0)) {
					image[(i*4)] -= 0x4;
				}
				if (pixelAdjustInfo & BIT(1)) {
					image[(i*4)+1] -= 0x4;
				}
				if (pixelAdjustInfo & BIT(2)) {
					image[(i*4)+2] -= 0x4;
				}
				if (pixelAdjustInfo & BIT(3)) {
					image[(i*4)+3] -= 0x4;
				}
			} else {
				if (image[(i*4)] >= 0x4 && image[(i*4)] < 0xFC) {
					image[(i*4)] += 0x4;
				}
				if (image[(i*4)+1] >= 0x4 && image[(i*4)+1] < 0xFC) {
					image[(i*4)+1] += 0x4;
				}
				if (image[(i*4)+2] >= 0x4 && image[(i*4)+2] < 0xFC) {
					image[(i*4)+2] += 0x4;
				}
				if (image[(i*4)+3] >= 0x4 && image[(i*4)+3] < 0xFC) {
					image[(i*4)+3] += 0x4;
				}
			}
			color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
			if (colorTable) {
				color = colorTable[color % 0x8000] | BIT(15);
			}
			if (image[(i*4)+3] == 255) {
				bmpImageBuffer2[i] = color;
			} else {
				bmpImageBuffer2[i] = alphablend(color, _bgSubBuffer2[(photoY*256)+photoX], image[(i*4)+3]);
			}
			if ((i % boxArtWidth) == boxArtWidth-1) alternatePixel = !alternatePixel;
			alternatePixel = !alternatePixel;
		}
		photoX++;
		if (photoX == photoXend) {
			photoX = photoXstart;
			photoY++;
		}
	}

	u16 *src = bmpImageBuffer;
	u16 *src2 = bmpImageBuffer2;
	for (uint y = 0; y < boxArtHeight; y++) {
		for (uint x = 0; x < boxArtWidth; x++) {
			_bgSubBuffer[(y+imageYpos) * 256 + imageXpos + x] = *(src++);
			if (boxArtColorDeband) {
				_bgSubBuffer2[(y+imageYpos) * 256 + imageXpos + x] = *(src2++);
			}
		}
	}
	commitBgSubModify();

	delete[] bmpImageBuffer;
	if (boxArtColorDeband) {
		delete[] bmpImageBuffer2;
	}

	return true;
}

#define MAX_PHOTO_WIDTH 208
#define MAX_PHOTO_HEIGHT 156
#define PHOTO_OFFSET 24
// Redraw background and photo over the boxart bounds
void ThemeTextures::drawOverBoxArt(uint photoWidth, uint photoHeight) {
	if (boxArtWidth == 0 || boxArtHeight == 0) return;
	uint boxArtX = (SCREEN_WIDTH - boxArtWidth) / 2;
	uint boxArtY = (SCREEN_HEIGHT - boxArtHeight) / 2;

	beginBgSubModify();
	if (!ms().showPhoto || !tc().renderPhoto() || boxArtWidth > MAX_PHOTO_WIDTH || boxArtHeight > MAX_PHOTO_HEIGHT) {
		if (!topBorderBufferLoaded) {
			_backgroundTextures[0].copy(_topBorderBuffer, false);
			topBorderBufferLoaded = true;
		}
		for (uint y = 0; y < boxArtHeight; y++) {
			uint offset = boxArtX + (boxArtY + y) * SCREEN_WIDTH;
			tonccpy(_bgSubBuffer + offset, _topBorderBuffer + offset, sizeof(u16) * boxArtWidth);
			if (boxArtColorDeband) {
				tonccpy(_bgSubBuffer2 + offset, _topBorderBuffer + offset, sizeof(u16) * boxArtWidth);
			}
		}
	}
	
	if (ms().showPhoto && tc().renderPhoto()) {
		// fill black within boxart and photo bounds
		uint blackX = boxArtX > PHOTO_OFFSET ? boxArtX : PHOTO_OFFSET;
		uint blackY = boxArtY > PHOTO_OFFSET ? boxArtY : PHOTO_OFFSET;
		uint blackWidth = boxArtWidth < MAX_PHOTO_WIDTH ? boxArtWidth : MAX_PHOTO_WIDTH;
		uint blackHeight = boxArtHeight < MAX_PHOTO_HEIGHT ? boxArtHeight : MAX_PHOTO_HEIGHT;
		for (uint y = 0; y < blackHeight; y++) {
			uint offset = blackX + (blackY + y) * SCREEN_WIDTH;
			dmaFillHalfWords(0x8000, _bgSubBuffer + offset, sizeof(u16) * blackWidth);
			if (boxArtColorDeband) {
				dmaFillHalfWords(0x8000, _bgSubBuffer2 + offset, sizeof(u16) * blackWidth);
			}
		}
		// draw photo within boxart bounds
		uint photoX = PHOTO_OFFSET + (MAX_PHOTO_WIDTH - photoWidth) / 2;
		uint photoY = PHOTO_OFFSET + (MAX_PHOTO_HEIGHT - photoHeight) / 2;
		uint xOffset = boxArtX > photoX ? boxArtX - photoX : 0;
		uint yOffset = boxArtY > photoY ? boxArtY - photoY : 0;
		uint copyWidth = boxArtWidth < photoWidth ? boxArtWidth : photoWidth;
		uint copyHeight = boxArtHeight < photoHeight ? boxArtHeight : photoHeight;
		for (uint y = 0; y < copyHeight; y++) {
			uint offset = photoX + xOffset + (photoY + yOffset + y) * SCREEN_WIDTH;
			tonccpy(_bgSubBuffer + offset, _photoBuffer + xOffset + (yOffset + y) * photoWidth, sizeof(u16) * copyWidth);
			if (boxArtColorDeband) {
				tonccpy(_bgSubBuffer2 + offset, _photoBuffer2 + xOffset + (yOffset + y) * photoWidth, sizeof(u16) * copyWidth);
			}
		}
	}
	commitBgSubModify();
	boxArtWidth = boxArtHeight = 0;
}

// Redraw background over the rotating cubes bounds
void ThemeTextures::drawOverRotatingCubes() {
	// if (!rotatingCubesLoaded) return;

	extern u8 rocketVideo_height;
	extern int rocketVideo_videoYpos;

	beginBgSubModify();
	for (uint y = 0; y < rocketVideo_height; y++) {
		uint offset = (rocketVideo_videoYpos + y) * SCREEN_WIDTH;
		tonccpy(_bgSubBuffer + offset, _topBorderBuffer + offset, sizeof(u16) * SCREEN_WIDTH);
	}
	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawVolumeImage(int volumeLevel) {
	if (ms().theme == TWLSettings::EThemeDSi) return; // fork: no theme chrome (icons + top title only)
	if (!dsiFeatures() || sys().i2cBricked())
		return;
	beginBgSubModify();

	const Texture *tex = volumeTexture(volumeLevel);
	const u16 *src = tex->texture();
	int startX = tc().volumeRenderX();
	int startY = tc().volumeRenderY();
	for (uint y = 0; y < tex->texHeight(); y++) {
		for (uint x = 0; x < tex->texWidth(); x++) {
			u16 val = *(src++);
			if (!(val & BIT(15))) // If transparent, restore background image
					val = _topBorderBuffer[(startY + y) * 256 + startX + x];

			_bgSubBuffer[(startY + y) * 256 + startX + x] = val;
			if (boxArtColorDeband) {
				_bgSubBuffer2[(startY + y) * 256 + startX + x] = val;
			}
		}
	}
	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawVolumeImageMacro(int volumeLevel) {
	if (!dsiFeatures() || sys().i2cBricked())
		return;
	beginBgMainModify();

	const Texture *tex = volumeTexture(volumeLevel);
	const u16 *src = tex->texture();
	int startX = tc().volumeRenderX();
	int startY = tc().volumeRenderY();
	for (uint y = 0; y < tex->texHeight(); y++) {
		for (uint x = 0; x < tex->texWidth(); x++) {
			u16 val = *(src++);
			if (!(val & BIT(15))) // If transparent, restore background image
					val = _topBorderBuffer[(startY + y) * 256 + startX + x];

			_bgMainBuffer[(startY + y) * 256 + startX + x] = val;
		}
	}
	commitBgMainModify();
}

ITCM_CODE void ThemeTextures::drawVolumeImageCached() {
	if (ms().theme == TWLSettings::EThemeDSi) return; // fork: no theme chrome (icons + top title only)
	if (ms().macroMode && ms().theme == TWLSettings::EThemeSaturn) return;

	int volumeLevel = getVolumeLevel();
	if (_cachedVolumeLevel != volumeLevel) {
		_cachedVolumeLevel = volumeLevel;
		if (!topBorderBufferLoaded) {
			_backgroundTextures[ms().macroMode].copy(_topBorderBuffer, false);
			topBorderBufferLoaded = true;
		}
		ms().macroMode ? drawVolumeImageMacro(volumeLevel) : drawVolumeImage(volumeLevel);
	}
}

ITCM_CODE void ThemeTextures::resetCachedVolumeLevel() {
	_cachedVolumeLevel = -1;
}

ITCM_CODE int ThemeTextures::getVolumeLevel(void) {
	if (!dsiFeatures() || sys().i2cBricked())
		return -1;
	
	u8 volumeLevel = sys().volumeStatus();
	if (volumeLevel == 0)
		return 0;
	if (volumeLevel > 0x00 && volumeLevel < 0x07)
		return 1;
	if (volumeLevel >= 0x07 && volumeLevel < 0x11)
		return 2;
	if (volumeLevel >= 0x11 && volumeLevel < 0x1C)
		return 3;
	if (volumeLevel >= 0x1C && volumeLevel < 0x20)
		return 4;
	return -1;
}

ITCM_CODE int ThemeTextures::getBatteryLevel(void) {
	const u8 batteryLevel = sys().batteryStatus();
	if (batteryLevel & BIT(7))
		return 7;
	if (batteryLevel == 0xF)
		return 4;
	if (batteryLevel == 0xB)
		return 3;
	if (batteryLevel == 0x7)
		return 2;
	if (batteryLevel == 0x3 || batteryLevel == 0x1)
		return 1;
	return 0;
}

ITCM_CODE void ThemeTextures::drawBatteryImage(int batteryLevel, bool drawDSiMode, bool isRegularDS) {
	if (ms().theme == TWLSettings::EThemeDSi) return; // fork: no theme chrome (icons + top title only)
	// Start loading
	beginBgSubModify();
	const Texture *tex = batteryTexture(batteryLevel, drawDSiMode, isRegularDS);
	const u16 *src = tex->texture();
	for (uint y = tc().batteryRenderY(); y < tc().batteryRenderY() + tex->texHeight(); y++) {
		for (uint x = tc().batteryRenderX(); x < tc().batteryRenderX() + tex->texWidth(); x++) {
			u16 val = *(src++);
			if (!(val & BIT(15))) // If transparent, restore background image
				val = _topBorderBuffer[y * 256 + x];

			_bgSubBuffer[y * 256 + x] = val;
			if (boxArtColorDeband) {
				_bgSubBuffer2[y * 256 + x] = val;
			}
		}
	}
	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawBatteryImageMacro(int batteryLevel, bool drawDSiMode, bool isRegularDS) {
	// Start loading
	beginBgMainModify();
	const Texture *tex = batteryTexture(batteryLevel, drawDSiMode, isRegularDS);
	const u16 *src = tex->texture();
	for (uint y = tc().batteryRenderY(); y < tc().batteryRenderY() + tex->texHeight(); y++) {
		for (uint x = tc().batteryRenderX(); x < tc().batteryRenderX() + tex->texWidth(); x++) {
			u16 val = *(src++);
			if (!(val & BIT(15))) // If transparent, restore background image
					val = _topBorderBuffer[y * 256 + x];

			_bgMainBuffer[y * 256 + x] = val;
		}
	}
	commitBgMainModify();
}

ITCM_CODE void ThemeTextures::drawBatteryImageCached() {
	if (ms().theme == TWLSettings::EThemeDSi) return; // fork: no theme chrome (icons + top title only)
	if (ms().macroMode && ms().theme == TWLSettings::EThemeSaturn) return;

	int batteryLevel = getBatteryLevel();
	if (batteryLevel == 0 && showColon)	batteryLevel--;
	else if (batteryLevel == 7 && showColon)	batteryLevel++;
	if (_cachedBatteryLevel != batteryLevel) {
		_cachedBatteryLevel = batteryLevel;
		if (!topBorderBufferLoaded) {
			_backgroundTextures[ms().macroMode].copy(_topBorderBuffer, false);
			topBorderBufferLoaded = true;
		}
		ms().macroMode ? drawBatteryImageMacro(batteryLevel, dsiFeatures() && !sys().i2cBricked(), sys().isRegularDS()) : drawBatteryImage(batteryLevel, dsiFeatures() && !sys().i2cBricked(), sys().isRegularDS());
	}
}

ITCM_CODE void ThemeTextures::resetCachedBatteryLevel() {
	_cachedBatteryLevel = -1;
}

void ThemeTextures::drawShoulders(bool LShoulderActive, bool RShoulderActive) {
	if (ms().theme == TWLSettings::EThemeDSi) return; // fork: no theme chrome (icons + top title only)
	beginBgSubModify();

	const Texture *rightTex = RShoulderActive ? _rightShoulderTexture.get() : _rightShoulderGreyedTexture.get();
	const u16 *rightSrc = rightTex->texture();

	const Texture *leftTex = LShoulderActive ? _leftShoulderTexture.get() : _leftShoulderGreyedTexture.get();
	const u16 *leftSrc = leftTex->texture();

	// Draw R Shoulder
	for (uint y = tc().shoulderRRenderY(); y < tc().shoulderRRenderY() + rightTex->texHeight(); y++) {
		for (uint x = tc().shoulderRRenderX(); x < tc().shoulderRRenderX() + rightTex->texWidth(); x++) {
			u16 val = *(rightSrc++);
			if (val >> 15) { // Do not render transparent pixel
				_bgSubBuffer[y * 256 + x] = val;
				if (boxArtColorDeband) {
					_bgSubBuffer2[y * 256 + x] = val;
				}
			}
		}
	}
	toncset16(FontGraphic::textBuf[1], 0, SCREEN_WIDTH * smallFont()->height());
	smallFont()->print(0, 0, true, STR_NEXT, Alignment::left, RShoulderActive ? FontPalette::overlay : FontPalette::disabled);
	int width = smallFont()->calcWidth(STR_NEXT);
	// Copy text to background
	int align = tc().shoulderRTextAlign();
	int posX = tc().shoulderRTextX() - (align < 0 ? width : align == 0 ? width/2 : 0), posY = tc().shoulderRTextY();
	for (int y = 0; y < smallFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * SCREEN_WIDTH + x];
			u16 bg = _bgSubBuffer[(posY + y) * SCREEN_WIDTH + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			_bgSubBuffer[(posY + y) * SCREEN_WIDTH + (posX + x)] = val;
			if (boxArtColorDeband) {
				_bgSubBuffer2[(posY + y) * SCREEN_WIDTH + (posX + x)] = val;
			}
		}
	}

	// Draw L Shoulder
	for (uint y = tc().shoulderLRenderY(); y < tc().shoulderLRenderY() + leftTex->texHeight(); y++) {
		for (uint x = tc().shoulderLRenderX(); x < tc().shoulderLRenderX() + leftTex->texWidth(); x++) {
			u16 val = *(leftSrc++);
			if (val >> 15) { // Do not render transparent pixel
				_bgSubBuffer[y * 256 + x] = val;
				if (boxArtColorDeband) {
					_bgSubBuffer2[y * 256 + x] = val;
				}
			}
		}
	}
	toncset16(FontGraphic::textBuf[1], 0, SCREEN_WIDTH * smallFont()->height());
	smallFont()->print(0, 0, true, STR_PREV, Alignment::left, LShoulderActive ? FontPalette::overlay : FontPalette::disabled);
	width = smallFont()->calcWidth(STR_PREV);
	// Copy text to background
	align = tc().shoulderLTextAlign();
	posX = tc().shoulderLTextX() - (align < 0 ? width : align == 0 ? width/2 : 0), posY = tc().shoulderLTextY();
	for (int y = 0; y < smallFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * SCREEN_WIDTH + x];
			u16 bg = _bgSubBuffer[(posY + y) * SCREEN_WIDTH + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			_bgSubBuffer[(posY + y) * SCREEN_WIDTH + (posX + x)] = val;
			if (boxArtColorDeband) {
				_bgSubBuffer2[(posY + y) * SCREEN_WIDTH + (posX + x)] = val;
			}
		}
	}

	commitBgSubModify();
}

ITCM_CODE void ThemeTextures::drawDateTime(const char *str, int posX, int posY, bool isDate) {
	if (!topBorderBufferLoaded) {
		_backgroundTextures[0].copy(_topBorderBuffer, false);
		topBorderBufferLoaded = true;
	}

	toncset16(FontGraphic::textBuf[1], 0, 256 * dateTimeFont()->height());
	dateTimeFont()->print(0, 0, true, str, Alignment::left, FontPalette::dateTime);
	int width = std::max(dateTimeFont()->calcWidth(str), isDate ? _previousDateWidth : _previousTimeWidth);

	// Copy to background
	for (int y = 0; y < dateTimeFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(posY + y) * 256 + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			BG_GFX_SUB[(posY + y) * 256 + (posX + x)] = val;
			if (boxArtColorDeband) {
				_frameBufferBot[0][(posY + y) * 256 + (posX + x)] = val;
				_frameBufferBot[1][(posY + y) * 256 + (posX + x)] = val;
			}
		}
	}

	if (isDate) {
		_previousDateWidth = dateTimeFont()->calcWidth(str);
	} else {
		_previousTimeWidth = dateTimeFont()->calcWidth(str);
	}
}

// Renders the selected game's title/details (multi-line, centred, black) on the TOP screen,
// erasing the previous title. Used by our fork so the title lives on the top screen.
// Debug overlay on the top screen: FPS + texture VRAM usage (from the libnds allocator).
void ThemeTextures::drawTopTitle(std::u16string_view text) {
	FontGraphic *font = smallFont();
	if (!font) return;
	const int lineH = font->height();

	_topTitleText.assign(text.begin(), text.end()); // guarda p/ redraw quando o logo terminar de carregar

	if (!_titleboxLoaded)
		loadTitlebox();
	if (!_startboxLoaded)
		loadStartbox();

	// Anchor the box to the bottom of the top screen, centred horizontally; _boxSlide desloca
	// verticalmente durante a animação de subir/cair.
	const int margin = TOPBOX_MARGIN;
	const int sx = (SCREEN_WIDTH - _tbBoxW) / 2;
	const int sy = SCREEN_HEIGHT - _tbBoxH - margin + _boxSlide;

	// Compose off-screen so the live framebuffer is never seen half-drawn (titlebox flicker).
	// Restore the brick background first (clears the previous logo/title before redrawing).
	if (!_menuBgLoaded)
		loadMenuBg();
	if (!_menuBgDitherLoaded)
		loadMenuBgDither();
	u16 *dst = _topCompose;
	// Background composition. Three cases (video may be smaller than screen -> upscale via LUTs):
	//  - Idle (alpha == 255): brick sólido, sem vídeo.
	//  - Regime de reprodução (alpha no alvo) + asset dithered disponível: overlay BARATO — os
	//    pixels opacos do brick dithered vencem; os buracos mostram o vídeo. Zero blend por pixel.
	//  - Transição de fade (ou sem asset dithered): alphablend suave do brick sólido sobre o vídeo
	//    (dura poucos frames, então o custo é limitado).
	if (_videoBgAlpha < 255) {
		// Modo checker (dsiVideoFadeMode==0): usa o brick dithered (overlay barato) em regime.
		// Modo opacity (==1): sempre alphablend do brick a ~40% sobre o vídeo (sem dither).
		bool steadyDither = ms().dsiVideoFadeMode == 0 && _videoActive &&
		                    _videoBgAlpha == VIDEO_BG_ALPHA && _menuBgDitherHas;
		if (steadyDither) {
			for (int y = 0; y < SCREEN_HEIGHT; y++) {
				const u16 *vrow = _videoFrame + (int)_vidRowMap[y] * _videoW;
				const u16 *krow = _menuBgDither + y * SCREEN_WIDTH;
				u16 *drow = dst + y * SCREEN_WIDTH;
				for (int x = 0; x < SCREEN_WIDTH; x++) {
					u16 k = krow[x];
					drow[x] = k ? k : vrow[_vidColMap[x]]; // opaco = brick; buraco = vídeo
				}
			}
		} else {
			const u8 a = (u8)_videoBgAlpha;
			for (int y = 0; y < SCREEN_HEIGHT; y++) {
				const u16 *vrow = _videoFrame + (int)_vidRowMap[y] * _videoW;
				const u16 *brow = _menuBgBuffer + y * SCREEN_WIDTH;
				u16 *drow = dst + y * SCREEN_WIDTH;
				for (int x = 0; x < SCREEN_WIDTH; x++)
					drow[x] = alphablend(brow[x], vrow[_vidColMap[x]], a);
			}
		}
	} else {
		tonccpy(dst, _menuBgBuffer, sizeof(u16) * SCREEN_WIDTH * SCREEN_HEIGHT);
	}

	// Game logo centred on the top screen, drawn FIRST so the box stays in front (layer behind).
	// Scaled by _logoScale (zoom-in on appear / zoom-out on item change) via nearest-neighbor.
	if (_logoPresent && _logoScale > 0.01f) {
		int dw = (int)(_logoW * _logoScale);
		int dh = (int)(_logoH * _logoScale);
		if (dw < 1) dw = 1;
		if (dh < 1) dh = 1;
		int lx = (SCREEN_WIDTH - dw) / 2;
		int ly = (SCREEN_HEIGHT - dh) / 2;
		if (ly < 0) ly = 0;
		// Drop shadow: silhueta do logo deslocada (LOGO_SHADOW_DX/DY), preto alpha-blendado sobre
		// o fundo já composto. Desenhada ANTES do logo, então o logo fica por cima.
		for (int y = 0; y < dh; y++) {
			int sy = y * _logoH / dh;
			if (sy >= _logoH) sy = _logoH - 1;
			int py = ly + y + LOGO_SHADOW_DY;
			if (py < 0 || py >= SCREEN_HEIGHT) continue;
			for (int x = 0; x < dw; x++) {
				int sx = x * _logoW / dw;
				if (sx >= _logoW) sx = _logoW - 1;
				if (!_logoPix[sy * 256 + sx]) continue; // só onde o logo é opaco
				int px = lx + x + LOGO_SHADOW_DX;
				if ((unsigned)px >= SCREEN_WIDTH) continue;
				u16 &d = dst[py * SCREEN_WIDTH + px];
				d = alphablend(RGB15(0, 0, 0) | BIT(15), d, LOGO_SHADOW_ALPHA);
			}
		}
		// Logo por cima da sombra.
		for (int y = 0; y < dh; y++) {
			int sy = y * _logoH / dh;   // nearest-neighbor no eixo Y
			if (sy >= _logoH) sy = _logoH - 1;
			int py = ly + y;
			if ((unsigned)py >= SCREEN_HEIGHT) break;
			for (int x = 0; x < dw; x++) {
				int sx = x * _logoW / dw; // nearest-neighbor no eixo X
				if (sx >= _logoW) sx = _logoW - 1;
				u16 p = _logoPix[sy * 256 + sx];
				if (!p) continue;
				int px = lx + x;
				if ((unsigned)px < SCREEN_WIDTH)
					dst[py * SCREEN_WIDTH + px] = p;
			}
		}
	}

	if (_boxKind == 1 && _startboxHas) {
		// Enquanto o vídeo toca: a titlebox+texto dão lugar à start box (grf/topscreen_startbox.bmp),
		// ancorada no rodapé e centrada (+_boxSlide da animação). Sem texto de título.
		const int bx = (SCREEN_WIDTH - _sbBoxW) / 2;
		const int by = SCREEN_HEIGHT - _sbBoxH - margin + _boxSlide;
		for (int y = 0; y < _sbBoxH; y++) {
			int dy = by + y;
			if (dy < 0 || dy >= SCREEN_HEIGHT) continue; // clip ao sair pelo rodapé
			for (int x = 0; x < _sbBoxW; x++) {
				u16 p = _startboxPix[(_sbBoxY + y) * _startboxW + (_sbBoxX + x)];
				if (p)
					dst[dy * SCREEN_WIDTH + bx + x] = p;
			}
		}
	} else {
		// Blit the box (from its location in the asset) to the bottom of the top screen.
		// Opaque pixels overwrite (clearing any previous text inside), transparent shows the brick.
		for (int y = 0; y < _tbBoxH; y++) {
			int dy = sy + y;
			if (dy < 0 || dy >= SCREEN_HEIGHT) continue; // clip ao sair pelo rodapé
			for (int x = 0; x < _tbBoxW; x++) {
				u16 p = _titleboxPix[(_tbBoxY + y) * _titleboxW + (_tbBoxX + x)];
				if (p)
					dst[dy * SCREEN_WIDTH + sx + x] = p;
			}
		}

		int nLines = 1;
		for (size_t p = 0; p < text.size(); p++)
			if (text[p] == u'\n') nLines++;
		// Centre the text block vertically inside the box.
		int posY = sy + _tbBoxH / 2 - (nLines * lineH) / 2;

		// Draw each line centred, in black.
		size_t start = 0;
		int line = 0;
		while (true) {
			size_t nl = text.find(u'\n', start);
			std::u16string_view ln = text.substr(start, (nl == std::u16string_view::npos) ? text.size() - start : nl - start);
			int y0 = posY + line * lineH;
			toncset16(FontGraphic::textBuf[1], 0, SCREEN_WIDTH * lineH);
			font->print(0, 0, true, ln, Alignment::center, FontPalette::regular);
			for (int y = 0; y < lineH && y0 + y < SCREEN_HEIGHT; y++) {
				if (y0 + y < 0) continue;
				for (int x = 0; x < SCREEN_WIDTH; x++)
					if (FontGraphic::textBuf[1][y * SCREEN_WIDTH + x])
						dst[(y0 + y) * SCREEN_WIDTH + x] = RGB15(0, 0, 0) | BIT(15);
			}
			if (nl == std::u16string_view::npos) break;
			start = nl + 1;
			line++;
		}
	}

	// Present the finished frame in a single contiguous copy (no visible half-draw).
	tonccpy(BG_GFX_SUB, dst, sizeof(u16) * SCREEN_WIDTH * SCREEN_HEIGHT);
}

// DEBUG: contador de FPS numa box preta no canto superior-esquerdo da tela superior.
// Mede quadros do loop principal por segundo (revela drops); desenhado direto no BG_GFX_SUB
// a cada frame, DEPOIS da composição do topo, então nunca é sobrescrito pelo drawTopTitle.
void ThemeTextures::drawTopFps() {
	static int acc = 0;        // quadros contados no segundo atual
	static time_t last = 0;    // segundo (RTC) da última atualização
	static int fps = 0;        // valor exibido
	acc++;
	time_t now = time(NULL);
	if (now != last) { fps = acc; acc = 0; last = now; }

	FontGraphic *font = smallFont();
	if (!font) return;
	const int lineH = font->height();
	const int boxW = 44, boxH = lineH + 2;

	// Box preta opaca.
	for (int y = 0; y < boxH && y < SCREEN_HEIGHT; y++)
		for (int x = 0; x < boxW && x < SCREEN_WIDTH; x++)
			BG_GFX_SUB[y * SCREEN_WIDTH + x] = RGB15(0, 0, 0) | BIT(15);

	// Número em branco dentro da box.
	char buf[16];
	sprintf(buf, "%d fps", fps);
	std::u16string s = FontGraphic::utf8to16(buf);
	toncset16(FontGraphic::textBuf[1], 0, SCREEN_WIDTH * lineH);
	font->print(0, 0, true, s, Alignment::left, FontPalette::regular);
	for (int y = 0; y < lineH && (y + 1) < SCREEN_HEIGHT; y++)
		for (int x = 0; x < boxW - 4; x++)
			if (FontGraphic::textBuf[1][y * SCREEN_WIDTH + x])
				BG_GFX_SUB[(y + 1) * SCREEN_WIDTH + (x + 2)] = RGB15(31, 31, 31) | BIT(15);
}

ITCM_CODE void ThemeTextures::drawDateTimeMacro(const char *str, int posX, int posY, bool isDate) {
	if (ms().theme == TWLSettings::EThemeSaturn) return;

	if (!topBorderBufferLoaded) {
		_backgroundTextures[1].copy(_topBorderBuffer, false);
		topBorderBufferLoaded = true;
	}

	toncset16(FontGraphic::textBuf[1], 0, 256 * dateTimeFont()->height());
	dateTimeFont()->print(0, 0, true, str, Alignment::left, FontPalette::dateTime);
	int width = std::max(dateTimeFont()->calcWidth(str), isDate ? _previousDateWidth : _previousTimeWidth);

	// Copy to background
	for (int y = 0; y < dateTimeFont()->height() && posY + y < SCREEN_HEIGHT; y++) {
		if (posY + y < 0) continue;
		for (int x = 0; x < width && posX + x < SCREEN_WIDTH; x++) {
			if (posX + x < 0) continue;
			int px = FontGraphic::textBuf[1][y * 256 + x];
			u16 bg = _topBorderBuffer[(posY + y) * 256 + (posX + x)];
			u16 val = px ? themealphablend(BG_PALETTE[px], bg, (px % 4) < 2 ? 128 : 224) : bg;

			BG_GFX[(posY + y) * 256 + (posX + x)] = val;
		}
	}

	if (isDate) {
		_previousDateWidth = dateTimeFont()->calcWidth(str);
	} else {
		_previousTimeWidth = dateTimeFont()->calcWidth(str);
	}
}

void ThemeTextures::applyUserPaletteToAllGrfTextures() {
	if (_bipsTexture && tc().bipsUserPalette())
		_bipsTexture->applyUserPaletteFile(TFN_PALETTE_BIPS, effectDSiArrowButtonPalettes);
	if (_boxTexture && tc().boxUserPalette())
		_boxTexture->applyUserPaletteFile(TFN_PALETTE_BOX, effectDSiArrowButtonPalettes);
	if (_braceTexture && tc().braceUserPalette())
		_braceTexture->applyUserPaletteFile(TFN_PALETTE_BRACE, effectDSiArrowButtonPalettes);
	if (_bubbleTexture && tc().bubbleUserPalette())
		_bubbleTexture->applyUserPaletteFile(TFN_PALETTE_BUBBLE, effectDSiArrowButtonPalettes);
	if (_buttonArrowTexture && tc().buttonArrowUserPalette())
		_buttonArrowTexture->applyUserPaletteFile(TFN_PALETTE_BUTTON_ARROW, effectDSiArrowButtonPalettes);
	if (_cornerButtonTexture && tc().cornerButtonUserPalette())
		_cornerButtonTexture->applyUserPaletteFile(TFN_PALETTE_CORNERBUTTON, effectDSiArrowButtonPalettes);
	if (_dialogBoxTexture && tc().dialogBoxUserPalette())
		_dialogBoxTexture->applyUserPaletteFile(TFN_PALETTE_DIALOGBOX, effectDSiArrowButtonPalettes);
	if (_folderTexture && tc().folderUserPalette())
		_folderTexture->applyUserPaletteFile(TFN_PALETTE_FOLDER, effectDSiArrowButtonPalettes);
	if (_launchDotTexture && tc().launchDotsUserPalette())
		_launchDotTexture->applyUserPaletteFile(TFN_PALETTE_LAUNCH_DOT, effectDSiArrowButtonPalettes);
	if (_movingArrowTexture && tc().movingArrowUserPalette())
		_movingArrowTexture->applyUserPaletteFile(TFN_PALETTE_MOVING_ARROW, effectDSiArrowButtonPalettes);
	if (_progressTexture && tc().progressUserPalette())
		_progressTexture->applyUserPaletteFile(TFN_PALETTE_PROGRESS, effectDSiArrowButtonPalettes);
	if (_scrollWindowTexture && tc().scrollWindowUserPalette())
		_scrollWindowTexture->applyUserPaletteFile(TFN_PALETTE_SCROLL_WINDOW, effectDSiArrowButtonPalettes);
	if (_smallCartTexture && tc().smallCartUserPalette())
		_smallCartTexture->applyUserPaletteFile(TFN_PALETTE_SMALL_CART, effectDSiArrowButtonPalettes);
	if (_startBorderTexture && (tc().startBorderUserPalette() || tc().cursorUserPalette())) // same texture variable, different images in dsi/3ds themes
		_startBorderTexture->applyUserPaletteFile(TFN_PALETTE_START_BORDER, effectDSiStartBorderPalettes);
	if (_startTextTexture && tc().startTextUserPalette())
		_startTextTexture->applyUserPaletteFile(TFN_PALETTE_START_TEXT, effectDSiStartTextPalettes);
	if (_wirelessIconsTexture && tc().wirelessIconsUserPalette())
		_wirelessIconsTexture->applyUserPaletteFile(TFN_PALETTE_WIRELESSICONS, effectDSiArrowButtonPalettes);
	
	if (_boxEmptyTexture && tc().boxUserPalette())
		_boxEmptyTexture->applyUserPaletteFile(TFN_PALETTE_BOX_EMPTY, effectDSiArrowButtonPalettes);
	if (_boxFullTexture && tc().boxUserPalette())
		_boxFullTexture->applyUserPaletteFile(TFN_PALETTE_BOX_EMPTY, effectDSiArrowButtonPalettes);

	if (_manualIconTexture && tc().iconManualUserPalette())
		_manualIconTexture->applyUserPaletteFile(TFN_PALETTE_ICON_MANUAL, effectDSiArrowButtonPalettes);
	if (_settingsIconTexture && tc().iconSettingsUserPalette())
		_settingsIconTexture->applyUserPaletteFile(TFN_PALETTE_ICON_SETTINGS, effectDSiArrowButtonPalettes);
}

u16 *ThemeTextures::bgSubBuffer2() { return _bgSubBuffer2; }
u16 *ThemeTextures::photoBuffer() { return _photoBuffer; }
u16 *ThemeTextures::photoBuffer2() { return _photoBuffer2; }
//u16 *ThemeTextures::frameBuffer(bool secondBuffer) { return _frameBuffer[secondBuffer]; }
u16 *ThemeTextures::frameBufferBot(bool secondBuffer) { return _frameBufferBot[secondBuffer]; }

void loadRotatingCubes() {
	std::string cubes = TFN_RVID_CUBES;
	FILE *videoFrameFile = fopen(cubes.c_str(), "rb");

	if (videoFrameFile) {
		bool doRead = false;
		if (dsiFeatures()) {
			doRead = true;
		} else if (sys().isRegularDS() && (io_dldi_data->ioInterface.features & FEATURE_SLOT_NDS)) {
			sysSetCartOwner(BUS_OWNER_ARM9); // Allow arm9 to access GBA ROM (or in this case, the DS Memory
							 // Expansion Pak)
			if (*(u16*)(0x020000C0) == 0) {
				*(vu16*)(0x08240000) = 1;
			}
			if ((*(u16*)(0x020000C0) != 0 && *(u16*)(0x020000C0) != 0x5A45) || *(vu16*)(0x08240000) == 1) {
				// Set to load video into DS Memory Expansion Pak
				rotatingCubesLocation = (u8*)0x09000000;
				doRead = true;
			}
		}

		if (doRead) {
			// Compatible with RVID v2 & v3
			int rvidVer = 0;
			fseek(videoFrameFile, 0x4, SEEK_SET);
			fread((void*)&rvidVer, sizeof(u32), 1, videoFrameFile);

			extern int rocketVideo_videoFrames;
			// fseek(videoFrameFile, 0x8, SEEK_SET);
			fread((void*)&rocketVideo_videoFrames, sizeof(u32), 1, videoFrameFile);
			rocketVideo_videoFrames--;

			extern u8 rocketVideo_fps;
			// fseek(videoFrameFile, 0xC, SEEK_SET);
			fread((void*)&rocketVideo_fps, sizeof(u8), 1, videoFrameFile);
			if (rocketVideo_fps >= 0x80) {
				rocketVideo_fps -= 0x80;
			}

			extern u8 rocketVideo_height;
			// fseek(videoFrameFile, 0xD, SEEK_SET);
			fread((void*)&rocketVideo_height, sizeof(u8), 1, videoFrameFile);

			if (rvidVer == 3) {
				u8 isDualScreen = 0;
				fseek(videoFrameFile, 0xF, SEEK_SET);
				fread((void*)&isDualScreen, sizeof(u8), 1, videoFrameFile);

				if (isDualScreen) {
					fclose(videoFrameFile);
					return;
				}
			}

			u8 rvidBmpMode = 1;
			if (rvidVer == 3) {
				fseek(videoFrameFile, 0x13, SEEK_SET);
				fread((void*)&rvidBmpMode, sizeof(u8), 1, videoFrameFile);
			}

			u32 framesSize = (0x200*rocketVideo_height)*(rocketVideo_videoFrames+1);
			if (rocketVideo_height > 144 || framesSize > 0x700000) {
				fclose(videoFrameFile);
				return;
			}

			// Configured by tc().rotatingCubesRenderY()
			/* if (rocketVideo_height >= 58) {
				// Adjust video positioning
				extern int rocketVideo_videoYpos;
				for (int i = 58; i < rocketVideo_height; i += 2) {
					rocketVideo_videoYpos--;
				}
			} */

			u32 framesOffset = 0x200;
			if (rvidVer == 3) {
				u16* rotatingCubesLocation16 = (u16*)rotatingCubesLocation;

				u16* colors256 = NULL;
				u8* frameBuffer256 = NULL;
				if (rvidBmpMode == 0) {
					colors256 = new u16[256];
					frameBuffer256 = new u8[0xC000];
				}
				u32 frameTableOffset = 0x200;
				for (int i = 0; i <= rocketVideo_videoFrames; i++) {
					fseek(videoFrameFile, frameTableOffset, SEEK_SET);
					fread((void*)&framesOffset, sizeof(u32), 1, videoFrameFile);

					fseek(videoFrameFile, framesOffset, SEEK_SET);
					if (rvidBmpMode == 0) {
						fread(colors256, 2, 256, videoFrameFile);
						fread(frameBuffer256, 1, 0x100*rocketVideo_height, videoFrameFile);

						if (colorTable) {
							for (int c = 0; c < 256; c++) {
								colors256[c] = colorTable[colors256[c] % 0x8000] | BIT(15);
							}
						} else {
							for (int c = 0; c < 256; c++) {
								colors256[c] |= BIT(15);
							}
						}

						for (int p = 0; p < 0x100*rocketVideo_height; p++) {
							rotatingCubesLocation16[((0x100*rocketVideo_height)*i)+p] = colors256[frameBuffer256[p]];
						}
					} else {
						fread(rotatingCubesLocation+((0x200*rocketVideo_height)*i), 1, 0x200*rocketVideo_height, videoFrameFile);
					}

					frameTableOffset += 4;
				}
				if (rvidBmpMode == 0) {
					delete[] colors256;
					delete[] frameBuffer256;
				}
			} else {
				fseek(videoFrameFile, 0x14, SEEK_SET);
				fread((void*)&framesOffset, sizeof(u32), 1, videoFrameFile);

				fseek(videoFrameFile, framesOffset, SEEK_SET);

				fread(rotatingCubesLocation, 1, framesSize, videoFrameFile);
			}

			if (colorTable && rvidBmpMode > 0) {
				u16* rotatingCubesLocation16 = (u16*)rotatingCubesLocation;
				for (u32 i = 0; i < framesSize/2; i++) {
					rotatingCubesLocation16[i] = colorTable[rotatingCubesLocation16[i] % 0x8000] | BIT(15);
				}
			} else if (rvidBmpMode == 2) {
				u16* rotatingCubesLocation16 = (u16*)rotatingCubesLocation;
				for (u32 i = 0; i < framesSize/2; i++) {
					rotatingCubesLocation16[i] |= BIT(15);
				}
			}

			rotatingCubesLoaded = true;
			rocketVideo_playVideo = true;
		}
		fclose(videoFrameFile);
	}
}
void ThemeTextures::unloadRotatingCubes() {
	if (dsiFeatures() && !ms().macroMode && ms().theme == TWLSettings::ETheme3DS && ms().consoleModel == 0) {
		toncset32(rotatingCubesLocation, 0, 0x700000/sizeof(u32)); // Clear video before freeing
		delete[] rotatingCubesLocation;
	}
}
void ThemeTextures::unloadPhotoBuffer() {
	if (!_photoBuffer) {
		return;
	}

	delete[] _photoBuffer;
	if (boxArtColorDeband) {
		delete[] _photoBuffer2;
	}

	_photoBuffer = NULL;
	_photoBuffer2 = NULL;
}
void ThemeTextures::reloadPhotoBuffer() {
	_photoBuffer = new u16[208 * 156];
	if (boxArtColorDeband) {
		_photoBuffer2 = new u16[208 * 156];
	}

	extern void reloadPhoto();
	reloadPhoto();
}
void ThemeTextures::videoSetup() {
	logPrint("tex().videoSetup()\n");
	//////////////////////////////////////////////////////////
	videoSetMode(MODE_5_3D | DISPLAY_BG3_ACTIVE);
	videoSetModeSub(MODE_3_2D | DISPLAY_BG3_ACTIVE);

	// Initialize gl2d
	glScreen2D();
	// Make gl2d render on transparent stage.
	glClearColor(31, 31, 31, 0);
	glDisable(GL_CLEAR_BMP);

	// Clear the GL texture state
	glResetTextures();

	// Set up enough texture memory for our textures
	// Bank A is just 128kb and we are using 194 kb of
	// sprites
	vramSetBankA(VRAM_A_TEXTURE);
	vramSetBankB(VRAM_B_MAIN_BG_0x06020000);
	vramSetBankC(VRAM_C_SUB_BG_0x06200000);
	vramSetBankD(VRAM_D_MAIN_BG_0x06000000);
	vramSetBankE(VRAM_E_TEX_PALETTE);
	vramSetBankF(VRAM_F_TEX_PALETTE_SLOT4);
	vramSetBankG(VRAM_G_MAIN_SPRITE);
	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE);
	vramSetBankI(VRAM_I_SUB_SPRITE_EXT_PALETTE);

	//	vramSetBankH(VRAM_H_SUB_BG_EXT_PALETTE); // Not sure this does anything...
	lcdMainOnBottom();

	int bg3Main = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Main, 3);

	int bg2Main = bgInit(2, BgType_Bmp8, BgSize_B8_256x256, 6, 0);
	nocashMessage(std::to_string(bg2Main).c_str());
	bgSetPriority(bg2Main, 0);

	int bg3Sub = bgInitSub(3, BgType_Bmp16, BgSize_B16_256x256, 0, 0);
	bgSetPriority(bg3Sub, 3);

	bgSetPriority(0, 1); // Set 3D to below text

	/*if (widescreenEffects) {
		// Add black bars to left and right sides
		s16 c = cosLerp(0) >> 4;
		REG_BG3PA_SUB = ( c * 315)>>8;
		REG_BG3X_SUB = -29 << 8;
	}*/

	char currentSettingPath[40];
	sprintf(currentSettingPath, "%s:/_nds/colorLut/currentSetting.txt", (sys().isRunFromSD() ? "sd" : "fat"));

	if (access(currentSettingPath, F_OK) == 0) {
		// Load color LUT
		char lutName[128] = {0};
		FILE* file = fopen(currentSettingPath, "rb");
		fread(lutName, 1, 128, file);
		fclose(file);

		char colorTablePath[256];
		sprintf(colorTablePath, "%s:/_nds/colorLut/%s.lut", (sys().isRunFromSD() ? "sd" : "fat"), lutName);

		if (getFileSize(colorTablePath) == 0x10000) {
			colorTable = new u16[0x10000/sizeof(u16)];

			FILE* file = fopen(colorTablePath, "rb");
			fread(colorTable, 1, 0x10000, file);
			fclose(file);

			const u16 color0 = colorTable[0] | BIT(15);
			const u16 color7FFF = colorTable[0x7FFF] | BIT(15);

			invertedColors =
			  (color0 >= 0xF000 && color0 <= 0xFFFF
			&& color7FFF >= 0x8000 && color7FFF <= 0x8FFF);
			if (!invertedColors) noWhiteFade = (color7FFF < 0xF000);
		}
	}

	REG_BLDCNT = BLEND_SRC_BG3 | (invertedColors ? BLEND_FADE_WHITE : BLEND_FADE_BLACK);

	if (dsiFeatures() && !ms().macroMode && ms().theme != TWLSettings::EThemeHBL) {
		if (ms().consoleModel > 0) {
			rotatingCubesLocation = (u8*)0x0D700000;
			boxArtCache = (u8*)0x0D540000;
		} else {
			if (ms().theme == TWLSettings::ETheme3DS) {
				rotatingCubesLocation = new u8[0x700000];
			}
			if (ms().showBoxArt == 2) {
				boxArtCache = new u8[0x1B8000];
			}
		}
	}

	if (ms().theme == TWLSettings::ETheme3DS && !ms().macroMode) {
		loadRotatingCubes();
	}

	_photoBuffer = new u16[208 * 156];

	boxArtColorDeband = (ms().boxArtColorDeband && !ms().macroMode && (sys().isRegularDS() ? sys().dsDebugRam() : ndmaEnabled()) && !rotatingCubesLoaded && ms().theme != TWLSettings::EThemeHBL);

	if (boxArtColorDeband) {
		_bgSubBuffer2 = new u16[256 * 192];
		_photoBuffer2 = new u16[208 * 156];
		_frameBufferBot[0] = new u16[256 * 192];
		_frameBufferBot[1] = new u16[256 * 192];
	}
}