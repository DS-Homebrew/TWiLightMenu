#include "frontendLog.h"

#include <nds.h>
#include <cstdio>
#include <stdarg.h>

#include "common/systemdetails.h"

// Arquivo mantido aberto durante toda a sessao do frontend; cada linha e
// descarregada (fflush) imediatamente para sobreviver ao soft-reset que ocorre
// ao lancar um jogo.
static FILE *s_file = NULL;
static bool s_inited = false;
static unsigned s_seq = 0;

void frontendLogInit(void) {
	if (s_inited)
		return;

	const char *path = sys().isRunFromSD() ? "sd:/logfrontend.txt" : "fat:/logfrontend.txt";
	s_file = fopen(path, "w");
	if (!s_file)
		return;

	s_inited = true;
	fputs("=== TWiLightMenu frontend log ===\r\n", s_file);
	fflush(s_file);
}

void frontendLogWrite(const char *func, const char *format, ...) {
	if (!s_inited)
		return;

	char line[256];
	int n = snprintf(line, sizeof(line), "[%05u] %s", s_seq++, func ? func : "?");
	if (n < 0)
		return;
	if (n > (int)sizeof(line) - 1)
		n = sizeof(line) - 1;

	// Anexa a mensagem opcional apos "func: ".
	if (format && n < (int)sizeof(line) - 2) {
		line[n++] = ':';
		line[n++] = ' ';
		va_list args;
		va_start(args, format);
		vsnprintf(line + n, sizeof(line) - n, format, args);
		va_end(args);
	} else {
		line[n] = '\0';
	}

	fputs(line, s_file);
	fputs("\r\n", s_file);
	fflush(s_file);
}
