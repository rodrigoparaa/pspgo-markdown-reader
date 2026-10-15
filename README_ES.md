# 📖 PSP Go Markdown Reader (`02_MarkdownReader`)

[![Plataforma](https://img.shields.io/badge/Plataforma-Sony%20PSP%20Go%20%7C%20PSP--N1000-003791?style=for-the-badge&logo=playstation&logoColor=white)](https://es.wikipedia.org/wiki/PlayStation_Portable_Go)
[![Arquitectura](https://img.shields.io/badge/Arquitectura-MIPS%20Allegrex%20R4000%20%40%20333MHz-blue?style=for-the-badge)](https://en.wikipedia.org/wiki/PlayStation_Portable_hardware)
[![Lenguaje](https://img.shields.io/badge/Lenguaje-C99%20Puro-orange?style=for-the-badge&logo=c)](https://es.wikipedia.org/wiki/C_(lenguaje_de_programaci%C3%B3n))
[![Gráficos](https://img.shields.io/badge/Gr%C3%A1ficos-Hardware%20libpspgu%20%40%2060%20FPS-green?style=for-the-badge)](https://github.com/pspdev/pspsdk)
[![Toolchain](https://img.shields.io/badge/Toolchain-PSPDEV%20GCC%2015.2.0-blueviolet?style=for-the-badge)](https://github.com/pspdev/pspdev)
[![Licencia](https://img.shields.io/badge/Licencia-MIT-brightgreen?style=for-the-badge)](LICENSE)

> **Lector de documentación técnica distraction-free de alto rendimiento, desarrollado en C99 puro para la consola Sony PlayStation Portable Go (PSP-N1000).**  
> Incorpora maquetación Markdown en tiempo real, física de scroll cinético inercial, tres temas de alto contraste, renderizado acelerado por hardware a **60 FPS estables vía `libpspgu`** y soporte ergonómico para lectura con una mano en **Modo Slider Cerrado**.

---

### 🌐 Idioma / Language
* 🇪🇸 **Español** (Documento actual)
* 🇬🇧 **[English Documentation (README.md)](README.md)**

---

## 📑 Tabla de Contenidos
1. [Descripción General](#-descripción-general)
2. [Características Principales](#-características-principales)
3. [Ergonomía en Modo Slider Cerrado](#-ergonomía-en-modo-slider-cerrado)
4. [Diseño de Interfaz y Maquetación](#-diseño-de-interfaz-y-maquetación)
5. [Matriz de Controles e Interacción](#-matriz-de-controles-e-interacción)
6. [Arquitectura del Sistema y Módulos](#-arquitectura-del-sistema-y-módulos)
7. [Seguridad del Hardware y Prevención de Bricks](#-seguridad-del-hardware-y-prevención-de-bricks)
8. [Estructura del Proyecto](#-estructura-del-proyecto)
9. [Toolchain y Compilación desde Fuentes](#-toolchain-y-compilación-desde-fuentes)
10. [Instalación y Despliegue en la PSP Go](#-instalación-y-despliegue-en-la-psp-go)
11. [Suite de Pruebas Unitarias en Host](#-suite-de-pruebas-unitarias-en-host)
12. [Especificaciones Técnicas](#-especificaciones-técnicas)
13. [Licencia](#-licencia)

---

## 🎯 Descripción General

El **PSP Go Markdown Reader** transforma la Sony PSP Go en un lector digital técnico de bolsillo diseñado para programadores, administradores de sistemas y estudiantes. Al prescindir de navegadores pesados y motores web, el proyecto exprime el coprocesador gráfico (GE) del chip Allegrex para renderizar documentos Markdown complejos a **60 cuadros por segundo** con físicas inerciales fluidas.

A diferencia de los visores de PDF convencionales que consumen abundante memoria y sufren de refrescos lentos, este motor ha sido concebido específicamente para la arquitectura MIPS embebida con cero dependencias externas en tiempo de ejecución.

```
       +-------------------------------------------------------+
       | [MD] README.md                   45% | 14:30 | 98% [=] | <- Cabecera 24px
       +-------------------------------------------------------+
       |                                                       |
       |  # PSP Go Markdown Reader                             |
       |                                                       |
       |  Lector de documentacion tecnica distraction-free     |
       |  desarrollado en C99 puro para Sony PSP Go.           |
       |                                                       |
       |  * Pipeline grafico libpspgu a 60 FPS con doble bufer |
       |  * Scroll analogico inercial con friccion matematica  |
       |  * Lectura comoda a una mano con la pantalla plegada  |
       |                                                       |
       +-------------------------------------------------------+
       | [L/R] Pag   [Stick] Scroll   [/\ ] Salir   [[]] Dark  | <- Pie 22px
       +-------------------------------------------------------+
```

---

## ✨ Características Principales

* 🚀 **Pipeline Gráfico Oficial por Hardware a 60 FPS:**
  * Implementado mediante la librería oficial de bajo nivel `libpspgu`.
  * Doble búfer en la memoria VRAM embebida (eDRAM) sincronizado estrictamente con el refresco vertical de la pantalla (`sceDisplayWaitVblankStart()`).
  * Cero parpadeos (*flickering*), cero cortes de pantalla (*tearing*) y cero cuelgues del bus de memoria.
* 📝 **Motor de Maquetación Markdown en Streaming:**
  * **Encabezados:** `# H1` (Escala bitmap 2x), `## H2` (Escala bitmap 2x), `### H3` (Escala 1x con color de acento).
  * **Bloques de código cercados (```` ``` ````):** Contenedor rectangular oscuro con bordes y tipografía monoespaciada en verde terminal.
  * **Citas en bloque (`>`):** Barra lateral de acento de 3 px y paleta cromática atenuada.
  * **Listas (`-`, `*`, `+`):** Viñetas gráficas cuadradas alineadas automáticamente con 14 px de sangría.
  * **Reglas horizontales (`---`, `***`):** Separadores continuos limpios de 1 píxel.
  * **Ajuste de línea inteligente (*Word-Wrapping*):** Algoritmo que mide dinámicamente las cadenas y corta renglones en espacios entre palabras, garantizando hasta 55 caracteres/renglón para el ancho útil de 448 px.
* 🔤 **Sanitizador UTF-8 y Tipografía Robusta:**
  * Mapea caracteres acentuados españoles y latinos (`á`, `é`, `í`, `ó`, `ú`, `ñ`, `Á`, `É`, `¿`, `¡`) directamente a glifos ASCII legibles en la tipografía `font_8x8`.
  * Omite de forma transparente secuencias multibyte de emojis (`📖`, `🌟`, `🧪`) impidiendo que aparezcan interrogaciones corruptas (`????`).
  * Limpia el formato Markdown inline (`**negrita**`, `__negrita__`, `` `código inline` ``) sin alterar guiones bajos internos en variables o nombres de archivos de programación (`snake_case`, `02_MarkdownReader.md`).
* ⚡ **Física de Desplazamiento Cinético (Kinetic Scroll):**
  * Curva analógica con aceleración cuadrática proporcional y filtrado de zona muerta de 22 unidades.
  * Coeficiente de fricción inercial exponencial ($\mu = 0.88$) que produce una desaceleración suave y natural al soltar el stick.
  * Salto de página instantáneo mediante gatillos superiores (**L** / **R**) manteniendo 2 líneas de contexto visual para no perder el hilo de lectura.
  * Margen de seguridad inferior de 90 px al final del documento para garantizar que el último párrafo pueda desplazarse completamente por encima de la barra inferior.
* 🎨 **3 Temas Cromáticos de Alto Confort:**
  * **Dark Cyberpunk:** Fondo azul oscuro `#0F0F14`, texto claro `#E0E6ED`, acentos cian `#20AADD`.
  * **Sepia Paper:** Fondo pergamino cálido `#F4ECD8`, texto café `#3E3123`, encabezados terracota `#B84A28`.
  * **OLED High-Contrast:** Fondo negro absoluto `#000000`, texto blanco puro `#FFFFFF`, acentos oro `#F0B020`.
* 📁 **Explorador de Archivos Integrado:**
  * Detección y lectura directa desde la memoria interna eMMC NAND (`ef0:/DOCS/*.md`) con respaldo automático para tarjetas Memory Stick Micro (`ms0:/DOCS/*.md`).
  * Muestra tamaño de cada archivo en KB/Bytes, barra de scroll proporcional y resaltado visual del elemento activo.
* 🔋 **Telemetría y Gestión de Energía:**
  * Indicador gráfico de batería en tiempo real y porcentaje numérico con detección de cable de carga (`+`).
  * Reloj digital sincronizado con el RTC de la consola, con detección automática de preferencia horaria del sistema (**12 horas con AM/PM** o **24 horas**) vía `libpsputility`.
  * Latido constante `scePowerTick(PSP_POWER_TICK_ALL)` que evita que la consola entre en suspensión durante lecturas prolongadas.

---

## 📱 Ergonomía en Modo Slider Cerrado

La Sony PSP Go destaca por su factor de forma deslizable (*slider*). Un pilar fundamental de diseño en este proyecto fue lograr que la lectura sea completamente cómoda con la pantalla plegada:

```
+------------------------------------------+
|  [Gatillo L]                [Gatillo R]  |
|  (Pag. Previa)              (Pag. Siguiente)
|                                          |
|         +----------------------+         |
|         |                      |         |
|         |      PSP GO LCD      |         |
|         |    (SLIDER CERRADO)  |         |
|         |                      |         |
|         +----------------------+         |
|                                          |
|               [HOME / PS]                |
+------------------------------------------+
```

### Gestos Disponibles con Slider Cerrado:
1. **Sujeción con Una Mano:** Sostén la consola cómodamente como un lector de bolsillo ligero.
2. **Avanzar Página:** Pulsa el **Gatillo R** para avanzar una página completa de texto.
3. **Retroceder Página:** Pulsa el **Gatillo L** para releer texto anterior.
4. **Alternar Tema Cromático:** Mantén presionados **ambos gatillos L + R por más de 1 segundo**. El tema cambiará al instante y un seguro anti-rebote evitará que se pase de página al soltarlos.
5. **Salir al Menú del Sistema:** Pulsa el botón físico **PS / HOME** para regresar limpiamente al XMB oficial.

---

## 🖥️ Diseño de Interfaz y Maquetación

La interfaz ha sido diseñada al píxel para la resolución nativa de $480 \times 272$:

### 1. Explorador de Documentos (`screen_render_explorer`)
* **Cabecera (24 px):** Título de la aplicación a la izquierda, Reloj local RTC, Porcentaje de batería e Icono gráfico a la derecha.
* **Lista de Archivos:** Permite visualizar hasta 7 archivos simultáneamente con tamaño en KB y cursor de selección activo.
* **Scrollbar Lateral:** Barra gráfica que indica la posición relativa en carpetas con muchos documentos.
* **Pie (22 px):** Atajos de navegación directa (`[X] Abrir`, `[^v] Mover`, `[[]] Tema`, `[START] Ayuda`).

### 2. Visor de Documentos (`screen_render_reader`)
* **Cabecera (24 px):** Título del archivo Markdown (truncado automáticamente con `...` si es muy largo), porcentaje de avance (`100%`), reloj y telemetría de batería.
* **Área de Lectura Útil (226 px):** Márgenes laterales de 16 px (`CONTENT_MARGIN_X`) dejando 448 px útiles para el texto.
* **Clipping Vertical Estricto:** Descarta renglones fuera de la pantalla, manteniendo los 60 FPS inmutables incluso en archivos de cientos de líneas.
* **Modo Pantalla Completa:** Pulsa **Círculo ($\bigcirc$)** para ocultar las barras y disfrutar de lectura limpia sin distracciones.

### 3. Modal de Ayuda en Dos Columnas (`screen_render_help`)
* Aprovecha al máximo la relación de aspecto panorámica 16:9.
* **Columna 1 (Izquierda):** Controles con la consola abierta (Stick, cruceta, botones frontales).
* **Columna 2 (Derecha):** Controles con el slider cerrado (Gatillos, combo L+R, botón Home y consejos de postura).

---

## 🎮 Matriz de Controles e Interacción

| Botón / Gesto | Consola Abierta (Modo Activo) | Consola Cerrada (Slider Plegado) |
| :--- | :--- | :--- |
| **Stick Analógico** | Desplazamiento continuo inercial | *Plegado bajo la pantalla* |
| **Cruceta Arriba / Abajo**| Desplazamiento línea a línea | *Plegada bajo la pantalla* |
| **Gatillo R** | Avanzar página completa | Avanzar página completa |
| **Gatillo L** | Retroceder página completa | Retroceder página completa |
| **Combo L + R (>1s)** | Cambiar tema cromático | Cambiar tema cromático |
| **Botón Cuadrado ($\square$)**| Alternar tema (Dark $\to$ Sepia $\to$ OLED) | *Plegado bajo la pantalla* |
| **Botón Triángulo ($\Delta$)**| Volver al Explorador (`ef0:/DOCS`) | *Plegado bajo la pantalla* |
| **Botón Círculo ($\bigcirc$)**| Ocultar / Mostrar barras HUD | *Plegado bajo la pantalla* |
| **Botón Cruz ($\times$)** | Abrir documento en el explorador | *Plegado bajo la pantalla* |
| **Botón Start** | Abrir guía interactiva de controles | *Plegado bajo la pantalla* |
| **Botón PS / HOME** | Salir al XMB oficial de la PSP | Salir al XMB oficial de la PSP |

---

## 🏗️ Arquitectura del Sistema y Módulos

El proyecto sigue una arquitectura en 3 capas desacopladas, diseñada para facilitar el mantenimiento y las pruebas automatizadas:

```
MarkdownReader/
├── core/                  # Lógica C99 Pura - Cero dependencias del SDK (Compilable en Host)
│   ├── app_state.h/.c     # Máquina de estados finitos (Explorador, Lector, Ayuda)
│   ├── md_parser.h/.c     # Clasificador Markdown, word-wrapping y sanitizador UTF-8
│   ├── scroll_physics.h/.c# Física de fricción cinética y aceleración analógica
│   └── theme_engine.h/.c  # Paletas de color, temas cromáticos y ciclo de estilos
│
├── ui/                    # Capa de presentación (libpspgu y renderizado)
│   ├── font.h/.c          # Fuente bitmap monoespaciada 8x8 y reloj digital
│   ├── graphics.h/.c      # Doble búfer en VRAM, primitivas, cards, icono de batería
│   └── screens.h/.c       # Vistas de Explorador, Lector y Diálogo de Ayuda
│
├── platform/              # Capa de abstracción de hardware (SDK PSP)
│   ├── psp_fs.h/.c        # Descubrimiento de archivos en ef0:/ y ms0:/ y lectura en búfer
│   ├── psp_input.h/.c     # Lectura de sceCtrl, debounce y temporizador de combos
│   └── psp_power.h/.c     # Telemetría libpsppower, hora RTC y latido keep-awake
│
├── tests/                 # Suite de pruebas unitarias nativas
│   └── test_core.c        # Ejecutable autónomo para Mac/Linux (Clang/GCC)
│
├── docs_sample/           # Documentos Markdown de ejemplo listos para probar
├── Makefile               # Sistema de compilación oficial PSPDEV
├── activate.sh            # Script de activación del entorno virtual de desarrollo
└── main.c                 # Punto de entrada, reserva de memoria y bucle principal
```

### Mapa de Memoria y Prevención de Desbordamientos
Para evitar caídas de pila (*stack overflow*) y errores de bus en la arquitectura MIPS Allegrex:
* **Espacio de Heap:** Fijado en **20 MB** (`PSP_HEAP_SIZE_KB(20480)`) dentro de los 64 MB de memoria DDR SDRAM.
* **Pila del Hilo Principal:** Configurada en **512 KB** (`PSP_MAIN_THREAD_STACK_SIZE_KB(512)`).
* **Almacenamiento Estático BSS:** El estado maestro de la aplicación (`AppState s_app_state`) reside en la sección `.bss` y no en la pila de llamadas, evitando superar el límite de marco durante la lectura de documentos grandes.
* **Doble Búfer en VRAM:**
  * Búfer Frontal: `0x00000000` (Stride: 512 palabras, 32-bit RGBA 8888).
  * Búfer Trasero: `0x00088000` (Desplazamiento 512 x 272 x 4 = 557.056 bytes).

---

## ⚠️ Seguridad del Hardware y Prevención de Bricks

Al desarrollar software casero para la Sony PSP Go (PSP-N1000), deben respetarse dos reglas de seguridad esenciales:

### 1. Prevención de Bricks Permanentes: Prohibido CIPL Flasher
> [!CAUTION]
> **NUNCA ejecutes ni incluyas `CIPL_Flasher` en una consola PSP Go (PSP-N1000).**  
> Los modelos clásicos de PSP (1000 y primeras 2000) almacenan su Initial Program Loader (IPL) en una partición física de la memoria NAND que permite parches personalizados.  
> La **PSP Go NO admite Custom IPL**. Intentar flashear un CIPL en una PSP Go provocará un **brick permanente e irrecuperable de la placa base**.  
> Este proyecto se empaqueta en modo usuario estándar mediante `pack-pbp` compatible con **6.60 PROMOD-C2p + Infinity**, siendo 100% seguro.

### 2. Limpieza de Metadatos AppleDouble de macOS en FAT32
> [!IMPORTANT]
> Al transferir archivos a la PSP Go desde una computadora Mac por USB, macOS crea archivos ocultos con prefijo `._` (ej. `._EBOOT.PBP`).  
> El menú XMB de la consola interpreta estos archivos auxiliares como ejecutables corruptos, mostrando el aviso **"Datos dañados"**.  
> Ejecuta siempre `dot_clean` antes de expulsar el almacenamiento:
> ```bash
> dot_clean "/Volumes/PSP GO/PSP/GAME"
> dot_clean "/Volumes/PSP GO/DOCS"
> sync
> ```

---

## 🧰 Toolchain y Compilación desde Fuentes

### Requisitos Previos
1. **Toolchain PSPDEV:** Instalación funcional de [PSPDEV](https://github.com/pspdev/pspdev) que incluya `psp-gcc` (GCC 15.2+), `psp-fixup-imports`, `mksfoex` y `pack-pbp`.
2. **Compilador Local de Host:** `clang` o `gcc` en macOS / Linux para compilar la suite de pruebas unitarias.

### Paso 1: Activar el Entorno
```bash
# Clonar el repositorio
git clone https://github.com/tu-usuario/pspgo-markdown-reader.git
cd pspgo-markdown-reader

# Activar el entorno virtual aislado
source activate.sh
```

### Paso 2: Ejecutar Pruebas Unitarias en el Host
Antes de compilar para la arquitectura MIPS, verifica que toda la lógica del motor funciona:
```bash
cd tests
clang -Wall -Wextra -O2 -I../core ../core/*.c test_core.c -o run_tests
./run_tests
cd ..
```

### Paso 3: Compilar el Binario para PSP
```bash
# Limpiar objetos previos y generar EBOOT.PBP
make clean
make
```
El proceso generará el ejecutable final firmado: `EBOOT.PBP` (~152 KB).

---

## 🚀 Instalación y Despliegue en la PSP Go

1. Enciende la PSP Go y conéctala a la computadora con el cable USB oficial (con CFW 6.60 PRO / Infinity activo).
2. La memoria interna de 16 GB se montará como `/Volumes/PSP GO` (en macOS) o como una unidad extraíble (Windows/Linux).
3. Asegúrate de tener creadas las carpetas necesarias:
   * Carpeta de la App: `ef0:/PSP/GAME/MarkdownReader/`
   * Carpeta de Documentos: `ef0:/DOCS/`
4. Copia el binario y tus notas Markdown:
   ```bash
   # Copiar binario EBOOT.PBP
   cp EBOOT.PBP "/Volumes/PSP GO/PSP/GAME/MarkdownReader/EBOOT.PBP"

   # Copiar documentos de ejemplo
   cp docs_sample/*.md "/Volumes/PSP GO/DOCS/"
   ```
5. **Limpia los metadatos de macOS y vacía los búferes de escritura:**
   ```bash
   dot_clean "/Volumes/PSP GO/PSP/GAME"
   dot_clean "/Volumes/PSP GO/DOCS"
   sync
   ```
6. Desconecta el USB en la consola, dirígete a **Juego $\to$ Almacenamiento del Sistema** e inicia **PSP Go Markdown Reader**.

---

## 📊 Especificaciones Técnicas

| Parámetro | Especificación | Observaciones |
| :--- | :--- | :--- |
| **Dispositivo Objetivo** | Sony PlayStation Portable Go (PSP-N1000) | Compatible con PSP-1000/2000/3000/Street |
| **Firmware / CFW** | 6.60 PROMOD-C2p + Infinity 2.0 | Ejecución en User Mode |
| **CPU** | MIPS Allegrex R4000 de 32 bits Little-Endian | 333 MHz de reloj |
| **Resolución de Pantalla** | $480 \times 272$ píxeles a 60.00 Hz | Panel TFT LCD 16:9 |
| **Formato de Color** | 32-bit RGBA 8888 (`0xAABBGGRR`) | 16,7 millones de colores reales |
| **API Gráfica** | Sony `libpspgu` (Hardware Engine) | Doble búfer en VRAM |
| **Uso de Memoria RAM** | ~2,4 MB en tiempo de ejecución | Reserva de heap de 20 MB |
| **Tamaño del Binario** | ~152 KB (`EBOOT.PBP`) | Stripped ELF con cabecera SFO |
| **Ruta de Notas** | `ef0:/DOCS/` (Memoria interna) / `ms0:/DOCS/` | Archivos de texto plano UTF-8 `.md` |
| **Latencia de Scroll** | $<16,66\text{ ms}$ por cuadro (1 cuadro) | Sincronizado estrictamente con VBLANK |

---

## 📄 Licencia

Este proyecto está publicado bajo los términos de la **Licencia MIT**. Eres libre de estudiar, modificar, redistribuir e integrar este código en otros proyectos caseros. Consulta el archivo [LICENSE](LICENSE) para más detalles.

---

### Agradecimientos
* **[Comunidad PSPDEV](https://github.com/pspdev):** Por mantener activo el ecosistema de compiladores modernos para hardware clásico.
* **Sony Computer Entertainment:** Por fabricar una de las mejores consolas portátiles de la historia.
