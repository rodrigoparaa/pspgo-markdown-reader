# 📖 PSP Go Markdown Reader (`02_MarkdownReader`)

[![Platform](https://img.shields.io/badge/Platform-Sony%20PSP%20Go%20%7C%20PSP--N1000-003791?style=for-the-badge&logo=playstation&logoColor=white)](https://en.wikipedia.org/wiki/PlayStation_Portable_Go)
[![Architecture](https://img.shields.io/badge/Architecture-MIPS%20Allegrex%20R4000%20%40%20333MHz-blue?style=for-the-badge)](https://en.wikipedia.org/wiki/PlayStation_Portable_hardware)
[![Language](https://img.shields.io/badge/Language-C99%20Pure-orange?style=for-the-badge&logo=c)](https://en.wikipedia.org/wiki/C99)
[![Graphics](https://img.shields.io/badge/Graphics-Hardware%20libpspgu%20%40%2060%20FPS-green?style=for-the-badge)](https://github.com/pspdev/pspsdk)
[![Toolchain](https://img.shields.io/badge/Toolchain-PSPDEV%20GCC%2015.2.0-blueviolet?style=for-the-badge)](https://github.com/pspdev/pspdev)
[![License](https://img.shields.io/badge/License-MIT-brightgreen?style=for-the-badge)](LICENSE)

> **A high-performance, distraction-free technical documentation reader engineered in pure C99 for the Sony PlayStation Portable Go (PSP-N1000).**  
> Features real-time streaming markdown parsing, kinetic inertial scrolling, handcrafted high-contrast themes, hardware-accelerated 60 FPS rendering via `libpspgu`, and full one-handed navigation in **Slider-Closed Mode**.

---

### 🌐 Language / Idioma
* **English** (Current Document)
* 🇪🇸 **[Documentación en Español (README_ES.md)](README_ES.md)**

---

## 📑 Table of Contents
1. [Project Overview](#-project-overview)
2. [Key Features](#-key-features)
3. [Slider-Closed Ergonomics](#-slider-closed-ergonomics)
4. [User Interface & Layout](#-user-interface--layout)
5. [Controls & Interaction Matrix](#-controls--interaction-matrix)
6. [Architecture & Subsystems](#-architecture--subsystems)
7. [Hardware Safety & Brick Prevention](#-hardware-safety--brick-prevention)
8. [Project Structure](#-project-structure)
9. [Toolchain & Building from Source](#-toolchain--building-from-source)
10. [Deployment & Installation](#-deployment--installation)
11. [Host Unit Testing Suite](#-host-unit-testing-suite)
12. [Technical Specifications](#-technical-specifications)
13. [License](#-license)

---

## 🎯 Project Overview

The **PSP Go Markdown Reader** transforms the Sony PSP Go into a dedicated pocket-sized e-reader for developers, engineers, and students. By eliminating all distractions and utilizing the hardware graphics engine (GE) of the Allegrex processor, it renders complex Markdown documents at a rock-solid **60 frames per second** with sub-pixel inertial physics.

Unlike web-based readers or PDF renderers that consume extensive memory and suffer from sluggish refresh rates, this engine is built specifically for embedded MIPS architecture with zero external dynamic runtime dependencies.

```
       +-------------------------------------------------------+
       | [MD] README.md                   45% | 14:30 | 98% [=] | <- 24px HUD
       +-------------------------------------------------------+
       |                                                       |
       |  # PSP Go Markdown Reader                             |
       |                                                       |
       |  A high-performance, distraction-free technical       |
       |  documentation reader engineered in pure C99.         |
       |                                                       |
       |  * 60 FPS VBLANK double-buffered graphics pipeline    |
       |  * Inertial analog stick physics with friction        |
       |  * Full one-handed reading with display folded        |
       |                                                       |
       +-------------------------------------------------------+
       | [L/R] Pag   [Stick] Scroll   [/\ ] Exit   [[]] Dark   | <- 22px Footer
       +-------------------------------------------------------+
```

---

## ✨ Key Features

* 🚀 **Hardware-Accelerated 60 FPS Pipeline:**
  * Uses official `libpspgu` with double buffering in embedded VRAM (eDRAM).
  * Strict synchronization with vertical blanking (`sceDisplayWaitVblankStart()`).
  * Zero screen tearing, zero visual stuttering, and zero bus contention.
* 📝 **Real-Time Streaming Markdown Parser:**
  * **Headings:** `# H1` (2x scale bitmap), `## H2` (2x scale bitmap), `### H3` (1x accent color).
  * **Fenced Code Blocks (```` ``` ````):** Dark container with borders and monospace terminal syntax styling.
  * **Blockquotes (`>`):** Dedicated left accent bar (3 px width) and differentiated palette.
  * **Lists (`-`, `*`, `+`):** Geometric bullet glyphs with automatic 14 px margin indentation.
  * **Horizontal Dividers (`---`, `***`):** Clean single-pixel semantic separation rules.
  * **Word-Wrapping Engine:** Dynamic character measurement splitting lines at safe boundaries without breaking words across rows (up to 55 chars/row for 448 px usable width).
* 🔤 **Robust UTF-8 Sanitizer & Typography:**
  * Maps 2-byte UTF-8 Spanish/Latin characters (`á`, `é`, `í`, `ó`, `ú`, `ñ`, `Á`, `É`, `¿`, `¡`) directly to clean ASCII glyphs.
  * Automatically filters multi-byte emoji sequences (`📖`, `🌟`, `🧪`) without leaving stray question marks (`????`).
  * Cleans inline Markdown formatting (`**bold**`, `__bold__`, `` `inline code` ``) while strictly preserving intra-word underscores in programming identifiers and filenames (`snake_case`, `02_MarkdownReader.md`).
* ⚡ **Kinetic Inertial Scroll Physics:**
  * Non-linear analog stick response curve with quadratic acceleration and 22-unit deadzone filtering.
  * Exponential friction decay ($\mu = 0.88$) for smooth deceleration.
  * Instant page paging via Shoulder Triggers (**L** / **R**) with 2-line context overlap.
  * Clamping math with a 90 px end-of-document breathing cushion to ensure the final lines never get hidden behind the bottom bar.
* 🎨 **3 Handcrafted High-Contrast Themes:**
  * **Dark Cyberpunk:** Deep midnight blue `#0F0F14`, soft white `#E0E6ED`, vivid cyan accent `#20AADD`.
  * **Sepia Paper:** Warm eye-friendly parchment `#F4ECD8`, coffee ink `#3E3123`, terracotta headers `#B84A28`.
  * **OLED High-Contrast:** Pitch black `#000000`, pure white `#FFFFFF`, bright gold accent `#F0B020`.
* 📁 **Embedded File Explorer:**
  * Reads directly from internal eMMC NAND (`ef0:/DOCS/*.md`) and Memory Stick Micro (`ms0:/DOCS/*.md`).
  * Displays file size in KB/bytes, scrollbar indicators, and active file highlights.
* 🔋 **System Telemetry & Power Management:**
  * Real-time battery gauge with dynamic charging indicator (`+` / lightning bolt).
  * System RTC clock with automatic **12-hour (AM/PM)** vs **24-hour** preference detection via `libpsputility`.
  * Integrated `scePowerTick(PSP_POWER_TICK_ALL)` keep-awake heartbeat to prevent sleep timeouts during extended reading sessions.

---

## 📱 Slider-Closed Ergonomics

The Sony PSP Go features an ultra-portable slider design. One of the core design goals of this project was to make reading comfortable when the display is slid down (folded):

```
+------------------------------------------+
|  [L Trigger]                [R Trigger]  |
|  (Page Back)                (Page Next)  |
|                                          |
|         +----------------------+         |
|         |                      |         |
|         |      PSP GO LCD      |         |
|         |    (SLIDER CLOSED)   |         |
|         |                      |         |
|         +----------------------+         |
|                                          |
|               [HOME / PS]                |
+------------------------------------------+
```

### Supported Gestures in Slider-Closed Mode:
1. **Single Hand Grip:** Hold the console comfortably in one hand like a pocket e-reader.
2. **Page Forward:** Tap the **R Shoulder Trigger** to advance one full page of text.
3. **Page Backward:** Tap the **L Shoulder Trigger** to revisit previous text.
4. **Theme Switch:** Hold both **L + R triggers simultaneously for >1.0 second**. The theme cycles instantly, with an anti-trigger latch that prevents accidental page jumps when releasing.
5. **System Exit:** Press the dedicated **PS / HOME** hardware button to cleanly exit to the Sony XMB.

---

## 🖥️ User Interface & Layout

The interface is engineered pixel-by-pixel for the $480 \times 272$ native resolution:

### 1. File Explorer (`screen_render_explorer`)
* **Header (24 px):** App Title on left, System RTC Time, Battery % and Hardware Battery Icon on right.
* **File List Container:** Shows up to 7 documents simultaneously with file sizes and an active cursor.
* **Side Scrollbar:** Proportional thumb tracking scroll position across large directories.
* **Footer (22 px):** Quick action shortcuts (`[X] Open`, `[^v] Move`, `[[]] Theme`, `[START] Help`).

### 2. Document Reader (`screen_render_reader`)
* **Header (24 px):** Document title (auto-truncated with ellipsis if long), reading progress percentage (`100%`), RTC clock, and battery monitor.
* **Reading Viewport (226 px):** Generous 16 px lateral margins (`CONTENT_MARGIN_X`) with 448 px usable text width.
* **Vertical Clipping:** Eliminates drawing operations for off-screen lines, maintaining 60 FPS regardless of document length.
* **HUD Toggle:** Press **Circle ($\bigcirc$)** to toggle headers/footers for a true full-screen distraction-free reading experience.

### 3. Dual-Column Help Modal (`screen_render_help`)
* Takes full advantage of the 16:9 widescreen display.
* **Column 1 (Left):** Open Console controls (Analog stick, D-Pad, face buttons).
* **Column 2 (Right):** Closed Slider controls (Triggers, L+R combo, system navigation).

---

## 🎮 Controls & Interaction Matrix

| Input Action | Open Console (Active Mode) | Closed Console (Pocket Mode) |
| :--- | :--- | :--- |
| **Analog Stick** | Kinetic inertial smooth scrolling | *Protected under slider* |
| **D-Pad Up / Down** | Line-by-line fine navigation | *Protected under slider* |
| **Right Trigger (R)** | Advance full page down (with overlap) | Advance full page down |
| **Left Trigger (L)** | Advance full page up | Advance full page up |
| **Combo L + R (>1s)** | Cycle theme (Dark $\to$ Sepia $\to$ OLED) | Cycle theme (Dark $\to$ Sepia $\to$ OLED) |
| **Square ($\square$)** | Cycle color themes instantly | *Protected under slider* |
| **Triangle ($\Delta$)** | Return to File Explorer (`ef0:/DOCS`) | *Protected under slider* |
| **Circle ($\bigcirc$)** | Toggle HUD (Full-screen mode) | *Protected under slider* |
| **Cross ($\times$)** | Select / Open document in Explorer | *Protected under slider* |
| **Start** | Toggle Interactive Controls Guide | *Protected under slider* |
| **PS / HOME Button** | Return to Sony XMB | Return to Sony XMB |

---

## 🏗️ Architecture & Subsystems

The codebase follows a strict 3-tier modular architecture designed for maintainability and testability:

```
MarkdownReader/
├── core/                  # Pure C99 logic - Zero SDK dependencies (Compiles on Host)
│   ├── app_state.h/.c     # Finite state machine (Explorer, Reader, Help)
│   ├── md_parser.h/.c     # Markdown classifier, word wrapper & UTF-8 sanitizer
│   ├── scroll_physics.h/.c# Kinetic friction & quadratic analog physics
│   └── theme_engine.h/.c  # Color palettes, theme cycling & definitions
│
├── ui/                    # Presentation layer (libpspgu & rendering)
│   ├── font.h/.c          # 8x8 bitmap ASCII font & 7-segment digital glyphs
│   ├── graphics.h/.c      # Double buffering, primitives, cards, battery icons
│   └── screens.h/.c       # Explorer view, Reader view & Help dialog
│
├── platform/              # Hardware abstraction layer (PSP SDK)
│   ├── psp_fs.h/.c        # ef0:/ and ms0:/ file discovery & buffered I/O
│   ├── psp_input.h/.c     # sceCtrl polling, debounce & gesture detection
│   └── psp_power.h/.c     # libpsppower & libpsprtc telemetry & keep-awake
│
├── tests/                 # Native host unit testing suite
│   └── test_core.c        # Standalone verification executable (Clang/GCC)
│
├── docs_sample/           # Sample markdown documents for testing
├── Makefile               # Official PSPDEV build system
├── activate.sh            # Portable isolated environment activation script
└── main.c                 # Application entry point, memory pool & main loop
```

### Memory Map & Allocation Strategy
To prevent stack overflows and erratic bus crashes on the MIPS Allegrex architecture:
* **Heap Space:** Allocated at **20 MB** (`PSP_HEAP_SIZE_KB(20480)`) in 64 MB DDR SDRAM.
* **Thread Stack:** Configured at **512 KB** (`PSP_MAIN_THREAD_STACK_SIZE_KB(512)`).
* **Static BSS Allocation:** The main application state (`AppState s_app_state`) is allocated in the `.bss` section rather than on the thread stack, avoiding stack boundary violations during deep Markdown parsing.
* **VRAM Double Buffering:**
  * Front Buffer: `0x00000000` (Stride: 512 words, 32-bit RGBA 8888).
  * Back Buffer: `0x00088000` (Offset 512 x 272 x 4 = 557,056 bytes).

---

## ⚠️ Hardware Safety & Brick Prevention

When developing homebrew for the Sony PSP Go (PSP-N1000), strict safety rules must be adhered to:

### 1. Permanent Brick Prevention: No CIPL Flasher
> [!CAUTION]
> **NEVER flash or execute `CIPL_Flasher` on a PSP Go (PSP-N1000).**  
> Classic PSP models (1000, and early 2000) have their Initial Program Loader (IPL) stored in a physical NAND flash partition that can be patched with custom IPL code.  
> The **PSP Go does NOT support Classic IPL**. Flashing CIPL to a PSP Go will **permanently and irreversibly destroy (brick) the motherboard**.  
> This project uses standard user-mode homebrew packaging (`pack-pbp`) compatible with **6.60 PROMOD-C2p + Infinity**, which is 100% safe.

### 2. FAT32 AppleDouble Metadata Cleanup
> [!IMPORTANT]
> When copying files from macOS to the PSP Go over USB, macOS automatically writes hidden resource fork files (prefixed with `._`, such as `._EBOOT.PBP`).  
> The Sony XMB parser treats these hidden files as corrupt executables, causing the dreaded **"Datos dañados" (Corrupted Data)** icon.  
> Always run `dot_clean` on the volume before ejecting:
> ```bash
> dot_clean "/Volumes/PSP GO/PSP/GAME"
> dot_clean "/Volumes/PSP GO/DOCS"
> sync
> ```

---

## 🧰 Toolchain & Building from Source

### Prerequisites
1. **PSPDEV Toolchain:** A working installation of [PSPDEV](https://github.com/pspdev/pspdev) containing `psp-gcc` (GCC 15.2+), `psp-fixup-imports`, `mksfoex`, and `pack-pbp`.
2. **Host Compiler:** `clang` or `gcc` on macOS / Linux to run the native test suite.

### Step 1: Activate the Environment
If you have a local `pspdev` directory or a global `$PSPDEV` installation:
```bash
# Clone the repository
git clone https://github.com/your-username/pspgo-markdown-reader.git
cd pspgo-markdown-reader

# Activate the isolated virtual environment
source activate.sh
```

### Step 2: Run Host Unit Tests
Before compiling for MIPS hardware, verify core logic natively:
```bash
cd tests
clang -Wall -Wextra -O2 -I../core ../core/*.c test_core.c -o run_tests
./run_tests
cd ..
```
*Output:*
```text
==================================================
🧪 SUITE DE PRUEBAS UNITARIAS: PSP Go Markdown Reader
==================================================
[TEST] Verificando Theme Engine...
  -> Theme Engine OK.
[TEST] Verificando Markdown Parser y Word-Wrapping...
  -> Parser y Layout OK (Generadas 35 líneas, alto total 435 px).
[TEST] Verificando Sanitizacion UTF-8 (Tildes en espanol, Emojis y Marcadores)...
  -> Sanitizacion UTF-8 y Padding OK.
[TEST] Verificando Fisica Inercial de Scroll...
  -> Scroll Physics OK.
[TEST] Verificando Maquina de Estados y Explorador...
  -> App State OK.
==================================================
✅ TODAS LAS PRUEBAS DEL CORE PASARON SATISFACTORIAMENTE
==================================================
```

### Step 3: Compile the PSP Binary
```bash
# Clean previous objects and build EBOOT.PBP
make clean
make
```
This produces the final standalone signed homebrew file: `EBOOT.PBP` (~152 KB).

---

## 🚀 Deployment & Installation

1. Turn on your PSP Go and connect it to your computer via USB (**CFW 6.60 PRO / Infinity**).
2. The internal 16 GB storage will mount at `/Volumes/PSP GO` (macOS) or as a removable drive letter (Windows/Linux).
3. Create the target folders if they do not already exist:
   * Homebrew Directory: `ef0:/PSP/GAME/MarkdownReader/`
   * Documents Directory: `ef0:/DOCS/`
4. Copy the compiled application and your Markdown notes:
   ```bash
   # Copy binary
   cp EBOOT.PBP "/Volumes/PSP GO/PSP/GAME/MarkdownReader/EBOOT.PBP"

   # Copy documentation files
   cp docs_sample/*.md "/Volumes/PSP GO/DOCS/"
   ```
5. **Clean macOS filesystem artifacts and flush buffers:**
   ```bash
   dot_clean "/Volumes/PSP GO/PSP/GAME"
   dot_clean "/Volumes/PSP GO/DOCS"
   sync
   ```
6. Disconnect USB on the PSP Go, navigate to **Juego (Game) $\to$ Almacenamiento del Sistema (System Storage)**, and launch **PSP Go Markdown Reader**.

---

## 📊 Technical Specifications

| Parameter | Specification | Notes |
| :--- | :--- | :--- |
| **Target Device** | Sony PlayStation Portable Go (PSP-N1000) | Compatible with PSP-1000/2000/3000/Street |
| **Operating System / CFW**| 6.60 PROMOD-C2p + Infinity 2.0 | User mode execution |
| **CPU Architecture** | MIPS Allegrex R4000 32-bit Little-Endian | Clocked at 333 MHz |
| **Display Resolution** | $480 \times 272$ pixels @ 60.00 Hz | 16:9 Aspect Ratio TFT LCD |
| **Color Depth** | 32-bit RGBA 8888 (`0xAABBGGRR`) | 16.7 million true colors |
| **Graphics API** | Sony `libpspgu` Hardware Engine | Hardware Double-buffered VRAM |
| **Memory Consumption** | ~2.4 MB RAM runtime | 20 MB heap reservation |
| **Binary Footprint** | ~152 KB (`EBOOT.PBP`) | Packed with stripped ELF & SFO header |
| **Document Storage** | `ef0:/DOCS/` (Internal NAND) / `ms0:/DOCS/` | Plain text UTF-8 `.md` files |
| **Scroll Latency** | $<16.66\text{ ms}$ per frame (1 frame latency) | Synchronized to hardware VBLANK |

---

## 📄 License

This project is licensed under the **MIT License**. You are free to inspect, modify, distribute, and integrate this software into other homebrew projects. See the [LICENSE](LICENSE) file for full details.

---

### Acknowledgments & References
* **[PSPDEV Community](https://github.com/pspdev):** For maintaining modern GCC toolchains for legacy hardware.
* **Sony Computer Entertainment:** For engineering one of the greatest portable consoles in history.
