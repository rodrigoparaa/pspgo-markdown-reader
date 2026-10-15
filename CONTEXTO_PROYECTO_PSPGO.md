# 🧠 MEMORIA TÉCNICA Y GUÍA DE CONTINUIDAD PARA AGENTES DE IA
## Proyecto: PSP Go Modern Lab (Sony PSP-N1000 & macOS Apple Silicon)

> **Destinatario:** Modelos de Inteligencia Artificial (LLMs) y Desarrolladores de Sistemas Embebidos.  
> **Propósito:** Transferencia integral de contexto técnico, decisiones de arquitectura, lecciones aprendidas en hardware real, trampas críticas evitadas y especificación de proyectos.  
> **Fecha de corte:** Octubre 2026.  
> **Rama activa:** `main`  

---

## 1. 📋 Ficha Técnica del Hardware Objetivo y Entorno Host

### 1.1. Dispositivo Físico: Sony PlayStation Portable Go (PSP-N1000)
* **Factor de forma:** Dispositivo ultracompacto tipo *slider* (pantalla deslizante sobre teclado/cruceta).
* **Almacenamiento:** **16 GB eMMC NAND interna (`ef0:`)**. No se utiliza Memory Stick Micro M2 (`ms0:`). Cualquier ruta en la consola debe apuntar a `ef0:/`.
* **Procesador (CPU):** MIPS R4000 (Allegrex) de 32 bits Little-Endian @ 333 MHz. Dispone de coprocesador vectorial VFPU y Media Engine.
* **Memoria RAM:** 64 MB DDR SDRAM (disponibles en modelos 2000, 3000 y Go).
* **Pantalla:** TFT LCD de 3.8 pulgadas, resolución nativa $480 \times 272$ píxeles, tasa de refresco física de 60 Hz (16.66 ms por cuadro). Paso de línea en memoria (*stride*): 512 palabras (2048 bytes). Formato de color: 32-bit RGBA 8888 (`0xAABBGGRR` en memoria little-endian).
* **Memoria de Vídeo (eDRAM):** 2 MB de memoria ultrarrápida integrada en el coprocesador gráfico (GE).
* **Firmware Base:** OFW 6.60 oficial con parche permanente Infinity (`6.60∞`).
* **Custom Firmware Activo:** **6.60 PROMOD-C2p VCFW** (Virtual Custom Firmware) cargado en RAM mediante `PROUPDATE` y recuperable con `FastRecovery`. El menú VSH está activo pulsando `SELECT` en el XMB.

### 1.2. Entorno Host de Desarrollo (macOS Apple Silicon)
* **Máquina de compilación:** Mac con arquitectura ARM64 (Apple Silicon M1/M2/M3/M4).
* **Toolchain PSPDEV local:** Ubicado de forma autocontenida en `MandoPSP/pspdev/` (GCC 15.2.0 `mips-allegrex-psp-elf`). No altera `/usr/local` ni dependencias globales.
* **Entorno Virtual (`activate.sh`):** Emula un `virtualenv` de Python.
  * Para activar: `source activate.sh` (modifica `PATH`, exporta `PSPDEV` y altera el prompt a `(psp-env)`).
  * Para compilar código PSP: invocar siempre `psp-gcc`, `psp-ar`, `mips-allegrex-psp-elf-strip` y `pack-pbp`.
  * Para desactivar: `deactivate_psp`.
* **Dependencias previas en macOS:** `brew install libmpc mpfr gmp`.

---

## 2. 🔬 Hallazgos Críticos en Hardware Real (Fase 0 de Telemetría)

Antes de programar la interfaz gráfica, se desarrolló y ejecutó en la consola física una aplicación de telemetría de bajo nivel (`00_Probe`), resolviendo incógnitas de hardware que condicionan toda la arquitectura de software:

