# 🏛️ Technical Architecture & System Design
## PSP Go Markdown Reader (`02_MarkdownReader`)

This document provides an exhaustive, low-level technical specification of the software architecture, memory layout, graphics pipeline, mathematical models, and embedded hardware constraints utilized in the **PSP Go Markdown Reader**.

---

## 📑 Table of Contents
1. [Architectural Overview](#1-architectural-overview)
2. [Memory Budget & Safety Blueprint](#2-memory-budget--safety-blueprint)
3. [Markdown Parser & Layout Engine](#3-markdown-parser--layout-engine)
4. [Kinetic Inertial Scroll Physics](#4-kinetic-inertial-scroll-physics)
5. [Hardware-Accelerated Graphics Pipeline](#5-hardware-accelerated-graphics-pipeline)
6. [Platform Abstraction Layer (HAL)](#6-platform-abstraction-layer-hal)
7. [Slider-Closed State Machine](#7-slider-closed-state-machine)
8. [Hardware Safety & Verification](#8-hardware-safety--verification)

---

## 1. Architectural Overview

The application follows a strictly decoupled **3-tier modular architecture**:

```
+--------------------------------------------------------------------------+
|                          MAIN APPLICATION LOOP                           |
|         (60 FPS VBLANK Cycle: Poll Input -> Update -> Render)            |
+--------------------------------------------------------------------------+
                                     |
         +---------------------------+---------------------------+
         |                           |                           |
         v                           v                           v
+------------------+       +------------------+       +--------------------+
|   CORE ENGINE    |       |  UI & GRAPHICS   |       |  PLATFORM DRIVERS  |
| (Pure C99 Logic) |       | (libpspgu & VRAM)|       |     (PSP SDK)      |
+------------------+       +------------------+       +--------------------+
| • app_state      |       | • graphics       |       | • psp_fs           |
| • md_parser      | <---> | • font_8x8       | <---> | • psp_input        |
| • scroll_physics |       | • screens        |       | • psp_power        |
| • theme_engine   |       | • color_palettes |       | • psp_rtc          |
+------------------+       +------------------+       +--------------------+
         |                           |                           |
         v                           v                           v
[ Host Unit Tests ]        [ Hardware GE eDRAM ]      [ MIPS Allegrex R4000]
 (Clang / Native)           (512-word Stride)           (ef0:/ NAND Storage)
```

### Architectural Principles:
1. **Host-Independent Core:** All files in `core/` contain zero Sony SDK headers. They compile natively on macOS, Linux, and Windows with standard Clang or GCC for instant unit testing and regression detection.
2. **Zero Dynamic Allocation During Frame Render:** Memory for the document model, layout lines, and state machine is pre-allocated at load time. The 60 FPS rendering loop performs zero calls to `malloc()` or `free()`.
3. **Hardware-Paced Execution:** Framerate pacing is driven strictly by vertical blank hardware interrupts (`sceDisplayWaitVblankStart()`), guaranteeing zero jitter and zero tearing.

---

## 2. Memory Budget & Safety Blueprint

The Sony PSP Go (PSP-N1000) features 64 MB of DDR SDRAM and 2 MB of embedded eDRAM inside the Graphics Engine (GE).

### 2.1. Memory Reservation Table

| Segment | Memory Region | Size | Rationale |
| :--- | :--- | :--- | :--- |
| **Heap Memory** | DDR SDRAM | 20,480 KB (20 MB) | Defined via `PSP_HEAP_SIZE_KB(20480)` for buffered document I/O. |
| **Main Thread Stack**| DDR SDRAM | 512 KB | Defined via `PSP_MAIN_THREAD_STACK_SIZE_KB(512)`. |
| **Application State**| DDR SDRAM (`.bss`)| ~148 KB | `static AppState s_app_state` allocated in BSS to prevent stack overflow. |
| **VRAM Framebuffer 0**| eDRAM (0x00000000) | 557,056 bytes | $512 \times 272 \times 4\text{ bytes}$ (32-bit RGBA 8888, stride 512). |
| **VRAM Framebuffer 1**| eDRAM (0x00088000) | 557,056 bytes | Second double-buffer surface in internal eDRAM. |
| **Binary Size** | Flash / RAM | 152,232 bytes | Stripped MIPS ELF packed inside signed `EBOOT.PBP`. |

### 2.2. Preventing Stack Overflow
A standard PSP user-mode thread default stack is only 4 KB to 16 KB. Parsing a multi-kilobyte Markdown document with hundreds of wrapped lines inside a local stack variable causes immediate memory corruption and kernel bus exceptions (`Exception: Bus error / Data bus error`).  
**Architectural Safeguards:**
* Thread stack enlarged to **512 KB**.
* Master `AppState` instantiated as a module-static structure in `.bss`:
  ```c
  static AppState s_app_state; // Zero stack overhead
  ```

---

## 3. Markdown Parser & Layout Engine

The parser in `core/md_parser.c` is a streaming classifier and maquetation engine designed for high throughput.

### 3.1. Parsing Pipeline

```
Raw File Buffer (ef0:/DOCS/*.md)
               |
               v
  [1. Line Extraction (CR/LF)]
               |
               v
  [2. Block Classifier] --------> Code Blocks (```), Headings (#), Quotes (>), Lists (-), HR (---)
               |
               v
  [3. UTF-8 Sanitizer] ---------> 2-byte Spanish mapping, Emoji filtering, Inline cleanup
               |
               v
  [4. Word-Wrapping Engine] ----> Splits at space boundaries for 448 px usable width
               |
               v
  [5. Document Layout Array] ---> Array of LayoutLine { y_offset, height, scale, indent, text }
```

### 3.2. UTF-8 Sanitizer & Spanish Character Decoding
Because the built-in monospace typography engine renders ASCII glyphs from 32 to 127, raw multi-byte UTF-8 sequences would normally map to `?`.

The sanitizer converts multibyte sequences in a single pass:
* **Spanish / Latin 2-Byte UTF-8 (`0xC3`):**
  * `0xA1` (`á`), `0xA0` (`à`), `0xA4` (`ä`) $\to$ `'a'`
  * `0xA9` (`é`), `0xA8` (`è`), `0xAB` (`ë`) $\to$ `'e'`
  * `0xAD` (`í`), `0xAC` (`ì`), `0xAF` (`ï`) $\to$ `'i'`
  * `0xB3` (`ó`), `0xB2` (`ò`), `0xB6` (`ö`) $\to$ `'o'`
  * `0xBA` (`ú`), `0xB9` (`ù`), `0xBC` (`ü`) $\to$ `'u'`
  * `0xB1` (`ñ`) $\to$ `'n'`
  * `0x81` (`Á`), `0x89` (`É`), `0x8D` (`Í`), `0x93` (`Ó`), `0x9A` (`Ú`), `0x91` (`Ñ`) $\to$ `'A'`, `'E'`, `'I'`, `'O'`, `'U'`, `'N'`
* **Spanish Inverted Punctuation (`0xC2`):**
  * `0xBF` (`¿`) $\to$ `'?'`
  * `0xA1` (`¡`) $\to$ `'!'`
* **3-Byte & 4-Byte Emojis:** Skipped completely (`p += 3` or `p += 4`), preventing `????` sequences from emojis like `📖`, `🌟`, `🧪`.

### 3.3. CommonMark Intra-Word Underscore Preservation
Standard markdown rules dictate that underscores inside programming identifiers must not trigger italics. The sanitizer checks boundary alphanumeric characters:
```c
if (c == '_') {
    bool prev_alnum = (out_idx > 0 && isalnum((unsigned char)dst[out_idx - 1]));
    bool next_alnum = (*p && isalnum((unsigned char)*p));
    if (prev_alnum && next_alnum) {
        /* Preserve identifier underscore (e.g. 02_MarkdownReader, snake_case) */
    } else {
        continue; /* Strip markdown marker */
    }
}
```

### 3.4. Viewport Height & Trailing Padding
* Screen Height: $272\text{ px}$.
* HUD Top Bar: $24\text{ px}$.
* HUD Bottom Bar: $22\text{ px}$.
* **Active Reading Viewport:** $272 - 24 - 22 = 226\text{ px}$.
* **Bottom Cushion:** A trailing line of type `LINE_EMPTY` with a vertical height of **90 px** is appended at document end. This guarantees that the final lines can be scrolled comfortably above the footer.

---

## 4. Kinetic Inertial Scroll Physics

The scrolling physics engine in `core/scroll_physics.c` implements continuous sub-pixel movement.

### 4.1. Quadratic Analog Input Curve
To allow both microscopic line-by-line reading and rapid document skimming without mode switching:
$$\text{norm} = \frac{|A_y| - \text{DEADZONE}}{128 - \text{DEADZONE}} \quad (\text{for } |A_y| > 22)$$
$$v_{\text{input}} = \text{sign}(A_y) \times \left(14.0 \times \text{norm}^2 + 2.0 \times \text{norm}\right)$$

* At low stick tilt ($\text{norm} \approx 0.1$): velocity is slow ($\approx 0.34\text{ px/frame}$) for precise reading.
* At full stick tilt ($\text{norm} = 1.0$): velocity accelerates smoothly to $16.0\text{ px/frame}$.

### 4.2. Exponential Friction Decay
When the user releases the stick:
$$v_{t+1} = v_t \times \mu \quad (\mu = 0.88)$$
When $|v_t| < 0.05\text{ px/frame}$, velocity snaps to $0.0$.

### 4.3. Smooth Page Jumps (L / R Triggers)
When the user presses **L** (page up) or **R** (page down):
$$\Delta y_{\text{page}} = \text{viewport\_height} - \text{PAGE\_OVERLAP\_PX} = 226 - 32 = 194\text{ px}$$
The camera transitions smoothly toward $y_{\text{target}}$ using an interpolation dampener:
$$y_{t+1} = y_t + (y_{\text{target}} - y_t) \times 0.35$$

---

## 5. Hardware-Accelerated Graphics Pipeline

The graphics system in `ui/graphics.c` communicates with the Sony PSP Graphics Engine (GE).

### 5.1. VRAM Configuration & Stride
* The PSP LCD controller requires line strides in powers of two or aligned multiples. The native resolution is $480 \times 272$, but VRAM stride is configured to **512 words** (2048 bytes per line).
* Buffers:
  ```c
  sceGuDrawBuffer(GU_PSM_8888, (void*)0, 512);
  sceGuDispBuffer(480, 272, (void*)0x88000, 512);
  ```

### 5.2. Double-Buffering Synchronization
Every frame adheres strictly to the display sequence:
```c
gfx_start_frame(t->bg_color); // Clear back buffer
// ... Draw markdown lines, cards, text, icons ...
gfx_end_frame();              // Flush GE list, sync VBLANK & swap buffers
```
Under the hood:
```c
sceGuFinish();
sceGuSync(0, 0);
sceDisplayWaitVblankStart();
sceGuSwapBuffers();
```

### 5.3. Vertical Line Clipping
In `ui/screens.c`, lines outside the visible screen window are culled before issuing drawing primitives:
```c
if (y_screen + l->height < top_boundary) continue;
if (y_screen > bottom_boundary) break; // Consecutive layout allows early loop break
```
This guarantees constant $O(\text{visible lines})$ complexity rather than $O(\text{total lines})$.

---

## 6. Platform Abstraction Layer (HAL)

### 6.1. Dual Storage Fallback (`platform/psp_fs.c`)
The PSP Go mounts internal 16 GB eMMC flash storage as `ef0:`, whereas standard PSP models mount Memory Sticks as `ms0:`.  
The file driver automatically scans `ef0:/DOCS/` first; if unmounted or empty, it seamlessly falls back to `ms0:/DOCS/`.

### 6.2. System Parameter Integration (`platform/psp_power.c`)
Queries the PSP system registry via `libpsputility` (`sceUtilityGetSystemParamInt`):
* Detects whether user has configured **12-hour (AM/PM)** or **24-hour** format in XMB settings, formatting the status bar clock accordingly.
* Queries real-time battery percentage (`scePowerGetBatteryLifePercent()`) and charging status (`scePowerIsBatteryCharging()`).

---

## 7. Slider-Closed State Machine

Empirical hardware probing proved that the PSP Go processor **does not sleep** when the slider display is folded closed.

### 7.1. Hardware Button Isolation
Kernel SYSCON disconnects the D-Pad, face buttons ($\times, \bigcirc, \square, \Delta$), and analog stick when closed. Only **L Trigger**, **R Trigger**, and **PS/HOME** remain active.

### 7.2. Anti-Bounce Trigger Latch
To prevent accidental page turns when executing the L+R theme switch combo:
```
User holds L + R:
  ├── t < 1000 ms: Accumulate timer
  └── t >= 1000 ms:
        ├── Trigger theme cycle
        ├── Set suppress_next_l_release = true
        └── Set suppress_next_r_release = true

User releases L or R:
  ├── If suppress flag active: Consume event quietly
  └── Else: Trigger single page turn
```

---

## 8. Hardware Safety & Verification

1. **Motherboard Brick Protection:**  
   The PSP Go has no physical NAND partition for Classic IPL. Flashing `CIPL_Flasher` will permanently destroy the device. This project is strictly compiled as an unprivileged user-mode homebrew application (`pack-pbp`).
2. **FAT32 Resource Fork Cleansing:**  
   macOS generates hidden AppleDouble metadata (`._*`). Always execute `dot_clean` before unmounting to prevent the PSP XMB "Corrupted Data" error.
3. **Automated Unit Testing:**  
   Before any PSP binary is deployed, `tests/test_core.c` verifies state machines, physics clamping, parsing fidelity, and UTF-8 sanitization on the host platform.
