#include "app_state.h"
#include <string.h>

void app_state_init(AppState *state) {
    if (!state) return;
    memset(state, 0, sizeof(AppState));

    state->mode = APP_STATE_EXPLORER;
    state->prev_mode = APP_STATE_EXPLORER;
    state->file_count = 0;
    state->selected_file_idx = 0;
    state->explorer_scroll_top = 0;
    state->has_scanned_docs = false;
    state->show_hud = true;

    doc_init(&state->doc);
    scroll_init(&state->scroll, 0, 226);
}

void app_state_set_mode(AppState *state, AppMode mode) {
    if (!state) return;
    state->prev_mode = state->mode;
    state->mode = mode;
}

void app_state_explorer_move(AppState *state, int delta) {
    if (!state || state->file_count == 0) return;

    state->selected_file_idx += delta;
    if (state->selected_file_idx < 0) {
        state->selected_file_idx = 0;
    } else if (state->selected_file_idx >= state->file_count) {
        state->selected_file_idx = state->file_count - 1;
    }

    /* Paginación en lista si hay más de 7 archivos visibles */
    const int max_visible = 7;
    if (state->selected_file_idx < state->explorer_scroll_top) {
        state->explorer_scroll_top = state->selected_file_idx;
    } else if (state->selected_file_idx >= state->explorer_scroll_top + max_visible) {
        state->explorer_scroll_top = state->selected_file_idx - max_visible + 1;
    }
}

void app_state_explorer_clear_files(AppState *state) {
    if (!state) return;
    state->file_count = 0;
    state->selected_file_idx = 0;
    state->explorer_scroll_top = 0;
}

void app_state_explorer_add_file(AppState *state, const char *name, uint32_t size_bytes) {
    if (!state || !name || state->file_count >= MAX_DOC_FILES) return;

    DocFileInfo *f = &state->files[state->file_count];
    strncpy(f->filename, name, sizeof(f->filename) - 1);
    f->filename[sizeof(f->filename) - 1] = '\0';
    f->size_bytes = size_bytes;

    state->file_count++;
}
