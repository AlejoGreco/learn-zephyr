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

## Defensa e01
*Respondida el 2026-11-20, sin consultar documentación ni notas.*

**1. ¿En qué orden se mergean `prj.conf`, `boards/<target>.conf` y los defaults de la board?**

Respuesta: El orden es similar al del log de compilacion de `west`. Default de la placa -> `prj.conf` -> `boards/<placa>.conf`
Donde la fuente de la izquierda es pisada por la de la derecha. El default de la placa esta compuesta por kconfig del kernel y librerias de zephyr y _defconfig de las placas provistas por el fabricante (tambien con orden de superposicion de izquiera a derecha).

**2. ¿Por qué `zephyr.dts` no es igual al `.dts` de la board?**

Respuesta: porque zephyr.dts es el devicetree final resultante de la fusion de los dts del board, los dtsi del soc y la arquitectura, y los overlays de la aplicacion. El .dts de la board es la especificacion de hardware tal como viene de fabrica 

**3. ¿Qué diferencia hay entre poner `CONFIG_X=y` en `prj.conf` y pasarlo como `-DCONFIG_X=y`?**

Respuesta: Pasarlo por linea de comando va directo a la cache de cmake y tiene mayor prioridad en la cadena de generacion/fusion de kconfig.
Por otro lado, no queda versionado ya que se aloja en el build de cmake, mientras que en el prj.conf forma parte del codigo fuente del proyecto.
Es importante aclarar que pasar una variable por línea de comandos `-DCONFIG_X=y` **no fuerza la opción mágicamente ignorando las reglas de Kconfig**:

* Kconfig procesa la petición y valida sus dependencias.
* Si `CONFIG_X` requiere de otra opción que no está activa, o si depende de un nodo de hardware que está desactivado en el Devicetree (`status = "disabled";`), Kconfig emitirá un aviso (*warning*) durante la fase de configuración de CMake y **descartará el valor solicitado**,

#### Autoevaluación
| Pregunta | ¿Respondida correctamente? | Qué me faltó |
| --- | --- | --- |
| 1 | parcial | Distinguir los `default` de los símbolos en `Kconfig`/`Kconfig.defconfig` de board y SoC del `<board>_defconfig`, y que este último lo mantiene el proyecto Zephyr, no el fabricante. |
| 2 | sí | |
| 3 | sí | |

**Resultado:** ✅ aprobada (2 sí + 1 parcial)
**Dudas que quedaron:** Diferencias entre `_defconfig` y `Kconfig.defconfig`

## e01.6 — Compilacion condicional

**Fecha:** 2026-11-16 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 (`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`) · **Commit:** `e01.6: Archivo fuente condicional segun opcion Kconfig` (pendiente)
**Estado:** ✅ checkpoint verificado

### Objetivo
Compilar condicionalmente un fuente para que se incluya o no en el binario final
Checkpoint: ver aparecer y desaparecer el modulo en el rom_report segun si la opcion de configuracion esta activa o no.

### Qué hice
- Cree un fuente y su correspondiente cabecera `feat_conditional.c` y `feat_conditional.h`
- Cree un archivo nuevo `Kconfig` en la raiz de la app con una opcion nueva desactivada por default `FEATURE_CONDITIONAL`
- Agregue en el `CMakeLists.txt` la compilacion condicionada a la opcion anterior con `target_sources_ifdef(CONFIG_FEATURE_CONDITIONAL app PRIVATE src/feat_conditional.c)` (Kconfig agrega el prefijo `CONFIG_` al simbolo `FEATURE_CONDITIONAL`; el nombre completo lo escribo yo explicitamente en CMake)
- En `main.c` incluyo y llamo a la funcion del modulo condicional solo si la macro correspondiente esta definida `#ifdef CONFIG_FEATURE_CONDITIONAL`. Esta macro es generada automaticamente en `autoconf.h` por zephyr
- Compile 2 veces, una usando `-- -DCONFIG_FEATURE_CONDITIONAL=y` y otra sin nada con `west` y genere en ambos casos el `rom_report`

```bash
$ west build -p always -b esp32s3_devkitc/esp32s3/procpu ./etapas/e01-app-layout/ -- -DCONFIG_FEATURE_CONDITIONAL=y
$ west build -t rom_report

$ west build -p always -b esp32s3_devkitc/esp32s3/procpu ./etapas/e01-app-layout/
$ west build -t rom_report
```

Archivos tocados: `etapas/e01-app-layout/include/feat_conditional.h`, `etapas/e01-app-layout/src/feat_conditional.c`, `etapas/e01-app-layout/src/main.c`, `etapas/e01-app-layout/Kconfig`

### Resultado

Se pudo encontrar en la rama `src` que aparecio en un build y desaparecio en el otro segun la `CONFIG`

```bash
── /                                                                                              67   0.11%  - 
│   └── home                                                                                       67   0.11%  - 
│       └── uki                                                                                    67   0.11%  - 
│           ├── Documentos                                                                         47   0.08%  - 
│           │   └── codigos                                                                        47   0.08%  - 
│           │       └── learn-zephyr                                                               47   0.08%  - 
│           │           └── etapas                                                                 47   0.08%  - 
│           │               └── e01-app-layout                                                     47   0.08%  - 
│           │                   └── src                                                            47   0.08%  - 
│           │                       ├── feat_conditional.c                                         14   0.02%  - 
│           │                       │   └── feature_conditional                                    14   0.02%  0x4200036c .text
│           │                       ├── main.c                                                     19   0.03%  - 
│           │                       │   └── main                                                   19   0.03%  0x42000348 .text
│           │                       └── saludos.c                                                  14   0.02%  - 
│           │                           └── saludo_base                                            14   0.02%  0x4200035c .text

```

### Evidencia
- [01.6-compilacion-condicional.md](evidencias/e01/01.6-compilacion-condicional.md) — salida completa de las compilaciones + fragmento del `rom_report` 

### Particularidades de la placa
- Dado que se demostro en el micropaso anterior que el reporte no funciona con `native_sim`, el `rom_report` solo se realizo para la placa real `esp32s3` (FLASH: 135060 B con la opcion en `y` vs 135028 B en `n`, +32 B)
- Por la regla de los dos targets, verifique tambien `native_sim`: compila sin warnings con la opcion en `y` y en `n`; con `y` el `.config` tiene `CONFIG_FEATURE_CONDITIONAL=y` y se imprime "Feature conditional is enabled!", con `n` no se imprime

