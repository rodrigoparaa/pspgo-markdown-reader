#include "psp_input.h"
#include "../core/theme_engine.h"
#include "psp_fs.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined(__PSP__) || defined(PSP)
#include <pspctrl.h>

static uint32_t s_last_buttons = 0;
static uint32_t s_up_hold_start = 0;
static uint32_t s_down_hold_start = 0;
static uint32_t s_last_repeat = 0;

void psp_input_init(void) {
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
    s_last_buttons = 0;
}

void psp_input_poll(AppState *state, uint32_t now_ms) {
    if (!state) return;

    SceCtrlData pad;
    sceCtrlPeekBufferPositive(&pad, 1);

    uint32_t buttons = pad.Buttons;
    uint32_t pressed = buttons & ~s_last_buttons;
    uint32_t released = ~buttons & s_last_buttons;
    s_last_buttons = buttons;

    /* 1. MODO AYUDA (MODAL) */
    if (state->mode == APP_STATE_HELP) {
        if (pressed & (PSP_CTRL_START | PSP_CTRL_CIRCLE | PSP_CTRL_CROSS | PSP_CTRL_TRIANGLE)) {
            app_state_set_mode(state, state->prev_mode);
        }
        return;
    }

    /* 2. GESTIÓN DEL COMBO L + R (Para consola cerrada y abierta) */
    bool l_down = (buttons & PSP_CTRL_LTRIGGER) != 0;
    bool r_down = (buttons & PSP_CTRL_RTRIGGER) != 0;

    if (l_down && r_down) {
        if (state->lr_combo_start_ms == 0) {
            state->lr_combo_start_ms = now_ms;
            state->lr_combo_triggered = false;
        } else if (!state->lr_combo_triggered && (now_ms - state->lr_combo_start_ms > 1000)) {
            /* Combo L+R mantenido por >1 segundo: Cambiar tema */
            theme_cycle_next();
            state->lr_combo_triggered = true;
            state->suppress_next_l_release = true;
            state->suppress_next_r_release = true;
        }
    } else {
        state->lr_combo_start_ms = 0;
    }

    /* 3. MODO EXPLORADOR */
    if (state->mode == APP_STATE_EXPLORER) {
        /* Desplazamiento en lista con Cruceta */
        if (pressed & PSP_CTRL_UP) {
            app_state_explorer_move(state, -1);
            s_up_hold_start = now_ms;
            s_last_repeat = now_ms;
        } else if (buttons & PSP_CTRL_UP) {
            if (now_ms - s_up_hold_start > 350 && now_ms - s_last_repeat > 90) {
                app_state_explorer_move(state, -1);
                s_last_repeat = now_ms;
            }
        }

        if (pressed & PSP_CTRL_DOWN) {
            app_state_explorer_move(state, 1);
            s_down_hold_start = now_ms;
            s_last_repeat = now_ms;
        } else if (buttons & PSP_CTRL_DOWN) {
            if (now_ms - s_down_hold_start > 350 && now_ms - s_last_repeat > 90) {
                app_state_explorer_move(state, 1);
                s_last_repeat = now_ms;
            }
        }

        /* Botón Cuadrado: Cambiar tema */
        if (pressed & PSP_CTRL_SQUARE) {
            theme_cycle_next();
        }

        /* Botón Start: Ayuda */
        if (pressed & PSP_CTRL_START) {
            app_state_set_mode(state, APP_STATE_HELP);
        }

        /* Botón Cruz: Abrir documento */
        if (pressed & PSP_CTRL_CROSS) {
            if (state->file_count > 0 && state->selected_file_idx < state->file_count) {
                char full_path[128];
                snprintf(full_path, sizeof(full_path), "%s/%s",
                         PSP_DOCS_DIR, state->files[state->selected_file_idx].filename);

                char *file_buf = NULL;
                size_t file_sz = 0;
                if (psp_fs_read_file(full_path, &file_buf, &file_sz)) {
                    snprintf(state->current_path, sizeof(state->current_path), "%s", full_path);
                    snprintf(state->current_filename, sizeof(state->current_filename), "%s",
                             state->files[state->selected_file_idx].filename);

                    doc_parse_markdown(&state->doc, file_buf, file_sz, state->current_filename);
                    psp_fs_free_buffer(file_buf);

                    scroll_init(&state->scroll, state->doc.total_height_px, 226);
                    app_state_set_mode(state, APP_STATE_READER);
                }
            }
        }
        return;
    }

    /* 4. MODO LECTOR DE DOCUMENTO (READER) */
    if (state->mode == APP_STATE_READER) {
        /* Desplazamiento por Stick Analógico */
        int dy = (int)pad.Ly - 128;
        scroll_apply_analog(&state->scroll, dy);

        /* Cruceta: Scroll línea a línea */
        if (pressed & PSP_CTRL_UP) {
            scroll_line_up(&state->scroll);
            s_up_hold_start = now_ms;
            s_last_repeat = now_ms;
        } else if (buttons & PSP_CTRL_UP) {
            if (now_ms - s_up_hold_start > 350 && now_ms - s_last_repeat > 60) {
                scroll_line_up(&state->scroll);
                s_last_repeat = now_ms;
            }
        }

        if (pressed & PSP_CTRL_DOWN) {
            scroll_line_down(&state->scroll);
            s_down_hold_start = now_ms;
            s_last_repeat = now_ms;
        } else if (buttons & PSP_CTRL_DOWN) {
            if (now_ms - s_down_hold_start > 350 && now_ms - s_last_repeat > 60) {
                scroll_line_down(&state->scroll);
                s_last_repeat = now_ms;
            }
        }

        /* Gatillo L (Página Anterior) */
        if (pressed & PSP_CTRL_LTRIGGER) {
            state->l_press_start_ms = now_ms;
        }
        if (released & PSP_CTRL_LTRIGGER) {
            if (state->suppress_next_l_release) {
                state->suppress_next_l_release = false;
            } else {
                scroll_page_up(&state->scroll);
            }
            state->l_press_start_ms = 0;
        } else if (l_down && !r_down && (now_ms - state->l_press_start_ms > 450)) {
            /* Scroll continuo si se mantiene L */
            scroll_line_up(&state->scroll);
        }

        /* Gatillo R (Página Siguiente) */
        if (pressed & PSP_CTRL_RTRIGGER) {
            state->r_press_start_ms = now_ms;
        }
        if (released & PSP_CTRL_RTRIGGER) {
            if (state->suppress_next_r_release) {
                state->suppress_next_r_release = false;
            } else {
                scroll_page_down(&state->scroll);
            }
            state->r_press_start_ms = 0;
        } else if (r_down && !l_down && (now_ms - state->r_press_start_ms > 450)) {
            /* Scroll continuo si se mantiene R */
            scroll_line_down(&state->scroll);
        }

        /* Triángulo: Volver al explorador de archivos */
        if (pressed & PSP_CTRL_TRIANGLE) {
            app_state_set_mode(state, APP_STATE_EXPLORER);
        }

        /* Cuadrado: Cambiar tema cromático */
        if (pressed & PSP_CTRL_SQUARE) {
            theme_cycle_next();
        }

        /* Círculo: Alternar visibilidad de la barra superior/inferior (HUD) */
        if (pressed & PSP_CTRL_CIRCLE) {
            state->show_hud = !state->show_hud;
        }

        /* Start: Ayuda y cheatsheet */
        if (pressed & PSP_CTRL_START) {
            app_state_set_mode(state, APP_STATE_HELP);
        }
    }
}

#else
/* Stub para macOS */
void psp_input_init(void) {}
void psp_input_poll(AppState *state, uint32_t now_ms) {
    (void)state; (void)now_ms;
}
#endif
