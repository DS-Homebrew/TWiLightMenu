// fileQR — receptor DSi.
// Fase 1: seletor de pasta de destino no SD (lista, A entra, B volta, START confirma).
// Fase 2: camera (256x192 RGB555) -> quirc -> remontador FQR1 -> grava na pasta.
#include "camera.h"
#include "protocol.h"
#include "quirc.h"

#include <dirent.h>
#include <fat.h>
#include <nds.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define CAM_W 256
#define CAM_H 192

#define MAX_ENTRIES 256
#define NAME_LEN 128
#define PATH_LEN 512
#define VISIBLE_ROWS 18 // linhas de lista visíveis na tela inferior

// ---------------- utilidades de caminho ----------------
static void path_join(char *dst, const char *dir, const char *name)
{
	if (strcmp(dir, "/") == 0)
		siprintf(dst, "/%s", name);
	else
		siprintf(dst, "%s/%s", dir, name);
}

static void path_parent(char *dir)
{
	if (strcmp(dir, "/") == 0)
		return;
	char *slash = strrchr(dir, '/');
	if (!slash)
		return;
	if (slash == dir)
		dir[1] = '\0'; // "/x" -> "/"
	else
		*slash = '\0';
}

// lista subpastas de `path` em names[], retorna a quantidade (ou -1).
static int list_dirs(const char *path, char names[][NAME_LEN], int maxn)
{
	DIR *d = opendir(path);
	if (!d)
		return -1;
	int n = 0;
	struct dirent *e;
	while ((e = readdir(d)) != NULL && n < maxn) {
		if (e->d_name[0] == '.') // pula ".", ".." e ocultos
			continue;
		char full[PATH_LEN];
		path_join(full, path, e->d_name);
		struct stat st;
		if (stat(full, &st) == 0 && S_ISDIR(st.st_mode)) {
			strncpy(names[n], e->d_name, NAME_LEN - 1);
			names[n][NAME_LEN - 1] = '\0';
			n++;
		}
	}
	closedir(d);
	return n;
}

// ---------------- RGB555 -> luma p/ o quirc ----------------
static inline uint8_t rgb555_luma(u16 px)
{
	int r = px & 0x1F, g = (px >> 5) & 0x1F, b = (px >> 10) & 0x1F;
	return (uint8_t)((r * 77 + g * 150 + b * 29) >> 5);
}

// ---------------- FASE 1: seletor de pasta ----------------
static char names[MAX_ENTRIES][NAME_LEN];

static bool s_fat_ok = false;

static void draw_browser(const char *cur, int count, int sel, int top)
{
	consoleClear();
	iprintf("fileQR - pasta de destino\n");
	iprintf("SD: %s\n", s_fat_ok ? "OK" : "INDISPONIVEL");
	iprintf("Local: %.25s\n", cur);
	iprintf("--------------------------------");
	if (count <= 0) {
		iprintf("  (sem subpastas aqui)\n");
	} else {
		for (int i = 0; i < VISIBLE_ROWS && top + i < count; i++) {
			int idx = top + i;
			iprintf("%c %.28s/\n", idx == sel ? '>' : ' ', names[idx]);
		}
	}
	// rodapé fixo (linha 23)
	iprintf("\x1b[22;0H--------------------------------");
	iprintf("UP/DN  A:entrar B:voltar START:ok");
}

// devolve em out_dir a pasta escolhida
static void folder_select(char *out_dir)
{
	char cur[PATH_LEN];
	strcpy(cur, "/");
	int count = list_dirs(cur, names, MAX_ENTRIES);
	int sel = 0, top = 0;
	draw_browser(cur, count, sel, top);

	while (1) {
		swiWaitForVBlank();
		scanKeys();
		u16 k = keysDown();
		bool dirty = false;

		if (k & KEY_UP && sel > 0) {
			sel--;
			if (sel < top)
				top = sel;
			dirty = true;
		}
		if (k & KEY_DOWN && sel < count - 1) {
			sel++;
			if (sel >= top + VISIBLE_ROWS)
				top = sel - VISIBLE_ROWS + 1;
			dirty = true;
		}
		if (k & KEY_A && count > 0) {
			char next[PATH_LEN];
			path_join(next, cur, names[sel]);
			int c = list_dirs(next, names, MAX_ENTRIES);
			if (c >= 0) {
				strcpy(cur, next);
				count = c;
				sel = top = 0;
				dirty = true;
			}
		}
		if (k & KEY_B) {
			path_parent(cur);
			count = list_dirs(cur, names, MAX_ENTRIES);
			sel = top = 0;
			dirty = true;
		}
		if (k & KEY_START) {
			strcpy(out_dir, cur);
			return;
		}
		if (dirty)
			draw_browser(cur, count, sel, top);
	}
}

