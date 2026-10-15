#ifndef MD_PARSER_H
#define MD_PARSER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DOC_LINES       4096
#define MAX_LINE_TEXT_LEN   80
#define CONTENT_MARGIN_X    16
#define USABLE_WIDTH_PX     (480 - (CONTENT_MARGIN_X * 2)) /* 448 px */

typedef enum {
    LINE_EMPTY = 0,
    LINE_TEXT,
    LINE_H1,
    LINE_H2,
    LINE_H3,
    LINE_LIST,
    LINE_QUOTE,
    LINE_CODE_START,
    LINE_CODE_BODY,
    LINE_CODE_END,
    LINE_HRULE
} LineType;

typedef struct {
    int y_offset;                   /* Posición vertical acumulada en píxeles */
    int height;                     /* Altura que ocupa la línea en píxeles */
    LineType line_type;             /* Tipo semántico del renglón */
    uint8_t scale;                  /* Escala de renderizado (1x o 2x) */
    uint8_t indent;                 /* Sangría en píxeles */
    char text[MAX_LINE_TEXT_LEN];   /* Texto plano para renderizar */
} LayoutLine;

typedef struct {
    LayoutLine lines[MAX_DOC_LINES];
    int line_count;
    int total_height_px;
    char title[64];
    bool is_loaded;
} Document;

/* Inicializa la estructura del documento a cero */
void doc_init(Document *doc);

/* Parsea un búfer Markdown y genera el maquetado (LayoutLine) listo para pantalla */
bool doc_parse_markdown(Document *doc, const char *raw_md, size_t length, const char *fallback_title);

#ifdef __cplusplus
}
#endif

#endif /* MD_PARSER_H */