1. **Comportamiento de la CPU con el Slider Plegado (Cerrado):**
   * **Hallazgo:** La CPU **no se congela ni entra en reposo** al deslizar y cerrar la pantalla mientras corre una aplicación en modo usuario.
   * **Dato medido:** Durante más de 60 segundos consecutivos con la consola cerrada, el bucle principal continuó iterando con un `MaxGap = 0 ms` a 60 FPS estables. Esto valida al 100% el uso de la PSP Go como reloj de mesa permanente.
2. **Visibilidad del Sensor Magnético (Efecto Hall) del Slider:**
   * **Hallazgo:** El sensor de cierre **NO es accesible en espacio de usuario (User Mode)**. No altera ningún bit en `sceCtrlData.Buttons` ni envía eventos a través de los callbacks estándar de `scePowerRegisterCallback`. Es gobernado exclusivamente por el microcontrolador SYSCON a nivel de kernel.
   * **Decisión de diseño:** Las interfaces no dependen de detectar si la consola está abierta o cerrada; la UI se diseña legible en ambos estados y las transiciones se gestionan mediante gestos en los botones físicos.
3. **Mapeo de Botones Accesibles con la Pantalla Plegada:**
   * **Hallazgo:** Los únicos botones funcionales con la consola cerrada son:
     * **Gatillo L** (`PSP_CTRL_LTRIGGER` = `0x00000100`).
     * **Gatillo R** (`PSP_CTRL_RTRIGGER` = `0x00000200`).
     * **Gatillo L + R simultáneo** (`0x00000300`).
     * **Botón PS / HOME** (`0x00010000`) para volver al XMB.
   * *Nota:* Los botones de volumen superior los gestiona el hardware de sonido y no se transmiten al buffer de usuario de `sceCtrl`.

---

## 3. 🛠️ Proyectos Completados y Verificados

```text
Repositorio MandoPSP/
├── activate.sh                     # Entorno virtual MIPS (source activate.sh)
├── .gitignore                      # Excluye pspdev (400MB), CIPL_Flasher, .o, .elf
├── README.md                       # Documentación en Inglés (Principal en GitHub)
├── README_ES.md                    # Documentación en Español
├── PROJECT_CONTEXT.md              # Contexto técnico de desarrollo
├── projects/
│   ├── 00_HelloWorld/              # Piloto mínimo de compilación y lectura de pads
│   ├── 00_Probe/                   # Suite de telemetría experimental (Fase 0)
│   └── 01_DeskClock_Pomodoro/      # Suite de Productividad y Reloj de Mesa (PRODUCCIÓN v1.0)
```

---

### Proyecto 1: PSP Go como Mando USB HID de Baja Latencia para macOS
* **Driver:** `FuSa GamePad v0.3` instalado en `ef0:/PSP/GAME/FusaGamePad/`.
* **Refutación del mito de la comunidad:** Se demostró mediante ingeniería inversa que no es necesario parchear cadenas `ms0:` por `ef0:` en los binarios con perl. FuSa invoca `getcwd()` dinámicamente. El fallo común radicaba en ejecutar en OFW en vez de CFW PRO-C.
* **Integración con macOS Apple Silicon & Unity:** macOS reconoce el mando por HID genérico, pero motores modernos (Unity en *Cult of the Lamb*) y Apple GameController framework lo descartan.
* **Solución Implementada:** Activar **Steam Input para mandos genéricos**. Steam virtualiza el gamepad mapeando stick analógico, cruceta, botones y gatillos sin input lag apreciable.
* **Protocolo de Conexión:**
  1. Abrir FuSa en la PSP antes de enchufar el cable.
  2. Conectar USB al Mac (`:: CONNECTED ::`).
  3. Para salir: desconectar cable, presionar `PS/HOME` y pulsar `X`.

---

### Proyecto 2: Desk Clock & Pomodoro Engine (`01_DeskClock_Pomodoro`)
Una suite de escritorio nativa en C99 a 60 FPS diseñada para operar la consola cerrada en su dock/mesa mediante los gatillos **L** y **R**.

