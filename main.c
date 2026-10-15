#include <pspkernel.h>
#include <pspdisplay.h>
#include <psprtc.h>
#include <stdio.h>
#include <stdint.h>

#include "core/app_state.h"
#include "core/theme_engine.h"
#include "ui/graphics.h"
#include "ui/screens.h"
#include "platform/psp_fs.h"
#include "platform/psp_input.h"
#include "platform/psp_power.h"

PSP_MODULE_INFO("MarkdownReader", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(512);
PSP_HEAP_SIZE_KB(20480);

static volatile int g_running = 1;

/* Almacenamiento estático en sección .bss (RAM DDR de 64MB) para prevenir Stack Overflow */
static AppState s_app_state;

static int exit_callback(int arg1, int arg2, void *common) {
    (void)arg1; (void)arg2; (void)common;
    g_running = 0;
    sceKernelExitGame();
    return 0;
}

static int callback_thread(SceSize args, void *argp) {
    (void)args; (void)argp;
    int cbid = sceKernelCreateCallback("Exit Callback", exit_callback, NULL);
    sceKernelRegisterExitCallback(cbid);
    sceKernelSleepThreadCB();
    return 0;
}

static void setup_callbacks(void) {
    int thid = sceKernelCreateThread("cb_thread", callback_thread, 0x11, 0xFA0, 0, NULL);
    if (thid >= 0) {
        sceKernelStartThread(thid, 0, NULL);
    }
}

static uint64_t get_now_ms(void) {
    uint64_t tick;
    sceRtcGetCurrentTick(&tick);
    return tick / (sceRtcGetTickResolution() / 1000);
}

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;

    setup_callbacks();

    /* 1. Inicialización de subsistemas */
    gfx_init();
    theme_init();
    psp_fs_init();
    psp_input_init();
    psp_power_init();

    app_state_init(&s_app_state);

    /* 2. Escaneo inicial de documentos en ef0:/DOCS (con fallback a ms0:/DOCS) */
    s_app_state.file_count = psp_fs_scan_directory(PSP_DOCS_DIR, s_app_state.files, MAX_DOC_FILES);
    s_app_state.has_scanned_docs = true;

    uint64_t last_power_tick_ms = get_now_ms();

    /* 3. Bucle principal a 60 Hz sincronizado con VBLANK */
    while (g_running) {
        uint64_t now_ms = get_now_ms();

        /* Polling de mandos, gestos y atajos */
        psp_input_poll(&s_app_state, (uint32_t)now_ms);

        /* Actualización física inercial de lectura */
        if (s_app_state.mode == APP_STATE_READER) {
            scroll_update(&s_app_state.scroll);
        }

        /* Keep-Awake: Mantener encendida la pantalla durante lectura */
        if (now_ms - last_power_tick_ms >= 4000) {
            psp_power_tick();
            last_power_tick_ms = now_ms;
        }

        /* Renderizado con doble búfer garantizado cada fotograma */
        const Theme *t = theme_get_current();
        gfx_start_frame(t ? t->bg_color : 0xFF140F0F);

        switch (s_app_state.mode) {
            case APP_STATE_EXPLORER:
                screen_render_explorer(&s_app_state);
                break;

            case APP_STATE_READER:
                screen_render_reader(&s_app_state);
                break;

            case APP_STATE_HELP:
                screen_render_help(&s_app_state);
                break;
        }

        gfx_end_frame();
    }

    gfx_shutdown();
    return 0;
}
