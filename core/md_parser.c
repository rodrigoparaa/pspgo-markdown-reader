#include "md_parser.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>

void doc_init(Document *doc) {
    if (!doc) return;
    doc->line_count = 0;
    doc->total_height_px = 0;
    doc->title[0] = '\0';
    doc->is_loaded = false;
}

static void append_layout_line(Document *doc, LineType type, uint8_t scale, uint8_t indent, int height, const char *text) {
    if (!doc || doc->line_count >= MAX_DOC_LINES) return;

    LayoutLine *l = &doc->lines[doc->line_count];
    l->y_offset = doc->total_height_px;
    l->height = height;
    l->line_type = type;
    l->scale = scale;
    l->indent = indent;

    if (text) {
        strncpy(l->text, text, MAX_LINE_TEXT_LEN - 1);
        l->text[MAX_LINE_TEXT_LEN - 1] = '\0';
    } else {
        l->text[0] = '\0';
    }

    doc->total_height_px += height;
    doc->line_count++;
}

/* Sanitiza texto UTF-8 convirtiendo caracteres españoles y omitiendo emojis/símbolos multibyte */
static void sanitize_text(const char *src, char *dst, size_t dst_size, bool strip_markdown_markers) {
    if (!src || !dst || dst_size == 0) return;

    size_t out_idx = 0;
    const unsigned char *p = (const unsigned char *)src;

    while (*p && out_idx + 1 < dst_size) {
        /* Decodificación UTF-8 multibyte */
        if (*p >= 0xC0) {
            if ((*p & 0xE0) == 0xC0 && *(p + 1)) {
                /* Secuencia de 2 bytes (Tildes en español, ñ, ¿, ¡) */
                unsigned char c1 = *p;
                unsigned char c2 = *(p + 1);
                p += 2;

                char rep = ' ';
                if (c1 == 0xC3) {
                    switch (c2) {
                        case 0xA1: case 0xA0: case 0xA4: case 0xA2: rep = 'a'; break; /* á, à, ä, â */
                        case 0xA9: case 0xA8: case 0xAB: case 0xAA: rep = 'e'; break; /* é, è, ë, ê */
                        case 0xAD: case 0xAC: case 0xAF: case 0xAE: rep = 'i'; break; /* í, ì, ï, î */
                        case 0xB3: case 0xB2: case 0xB6: case 0xB4: rep = 'o'; break; /* ó, ò, ö, ô */
                        case 0xBA: case 0xB9: case 0xBC: case 0xBB: rep = 'u'; break; /* ú, ù, ü, û */
                        case 0xB1: rep = 'n'; break; /* ñ */
                        case 0x81: case 0x80: case 0x84: rep = 'A'; break; /* Á, À, Ä */
                        case 0x89: case 0x88: case 0x8B: rep = 'E'; break; /* É, È, Ë */
                        case 0x8D: case 0x8C: case 0x8F: rep = 'I'; break; /* Í, Ì, Ï */
                        case 0x93: case 0x92: case 0x96: rep = 'O'; break; /* Ó, Ò, Ö */
                        case 0x9A: case 0x99: case 0x9C: rep = 'U'; break; /* Ú, Ù, Ü */
                        case 0x91: rep = 'N'; break; /* Ñ */
                        default: rep = ' '; break;
                    }
                } else if (c1 == 0xC2) {
                    if (c2 == 0xBF) rep = '?'; /* ¿ */
                    else if (c2 == 0xA1) rep = '!'; /* ¡ */
                    else rep = ' ';
                }
                if (rep != ' ' || (out_idx > 0 && dst[out_idx - 1] != ' ')) {
                    dst[out_idx++] = rep;
                }
                continue;
            } else if ((*p & 0xF0) == 0xE0 && *(p + 1) && *(p + 2)) {
                /* Secuencia de 3 bytes (Símbolos, iconos orientales) -> omitir */
                p += 3;
                continue;
            } else if ((*p & 0xF8) == 0xF0 && *(p + 1) && *(p + 2) && *(p + 3)) {
                /* Secuencia de 4 bytes (Emojis: 📖, 🌟, 🧪, etc.) -> omitir */
                p += 4;
                continue;
            } else {
                p++;
                continue;
            }
        }

        /* Caracteres ASCII estándar */
        char c = (char)*p++;

        if (strip_markdown_markers) {
            /* Negrita doble ** o __ */
            if (c == '*' && *p == '*') {
                p++; /* Saltar par ** */
                continue;
            }
            if (c == '_' && *p == '_') {
                p++; /* Saltar par __ */
                continue;
            }
            /* Asterisco simple para cursiva * */
            if (c == '*') {
                continue;
            }
            /* Guion bajo simple solo si no es intra-palabra (ej. snake_case se preserva) */
            if (c == '_') {
                bool prev_alnum = (out_idx > 0 && isalnum((unsigned char)dst[out_idx - 1]));
                bool next_alnum = (*p && isalnum((unsigned char)*p));
                if (prev_alnum && next_alnum) {
                    /* Preservar guion bajo en nombres y rutas */
                } else {
                    continue;
                }
            }
            /* Backtick en código inline ` */
            if (c == '`') {
                continue;
            }
        }

        if (c >= 32 && c <= 126) {
            dst[out_idx++] = c;
        } else if (c == '\t') {
            dst[out_idx++] = ' ';
        }
    }

    dst[out_idx] = '\0';
}

