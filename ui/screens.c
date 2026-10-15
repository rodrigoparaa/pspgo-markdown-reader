#include "screens.h"
#include "graphics.h"
#include "../core/theme_engine.h"
#include "../platform/psp_power.h"
#include <stdio.h>
#include <string.h>

void screen_render_explorer(const AppState *state) {
    if (!state) return;
    const Theme *t = theme_get_current();

    /* 1. Barra Superior (Header) */
    gfx_draw_rect(0, 0, SCREEN_WIDTH, 24, t->header_bg);
    gfx_draw_rect(0, 24, SCREEN_WIDTH, 1, t->code_border);

    gfx_draw_text(12, 8, "PSP Go Markdown Reader", t->h1_color);

    /* Reloj, Batería e Icono en el Header */
    char time_str[16];
    psp_power_get_time_str(time_str, sizeof(time_str));
    int battery = psp_power_get_battery_percent();
    bool charging = psp_power_is_charging();

    /* Icono de batería a la derecha */
    gfx_draw_battery_icon(SCREEN_WIDTH - 40, 5, battery, charging, charging, t->accent);

    /* Porcentaje de batería */
    char bat_str[16];
    snprintf(bat_str, sizeof(bat_str), "%d%%%s", battery, charging ? "+" : "");
    int bat_w = (int)strlen(bat_str) * 8;
    int bat_x = SCREEN_WIDTH - 40 - bat_w - 6;
    gfx_draw_text(bat_x, 8, bat_str, charging ? COLOR_POMO_BREAK : t->text_muted);

    /* Separador */
    int sep_x = bat_x - 12;
    gfx_draw_text(sep_x, 8, "|", t->code_border);

    /* Hora */
    int time_w = (int)strlen(time_str) * 8;
    int time_x = sep_x - time_w - 6;
    gfx_draw_text(time_x, 8, time_str, t->text_muted);

    /* 2. Contenedor de Archivos */
    if (state->file_count == 0) {
        /* Estado vacío: Sin archivos en ef0:/DOCS */
        int card_x = 40;
        int card_y = 60;
        int card_w = 400;
        int card_h = 140;

        gfx_draw_card(card_x, card_y, card_w, card_h, t->code_bg, t->code_border);
        gfx_draw_text(card_x + 20, card_y + 20, "No se encontraron documentos Markdown", t->h2_color);
        gfx_draw_text(card_x + 20, card_y + 44, "Conecta la PSP Go por USB y copia tus notas en:", t->text_color);

        gfx_draw_card(card_x + 20, card_y + 64, 360, 24, t->bg_color, t->code_border);
        gfx_draw_text(card_x + 30, card_y + 72, "ef0:/DOCS/*.md", t->code_text);

        gfx_draw_text(card_x + 20, card_y + 104, "Presiona [START] para ver la guia de atajos", t->text_muted);
    } else {
        /* Lista de archivos encontrados */
        const int row_h = 28;
        const int start_y = 32;
        const int max_visible = 7;

        for (int i = 0; i < max_visible; i++) {
            int file_idx = state->explorer_scroll_top + i;
            if (file_idx >= state->file_count) break;

            int y = start_y + (i * row_h);
            bool is_selected = (file_idx == state->selected_file_idx);

            if (is_selected) {
                gfx_draw_card(12, y, SCREEN_WIDTH - 28, row_h - 2, t->code_bg, t->accent);
                gfx_draw_text(20, y + 8, ">", t->accent);
            }

            /* Icono y Nombre */
            int text_x = is_selected ? 36 : 24;
            gfx_draw_text(text_x, y + 8, "[MD]", is_selected ? t->accent : t->text_muted);

            /* Truncar nombre si excede ancho seguro */
            char disp_name[34];
            strncpy(disp_name, state->files[file_idx].filename, sizeof(disp_name) - 1);
            disp_name[sizeof(disp_name) - 1] = '\0';
            gfx_draw_text(text_x + 36, y + 8, disp_name, is_selected ? t->text_color : t->text_muted);

            /* Tamaño del archivo en KB */
            char size_str[16];
            unsigned int sz = (unsigned int)state->files[file_idx].size_bytes;
            if (sz >= 1024) {
                snprintf(size_str, sizeof(size_str), "%u.%u KB", sz / 1024, (sz % 1024) / 100);
            } else {
                snprintf(size_str, sizeof(size_str), "%u B", sz);
            }
            gfx_draw_text(SCREEN_WIDTH - 96, y + 8, size_str, t->text_muted);
        }

        /* Barra de scroll lateral para lista de archivos */
        if (state->file_count > max_visible) {
            int track_h = max_visible * row_h;
            gfx_draw_rect(SCREEN_WIDTH - 12, start_y, 4, track_h, t->scrollbar_track);

            int thumb_h = (max_visible * track_h) / state->file_count;
            if (thumb_h < 12) thumb_h = 12;

            int max_scroll_top = state->file_count - max_visible;
            int thumb_y = start_y + (state->explorer_scroll_top * (track_h - thumb_h)) / max_scroll_top;
            gfx_draw_rect(SCREEN_WIDTH - 12, thumb_y, 4, thumb_h, t->scrollbar_thumb);
        }
    }

    /* 3. Barra Inferior (Footer) */
    gfx_draw_rect(0, SCREEN_HEIGHT - 22, SCREEN_WIDTH, 22, t->header_bg);
    gfx_draw_rect(0, SCREEN_HEIGHT - 22, SCREEN_WIDTH, 1, t->code_border);

    gfx_draw_text(12, SCREEN_HEIGHT - 15,
                  "[X] Abrir   [^v] Mover   [[]] Tema   [START] Ayuda",
                  t->header_text);
}

