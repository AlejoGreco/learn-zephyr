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

## e00.3 — Blobs de Espressif

**Fecha:** 2026-10-26 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b66 · **Commit:** `e00.3: Descarga de blobs de Espressif` (-)
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
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b66 · **Commit:** `e00.2: Workspace west inicializado y HAL de Espressif presente` (5a1c4c2)
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