#### Arquitectura Modular en 3 Capas:
1. **`core/` (Lógica C Pura - Desacoplada del SDK):**
   * `pomodoro.c`: Máquina de estados (IDLE, RUNNING, PAUSED, READY). Modelo de tiempo basado en RTC real en milisegundos (`now_ms`), inmune a variaciones de framerate. Ciclo estándar: 25 min trabajo, 5 min descanso, 15 min descanso largo cada 4 rondas.
   * `input_logic.c`: Procesador de gestos. Filtra toques rápidos (<600 ms), pulsación larga en R (>1.500 ms para reset) y combo L+R (>1.000 ms para ciclar pantallas). **Implementa lógica de cancelación:** al soltar un combo L+R, no dispara toques individuales accidentales.
   * `app_state.c`: Enrutador global de estado y navegación entre pantallas.
   * `tests/test_core.c`: Suite de pruebas unitarias que compila en macOS con `clang` puro en <1 segundo.
2. **`ui/` (Motor Gráfico Oficial por Hardware `libpspgu`):**
   * `graphics.c`: Double buffering acelerado por hardware GE.
   * `font.c`: Tipografía bitmap de 7 segmentos continua con trazo $t = h/8$ e intersección sólida (cero huecos entre segmentos).
   * `screens.c`: Renderizado de las 4 vistas: Reloj, Pomodoro, Batería y Guía Cheatsheet.
3. **`platform/` (Drivers del Hardware PSP):**
   * `psp_audio.c`: Síntesis procedural senoidal a 44.1 kHz en hilo dedicado (`audio_worker_thread`, prioridad `0x18`). Dos tonos armónicos (La5 880 Hz y Re6 1174 Hz) con envolvente lineal anti-pop. Cero archivos WAV/MP3 externos.
   * `psp_battery.c`: Telemetría vía `libpsppower`. Filtra registros anómalos de temperatura no implementados en la PSP Go, mostrando `-- (Sensor no expuesto)` sin romper la UI.
   * **Keep-Awake:** `scePowerTick(PSP_POWER_TICK_ALL)` ejecutado cada 4s para evitar suspensión automática.

---

## 4. 🚨 Post-Mortem Gráfico y Trampas de Bajo Nivel Evitadas

Cualquier IA que desarrolle para PSP debe conocer los 4 fallos que se presentaron y cómo se resolvieron:

| Problema | Síntoma en Pantalla | Causa Raíz a Bajo Nivel | Solución Definitiva |
| :--- | :--- | :--- | :--- |
| **1. Tearing en Búfer Simple** | Texto entrecortado en las últimas líneas de pantalla (`00_Probe`). | `pspDebugScreenPrintf` escribe directamente sobre el Front Buffer mientras el raster LCD barre a 60 Hz. | Usar siempre doble búfer VRAM con sincronización VBLANK. |
| **2. Franja Superior Negra (`NEXTFRAME`)** | Primeros 100 píxeles superiores ($Y=0$ a $Y=110$) en negro absoluto en `DeskClock`. | Llamar a `sceDisplayWaitVblankStart()` y acto seguido a `sceDisplaySetFrameBuf(..., PSP_DISPLAY_SETBUF_NEXTFRAME)` en modo usuario retrasa el cambio al *siguiente* VBLANK (16.6 ms después), mientras la CPU ya borraba a negro desde $Y=0$. | Descartar el intercambio manual por software en user mode y usar el hardware GE (`libpspgu`). |
| **3. Cuelgue de Bus (`EXC_BUS`)** | Pantalla negra absoluta y congelación total de la consola. | Invocar `sceGeEdramGetAddr()` sin haber inicializado el hardware gráfico devuelve `0`. La operación `0x40000000 | edram` direccionaba a `0x40000000` (no asignada en el bus MIPS). | Inicializar siempre primero el Graphic Engine mediante `sceGuInit()`. |
| **4. Huecos en 7 Segmentos** | Sensación visual de dígitos "rotos" o entrecortados. | Retranqueos (*insets*) de 1 píxel en esquinas. En el panel LCD pequeño de la PSP se perciben como saltos de línea. | Geometría continua: `inset = 0`, grosor `t = h/8`, solapando rectángulos en esquinas. |

