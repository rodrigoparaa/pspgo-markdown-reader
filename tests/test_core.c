#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../core/theme_engine.h"
#include "../core/md_parser.h"
#include "../core/scroll_physics.h"
#include "../core/app_state.h"

static void test_themes(void) {
    printf("[TEST] Verificando Theme Engine...\n");
    theme_init();
    assert(theme_get_current_type() == THEME_DARK);

    const Theme *t1 = theme_get_current();
    assert(t1 != NULL);
    assert(strcmp(t1->name, "Dark Cyberpunk") == 0);

    theme_cycle_next();
    assert(theme_get_current_type() == THEME_SEPIA);
    assert(strcmp(theme_get_current()->name, "Sepia Paper") == 0);

    theme_cycle_next();
    assert(theme_get_current_type() == THEME_OLED);
    assert(strcmp(theme_get_current()->name, "OLED High-Contrast") == 0);

    theme_cycle_next();
    assert(theme_get_current_type() == THEME_DARK);

    printf("  -> Theme Engine OK.\n");
}

static void test_parser(void) {
    printf("[TEST] Verificando Markdown Parser y Word-Wrapping...\n");
    static Document doc;

    const char *sample_md =
        "# Guia de PSP Go\n"
        "\n"
        "Este es un parrafo normal de prueba que contiene suficiente texto como para verificar "
        "que el motor de maquetacion realiza un ajuste de linea (word wrapping) limpio y fluido "
        "sin cortar palabras a la mitad ni desbordar el margen util de la pantalla TFT LCD.\n"
        "\n"
        "## Arquitectura de Hardware\n"
        "\n"
        "### Especificaciones Clave\n"
        "- Procesador MIPS R4000 a 333 MHz con coprocesador VFPU\n"
        "- 16 GB de memoria eMMC interna montada en ef0:/\n"
        "- Pantalla LCD 480x272 a 60 Hz con paso de linea de 512 palabras\n"
        "\n"
        "--- \n"
        "\n"
        "> La sencillez y la estabilidad a 60 FPS son la maxima prioridad de diseno.\n"
        "\n"
        "```c\n"
        "void init_hardware(void) {\n"
        "    sceGuInit();\n"
        "    sceGuDrawBuffer(GU_PSM_8888, 0, 512);\n"
        "}\n"
        "```\n";

    bool ok = doc_parse_markdown(&doc, sample_md, strlen(sample_md), "Fallback Title");
    assert(ok == true);
    assert(doc.is_loaded == true);
    assert(doc.line_count > 10);
    assert(doc.total_height_px > 100);
    assert(strcmp(doc.title, "Guia de PSP Go") == 0);

    /* Verificar tipos de líneas presentes */
    bool found_h1 = false;
    bool found_h2 = false;
    bool found_h3 = false;
    bool found_list = false;
    bool found_hrule = false;
    bool found_quote = false;
    bool found_code = false;

    for (int i = 0; i < doc.line_count; i++) {
        LayoutLine *l = &doc.lines[i];
        assert((int)strlen(l->text) < MAX_LINE_TEXT_LEN);
        assert(l->height >= 0);

        if (l->line_type == LINE_H1) found_h1 = true;
        if (l->line_type == LINE_H2) found_h2 = true;
        if (l->line_type == LINE_H3) found_h3 = true;
        if (l->line_type == LINE_LIST) found_list = true;
        if (l->line_type == LINE_HRULE) found_hrule = true;
        if (l->line_type == LINE_QUOTE) found_quote = true;
        if (l->line_type == LINE_CODE_BODY) found_code = true;
    }

    assert(found_h1);
    assert(found_h2);
    assert(found_h3);
    assert(found_list);
    assert(found_hrule);
    assert(found_quote);
    assert(found_code);

    printf("  -> Parser y Layout OK (Generadas %d líneas, alto total %d px).\n",
           doc.line_count, doc.total_height_px);
}

