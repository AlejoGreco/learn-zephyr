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

## e00.7 — Documentar el entorno (`docs/setup.md`)

**Fecha:** 2026-11-03 · **Sesiones:** 1 · **Target:** ambos
**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 (`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`) · **Commit:** `e00.7: setup.md para reproducir el entorno` (-)
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
