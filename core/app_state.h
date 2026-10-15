#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include "md_parser.h"
#include "scroll_physics.h"
#include "theme_engine.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_DOC_FILES 64

typedef enum {
    APP_STATE_EXPLORER = 0,
    APP_STATE_READER,
    APP_STATE_HELP
} AppMode;

typedef struct {
    char filename[64];
    uint32_t size_bytes;
} DocFileInfo;

typedef struct {
    AppMode mode;
    AppMode prev_mode;

    /* Explorador de archivos */
    DocFileInfo files[MAX_DOC_FILES];
    int file_count;
    int selected_file_idx;
    int explorer_scroll_top;
    bool has_scanned_docs;

    /* Lector activo */
    Document doc;
    ScrollState scroll;
    char current_path[128];
    char current_filename[64];
    bool show_hud;

    /* Gestión de gestos y combo L+R */
    uint32_t l_press_start_ms;
    uint32_t r_press_start_ms;
    uint32_t lr_combo_start_ms;
    bool lr_combo_triggered;
    bool suppress_next_l_release;
    bool suppress_next_r_release;
} AppState;

void app_state_init(AppState *state);
void app_state_set_mode(AppState *state, AppMode mode);
void app_state_explorer_move(AppState *state, int delta);
void app_state_explorer_clear_files(AppState *state);
void app_state_explorer_add_file(AppState *state, const char *name, uint32_t size_bytes);

#ifdef __cplusplus
}
#endif

#endif /* APP_STATE_H */
