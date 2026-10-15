# Apuntes de Programacion C
## Guia Rapida y Buenas Practicas

Esta es una guia de referencia rapida para desarrollo de sistemas embebidos y C99 moderno.

### Operaciones a Nivel de Bit (Bitwise)
- AND bit a bit: a & b (util para mascaras)
- OR bit a bit: a | b (util para activar flags)
- XOR bit a bit: a ^ b (util para alternar bits)
- NOT bit a bit: ~a (inversion)
- Desplazamiento izquierda: a << n
- Desplazamiento derecha: a >> n

```c
/* Mascara para empaquetar color RGBA 8888 */
uint32_t color = (a << 24) | (b << 16) | (g << 8) | r;
```

---

### Manejo de Memoria Dinamica
> Regla de Oro: Todo malloc() debe tener un free() correspondiente, preferiblemente en el mismo modulo o capa de abstraccion.