### Qué aprendí y pendiente
- **Aprendizaje:** Crear opcion de `Kconfig` e inyectarla como parametro en la compilacion con west. Ademas se podria usar los archivos `prj.conf` o `boards/*.conf` para habilitarla de forma permanente
- **Pendiente:** ninguno

## e01.5 — Reportes de memoria ROM/RAM en ambos targets

**Fecha:** 2026-11-15 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 (`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`) · **Commit:** `e01.5: Reportes rom_report y ram_report en ambos targets` (ada19ef)
**Estado:** ✅ checkpoint verificado

### Objetivo
Correr `west build -t rom_report` y `-t ram_report` en los dos targets e interpretar qué componente ocupa más.
Checkpoint: saber leer el árbol del reporte y decir qué parte de la memoria es de la app y cuál es del SoC/HAL.

### Qué hice
- Corrí `rom_report` y `ram_report` sobre `esp32s3_devkitc/esp32s3/procpu` con el build ya configurado (`-t` es el target de CMake, la placa se elige con `-b`).
- Repetí ambos reportes con `-b native_sim` (`-p always` en el primero para cambiar de placa).
- Leí `build/rom.json` y `build/ram.json` para ordenar el árbol por tamaño, y para el RAM del ESP32-S3 revisé además `build/zephyr/zephyr_final.map` y `readelf -SW zephyr.elf`.

```bash
west build -t rom_report
west build -t ram_report
west build -p always -b native_sim -t rom_report ./etapas/e01-app-layout/
west build -b native_sim -t ram_report ./etapas/e01-app-layout/
```

Archivos tocados: ninguno de la app (solo evidencia en `docs/evidencias/e01/`).

### Resultado

**ESP32-S3.** ROM total 58 292 B, repartido así:

| Componente | Bytes | % |
| --- | --- | --- |
| HAL de Espressif (`WORKSPACE/modules/hal/espressif`) | 34 288 | 58.8 |
| Zephyr propio (`ZEPHYR_BASE`) | 14 754 | 25.3 |
| `(hidden)` | 6 998 | 12.0 |
| `(no paths)` (incluye `__l_vfprintf`, 1 416 B) | 2 200 | 3.8 |

Dentro del HAL: `esp_hw_support` 9 808 B, `spi_flash` 7 956 B, `esp_hal_mspi` 5 301 B. Dentro de Zephyr: `drivers` 6 812 B, `kernel` 3 363 B.

RAM: el JSON suma 123 160 B, pero el 90.3 % es `(hidden)` (111 224 B), un nodo sin hijos. Lo que se pudo atribuir con nombre, confirmado en el `.map`:

| Símbolo | Bytes |
| --- | --- |
| `kheap__system_heap` (`mempool.c`) | 4 164 |
| `z_interrupt_stacks` (`init.c`, `ISR_STACK_SIZE=2048`) | 2 048 |
| `z_main_stack` (`init.c`, `MAIN_STACK_SIZE=1024`) | 1 024 |
| stack idle (`init.c`, `IDLE_STACK_SIZE=1024`) | 1 024 |

Las secciones de datos del enlazador (`.dram0.data` + `.dram0.noinit` + `.dram0.bss`) suman unos 15.5 KB, que es la RAM real de la app. Del `(hidden)`, `.dram0.dummy` (22.5 KB) y el IRAM (~38 KB) explican unos 60 KB; **el resto no lo pude atribuir**.

**native_sim.** ROM 10 642 B y RAM 5 557 B, con ~94 % y ~89 % en `(hidden)`. Lo único con nombre es `__udivmoddi4` (453 B), `native_sim` (125 B) y archivos de pocos bytes. El reporte no sirve para medir tamaño: la app es un ejecutable de Linux, sin regiones de flash/RAM, y el runtime del host no entra en el análisis.

### Mediciones
| Métrica | native_sim | ESP32-S3 | Método / comando |
| --- | --- | --- | --- |
| ROM (flash) | 10 642 B (no comparable) | 58 292 B | `west build -t rom_report` |
| RAM | 5 557 B (no comparable) | 123 160 B en el JSON; ~15.5 KB en secciones de datos | `west build -t ram_report` |

### Evidencia
- [01.5-reports.md](evidencias/e01/01.5-reports.md) — salida completa de los cuatro reportes

### Problemas y diagnóstico
- **Síntoma:** en `native_sim` ambos reportes salen casi vacíos (`(hidden)` ~90 %). En el ESP32-S3, el RAM también tiene un `(hidden)` enorme.
- **Hipótesis descartadas:** que fuera un error de mi build. Los nodos con nombre coinciden con el `.map` en el ESP32-S3, así que el reporte está bien armado.
- **Causa raíz:** en `native_sim` no hay imagen de firmware; en el ESP32-S3, el origen de los 111 KB de `(hidden)` queda sin verificar (hay relleno `.dram0.dummy` y código en IRAM, pero no cubre todo).
- **Arreglo:** no hay arreglo; el método es usar una placa con firmware real y cruzar con el `.map`.
- **Método para la próxima vez:** nunca leer el total del reporte; mirar los nodos con nombre de archivo y verificarlos en `zephyr_final.map`.

### Particularidades de la placa
- El RAM total del reporte del ESP32-S3 incluye IRAM y relleno del enlazador, por eso no es el consumo real de la app.

### Qué aprendí y pendiente
- **Aprendizaje:** `-t` es el target de CMake y la placa va con `-b`. Los reportes se leen de arriba hacia abajo por componente, y `native_sim` no sirve para medir memoria.
- **Pendiente:** el ROM del reporte (58 292 B) no coincide con el `FLASH` de la tabla del build (135 028 B en 01.4); no sé aún por qué. Tampoco sé qué contiene el `(hidden)` de 111 KB del RAM.

## e01.4 — Consola del sistema en device tree

**Fecha:** 2026-11-14 · **Sesiones:** 1 · **Target:** esp32s3
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 (`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`) · **Commit:** `e01.4: Identificado la consola del sistema en dts final` (a2022e1)
**Estado:** ✅ checkpoint verificado