void screen_render_reader(const AppState *state) {
    if (!state) return;
    const Theme *t = theme_get_current();

    int top_margin = state->show_hud ? 24 : 6;
    int scroll_y = scroll_get_render_y(&state->scroll);

    /* Renderizado de renglones del documento con clipping vertical estricto */
    const Document *doc = &state->doc;
    for (int i = 0; i < doc->line_count; i++) {
        const LayoutLine *l = &doc->lines[i];

        int y_screen = l->y_offset - scroll_y + top_margin;

        /* Descarte si está completamente fuera de la pantalla */
        if (y_screen + l->height < (state->show_hud ? 24 : 0)) {
            continue;
        }
        if (y_screen > (state->show_hud ? SCREEN_HEIGHT - 22 : SCREEN_HEIGHT)) {
            break; /* Las líneas son consecutivas: podemos terminar el bucle */
        }

        switch (l->line_type) {
            case LINE_H1:
                gfx_draw_text_scaled(CONTENT_MARGIN_X + l->indent, y_screen, l->text, 2, t->h1_color);
                break;

            case LINE_H2:
                gfx_draw_text_scaled(CONTENT_MARGIN_X + l->indent, y_screen, l->text, 2, t->h2_color);
                break;

            case LINE_H3:
                gfx_draw_text(CONTENT_MARGIN_X + l->indent, y_screen + 2, l->text, t->h3_color);
                break;

            case LINE_HRULE:
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen + 5, USABLE_WIDTH_PX, 1, t->hrule_color);
                break;

            case LINE_CODE_START:
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen, USABLE_WIDTH_PX, l->height, t->code_bg);
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen, USABLE_WIDTH_PX, 1, t->code_border);
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen, 1, l->height, t->code_border);
                gfx_draw_rect(CONTENT_MARGIN_X + USABLE_WIDTH_PX - 1, y_screen, 1, l->height, t->code_border);
                break;

            case LINE_CODE_BODY:
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen, USABLE_WIDTH_PX, l->height, t->code_bg);
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen, 1, l->height, t->code_border);
                gfx_draw_rect(CONTENT_MARGIN_X + USABLE_WIDTH_PX - 1, y_screen, 1, l->height, t->code_border);
                gfx_draw_text(CONTENT_MARGIN_X + l->indent, y_screen + 1, l->text, t->code_text);
                break;

            case LINE_CODE_END:
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen, USABLE_WIDTH_PX, l->height, t->code_bg);
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen + l->height - 1, USABLE_WIDTH_PX, 1, t->code_border);
                gfx_draw_rect(CONTENT_MARGIN_X, y_screen, 1, l->height, t->code_border);
                gfx_draw_rect(CONTENT_MARGIN_X + USABLE_WIDTH_PX - 1, y_screen, 1, l->height, t->code_border);
                break;

            case LINE_QUOTE:
                gfx_draw_rect(CONTENT_MARGIN_X + 4, y_screen, 3, l->height, t->quote_bar);
                gfx_draw_text(CONTENT_MARGIN_X + l->indent, y_screen + 2, l->text, t->quote_text);
                break;

            case LINE_LIST:
                gfx_draw_rect(CONTENT_MARGIN_X + 4, y_screen + 4, 3, 3, t->list_bullet);
                gfx_draw_text(CONTENT_MARGIN_X + l->indent, y_screen + 2, l->text, t->text_color);
                break;

            case LINE_TEXT:
                gfx_draw_text(CONTENT_MARGIN_X + l->indent, y_screen + 2, l->text, t->text_color);
                break;

            case LINE_EMPTY:
            default:
                break;
        }
    }

    /* Barra de scroll lateral del documento */
    int sb_y = state->show_hud ? 24 : 0;
    int sb_h = state->show_hud ? (SCREEN_HEIGHT - 46) : SCREEN_HEIGHT;
    gfx_draw_rect(SCREEN_WIDTH - 6, sb_y, 4, sb_h, t->scrollbar_track);

    int pct = scroll_get_percent(&state->scroll);
    int thumb_size = 24;
    int thumb_y = sb_y + ((sb_h - thumb_size) * pct) / 100;
    gfx_draw_rect(SCREEN_WIDTH - 6, thumb_y, 4, thumb_size, t->scrollbar_thumb);

    /* Barras HUD Superior e Inferior (si están activas) */
    if (state->show_hud) {
        /* Barra Superior */
        gfx_draw_rect(0, 0, SCREEN_WIDTH, 24, t->header_bg);
        gfx_draw_rect(0, 24, SCREEN_WIDTH, 1, t->code_border);

        /* Icono de Batería a la derecha */
        int bat = psp_power_get_battery_percent();
        bool charging = psp_power_is_charging();
        gfx_draw_battery_icon(SCREEN_WIDTH - 40, 5, bat, charging, charging, t->accent);

        /* Porcentaje de batería */
        char bat_str[16];
        snprintf(bat_str, sizeof(bat_str), "%d%%%s", bat, charging ? "+" : "");
        int bat_w = (int)strlen(bat_str) * 8;
        int bat_x = SCREEN_WIDTH - 40 - bat_w - 6;
        gfx_draw_text(bat_x, 8, bat_str, charging ? COLOR_POMO_BREAK : t->text_muted);

        /* Separador 2 */
        int sep2_x = bat_x - 12;
        gfx_draw_text(sep2_x, 8, "|", t->code_border);

        /* Reloj */
        char time_str[16];
        psp_power_get_time_str(time_str, sizeof(time_str));
        int time_w = (int)strlen(time_str) * 8;
        int time_x = sep2_x - time_w - 6;
        gfx_draw_text(time_x, 8, time_str, t->text_muted);

        /* Separador 1 */
        int sep1_x = time_x - 12;
        gfx_draw_text(sep1_x, 8, "|", t->code_border);

        /* Porcentaje de lectura */
        char pct_str[16];
        snprintf(pct_str, sizeof(pct_str), "%d%%", pct);
        int pct_w = (int)strlen(pct_str) * 8;
        int pct_x = sep1_x - pct_w - 6;
        gfx_draw_text(pct_x, 8, pct_str, t->accent);

        /* Título del documento truncado dinámicamente si es largo */
        int max_title_w = pct_x - 12 - 12;
        int max_chars = max_title_w / 8;
        if (max_chars < 8) max_chars = 8;
        char title_disp[48];
        if ((int)strlen(doc->title) > max_chars) {
            strncpy(title_disp, doc->title, max_chars - 3);
            title_disp[max_chars - 3] = '\0';
            strcat(title_disp, "...");
        } else {
            strncpy(title_disp, doc->title, sizeof(title_disp) - 1);
            title_disp[sizeof(title_disp) - 1] = '\0';
        }
        gfx_draw_text(12, 8, title_disp, t->h2_color);

        /* Barra Inferior (Footer) */
        gfx_draw_rect(0, SCREEN_HEIGHT - 22, SCREEN_WIDTH, 22, t->header_bg);
        gfx_draw_rect(0, SCREEN_HEIGHT - 22, SCREEN_WIDTH, 1, t->code_border);

        /* Atajos izquierda */
        gfx_draw_text(12, SCREEN_HEIGHT - 15,
                      "[L/R] Pag   [Stick] Scroll   [/\\ ] Salir",
                      t->header_text);

        /* Tema activo a la derecha sin solapamiento */
        char theme_disp[32];
        snprintf(theme_disp, sizeof(theme_disp), "[[]] %s", t->name);
        int theme_w = (int)strlen(theme_disp) * 8;
        gfx_draw_text(SCREEN_WIDTH - 12 - theme_w, SCREEN_HEIGHT - 15, theme_disp, t->accent);
    }
}