### Configuración Canónica de Doble Búfer con `libpspgu`:
```c
/* Inicialización */
sceGuInit();
sceGuDrawBuffer(GU_PSM_8888, (void*)0x00000, 512);          /* VRAM A (Offset 0) */
sceGuDispBuffer(480, 272, (void*)0x88000, 512);             /* VRAM B (Offset 0x88000) */
sceGuDisplay(GU_TRUE);

/* Bucle de fotograma (60 Hz) */
/* 1. CPU dibuja en el búfer inactivo uncached (0x44000000 o 0x44088000) */
render_ui(g_vram_draw);

/* 2. Sincronización física con el haz del LCD */
sceDisplayWaitVblankStart();

/* 3. Intercambio por hardware en el controlador de pantalla */
void *new_draw = sceGuSwapBuffers();
g_vram_draw = (uint32_t *)((uint32_t)new_draw | 0x44000000);
```

---

## 5. 🛡️ Protocolos de Seguridad y Reglas de Oro

1. **⚠️ PROTECCIÓN ANTI-BRICK (Hardware Safety):**
   * **NUNCA** ejecutar, copiar ni recomendar el uso de `CIPL_Flasher` en la PSP Go. Esta herramienta intenta flashear la IPL del modelo PSP-1000/2000 en el almacenamiento del sistema; en la PSP Go **provoca un brick irreversible de la placa base**. Está bloqueada en `.gitignore`.
2. **🧹 LIMPIEZA DE METADATOS DE MACOS (Anti "Datos Dañados"):**
   * macOS genera automáticamente archivos ocultos `._*` y `.DS_Store` en unidades FAT32. El XMB de PSP interpreta estos archivos ocultos como juegos corruptos mostrando iconos de *"Datos dañados"*.
   * **Comando obligatorio tras transferir a `/Volumes/PSP GO`:**
     ```bash
     dot_clean "/Volumes/PSP GO/PSP/GAME" && sync
     ```
3. **🔒 HIGIENE DE PRIVACIDAD EN GIT:**
   * Nunca commitear archivos `.elf` ni `.o` (contienen rutas absolutas del host).
   * Nunca exponer rutas como `/Users/<nombre>/...`. Usar siempre alias relativos (`~/Desktop/MandoPSP` o `MandoPSP/`).
   * No commitear la carpeta `pspdev/` (400 MB).

---

## 6. 🗺️ Roadmap: Proyectos Pendientes de Implementación

A continuación se detalla la especificación técnica de los tres proyectos planificados que cualquier IA o desarrollador puede abordar a continuación:

---

