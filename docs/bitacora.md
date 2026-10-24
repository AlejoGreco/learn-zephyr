# Bitácora

Una entrada por micro-paso, la más nueva arriba. Se escribe **al terminar** el micro-paso, justo
antes del commit `eNN.M: <qué logré>`. Las secciones marcadas *(si aplica)* se borran cuando no
corresponden; el resto no se saltea.

Reglas rápidas:

- **Lectura dirigida** va una sola vez por etapa, en la entrada del primer micro-paso (§3 del roadmap).
- **Predicción** se escribe **antes** de ejecutar (5.4, 6.x, 8.x...), nunca después.
- Toda cifra lleva unidad, target y comando. Sin evidencia en `docs/evidencias/eNN/`, no cuenta.
- Si el micro-paso te llevó más de tres sesiones, está mal partido: anotalo y dividilo.

---

## Plantilla

Copiá el bloque de abajo y pegalo debajo de "Entradas".

````markdown
## eNN.M — <nombre del micro-paso>

**Fecha:** AAAA-MM-DD · **Sesiones:** N · **Target:** native_sim | esp32s3_devkitc/esp32s3/procpu | ambos
**Zephyr:** <west list zephyr → rev> · **Commit:** `eNN.M: <mensaje>` (<hash corto>)
**Estado:** ✅ checkpoint verificado | 🟡 parcial | ❌ bloqueado

### Objetivo
Una frase: qué quería lograr y cuál es el checkpoint del roadmap.

### Lectura dirigida *(solo primer micro-paso de la etapa)*
- **Qué problema resuelve:**
- **APIs / opciones centrales (2–3):**
- **Limitación que menciona la doc:**
- **Fuentes leídas:** <enlaces>

### Predicción *(si aplica; escrita antes de ejecutar)*
Qué espero ver y por qué (prioridades, orden de salida, valores, tamaños).

### Qué hice
Comandos exactos y cambios relevantes, en orden.

```bash
west build -p always -b <target> <ruta-app>
```

Archivos tocados: `etapas/eNN-.../...`

### Resultado
Qué pasó realmente. Salida recortada a lo relevante.

```text
<log>
```

**Predicción vs realidad** *(si aplica)*: coincidió | divergió en ___ porque ___.

### Mediciones *(si aplica)*
| Métrica | native_sim | ESP32-S3 | Método / comando |
| --- | --- | --- | --- |
| ROM (flash) | | | `west build -t rom_report` |
| RAM | | | `west build -t ram_report` |
| | | | |

### Evidencia
- `docs/evidencias/eNN/<archivo>` — qué muestra

### Problemas y diagnóstico *(si aplica)*
- **Síntoma:** mensaje exacto (CMake / DT / linker / runtime)
- **Hipótesis descartadas:** y cómo las descarté
- **Causa raíz:**
- **Arreglo:**
- **Método para la próxima vez:**

### Particularidades de la placa *(si aplica)*
Blobs de RF, `--reset-type watchdog-reset`, alias de LED/botón, puerto USB de consola, etc.

### Qué aprendí y pendiente
- **Aprendizaje:** 2–3 líneas, con tus palabras. Qué me habría hecho falta saber antes.
- **Pendiente:** Algo que quedó abierto o para profundizar después

### Defensa *(solo último micro-paso de la etapa; respondé sin mirar la doc)*
1. Pregunta 1 → respuesta
2. Pregunta 2 → respuesta
3. Pregunta 3 → respuesta
````

---

# Entradas

<!-- Pegá las entradas nuevas debajo de esta línea, la más reciente arriba. -->
