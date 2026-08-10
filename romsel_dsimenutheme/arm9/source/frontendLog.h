#ifndef FRONTEND_LOG_H
#define FRONTEND_LOG_H

// Logger dedicado ao frontend do romsel_dsimenutheme.
//
// Gera "logfrontend.txt" na raiz do cartao (sd:/ ou fat:/ conforme o dispositivo
// de boot) e registra cada funcao/metodo do frontend conforme ela e chamada.
// Cada linha tem o formato:  [NNNNN] nomeDaFuncao: mensagem-opcional
// onde NNNNN e um contador sequencial monotonico da ordem das chamadas.
//
// Uso:
//   FLOG_FN();                    // registra so o nome da funcao atual
//   FLOG("cursor=%d", CURPOS);    // registra o nome da funcao + mensagem
//
// As funcoes chamadas por frame (render/IRQ) usam FLOG_FRAME(), que so grava
// quando FRONTEND_LOG_FRAME esta definido, para nao inundar o arquivo.

#ifdef __cplusplus
extern "C" {
#endif

// Cria/zera o logfrontend.txt. Chamar uma vez, apos a montagem do SD.
void frontendLogInit(void);

// Grava uma linha. `func` = nome da funcao; `format` pode ser NULL (so o nome).
void frontendLogWrite(const char *func, const char *format, ...) __attribute__((format(printf, 2, 3)));

#ifdef __cplusplus
}
#endif

// Registra a entrada da funcao atual (apenas o nome).
#define FLOG_FN()   frontendLogWrite(__func__, (const char *)0)
// Registra a funcao atual com uma mensagem formatada.
#define FLOG(...)   frontendLogWrite(__func__, __VA_ARGS__)

// Para funcoes chamadas todo frame; ativado apenas com -DFRONTEND_LOG_FRAME.
#ifdef FRONTEND_LOG_FRAME
#define FLOG_FRAME(...) frontendLogWrite(__func__, __VA_ARGS__)
#define FLOG_FRAME_FN() frontendLogWrite(__func__, (const char *)0)
#else
#define FLOG_FRAME(...) ((void)0)
#define FLOG_FRAME_FN() ((void)0)
#endif

#endif // FRONTEND_LOG_H