static void wrap_and_append(Document *doc, LineType first_type, LineType next_type,
                            uint8_t scale, uint8_t indent, int height,
                            int max_chars, const char *raw_text) {
    if (!doc || !raw_text) return;

    if (max_chars >= MAX_LINE_TEXT_LEN) {
        max_chars = MAX_LINE_TEXT_LEN - 1;
    }
    if (max_chars < 10) max_chars = 10;

    const char *p = raw_text;
    while (*p && isspace((unsigned char)*p)) p++;

    if (!*p) {
        append_layout_line(doc, first_type, scale, indent, height, "");
        return;
    }

    bool is_first = true;
    char line_buf[MAX_LINE_TEXT_LEN];

    while (*p) {
        while (*p && isspace((unsigned char)*p)) p++;
        if (!*p) break;

        int cur_len = 0;
        const char *line_start = p;
        const char *last_space = NULL;

        while (*p && cur_len < max_chars) {
            if (isspace((unsigned char)*p)) {
                last_space = p;
            }
            cur_len++;
            p++;
        }

        int copy_len = 0;
        if (!*p) {
            copy_len = (int)(p - line_start);
        } else if (last_space && last_space > line_start) {
            copy_len = (int)(last_space - line_start);
            p = last_space + 1;
        } else {
            copy_len = max_chars;
        }

        if (copy_len > max_chars) copy_len = max_chars;
        memcpy(line_buf, line_start, copy_len);
        line_buf[copy_len] = '\0';

        while (copy_len > 0 && isspace((unsigned char)line_buf[copy_len - 1])) {
            line_buf[--copy_len] = '\0';
        }

        if (copy_len > 0) {
            LineType type = is_first ? first_type : next_type;
            append_layout_line(doc, type, scale, indent, height, line_buf);
            is_first = false;
        }
    }
}

static bool is_horizontal_rule(const char *str) {
    int dashes = 0;
    int asterisks = 0;
    int underscores = 0;
    for (int i = 0; str[i]; i++) {
        if (str[i] == '-') dashes++;
        else if (str[i] == '*') asterisks++;
        else if (str[i] == '_') underscores++;
        else if (!isspace((unsigned char)str[i])) return false;
    }
    return (dashes >= 3 || asterisks >= 3 || underscores >= 3);
}

