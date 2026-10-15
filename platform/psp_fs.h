#ifndef PSP_FS_H
#define PSP_FS_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include "../core/app_state.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_DOCS_DIR "ef0:/DOCS"

/* Inicializa el sistema de archivos y asegura que ef0:/DOCS exista */
void psp_fs_init(void);

/* Escanea ef0:/DOCS en busca de archivos .md */
int psp_fs_scan_directory(const char *dir_path, DocFileInfo *out_files, int max_files);

/* Carga un archivo en memoria (retorna buffer reservado en heap con null-terminator) */
bool psp_fs_read_file(const char *full_path, char **out_buffer, size_t *out_size);

/* Libera el búfer obtenido con psp_fs_read_file */
void psp_fs_free_buffer(char *buffer);

#ifdef __cplusplus
}
#endif

#endif /* PSP_FS_H */
