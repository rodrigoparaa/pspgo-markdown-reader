# Toolchain Setup & Compilation Guide
## PSP Go Modern Lab (`02_MarkdownReader`)

This guide explains how to set up the development environment, compile the project from source code, execute the native host unit testing suite, and deploy the application onto physical Sony PSP Go hardware.

---

## Table of Contents
1. [Toolchain Overview](#1-toolchain-overview)
2. [Prerequisites & Host Dependencies](#2-prerequisites--host-dependencies)
3. [Setting Up the PSPDEV Environment](#3-setting-up-the-pspdev-environment)
4. [Using the Virtual Environment (`activate.sh`)](#4-using-the-virtual-environment-activatesh)
5. [Running Host Unit Tests](#5-running-host-unit-tests)
6. [Compiling the PSP Binary (`EBOOT.PBP`)](#6-compiling-the-psp-binary-ebootpbp)
7. [Deploying to Hardware (USB Transfer)](#7-deploying-to-hardware-usb-transfer)
8. [Troubleshooting & FAQs](#8-troubleshooting--faqs)

---

## 1. Toolchain Overview

The project is built using modern **PSPDEV** toolchains:
* **Target Architecture:** MIPS Allegrex R4000 (`mips-allegrex-psp-elf`).
* **Cross-Compiler:** GCC 15.2.0 (with C99 support).
* **Standard C Library:** Newlib.
* **Sony PSP SDK:** Official open-source PSPSDK providing `libpspgu`, `libpsppower`, `libpsprtc`, and `libpsputility`.
* **Packaging Utilities:** `pack-pbp` and `mksfoex`.

---

## 2. Prerequisites & Host Dependencies

### macOS (Apple Silicon & Intel)
Install host build dependencies via Homebrew:
```bash
brew install git make cmake gmp mpfr libmpc
```

### Linux (Debian / Ubuntu / Pop!_OS)
```bash
sudo apt update
sudo apt install -y build-essential cmake git libgmp-dev libmpfr-dev libmpc-dev
```

---

## 3. Setting Up the PSPDEV Environment

You can install PSPDEV either as a pre-built binary distribution or compile it from source.

### Option A: Local Portable Folder (Recommended)
Place the compiled toolchain inside the repository root as a `pspdev/` directory (or symlink):
```bash
# Path structure
repo_root/
├── pspdev/
│   ├── bin/           # psp-gcc, pack-pbp, mksfoex, etc.
│   ├── psp/           # Libraries and SDK headers
│   └── lib/
```
The included `activate.sh` script automatically detects `repo_root/pspdev`.

### Option B: Global System Installation
If you installed PSPDEV globally (e.g. `/usr/local/pspdev` or `/opt/pspdev`), export the environment variable:
```bash
export PSPDEV="/usr/local/pspdev"
export PATH="$PSPDEV/bin:$PATH"
```

### Option C: Docker Container
If you prefer building inside a container:
```bash
docker run -it --rm -v $(pwd):/workspace -w /workspace pspdev/pspdev:latest bash -c "make clean && make"
```

---

## 4. Using the Virtual Environment (`activate.sh`)

Similar to Python's `virtualenv`, this project provides an isolated environment activator script that modifies your `$PATH` and `$PSPDEV` without polluting global system paths.

### Activating:
```bash
source activate.sh
```
*Console output:*
```text
==========================================================
🎮 PSPDEV Environment Activated
   Compiler: psp-gcc (GCC) 15.2.0
   SDK Path: /path/to/pspdev
   Exit:     deactivate_psp
==========================================================
(psp-env) $
```

### Deactivating:
When finished, cleanly restore your shell state:
```bash
deactivate_psp
```

---

## 5. Running Host Unit Tests

Before compiling for MIPS hardware, verify core logic natively on your host machine (macOS or Linux). The `core/` directory contains pure C99 code decoupled from Sony hardware headers:

```bash
cd tests
clang -Wall -Wextra -O2 -I../core ../core/*.c test_core.c -o run_tests
./run_tests
cd ..
```

### What the Test Suite Verifies:
1. **`test_themes`:** Theme engine initialization and cycling (Dark Cyberpunk $\to$ Sepia Paper $\to$ OLED High-Contrast).
2. **`test_parser`:** Layout generation, vertical height calculation, line classification (H1, H2, H3, code blocks, lists, quotes, rules), and word-wrapping.
3. **`test_utf8_sanitization`:** Decoding 2-byte Spanish accented vowels (`á`, `é`, `í`, `ó`, `ú`, `ñ`), multi-byte emoji stripping (`📖`, `🌟`, `🧪`), markdown marker stripping, and CommonMark intra-word underscore preservation (`snake_case`).
4. **`test_scroll`:** Analog deadzone filtering, quadratic acceleration response, inertial decay, and clamping boundaries.
5. **`test_app_state`:** Navigation state machine and file list bounds clamping.

---

## 6. Compiling the PSP Binary (`EBOOT.PBP`)

With the PSPDEV environment activated:

```bash
make clean
make
```

### Build Pipeline Steps:
1. `psp-gcc -c`: Compiles each C file with `-O2 -G0 -Wall -Wextra -D_PSP_FW_VERSION=600`.
2. `psp-gcc -o markdown_reader.elf`: Links object files against `-lpspgu -lpsppower -lpsprtc -lpsputility -lm`.
3. `psp-fixup-imports`: Fixes SDK module import stubs in the ELF executable.
4. `mksfoex`: Generates the `PARAM.SFO` system descriptor with `MEMSIZE=2` (enabling full 64 MB RAM access).
5. `psp-strip`: Strips debugging symbols to reduce file size.
6. `pack-pbp`: Packages the stripped ELF and `PARAM.SFO` into the final executable container: `EBOOT.PBP` (~152 KB).

---

## 7. Deploying to Hardware (USB Transfer)

1. Connect your PSP Go to your computer with the USB cable.
2. In the PSP XMB menu, select **Ajustes (Settings) $\to$ Conexión USB (USB Connection)**.
3. The internal 16 GB eMMC flash storage will mount at `/Volumes/PSP GO` (macOS) or as a removable drive (Windows/Linux).
4. Copy the binary and test documents:
   ```bash
   # Create directories if they do not exist
   mkdir -p "/Volumes/PSP GO/PSP/GAME/MarkdownReader"
   mkdir -p "/Volumes/PSP GO/DOCS"

   # Copy application and markdown notes
   cp EBOOT.PBP "/Volumes/PSP GO/PSP/GAME/MarkdownReader/EBOOT.PBP"
   cp docs_sample/*.md "/Volumes/PSP GO/DOCS/"
   ```
5. **Mandatory macOS Cleanup:**
   ```bash
   dot_clean "/Volumes/PSP GO/PSP/GAME"
   dot_clean "/Volumes/PSP GO/DOCS"
   sync
   ```
6. Press **Circle ($\bigcirc$)** to disconnect USB, navigate to **Juego (Game) $\to$ Almacenamiento del Sistema**, and launch the app.

---

## 8. Troubleshooting & FAQs

### Q: Why does the PSP XMB show "Datos dañados" (Corrupted Data)?
* **Cause:** macOS wrote hidden AppleDouble files (`._*`) onto the FAT32 volume.
* **Fix:** Run `dot_clean "/Volumes/PSP GO/PSP/GAME"` and `sync` before disconnecting.

### Q: Why does the screen stay black upon launch?
* **Cause:** The thread stack was too small, or an unhandled hardware exception occurred during startup.
* **Fix:** The codebase already defines `PSP_MAIN_THREAD_STACK_SIZE_KB(512)` and `PSP_HEAP_SIZE_KB(20480)` in `main.c`, and allocates the application state in `.bss`. Recompile with `make clean && make`.

### Q: Can I run this on a standard PSP-1000, 2000, 3000, or PSP Street (E1000)?
* **Yes!** The application is 100% compatible with all PSP hardware revisions running Custom Firmware (6.60 or 6.61 PRO / ME). It automatically detects `ms0:/DOCS/` if `ef0:/DOCS/` is not present.