void screen_render_help(const AppState *state) {
    if (!state) return;
    const Theme *t = theme_get_current();

    /* Tarjeta modal centrada */
    int card_x = 20;
    int card_y = 14;
    int card_w = 440;
    int card_h = 244;

    gfx_draw_card(card_x, card_y, card_w, card_h, t->code_bg, t->accent);

    /* Encabezado */
    gfx_draw_text(card_x + 16, card_y + 12, "GUIA DE CONTROLES: PSP GO MARKDOWN READER", t->h1_color);
    gfx_draw_rect(card_x + 16, card_y + 26, card_w - 32, 1, t->code_border);

    /* Dos Columnas: Izquierda (Consola Abierta) y Derecha (Slider Plegado) */
    int col1_x = card_x + 16;
    int col2_x = card_x + 224;

    /* Columna 1 */
    int y1 = card_y + 36;
    gfx_draw_text(col1_x, y1, "CONSOLA ABIERTA (Modo Activo):", t->h2_color);
    y1 += 16;
    gfx_draw_text(col1_x, y1, "* Stick   : Scroll inercial", t->text_color); y1 += 13;
    gfx_draw_text(col1_x, y1, "* Gatillos: Pagina previa/sig", t->text_color); y1 += 13;
    gfx_draw_text(col1_x, y1, "* Cruceta : Renglon a renglon", t->text_color); y1 += 13;
    gfx_draw_text(col1_x, y1, "* Cuadrado: Cambiar tema", t->text_color); y1 += 13;
    gfx_draw_text(col1_x, y1, "* Triang. : Volver al listado", t->text_color); y1 += 13;
    gfx_draw_text(col1_x, y1, "* Circulo : Ocultar/Ver HUD", t->text_color); y1 += 13;
    gfx_draw_text(col1_x, y1, "* Cruz [X]: Abrir archivo", t->text_color); y1 += 13;

    /* Columna 2 */
    int y2 = card_y + 36;
    gfx_draw_text(col2_x, y2, "SLIDER CERRADO (Lectura/Dock):", t->h2_color);
    y2 += 16;
    gfx_draw_text(col2_x, y2, "* Gatillo R : Pagina siguiente", t->text_color); y2 += 13;
    gfx_draw_text(col2_x, y2, "* Gatillo L : Pagina anterior", t->text_color); y2 += 13;
    gfx_draw_text(col2_x, y2, "* Combo L+R : Cambiar tema (>1s)", t->text_color); y2 += 13;
    gfx_draw_text(col2_x, y2, "* Boton HOME: Salir al XMB", t->text_color); y2 += 13;
    y2 += 8;
    gfx_draw_text(col2_x, y2, "CONSEJO:", t->accent); y2 += 13;
    gfx_draw_text(col2_x, y2, "Lee comodamente con slider", t->text_muted); y2 += 13;
    gfx_draw_text(col2_x, y2, "cerrado sujetando a una mano.", t->text_muted); y2 += 13;

    /* Pie modal */
    int foot_y = card_y + card_h - 24;
    gfx_draw_rect(card_x + 16, foot_y - 6, card_w - 32, 1, t->code_border);
    gfx_draw_text_centered(foot_y, "Presiona START, X, O o Triangulo para volver", 1, t->list_bullet);
}