### Proyecto Pendiente 1: `02_MarkdownReader` (Lector Distraction-Free de Documentación)
* **Objetivo:** Convertir la PSP Go en un dispositivo de lectura técnica portátil para consultar apuntes, especificaciones y cheatsheets `.md` sin distracciones de internet ni notificaciones.
* **Ubicación de archivos:** Leerá documentos almacenados en `ef0:/DOCS/*.md`.
* **Especificaciones Técnicas Requeridas:**
  1. **Motor de Renderizado:** Reutilizar el pipeline oficial de doble búfer `libpspgu` a 60 FPS desarrollado en `DeskClock`.
  2. **Parser Markdown Ligero en C (Streaming):**
     * Parseo de encabezados (`#`, `##`, `###`) con escala de tipografía bitmap.
     * Soporte de listas con viñetas (`*`, `-`).
     * Bloques de código con fondo diferenciado (` ``` `) y texto monoespaciado.
     * Énfasis en texto (`**negrita**`).
  3. **Control y Navegación Cinética:**
     * **Scroll vertical analógico a 60 FPS:** Uso del stick analógico con aceleración inercial y fricción suave para desplazamiento natural entre párrafos.
     * **Paso de página rápido:** Gatillos **L** (Página anterior) y **R** (Página siguiente).
     * **Slider cerrado:** Lectura pasiva con avance mediante gatillos L/R.
  4. **Temas Cromáticos para el Panel LCD:**
     * *Dark Mode (Cyberpunk/Minimal):* Fondo `#0F0F14`, texto `#E0E6ED`.
     * *Sepia Mode (Lectura prolongada):* Fondo `#F4ECD8`, texto `#3E3123`.
     * *Alto Contraste OLED style:* Fondo negro `#000000`, texto blanco puro `#FFFFFF`.
  5. **Explorador de Archivos:** Menú inicial para explorar y seleccionar archivos `.md` dentro de `ef0:/DOCS/` usando la API nativa `sceIoDopen` / `sceIoDread`.

---

### Proyecto Pendiente 2: `03_PocketToolkit` (Caja de Herramientas Offline para Hackers/Devs)
* **Objetivo:** Una suite integral de utilidades matemáticas, de criptografía y de diagnóstico de sistemas de bolsillo, operable sin conexión.
* **Módulos que debe integrar:**
  1. **Conversor de Bases y Calculadora Bitwise:**
     * Conversión instantánea simultánea entre **Hexadecimal**, **Decimal**, **Binario** (32 bits con cuadrícula interactiva de bits) y **Octal**.
     * Calculadora de operaciones lógicas a nivel de bit: `AND`, `OR`, `XOR`, `NOT`, `SHL`, `SHR`, rotaciones de bits y cálculo de máscaras.
     * Selector de endianness: conmutación rápida Little-Endian $\leftrightarrow$ Big-Endian.
  2. **Cronómetro y Taquímetro de Alta Precisión:**
     * Temporizador con resolución de milisegundos reales apoyado en `sceRtcGetCurrentTick()`.
     * Registro de vueltas (*laps*) en memoria con exportación opcional a archivo de texto en `ef0:/POCKET_LOGS/`.
  3. **Bóveda de Notas Cifrada (AES-256):**
     * Almacenamiento seguro de notas críticas en `ef0:/VAULT.BIN`.
     * Cifrado simétrico AES-256 en memoria.
     * Desbloqueo mediante combinación de botones físicos (*master key combo*, ej. `L + R + Triángulo + Cruz`), sin teclado virtual lento.

---

### Proyecto Pendiente 3: `DeskClock v2 (Plugin Kernel Residente PRX)`
* **Objetivo:** Eliminar la necesidad de abrir manualmente `DeskClock` desde el menú de juegos de la consola.
* **Comportamiento Deseado:**
  * Al deslizar y cerrar la pantalla física de la PSP Go en cualquier momento mientras se está en el menú XMB oficial, la consola debe lanzar automáticamente `DeskClock`.
* **Implementación Requerida:**
  * Compilar un módulo en modo kernel (`.prx`) residente en memoria configurado en `ef0:/seplugins/vsh.txt`.
  * Enganchar la interrupción SYSCON del sensor magnético Hall del slider.
  * Al detectar el flanco de bajada (cierre de pantalla), invocar la carga del ejecutable `ef0:/PSP/GAME/DeskClock/EBOOT.PBP` utilizando `sceKernelLoadExec()`.

---

## 7. 🚀 Recetas de Compilación y Comandos Rápidos

```bash
# 1. Acceder al workspace y activar toolchain
cd ~/Desktop/MandoPSP
source activate.sh

# 2. Compilar DeskClock (Genera EBOOT.PBP en projects/01_DeskClock_Pomodoro/)
cd projects/01_DeskClock_Pomodoro
make clean && make

# 3. Ejecutar pruebas unitarias en macOS con clang
cd tests
clang -Wall -Wextra -O2 -I../core ../core/*.c test_core.c -o run_tests && ./run_tests

# 4. Despliegue en la PSP Go conectada por USB
mkdir -p "/Volumes/PSP GO/PSP/GAME/DeskClock"
cp ../EBOOT.PBP "/Volumes/PSP GO/PSP/GAME/DeskClock/"
dot_clean "/Volumes/PSP GO/PSP/GAME" && sync

# 5. Desactivar entorno virtual
deactivate_psp
```

---
*Fin de la especificación de contexto. Documento listo para consumo por agentes de IA.*