static void test_utf8_sanitization(void) {
    printf("[TEST] Verificando Sanitizacion UTF-8 (Tildes en espanol, Emojis y Marcadores)...\n");
    static Document doc;

    const char *test_md =
        "# 📖 PSP Go Markdown Reader (`02_MarkdownReader`)\n"
        "\n"
        "> **Lector de Documentación Técnica Distraction-Free a 60 FPS**  \n"
        "\n"
        "## 🌟 Características Principales\n"
        "- 🧪 Pruebas Unitarias en macOS Host\n"
        "- Ñandú, pingüino, camión, información y ¿¡preguntas!?\n";

    bool ok = doc_parse_markdown(&doc, test_md, strlen(test_md), "Fallback");
    assert(ok == true);
    assert(strcmp(doc.title, "PSP Go Markdown Reader (02_MarkdownReader)") == 0);

    /* Verificar que ninguna línea contiene '?' provenientes de tildes mal decodificadas */
    bool found_sanitized_nandu = false;
    for (int i = 0; i < doc.line_count; i++) {
        const char *t = doc.lines[i].text;
        /* No debe contener secuencia '??' ni '????' */
        assert(strstr(t, "??") == NULL);
        assert(strstr(t, "**") == NULL);
        if (strstr(t, "Nandu, pinguino, camion, informacion y ?!preguntas!?") != NULL) {
            found_sanitized_nandu = true;
        }
    }
    assert(found_sanitized_nandu);

    /* Verificar padding final de 90px para asegurar lectura hasta el final */
    assert(doc.line_count > 0);
    assert(doc.lines[doc.line_count - 1].line_type == LINE_EMPTY);
    assert(doc.lines[doc.line_count - 1].height == 90);

    printf("  -> Sanitizacion UTF-8 y Padding OK.\n");
}

static void test_scroll(void) {
    printf("[TEST] Verificando Fisica Inercial de Scroll...\n");
    ScrollState s;
    scroll_init(&s, 1000, 272);

    assert(scroll_get_render_y(&s) == 0);
    assert(scroll_get_percent(&s) == 0);

    /* Salto de página abajo (Gatillo R) */
    scroll_page_down(&s);
    assert(s.is_animating == true);
    assert(s.target_y == (272 - 32)); /* 240 */

    /* Simular 20 fotogramas de transición suave */
    for (int i = 0; i < 20; i++) {
        scroll_update(&s);
    }
    assert(scroll_get_render_y(&s) == 240);
    assert(s.is_animating == false);

    /* Stick analógico hacia abajo */
    scroll_apply_analog(&s, 100);
    assert(s.velocity_y > 0.0f);
    float prev_y = s.current_y;
    scroll_update(&s);
    assert(s.current_y > prev_y);

    /* Test de Clamping inferior */
    s.current_y = 2000.0f;
    s.target_y = 2000.0f;
    scroll_update(&s);
    int max_y = 1000 - 272; /* 728 */
    assert(scroll_get_render_y(&s) == max_y);
    assert(scroll_get_percent(&s) == 100);

    /* Test de Clamping superior */
    s.current_y = -50.0f;
    scroll_update(&s);
    assert(scroll_get_render_y(&s) == 0);
    assert(scroll_get_percent(&s) == 0);

    printf("  -> Scroll Physics OK.\n");
}

static void test_app_state(void) {
    printf("[TEST] Verificando Maquina de Estados y Explorador...\n");
    AppState state;
    app_state_init(&state);

    assert(state.mode == APP_STATE_EXPLORER);
    assert(state.file_count == 0);

    app_state_explorer_add_file(&state, "Doc1.md", 1024);
    app_state_explorer_add_file(&state, "Doc2.md", 2048);
    app_state_explorer_add_file(&state, "Doc3.md", 4096);

    assert(state.file_count == 3);
    assert(state.selected_file_idx == 0);

    app_state_explorer_move(&state, 1);
    assert(state.selected_file_idx == 1);
    app_state_explorer_move(&state, 1);
    assert(state.selected_file_idx == 2);
    app_state_explorer_move(&state, 1); /* Clamping final */
    assert(state.selected_file_idx == 2);

    app_state_set_mode(&state, APP_STATE_READER);
    assert(state.mode == APP_STATE_READER);

    printf("  -> App State OK.\n");
}

int main(void) {
    printf("==================================================\n");
    printf("🧪 SUITE DE PRUEBAS UNITARIAS: PSP Go Markdown Reader\n");
    printf("==================================================\n");

    test_themes();
    test_parser();
    test_utf8_sanitization();
    test_scroll();
    test_app_state();

    printf("==================================================\n");
    printf("✅ TODAS LAS PRUEBAS DEL CORE PASARON SATISFACTORIAMENTE\n");
    printf("==================================================\n");
    return 0;
}
