#include "psp_fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#if defined(__PSP__) || defined(PSP)
#include <pspkernel.h>
#include <pspiofilemgr.h>

void psp_fs_init(void) {
    /* Crear ef0:/DOCS si no existe aún */
    sceIoMkdir(PSP_DOCS_DIR, 0777);
}

static bool has_markdown_extension(const char *name) {
    size_t len = strlen(name);
    if (len >= 3) {
        if (strcasecmp(name + len - 3, ".md") == 0) return true;
    }
    if (len >= 4) {
        if (strcasecmp(name + len - 4, ".txt") == 0) return true;
    }
    return false;
}

int psp_fs_scan_directory(const char *dir_path, DocFileInfo *out_files, int max_files) {
    if (!dir_path || !out_files || max_files <= 0) return 0;

    SceUID dfd = sceIoDopen(dir_path);
    if (dfd < 0) {
        char alt_path[64];
        snprintf(alt_path, sizeof(alt_path), "%s/", dir_path);
        dfd = sceIoDopen(alt_path);
    }
    if (dfd < 0) {
        if (strncmp(dir_path, "ef0:", 4) == 0) {
            char ms_path[64];
            snprintf(ms_path, sizeof(ms_path), "ms0:%s", dir_path + 4);
            dfd = sceIoDopen(ms_path);
            if (dfd < 0) {
                snprintf(ms_path, sizeof(ms_path), "ms0:%s/", dir_path + 4);
                dfd = sceIoDopen(ms_path);
            }
        } else if (strncmp(dir_path, "ms0:", 4) == 0) {
            char ef_path[64];
            snprintf(ef_path, sizeof(ef_path), "ef0:%s", dir_path + 4);
            dfd = sceIoDopen(ef_path);
            if (dfd < 0) {
                snprintf(ef_path, sizeof(ef_path), "ef0:%s/", dir_path + 4);
                dfd = sceIoDopen(ef_path);
            }
        }
    }
    if (dfd < 0) {
        return 0;
    }

    int count = 0;
    SceIoDirent entry;
    memset(&entry, 0, sizeof(entry));

    while (count < max_files && sceIoDread(dfd, &entry) > 0) {
        if (entry.d_name[0] == '.') {
            continue; /* Ignorar . , .. y archivos ocultos de macOS ._* */
        }

        if (FIO_SO_ISDIR(entry.d_stat.st_attr)) {
            continue; /* Ignorar subcarpetas por ahora */
        }

        if (has_markdown_extension(entry.d_name)) {
            DocFileInfo *f = &out_files[count];
            strncpy(f->filename, entry.d_name, sizeof(f->filename) - 1);
            f->filename[sizeof(f->filename) - 1] = '\0';
            f->size_bytes = (uint32_t)entry.d_stat.st_size;
            count++;
        }
    }

    sceIoDclose(dfd);

    /* Ordenar alfabéticamente por nombre */
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (strcasecmp(out_files[i].filename, out_files[j].filename) > 0) {
                DocFileInfo temp = out_files[i];
                out_files[i] = out_files[j];
                out_files[j] = temp;
            }
        }
    }

    return count;
}

bool psp_fs_read_file(const char *full_path, char **out_buffer, size_t *out_size) {
    if (!full_path || !out_buffer || !out_size) return false;

    *out_buffer = NULL;
    *out_size = 0;

    SceUID fd = sceIoOpen(full_path, PSP_O_RDONLY, 0777);
    if (fd < 0) {
        if (strncmp(full_path, "ef0:", 4) == 0) {
            char alt[128];
            snprintf(alt, sizeof(alt), "ms0:%s", full_path + 4);
            fd = sceIoOpen(alt, PSP_O_RDONLY, 0777);
        } else if (strncmp(full_path, "ms0:", 4) == 0) {
            char alt[128];
            snprintf(alt, sizeof(alt), "ef0:%s", full_path + 4);
            fd = sceIoOpen(alt, PSP_O_RDONLY, 0777);
        }
    }
    if (fd < 0) {
        return false;
    }

    SceOff file_size = sceIoLseek(fd, 0, PSP_SEEK_END);
    sceIoLseek(fd, 0, PSP_SEEK_SET);

    if (file_size <= 0 || file_size > (10 * 1024 * 1024)) { /* Límite de seguridad 10MB */
        sceIoClose(fd);
        return false;
    }

    char *buf = (char *)malloc((size_t)file_size + 1);
    if (!buf) {
        sceIoClose(fd);
        return false;
    }

    int bytes_read = sceIoRead(fd, buf, (int)file_size);
    sceIoClose(fd);

    if (bytes_read < 0) {
        free(buf);
        return false;
    }

    buf[bytes_read] = '\0';
    *out_buffer = buf;
    *out_size = (size_t)bytes_read;
    return true;
}

void psp_fs_free_buffer(char *buffer) {
    if (buffer) {
        free(buffer);
    }
}

#else
/* Implementación Host (macOS/POSIX) para pruebas y compilación local */
#include <dirent.h>
#include <sys/stat.h>

void psp_fs_init(void) {}

static bool has_markdown_extension(const char *name) {
    size_t len = strlen(name);
    if (len >= 3 && strcasecmp(name + len - 3, ".md") == 0) return true;
    if (len >= 4 && strcasecmp(name + len - 4, ".txt") == 0) return true;
    return false;
}

int psp_fs_scan_directory(const char *dir_path, DocFileInfo *out_files, int max_files) {
    if (!dir_path || !out_files || max_files <= 0) return 0;
    DIR *dir = opendir(dir_path);
    if (!dir) return 0;

    int count = 0;
    struct dirent *entry;
    while (count < max_files && (entry = readdir(dir)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        if (has_markdown_extension(entry->d_name)) {
            DocFileInfo *f = &out_files[count];
            strncpy(f->filename, entry->d_name, sizeof(f->filename) - 1);
            f->filename[sizeof(f->filename) - 1] = '\0';
            f->size_bytes = 1024;
            count++;
        }
    }
    closedir(dir);
    return count;
}

bool psp_fs_read_file(const char *full_path, char **out_buffer, size_t *out_size) {
    FILE *f = fopen(full_path, "rb");
    if (!f) return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) { fclose(f); return false; }
    char *b = malloc(sz + 1);
    if (!b) { fclose(f); return false; }
    size_t r = fread(b, 1, sz, f);
    fclose(f);
    b[r] = '\0';
    *out_buffer = b;
    *out_size = r;
    return true;
}

void psp_fs_free_buffer(char *buffer) {
    if (buffer) free(buffer);
}
#endif