### Objetivo
Encontrar el nodo chosen en el device tree final `build/zephyr/zephyr.dts` e identificar cual es la consola del sistema
Checkpoint: sabés qué periférico y qué puerto USB estás usando para ver los logs.

### Qué hice

Compile nuevamente la aplicacion para el target `esp32s3_devkitc_procpu` y utilice `grep` para buscar el nodo chosen en el device tree final resultante del build.
Con eso pude ver todo el contenido del nodo chosen, que es utilizado para mapear caracteristicas del sistema con un periferico especifico, que cumplira una labor global en el sistema (no es un nodo de hardware ordinario)

```bash
$ grep -A 10 "chosen {" build/zephyr/zephyr.dts
    chosen {
        zephyr,canbus = &twai;                    /* in zephyr/dts/xtensa/espressif/esp32s3/esp32s3_common.dtsi:22 */
        zephyr,entropy = &trng0;                  /* in zephyr/dts/xtensa/espressif/esp32s3/esp32s3_common.dtsi:23 */
        zephyr,flash-controller = &flash;         /* in zephyr/dts/xtensa/espressif/esp32s3/esp32s3_common.dtsi:24 */
        zephyr,sram = &sram1;                     /* in zephyr/boards/espressif/esp32s3_devkitc/esp32s3_devkitc_procpu.dts:26 */
        zephyr,console = &uart0;                  /* in zephyr/boards/espressif/esp32s3_devkitc/esp32s3_devkitc_procpu.dts:27 */
        zephyr,shell-uart = &uart0;               /* in zephyr/boards/espressif/esp32s3_devkitc/esp32s3_devkitc_procpu.dts:28 */
        zephyr,flash = &flash0;                   /* in zephyr/boards/espressif/esp32s3_devkitc/esp32s3_devkitc_procpu.dts:29 */
        zephyr,code-partition = &slot0_partition; /* in zephyr/boards/espressif/esp32s3_devkitc/esp32s3_devkitc_procpu.dts:30 */
        zephyr,bt-hci = &esp32_bt_hci;            /* in zephyr/boards/espressif/esp32s3_devkitc/esp32s3_devkitc_procpu.dts:31 */
    };
```

### Resultado

Se encontro que los nodos `zephyr,console` y `zephyr,shell-uart` utilizan ambos la `uart0` del micro que esta fisicamente mapeado con el bridge UART/USB en la placa, y que el puerto nativo es el que Zephyr tiene mapeado al OTG. El bloque Serial/JTAG está deshabilitado en el DT de esta board.

```bash
    zephyr,console = &uart0;
    zephyr,shell-uart = &uart0;
```

### Evidencia
- [01.4-consola-dts.md](evidencias/e01/01.4-consola-dts.md) — el entregable

### Qué aprendí y pendiente
- **Aprendizaje:** los nodos standar del sistema tienen el prefijo `zephyr,` y se concentran dentro de `chosen`.
- **Pendiente:** con usb_otg activo, ¿sigue funcionando el JTAG del puerto nativo?

## e01.3 — Configuracion de software orden de precedencia en .conf

**Fecha:** 2026-11-12 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 (`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`) · **Commit:** `e01.3: Precedencia prj.conf vs boards/<board>.conf` (-)
**Estado:** ✅ checkpoint verificado

### Objetivo
Encontrar el valor resultante de la opcion configurada en la config final `build/zephyr/.config` en cada target.
Checkpoint: definir el orden de precedencia de la configuracion

### Qué hice

- Agregué una nueva entrada en la config de software del proyecto `prj.conf` con distinto valor de la misma opción `CONFIG_MAIN_STACK_SIZE=3072`. Quiero ver cuál prevalece
- Compilé para ambos targets con `-p always` 
- Finalmente ejecuté el comando `$ grep -E "^(CONFIG_MAIN_STACK_SIZE=|# CONFIG_MAIN_STACK_SIZE is not set)" build/zephyr/.config` para ver el valor post-merge. 

El sistema de compilación de Zephyr resuelve las variables de Kconfig evaluándolas en el siguiente orden estricto de menor a mayor prioridad (donde cada capa posterior sobrescribe las anteriores)

1. Defaults del árbol Kconfig `(Kconfig / Kconfig.defconfig base)` [ Nivel 1: Menor prioridad ] 
2. defconfig de la placa `(boards/.../<BOARD>_defconfig)`
3. `prj.conf` (Configuración global de la aplicación)
4. `boards/<BOARD>.conf` (Configuración específica por placa en la app) [ Nivel 4: Mayor prioridad ]

*Se pudo obtener el orden de precedencia del log de west durante la compilación*

Archivos tocados: `etapas/e01-app-layout/prj.conf`

### Resultado

Se observó coincidencia con el presupuesto teorico y pudimos comprobar: 

1- **Config global del proyecto `prj.conf` pisa a `Kconfig` base y `boards/.../<BOARD>_defconfig` de la placa**
2- **Config de la placa del proyecto `boards/<board>.conf` pisa a config global del proyecto `prj.conf` y por ende a los otros 2 archivos de configuracion**

*El orden entre `kconfig` y `<board>_defconfig` no se comprueba aqui, ya que escapa los objetivos de este micropaso*

```bash
# Resultado native_sim
$ grep -E "^(CONFIG_MAIN_STACK_SIZE=|# CONFIG_MAIN_STACK_SIZE is not set)" build/zephyr/.config
CONFIG_MAIN_STACK_SIZE=2048

# Resultado esp32s3
$ grep -E "^(CONFIG_MAIN_STACK_SIZE=|# CONFIG_MAIN_STACK_SIZE is not set)" build/zephyr/.config
CONFIG_MAIN_STACK_SIZE=1024

# Resultado para prj.conf (sin conf por board)
$ grep -E "^(CONFIG_MAIN_STACK_SIZE=|# CONFIG_MAIN_STACK_SIZE is not set)" build/zephyr/.config
CONFIG_MAIN_STACK_SIZE=3072
```

> También se puede observar desde el log de west durante la compilación

