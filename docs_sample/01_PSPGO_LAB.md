# PSP Go Modern Lab
## Ficha Tecnica del Dispositivo

La Sony PlayStation Portable Go (PSP-N1000) es una consola ultraportatil con factor de forma deslizante (slider).

### Especificaciones de Hardware
- CPU: MIPS R4000 Allegrex de 32 bits a 333 MHz
- Coprocesador: VFPU vectorial de punto flotante
- Memoria RAM: 64 MB DDR SDRAM
- Memoria Interna: 16 GB eMMC NAND montada en ef0:/
- Pantalla: TFT LCD 3.8 pulgadas, 480x272 pixeles a 60 Hz

---

### Mapeo de Botones en Modo Slider Cerrado
Al plegar la pantalla, los unicos botones accesibles son:
- Gatillo L (Pagina anterior)
- Gatillo R (Pagina siguiente)
- Combo L + R sostenido (Cambiar tema de color)
- Boton PS / HOME (Menu del sistema XMB)

> Recordatorio: La CPU de la PSP Go continua iterando a 60 FPS estables incluso con el slider cerrado.

```c
/* Ejemplo de inicializacion de VRAM */
sceGuInit();
sceGuDrawBuffer(GU_PSM_8888, 0, 512);
sceGuDispBuffer(480, 272, 0x88000, 512);
```