// ---------------- FASE 2: scan ----------------
static void scan_loop(const char *save_dir, bool fatOk)
{
	// tela superior = preview da câmera
	videoSetMode(MODE_5_2D);
	vramSetBankA(VRAM_A_MAIN_BG);
	int bg = bgInit(3, BgType_Bmp16, BgSize_B16_256x256, 1, 0);
	u16 *preview = bgGetGfxPtr(bg);

	consoleClear();
	iprintf("fileQR - escaneando\n");
	iprintf("--------------------------------");
	iprintf("SD: %s\n", fatOk ? "OK" : "INDISPONIVEL");
	iprintf("Destino: %.23s\n", save_dir);

	iprintf("Iniciando camera...\n");
	pxiWaitRemote(PXI_CAMERA);
	if (!cameraInit()) {
		iprintf("Falha na camera.\n(precisa DSi real ou melonDS\n modo DSi)\n");
		while (1)
			swiWaitForVBlank();
	}
	Camera cam = CAM_OUTER;
	cameraActivate(cam);

	struct quirc *q = quirc_new();
	if (!q || quirc_resize(q, CAM_W, CAM_H) < 0) {
		iprintf("Sem memoria p/ o decoder.\n");
		while (1)
			swiWaitForVBlank();
	}

	fqr_stream r;
	fqr_stream_init(&r, save_dir); // grava direto no SD (streaming)

	iprintf("Aponte para os QR do sender.\n");
	iprintf("A: trocar camera  START: sair\n");

	int last_got = -1;
	bool announced = false;
	bool done = false;

	while (1) {
		scanKeys();
		u16 down = keysDown();
		if (down & KEY_START)
			break;
		if (down & KEY_A) {
			while (cameraTransferActive())
				swiWaitForVBlank();
			cameraTransferStop();
			cam = (cam == CAM_INNER) ? CAM_OUTER : CAM_INNER;
			cameraActivate(cam);
		}
		if (done) {
			swiWaitForVBlank();
			continue;
		}

		cameraTransferStart(preview, CAPTURE_MODE_PREVIEW);
		while (cameraTransferActive())
			swiWaitForVBlank();

		int w, h;
		uint8_t *img = quirc_begin(q, &w, &h);
		for (int i = 0; i < w * h; i++)
			img[i] = rgb555_luma(preview[i]);
		quirc_end(q);

		int n = quirc_count(q);
		for (int i = 0; i < n; i++) {
			struct quirc_code code;
			struct quirc_data data;
			quirc_extract(q, i, &code);
			if (quirc_decode(&code, &data) != QUIRC_SUCCESS)
				continue;
			// QR em Byte mode = pacote FQR1 cru (sem base64); grava direto no SD.
			// Só grava se o SD montou (senão nem tenta abrir arquivo).
			if (fatOk)
				fqr_stream_feed(&r, data.payload, data.payload_len);
		}

		if (r.open_failed && !announced) {
			announced = true;
			iprintf("\x1b[8;0HERRO ao criar arquivo no SD.\n");
		} else if (r.have_manifest && r.got != last_got) {
			last_got = r.got;
			iprintf("\x1b[8;0H");
			iprintf("Gravando: %-22s\n", r.name);
			iprintf("Tamanho : %-10lu bytes\n", (unsigned long)r.size);
			iprintf("Frames  : %5u / %-5u        \n", r.got, r.total);
			int pct = r.total ? (int)(100u * r.got / r.total) : 0;
			iprintf("Progresso: %3d%%               \n", pct);
		}

		if (fqr_stream_complete(&r)) {
			iprintf("\x1b[14;0HVerificando CRC (lendo SD)...\n");
			if (fqr_stream_verify(&r)) {
				iprintf("CRC32 OK. Arquivo gravado:    \n");
				iprintf("%.32s\n", r.path);
			} else {
				iprintf("CRC32 FALHOU. Corrompido.     \n");
			}
			iprintf("Pronto. START para sair.\n");
			done = true;
		}
	}

	cameraDeactivate(cam);
	quirc_destroy(q);
	fqr_stream_close(&r);
}

int main(void)
{
	consoleDemoInit(); // console de texto na tela inferior
	defaultExceptionHandler(); // em caso de crash, mostra o endereço da falha

	// Drena teclas herdadas do launcher (evita START/A espúrios no boot).
	for (int i = 0; i < 30; i++) {
		swiWaitForVBlank();
		scanKeys();
	}

	// FAT: uma tentativa por frame, poucas vezes (nada de loop apertado).
	for (int i = 0; i < 8 && !s_fat_ok; i++) {
		s_fat_ok = fatInitDefault();
		if (!s_fat_ok)
			swiWaitForVBlank();
	}

	char save_dir[PATH_LEN];
	folder_select(save_dir);        // FASE 1 (sempre; header mostra status do SD)
	scan_loop(save_dir, s_fat_ok);  // FASE 2

	consoleClear();
	iprintf("Encerrado. Pode fechar.\n");
	while (1)
		swiWaitForVBlank();
	return 0;
}