```bash
# Ejemplo native_sim
Parsing /home/uki/zephyrproject/zephyr/Kconfig
Loaded configuration '/home/uki/zephyrproject/zephyr/boards/native/native_sim/native_sim_defconfig' # kconfig placa soportada por zephyr
Merged configuration '/home/uki/Documentos/codigos/learn-zephyr/etapas/e01-app-layout/prj.conf' # kconfig general del proyecto
Merged configuration '/home/uki/Documentos/codigos/learn-zephyr/etapas/e01-app-layout/boards/native_sim.conf' # kconfig de la placa dentro del proyecto
Configuration saved to '/home/uki/Documentos/codigos/learn-zephyr/build/zephyr/.config' # Resultado
Kconfig header saved to '/home/uki/Documentos/codigos/learn-zephyr/build/zephyr/include/generated/zephyr/autoconf.h'

# Ejemplo native_sim sin boards/
Parsing /home/uki/zephyrproject/zephyr/Kconfig
Loaded configuration '/home/uki/zephyrproject/zephyr/boards/native/native_sim/native_sim_defconfig'
Merged configuration '/home/uki/Documentos/codigos/learn-zephyr/etapas/e01-app-layout/prj.conf'
Configuration saved to '/home/uki/Documentos/codigos/learn-zephyr/build/zephyr/.config'
Kconfig header saved to '/home/uki/Documentos/codigos/learn-zephyr/build/zephyr/include/generated/zephyr/autoconf.h'
```

### Evidencia
- [01.3-conf-precedencia.md](evidencias/e01/01.3-conf-precedencia.md) — el entregable

### Qué aprendí y pendiente
- **Aprendizaje:** como se aplican las diferentes partes de la config de software (`kconfig`) y como buscar simbolos en el kconfig final con `grep`.
- **Pendiente:** ninguno

## e01.2 — Configuracion de software .conf segun target (board)

**Fecha:** 2026-11-11 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 (`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`) · **Commit:** `e01.2: Configs de software distintas segun board` (-)
**Estado:** ✅ checkpoint verificado

### Objetivo
Crear 2 archivos `<board>.conf` dentro de `/boards` y configurar la misma opcion con valores distintos en cada target
Checkpoint: el `build/zephyr/.config` resultante luego de la compilacion difiere en dicha opcion

### Qué hice

- Cree ambos archivos `boards/*.conf` con distinto valor de la misma opción `CONFIG_MAIN_STACK_SIZE`.
- Compile para ambos targets con `-p always` y guardando el `build/zephyr/.config` generado en cada caso.
- Finalmente ejecute el comando `$ diff` entre ambos archivos y busque el valor de la opcion en la salida de la consola.

Archivos tocados: `etapas/e01-app-layout/boards/`

### Resultado
Se observo la diferencia esperada con los 2 valores configurados dentro de `boards/*.conf`

> Se observan mas diferencias en el diff debido a que ambas boards tienen **distinta arquitectura** y por lo tanto su kconfig base es distinto

```bash
$ diff docs/evidencias/e01/native.conf docs/evidencias/e01/esp32s3.conf

...
...
38c34
< CONFIG_MAIN_STACK_SIZE=2048
---
> CONFIG_MAIN_STACK_SIZE=1024
45,46c41,42
...
...
```

### Evidencia
- [01.2-boards-conf.md](evidencias/e01/01.2-boards-conf.md) — el entregable
- [native.conf](evidencias/e01/native.conf) — config resultante luego del merge para native_sim
- [esp32s3.conf](evidencias/e01/esp32s3.conf) — config resultante luego del merge para esp32s3

### Qué aprendí y pendiente
- **Aprendizaje:** Donde buscar el resultado luego de la compilacion de la configuracion de software `build/zephyr/.config`.
- **Pendiente:** ninguno

## e01.1 — Construir app desde cero

**Fecha:** 2026-11-10 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 (`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`) · **Commit:** `e01.1: app layout desde cero` (572a68e)
**Estado:** ✅ checkpoint verificado

### Objetivo
Crear una app con 2 archivos fuentes y un header file. Modificar cmake file en consecuencia
Checkpoint: la app compila para ambas placas

### Lectura dirigida
- **Qué problema resuelve:** compilacion basada en la aplicacion, zephyr se recompila con tu app para lograr el binario con huella de flash mas pequeña segun tu configuracion. Desacopla app de la placa, biblotecas y arquitectura del soc, permitiendo personalizar configuraciones y librerias utilizadas sin tocar el framework ni el RTOS.
- **APIs / opciones centrales (2–3):** CMakeLists.txt:
    - Vincular el build system de tu app con el de zephyr: `find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})`
    - Agregar directorios de busqueda de header files: `target_include_directories(app PRIVATE include)`
    - Agregar fuentes a la compilacion: `target_sources(app PRIVATE src/main.c src/saludos.c)`
- **Limitación que menciona la doc:** 
    - Orden de las llamadas en CMakeLists.txt: project() y target_sources(app ...) deben ir después de find_package(Zephyr). project() va después para no interferir con project(Zephyr-Kernel), y app es un target que define find_package().
    - El build system recompila solo lo que cree afectado por los cambios, y a veces falla y no recompila archivos necesarios. En ese caso hay que forzar una compilación desde cero (west build -p always). Es lo que me pasó al cambiar de app