bool doc_parse_markdown(Document *doc, const char *raw_md, size_t length, const char *fallback_title) {
    if (!doc || !raw_md || length == 0) return false;

    doc_init(doc);

    if (fallback_title && fallback_title[0]) {
        strncpy(doc->title, fallback_title, sizeof(doc->title) - 1);
        doc->title[sizeof(doc->title) - 1] = '\0';
    } else {
        strcpy(doc->title, "Documento");
    }

    bool in_code_block = false;
    bool last_was_empty = false;
    bool has_h1_title = false;

    /* Margen superior */
    append_layout_line(doc, LINE_EMPTY, 1, 0, 8, "");

    const char *cursor = raw_md;
    const char *end = raw_md + length;

    while (cursor < end) {
        const char *line_start = cursor;
        while (cursor < end && *cursor != '\n' && *cursor != '\r') {
            cursor++;
        }
        size_t raw_line_len = cursor - line_start;

        if (cursor < end && *cursor == '\r') cursor++;
        if (cursor < end && *cursor == '\n') cursor++;

        char raw_line[512];
        if (raw_line_len >= sizeof(raw_line)) {
            raw_line_len = sizeof(raw_line) - 1;
        }
        memcpy(raw_line, line_start, raw_line_len);
        raw_line[raw_line_len] = '\0';

        const char *trimmed = raw_line;
        while (*trimmed && isspace((unsigned char)*trimmed)) trimmed++;

        /* Detección de bloques de código */
        if (strncmp(trimmed, "```", 3) == 0) {
            if (!in_code_block) {
                in_code_block = true;
                append_layout_line(doc, LINE_CODE_START, 1, 8, 8, "");
            } else {
                in_code_block = false;
                append_layout_line(doc, LINE_CODE_END, 1, 8, 8, "");
            }
            last_was_empty = false;
            continue;
        }

        if (in_code_block) {
            char code_clean[256];
            sanitize_text(raw_line, code_clean, sizeof(code_clean), false);
            wrap_and_append(doc, LINE_CODE_BODY, LINE_CODE_BODY, 1, 10, 11, 54, code_clean);
            last_was_empty = false;
            continue;
        }

        /* Línea vacía */
        if (*trimmed == '\0') {
            if (!last_was_empty) {
                append_layout_line(doc, LINE_EMPTY, 1, 0, 8, "");
                last_was_empty = true;
            }
            continue;
        }

        /* Regla horizontal */
        if (is_horizontal_rule(trimmed)) {
            append_layout_line(doc, LINE_HRULE, 1, 0, 12, "");
            last_was_empty = false;
            continue;
        }

        /* Encabezado H1: # */
        if (trimmed[0] == '#' && trimmed[1] == ' ') {
            const char *h_text = trimmed + 2;
            while (*h_text && isspace((unsigned char)*h_text)) h_text++;

            char h_clean[256];
            sanitize_text(h_text, h_clean, sizeof(h_clean), true);

            const char *h_final = h_clean;
            while (*h_final && isspace((unsigned char)*h_final)) h_final++;

            if (!has_h1_title && *h_final) {
                strncpy(doc->title, h_final, sizeof(doc->title) - 1);
                doc->title[sizeof(doc->title) - 1] = '\0';
                has_h1_title = true;
            }

            append_layout_line(doc, LINE_EMPTY, 1, 0, 6, "");
            wrap_and_append(doc, LINE_H1, LINE_H1, 2, 0, 20, 27, h_final);
            append_layout_line(doc, LINE_EMPTY, 1, 0, 4, "");
            last_was_empty = false;
            continue;
        }

        /* Encabezado H2: ## */
        if (trimmed[0] == '#' && trimmed[1] == '#' && trimmed[2] == ' ') {
            const char *h_text = trimmed + 3;
            while (*h_text && isspace((unsigned char)*h_text)) h_text++;

            char h_clean[256];
            sanitize_text(h_text, h_clean, sizeof(h_clean), true);

            const char *h_final = h_clean;
            while (*h_final && isspace((unsigned char)*h_final)) h_final++;

            append_layout_line(doc, LINE_EMPTY, 1, 0, 4, "");
            wrap_and_append(doc, LINE_H2, LINE_H2, 2, 0, 18, 27, h_final);
            append_layout_line(doc, LINE_EMPTY, 1, 0, 4, "");
            last_was_empty = false;
            continue;
        }

        /* Encabezado H3: ### */
        if (trimmed[0] == '#' && trimmed[1] == '#' && trimmed[2] == '#' && trimmed[3] == ' ') {
            const char *h_text = trimmed + 4;
            while (*h_text && isspace((unsigned char)*h_text)) h_text++;

            char h_clean[256];
            sanitize_text(h_text, h_clean, sizeof(h_clean), true);

            const char *h_final = h_clean;
            while (*h_final && isspace((unsigned char)*h_final)) h_final++;

            append_layout_line(doc, LINE_EMPTY, 1, 0, 4, "");
            wrap_and_append(doc, LINE_H3, LINE_H3, 1, 0, 13, 54, h_final);
            last_was_empty = false;
            continue;
        }

        /* Cita en bloque: > */
        if (trimmed[0] == '>' && (trimmed[1] == ' ' || trimmed[1] == '\0')) {
            const char *q_text = trimmed + 1;
            while (*q_text && isspace((unsigned char)*q_text)) q_text++;

            char q_clean[256];
            sanitize_text(q_text, q_clean, sizeof(q_clean), true);

            wrap_and_append(doc, LINE_QUOTE, LINE_QUOTE, 1, 16, 12, 50, q_clean);
            last_was_empty = false;
            continue;
        }

        /* Lista con viñetas: - o * o + */
        if ((trimmed[0] == '-' || trimmed[0] == '*' || trimmed[0] == '+') && trimmed[1] == ' ') {
            const char *l_text = trimmed + 2;
            while (*l_text && isspace((unsigned char)*l_text)) l_text++;

            char l_clean[256];
            sanitize_text(l_text, l_clean, sizeof(l_clean), true);

            wrap_and_append(doc, LINE_LIST, LINE_TEXT, 1, 14, 12, 52, l_clean);
            last_was_empty = false;
            continue;
        }

        /* Párrafo normal de texto */
        char text_clean[512];
        sanitize_text(trimmed, text_clean, sizeof(text_clean), true);
        wrap_and_append(doc, LINE_TEXT, LINE_TEXT, 1, 0, 12, 55, text_clean);
        last_was_empty = false;
    }

    /* Padding inferior generoso para que el final del texto pueda desplazarse cómodamente por encima del footer */
    append_layout_line(doc, LINE_EMPTY, 1, 0, 90, "");

    doc->is_loaded = true;
    return true;
}