- **Fuentes leídas:** 
    - [Cuaderno propio NotebookLM](https://notebook.google.com/notebook/f26a63d8-ac5b-4c68-923d-04f407143172)

### Qué hice

En primer lugar cree el `CMakeLists.txt`, procurando la vinculacion de mi app con zephyr a traves de `find_package()` e incluir en el target lib de zephyr tanto los fuentes con `target_sources()`, como los directorios de busqueda de headers files `target_include_directories()`.

En segundo lugar cree el `prj.conf` para habilitar la escritura por consola (libreria `printk`) en el proyecto, por lo que es necesario habilitar un paquete de software a traves de kconfig.

Luego cree los fuentes en `src/*` con el punto de entrada de la app (`main.c`) y un modulo para realizar saludos (`saludos.c`). Finalmente cree el header file para el modulo de saludos en `include/saludos.h`.

Archivos tocados: `etapas/e01-app-layout/`

### Resultado
Se observo el saludo en la consola para ambas placas, por lo que se pudo comprobar el correcto funcionamiento del `CMakeLists.txt` y `prj.conf`

### Evidencia
- [01.1-crear-app.md](evidencias/e01/01.1-crear-app.md) — el entregable

### Problemas y diagnóstico
- **Síntoma:** `west build` se negó a compilar:

```bash
$ west build -b native_sim etapas/e01-app-layout/
ERROR: Build directory "/home/uki/Documentos/codigos/learn-zephyr/build" is for application "/home/uki/Documentos/codigos/learn-zephyr/etapas/e00-entorno/hello_world", but source directory "/home/uki/Documentos/codigos/learn-zephyr/etapas/e01-app-layout" was specified; please clean it, use --pristine, or use --build-dir to set another build directory
FATAL ERROR: refusing to proceed without --force due to above error
```

- **Causa raíz:** `build/` quedó configurado para la app de e00 (`hello_world`) y west no permite reutilizarlo para otra aplicación.
- **Arreglo:** `$ west build -p always -b native_sim etapas/e01-app-layout/`
- **Método para la próxima vez:** usar siempre `-p always` al cambiar de app o de target, o un `--build-dir` distinto por etapa.

### Particularidades de la placa
- **Puerto USB:** `west flash` autoseleccionó `/dev/ttyACM0` (USB-Serial/JTAG), pero el monitor lo abrí en `/dev/ttyUSB0` y mostró los logs. Hay que determinar en 1.4 cuál es la consola real y por qué.
- **Warning de arranque:** `SHA-256 comparison failed ... Attempting to boot anyway...`. La app arranca igual; no lo investigué en este micro-paso.

### Qué aprendí y pendiente
- **Aprendizaje:** la estructura correcta del `CMakeLists.txt`
- **Pendiente:** ninguno

## Defensa e00
*Respondida el 2026-11-09, sin consultar documentación ni notas.*

**1. ¿Qué diferencia hay entre el repo `zephyr` y el workspace de west?**

Respuesta:
### Zephyr
Es un repositorio ubicado *dentro* del west workspace, cuyo principal proposito es proveer el codigo fuente del kernel del RTOS y las reglas de compilacion con sus scripts auxiliares.
Ademas contiene drivers de dispositivos, descripcion de arquitectura de procesadores, subsistemas, test y ejemplos.

### West workspace
Es el `top dir` (directorio, no repositorio) del ecosistema y alberga la config de west, el repositorio de zephyr, librerias de terceros y HALs de fabricantes en `modules/`, herramientas en `/tools` y bootloaders como mcuBoot en `bootloader/`.
Se encarga de coordinar todos estos repositorios y sincronizar con sus versiones correctas para lograr una compilacion coherente

El manifest repository es aquel que contiene el archivo `west.yml` en su raiz y fija que modulos y versiones requiere el ecosisma, los cuales son sincronizados con `west update`.
El workspace es definido con el directorio oculto `.west` donde se aloja ademas la config de west

**2. ¿Por qué Espressif distribuye los blobs de RF por separado y qué implica para tu proyecto?**

Respuesta:

Los blobs son archivos binarios ya compilados que en este caso Espressif distribuye empaquetado. Habilitan el uso de RF, tanto WiFi como bluetooth. No estan como codigo fuente en el repo del hal porque se considera ventaja comercial de la empresa y porque al tener certificaciones de RF sobre su SOC, es mas facil mantenerlas si el codigo certificado no es modificable por el usuario final. En la practica, cada vez que se hace un west update -> hay que repetir la descarga de los blobs a demanda con `west blobs fetch` o sus funcionalidades asociadas pueden dejar de funcionar o tener errores de compilacion o linker por simbolos no definidos que cambiaron.

**3. ¿Qué pasa si no borrás `build/` tras cambiar `prj.conf`?**

Respuesta:

Tus cambios en el proj.conf pueden no impactarse, dado que en `/build` se guarda una cache de CMake y de .config que es el resultado de la consolidacion final jerarquica de todas las reglas Kconfig (tu app, la placa, valores por defecto, etc).
Una forma de evitar este problema es, a la hora de compilar, utilizar la opcion `-p always` pero solo cuando se se cambia de board, de app u opciones de prj.conf. No es necesario para cambio en el codigo fuente del proyecto y con `-p auto` puede detectar cuando se requiere un borrado de la misma o hacer compilacion incremental

#### Autoevaluación
| Pregunta | ¿Respondida correctamente? | Qué me faltó |
| --- | --- | --- |
| 1 | sí | Falta decir que `zephyr` es el manifest repository de *este* workspace (`west init -m`)|
| 2 | parcial | "Ventaja comercial" y "certificaciones de RF" están afirmados sin fuente: son hipótesis, no hechos. Falta el motivo documentado (licencia de los binarios distinta a la del árbol de Zephyr). Lo de "símbolos no definidos tras `west update`" es plausible pero no lo reprodujiste. |
| 3 | parcial | La idea central es correcta, pero la afirmación "los cambios de `prj.conf` pueden no impactarse" es imprecisa y no está probada. Falta distinguir qué detecta CMake solo y qué no (p. ej. archivos nuevos como `boards/*.conf` u overlays, cambio de board/app con el mismo `build/`). |

**Resultado:** ✅ aprobada (1 sí + 2 parciales; las correcciones de 2 y 3 quedan como pendiente)
**Dudas que quedaron:** fuente documentada del motivo de los blobs; experimento para la pregunta 3 (cambiar `prj.conf` sin `-p always` y comprobar en `build/zephyr/.config`).

## e00.7 — Documentar el entorno (`docs/setup.md`)

**Fecha:** 2026-11-03 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 (`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`) · **Commit:** `e00.7: setup.md para reproducir el entorno` (0b372b4)
**Estado:** 🟡 parcial (documento escrito, reproducción desde cero sin probar)

### Objetivo
Escribir `docs/setup.md` con comandos exactos, versiones y el commit de Zephyr en uso.
Checkpoint: otra persona podría reproducir el entorno.

### Qué hice
Redacté `docs/setup.md` a partir de las evidencias 00.1 a 00.6 y de consultas de solo lectura al
entorno real para completar las versiones. No compilé ni flasheé nada en este micro-paso.

```bash
$ west --version; west list zephyr; west list hal_espressif
$ cd ~/zephyrproject/zephyr && git rev-parse HEAD && git describe --tags --always && cat VERSION
$ cat ~/zephyr-sdk-1.0.1/sdk_version
$ python3 --version; pip --version; pip freeze
$ cmake --version; ninja --version; dtc --version; gcc --version
$ dpkg-query -W -f='${Package} ${Version}\n' <paquetes apt de 0.1>
$ lsb_release -d; uname -r; id -nG
```

Archivos tocados: `docs/setup.md`, `docs/bitacora.md`

### Resultado
`setup.md` cubre: tabla de versiones, dependencias apt, venv y west, workspace y SDK, fijado del
commit de Zephyr, blobs, variables de sesión (`ZEPHYR_BASE`), build/run/flash en ambos targets,
particularidades de la placa, permisos (`dialout`) y `pip freeze` completo (179 paquetes).

### Evidencia
- [setup.md](setup.md) — el entregable
- Las versiones salen de [00.1](evidencias/e00/00.1-entorno.md) a [00.6](evidencias/e00/00.6-out_of_tree.md) y de las consultas listadas arriba (no guardadas como archivo).

### Problemas y diagnóstico
- **Hallazgo:** `west init --mr main` sigue una rama que se mueve; repetir los comandos hoy no da el mismo árbol. `west list zephyr` muestra `HEAD` (el manifest repo no tiene rev fija), por eso el commit se obtiene con `git rev-parse HEAD` dentro de `zephyr/`.
- **Arreglo:** `setup.md` indica `git checkout <commit>` en `zephyr/` antes de `west update`.
- **Sin verificar:** ese procedimiento de fijado no lo corrí. Tampoco hay evidencia de cuándo se agregó el usuario al grupo `dialout`.

### Qué aprendí y pendiente
- **Aprendizaje:** reproducibilidad real = versión de cada herramienta + commit exacto de Zephyr + lista de paquetes Python (las dependencias de Zephyr no están pineadas con `==`).
- **Pendiente:** probar `setup.md` en un directorio/usuario limpio para pasar el estado a ✅; decidir si investigo las advertencias de SHA-256/tamaño de flash (arrastrado de 0.5); 0.8 (opcional, `west debug`).

## e00.6 — Compilar hello world como Out of Tree

**Fecha:** 2026-11-03 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 · **Commit:** `e00.6: Hello world como app out of tree` (14d95b9)
**Estado:** ✅ checkpoint verificado

### Objetivo
Compilar y flashear `samples/hello_world` en ambos targets desde un directorio fuera del workspace de zephyr
Checkpoint: Salida de consola con saludo propio y nombre de la placa para ambos targets

### Qué hice

```bash
$ cp -r ~/zephyrproject/zephyr/samples/hello_world/ ./etapas/e00-entorno/
$ export ZEPHYR_BASE=~/zephyrproject/zephyr

# Compilacion y flasheo en native_sim
$ west build -p always -b native_sim ./etapas/e00-entorno/hello_world/
$ west build -t run
-- west build: running target run
[2/3] cd /home/uki/Documentos/codigos/learn-zephyr/build && /home/uki/Documentos/codigos/learn-zephyr/build/zephyr/zephyr.exe
*** Booting Zephyr OS build v4.5.0-rc3-10-gbc5d59d1b665 ***
Hello World desde mi código! native_sim/native

...

# Compilacion y flaseo en esp32s3_devkitc/esp32s3/procpu
$ west build -p always -b esp32s3_devkitc/esp32s3/procpu ./etapas/e00-entorno/hello_world/
$ west flash
$ west espressif monitor -p /dev/ttyUSB0

...

*** Booting Zephyr OS build v4.5.0-rc3-10-gbc5d59d1b665 ***
Hello World desde mi código! esp32s3_devkitc/esp32s3/procpu
```

Archivos tocados: `~/Documentos/codigos/learn-zephyr/etapas/e00-entorno/hello_world/*`

### Resultado
Compilacion y ejecucion exitosa del ejemplo, pudiendo visualizar la salida por consola desde el monitor
para ambos targets

### Evidencia
- [00.6-out_of_tree.md](evidencias/e00/00.6-out_of_tree.md)

### Problemas y diagnóstico
- **Síntoma:** no pude compilar desde mi directorio
```bash
  usage: west [-h] [-z ZEPHYR_BASE] [-v] [-q] [-V] <command> ...
  west: unknown command "build"; do you need to run this inside a workspace?.
```
- **Causa raíz:** west no encontraba el comando build por que estaba siendo ejecutado desde fuera de un workspace
- **Arreglo:** definir la variable de entorno `ZEPHYR_BASE` -> asi busca ahi el directorio `.west/` y por ende detecta el workspace
- **Método para la próxima vez:** lanzar `source ~/zephyrproject/zephyr/zephyr-env.sh` luego de activar el entorno virtual para exportar `ZEPHYR_BASE`.

### Qué aprendí y pendiente
- **Aprendizaje:** tres tipos de app segun donde se ubique el repositorio de la app respecto el repositorio de zephyr. **Repository app** (dentro de `~/zephyrproject/zephyr/`) - **Workspace app** (dentro de west workspace pero fuera de `~/zephyrproject/zephyr/`)- **Freestanding app** (fuera de west).

> - `west zephyr-export` es para que CMake encuentre Zephyr por su cuenta.
> - `ZEPHYR_BASE` es para West encuentre el workspace cuando corre fuera del mismo.
>
> Lo busca subiendo desde el directorio actual hasta un `.west`y sino subiendo desde `ZEPHYR_BASE`

- **Pendiente:** 0.7 (crear archivo para reproduccion del entorno con versiones exactas de cada herramienta)

## e00.5 — Compilar hello world en target real

**Fecha:** 2026-11-01 · **Sesiones:** 1 · **Target:** esp32s3_devkitc/esp32s3/procpu
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 · **Commit:** `e00.5: Compilar y correr hello world en target real` (83bef76)
**Estado:** ✅ checkpoint verificado

### Objetivo
Compilar y flashear `samples/hello_world` en la placa real (esp32s3_devkitc/esp32s3/procpu) y monitorear la consola desde el host.
Checkpoint: Salida de consola con saludo y nombre de la placa

### Qué hice

```bash
$ cd ~/zephyrproject/zephyr
$ west build -p always -b esp32s3_devkitc/esp32s3/procpu samples/hello_world/

$ west flash
$ west espressif monitor -p /dev/ttyUSB0

...
...

*** Booting Zephyr OS build v4.5.0-rc3-10-gbc5d59d1b665 ***
Hello World! esp32s3_devkitc/esp32s3/procpu
```

Archivos tocados: `-`

### Resultado
Compilacion y ejecucion exitosa del ejemplo, pudiendo visualizar la salida por consola desde el monitor

```bash
(.venv) uki@uki-HP-Laptop-15-dy2xxx:~/zephyrproject/zephyr$ west espressif monitor -p /dev/ttyUSB0
--- idf_monitor on /dev/ttyUSB0 115200 ---
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x18 (SPI_FAST_FLASH_BOOT)
SPIWP:0xee
Octal Flash Mode Enabled
For OPI Flash, Use Default Flash Boot Mode

...
...

*** Booting Zephyr OS build v4.5.0-rc3-10-gbc5d59d1b665 ***
Hello World! esp32s3_devkitc/esp32s3/procpu
```

### Evidencia
- [00.5-target_real.md](evidencias/e00/00.5-target_real.md)

### Problemas y diagnóstico
- **Síntoma:** al enchufar la placa no aparecía en `lsusb` ni `/dev/ttyACM*`/`/dev/ttyUSB*`. `journalctl -k` mostraba:
  `usb 1-1: device descriptor read/64, error -71` y `usb usb1-port1: unable to enumerate USB device`.
- **Hipótesis descartadas:** puerto de la placa (falló igual en `UART` y en `USB`) y puerto de la PC (`port1` y `port2`). Drivers, permisos y Zephyr no aplican: el fallo ocurre antes de cargar drivers.
- **Causa raíz:** el cable. Con otro cable la placa enumeró de inmediato.
- **Arreglo:** cambiar el cable por uno que transfiera datos.
- **Método para la próxima vez:** `journalctl -k -f` mientras se enchufa. `error -71` = problema eléctrico/de protocolo del bus (cable, puerto), no de software. Ver `lsusb` y `ls -l /dev/serial/by-id/`.
- **Advertencias en el arranque (no resueltas):** el bootloader imprime `SHA-256 comparison failed ... Attempting to boot anyway` y `Detected size(32768k) larger than the size in the binary image header(8192k)`. Arranca igual. El build dijo `SHA256 digest is not appended`, lo que encaja con el hash esperado de ceros; es una hipótesis, no la verifiqué.

### Particularidades de la placa
- **Dos puertos USB:** `USB` (nativo, Espressif USB-Serial/JTAG, `303a:1001`, `/dev/ttyACM0`) y `UART` (Silicon Labs CP2102N, `10c4:ea60`, `/dev/ttyUSB0`).
- **Consola:** `zephyr,console = &uart0` en `esp32s3_devkitc_procpu.dts` y `CONFIG_ESP_CONSOLE_UART_NUM=0`, por eso la salida de la aplicación sale por el puerto `UART`. El bootloader sí se ve por el puerto `USB`.
- **Flasheo:** `west flash` autoseleccionó `/dev/ttyACM0` (puerto `USB`).
- **`--reset-type watchdog-reset`:** no hizo falta. `west flash` normal reinició y arrancó solo.
- Los números `ttyACM*`/`ttyUSB*` pueden cambiar entre conexiones; `/dev/serial/by-id/` es estable.

### Qué aprendí y pendiente
- **Aprendizaje:** el puerto `USB` sirve para flashear, y cuando Zephyr toma el control los mensajes de la aplicación salen por `UART`. Necesito ambos puertos conectados para flashear y ver el hello world, y especificar con `-p /dev/ttyUSB0` el puerto del monitor. Ante un error de enumeración USB, lo primero a descartar es el cable.
- **Pendiente:** 0.6 (`hello_world` en ambos targets desde directorio local, fuera del árbol de Zephyr). Decidir si investigo las advertencias de SHA-256 y tamaño de flash.

## e00.4 — Compilar hello world en native_sim

**Fecha:** 2026-10-28 · **Sesiones:** 1 · **Target:** native_sim
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 · **Commit:** `e00.4: Compilar y correr hello world en native_sim` (4ae5857)
**Estado:** ✅ checkpoint verificado

### Objetivo
Primera compilacion en zephyr. Se compila un ejemplo en placa native_sim que corre en el host
Checkpoint: Salida de consola con saludo y nombre de la placa

### Qué hice

```bash
$ cd ~/zephyrproject/zephyr

$ west build -p always -b native_sim samples/hello_world/

$ west build -t run
```

Archivos tocados: `-`

### Resultado
Compilacion y ejecucion exitosa del ejemplo, pudiendo visualizar la salida por consola

```bash
(.venv) uki@uki-HP-Laptop-15-dy2xxx:~/zephyrproject/zephyr$ west build -t run
-- west build: running target run
[2/3] cd /home/uki/zephyrproject/zephyr/build && /home/uki/zephyrproject/zephyr/build/zephyr/zephyr.exe
*** Booting Zephyr OS build v4.5.0-rc3-10-gbc5d59d1b665 ***
Hello World! native_sim/native
```

### Evidencia
- [00.4-native_sim.md](evidencias/e00/00.4-native_sim.md)

### Qué aprendí y pendiente
- **Aprendizaje:** Native_sim es una plataforma donde se compila zephyr y la aplicacion con el tooling de host. Genera un binario standar ejecutable desde la pc que permite testear y depurar con herramientas de host facilmente.
- **Pendiente:** 0.5 (`hello_world` en placa real `esp32s3_devkitc/esp32s3/procpu`).

## e00.3 — Blobs de Espressif

**Fecha:** 2026-10-26 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 · **Commit:** `e00.3: Descarga de blobs de Espressif` (75acaba)
**Estado:** ✅ checkpoint verificado

### Objetivo
Descargar los binarios de Espressif. Por que son necesarios? Para el uso de RF (WiFi, BLE)
Checkpoint: Blobs descargados

### Qué hice

```bash
$ cd ~/zephyrproject

$ source .venv/bin/activate

$ west blobs fetch hal_espressif
```

Archivos tocados: `-`

### Resultado
Ahora se puede usar las funciones de RF: WiFi y BLE
A significa presente (M es fallo de hash y D es ausente).

```bash
(.venv) uki@uki-HP-Laptop-15-dy2xxx:~/zephyrproject$ west blobs list hal_espressif
hal_espressif A lib/esp32c5/libble_app.a lib /home/uki/zephyrproject/modules/hal/espressif/zephyr/blobs/lib/esp32c5/libble_app.a

...
...

hal_espressif A lib/esp32s3/libcoexist.a lib /home/uki/zephyrproject/modules/hal/espressif/zephyr/blobs/lib/esp32s3/libcoexist.a
```

### Evidencia
- [00.3-blobs.md](evidencias/e00/00.3-blobs.md)

### Qué aprendí y pendiente
- **Aprendizaje:** Siempre activar el entorno de python venv, si no west no existe. Cada vez que se hace un `$ west update` se debe descargar los blobs porque al no estar en el repositorio del modulo `hal_espressif` no se sincroniza automaticamente `west update`. 
- **Pendiente:** 0.4 (`hello_world` en `native_sim`).

## e00.2 — Inicializar workspace y tooling

**Fecha:** 2026-10-25 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 · **Commit:** `e00.2: Workspace west inicializado y HAL de Espressif presente` (5a1c4c2)
**Estado:** ✅ checkpoint verificado

### Objetivo
Inicializar el workspace y herramientas de python usadas por west
Checkpoint: HAL de Espressif instalado

### Qué hice

```bash
$ cd ~/zephyrproject
$ west init -m https://github.com/zephyrproject-rtos/zephyr --mr main . # --mr main sigue el branch main y no un tag (reproducibilidad de 0.7)
$ west update

$ west packages pip --install
$ west zephyr-export

$ cd ~/zephyrproject/zephyr
$ west sdk install
```

Archivos tocados: `-`

### Resultado
`west init`, `west update`, `west packages pip --install`, `west zephyr-export` y `west sdk install` terminaron sin errores (`All done.` al final de la instalación del SDK).

```bash
(.venv) uki@uki-HP-Laptop-15-dy2xxx:~/zephyrproject/zephyr$ west list | grep hal_espressif
hal_espressif modules/hal/espressif        cc81db00233af6020f2fea9fbb0d0098fb3d0d86 https://github.com/zephyrproject-rtos/hal_espressif

(.venv) uki@uki-HP-Laptop-15-dy2xxx:~/zephyrproject/zephyr$ cat VERSION
VERSION_MAJOR = 4
VERSION_MINOR = 5
PATCHLEVEL = 0
VERSION_TWEAK = 0
EXTRAVERSION = rc3

(.venv) uki@uki-HP-Laptop-15-dy2xxx:~/zephyrproject/zephyr$ git rev-parse HEAD
bc5d59d1b6657cfeef30124b818c8fbd69da68ac

(.venv) uki@uki-HP-Laptop-15-dy2xxx:~/zephyrproject/zephyr$ git describe --tags --always
v4.5.0-rc3-10-gbc5d59d1b66
```

```bash
$ cat ~/zephyr-sdk-1.0.1/sdk_version
1.0.1
```

Zephyr SDK instalado: **1.0.1** (`~/zephyr-sdk-1.0.1`).

### Evidencia
- [00.2-workspace.md](evidencias/e00/00.2-workspace.md)

### Qué aprendí y pendiente
- **Aprendizaje:** zephyr es el manifest repository y no un módulo más, por eso la versión en uso se obtiene dentro de `zephyr/` con `cat VERSION` y `git describe --tags --always`.
- **Pendiente:** 0.3 (`west blobs fetch hal_espressif`) y 0.4 (`hello_world` en `native_sim`).

## e00.1 — Dependencias de host y west

**Fecha:** 2026-10-25 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** n/a (workspace sin inicializar, se hace en 0.2) · **Commit:** `e00.1: Instalacion de host deps y west` (b3b9240)
**Estado:** ✅ checkpoint verificado

### Objetivo
Instalar las dependencias de host, west y entorno de python funcionando.
Checkpoint: CLI de west y cmake responden al comando de version

### Lectura dirigida
- **Qué problema resuelve:** provisionamiento del framework, administracion de sus paquetes y modulos, herramientas de desarrollo, flasheo y depuracion.
Desacopla hardware, de drivers, librerias e IDE, utilizando build system nativos de C y cross compilation
- **APIs / opciones centrales (2–3):** CLI west:
    - Compilacion desde cero para correr en host: `$ west build -p always -b native_sim <ruta/app>`
    - Ejecucion de binario nativo de host: `$ west build -t run`
    - Grabar en un microcontrolador fisico el binario generado: `$ west flash`
- **Limitación que menciona la doc:** siempre activar el entorno virtual de python antes de arrancar una sesion de trabajo
- **Fuentes leídas:** 
    - [Cuaderno propio NotebookLM](https://notebook.google.com/notebook/f26a63d8-ac5b-4c68-923d-04f407143172)
    - [Getting Started de Zephyr](https://docs.zephyrproject.org/latest/develop/getting_started/index.html)

### Qué hice

```bash
$ sudo apt install --no-install-recommends -y git cmake ninja-build gperf ccache dfu-util device-tree-compiler wget python3-dev python3-venv python3-pip make gcc gcc-multilib g++-multilib libsdl2-dev libmagic1

$ mkdir ~/zephyrproject
$ cd ~/zephyrproject/

$ python3 -m venv .venv
$ source .venv/bin/activate

$ pip install --upgrade pip
$ pip install west
```

Archivos tocados: `-`

### Resultado
Todo instalado con normalidad 

```bash
$ west --version
West version: v1.5.0

$ cmake --version
cmake version 3.28.3

CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

### Evidencia
- [00.1-entorno.md](evidencias/e00/00.1-entorno.md)

### Qué aprendí y pendiente
- **Aprendizaje:** Instalacion del entorno y varios conceptos del ecosistema (build system, cross compiler, virtual env de python)
- **Pendiente:** Instalacion de SDK, requiere `$ west init` que se pide en el 0.2 y luego `$ west sdk install` (resuelto en e00.2, SDK 1.0.1)
