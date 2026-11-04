# Setup del entorno (E0)

Guía para reproducir desde cero el entorno usado en la etapa 0. Todo lo de abajo sale de las
evidencias en `docs/evidencias/e00/` y de consultas al entorno real (versiones) hechas el 2026-11-03.

## 1. Resumen de versiones

| Componente | Versión | Origen |
| --- | --- | --- |
| SO host | Ubuntu 24.04.5 LTS, kernel 7.0.0-38-generic, x86_64 | `lsb_release -d`, `uname -r` |
| Zephyr | `v4.5.0-rc3-10-gbc5d59d1b66` (VERSION 4.5.0-rc3) | `git describe --tags --always` |
| **Commit de Zephyr** | **`bc5d59d1b6657cfeef30124b818c8fbd69da68ac`** (rama `main`) | `git rev-parse HEAD` en `~/zephyrproject/zephyr` |
| Zephyr SDK | 1.0.1 (`~/zephyr-sdk-1.0.1`) | `cat ~/zephyr-sdk-1.0.1/sdk_version` |
| Toolchain ESP32-S3 | GCC 14.3.0, ld.bfd 2.43.1 (`xtensa-espressif_esp32s3_zephyr-elf`) | log de `west build` (00.5) |
| west | 1.5.0 | `west --version` |
| CMake | 3.28.3 | `cmake --version` |
| Ninja | 1.11.1 | `ninja --version` |
| dtc (host) / dtc (SDK) | 1.7.0 / 1.7.0 | `dtc --version` / log de build |
| GCC host | 13.3.0 (Ubuntu 4:13.2.0-7ubuntu1) | `gcc --version` |
| Python | 3.12.3 (en venv `~/zephyrproject/.venv`) | `python3 --version` |
| pip | 26.2.2 | `pip --version` |
| esptool | 5.6.0 | log de `west flash` (00.5) y `pip freeze` |
| ccache | 4.9.1 (Zephyr lo reporta como "unsuitable, requiere ≥ 4.12"; el build igual usa `/usr/bin/ccache` y funciona) | log de build |
| HAL Espressif | `cc81db00233af6020f2fea9fbb0d0098fb3d0d86` | `west list hal_espressif` |

Hardware: ESP32-S3-DevKitC (target `esp32s3_devkitc/esp32s3/procpu`), chip ESP32-S3 (QFN56) rev v0.1,
8 MB PSRAM, dos puertos USB (ver §7).

Layout en disco:

```text
~/zephyrproject/            # workspace west (.west/, .venv/, modules/, zephyr/)
~/zephyr-sdk-1.0.1/         # Zephyr SDK
~/Documentos/codigos/learn-zephyr/   # este repo (apps out-of-tree en etapas/)
```

## 2. Dependencias del host (apt)

```bash
sudo apt install --no-install-recommends -y git cmake ninja-build gperf ccache dfu-util \
  device-tree-compiler wget python3-dev python3-venv python3-pip make gcc gcc-multilib \
  g++-multilib libsdl2-dev libmagic1
```

Versiones instaladas (`dpkg-query`): git 1:2.43.0-1ubuntu7.4, cmake 3.28.3-1build7,
ninja-build 1.11.1-2, gperf 3.1-1build1, ccache 4.9.1-1, dfu-util 0.11-1,
device-tree-compiler 1.7.0-2build1, wget 1.21.4-1ubuntu4.5, python3-dev/venv 3.12.3-0ubuntu2.1,
python3-pip 24.0+dfsg-1ubuntu1.3, make 4.3-4.1build2, gcc/gcc-multilib/g++-multilib 4:13.2.0-7ubuntu1,
libsdl2-dev 2.30.0+dfsg-1ubuntu3.1, libmagic1t64 1:5.45-3build1 (`libmagic1` se resuelve a este).

## 3. Entorno virtual de Python y west

```bash
mkdir ~/zephyrproject
cd ~/zephyrproject
python3 -m venv .venv
source .venv/bin/activate
pip install --upgrade pip      # 24.0 -> 26.2.2
pip install west               # 1.5.0
west --version
cmake --version
```

## 4. Workspace, dependencias Python y SDK

```bash
cd ~/zephyrproject
west init -m https://github.com/zephyrproject-rtos/zephyr --mr main .
west update
west packages pip --install
west zephyr-export             # registra Zephyr en ~/.cmake/packages/Zephyr

cd ~/zephyrproject/zephyr
west sdk install               # lee zephyr/SDK_VERSION -> instala SDK 1.0.1 en ~/zephyr-sdk-1.0.1
```

### Fijar el commit de Zephyr (importante)

`--mr main` sigue una rama que se mueve: ejecutar los comandos de arriba hoy **no** da el mismo
árbol. El árbol de este entorno corresponde a este commit. Para reproducirlo exactamente, después de
`west init` y **antes** de `west update`:

```bash
cd ~/zephyrproject/zephyr
git checkout bc5d59d1b6657cfeef30124b818c8fbd69da68ac
cd ..
west update                    # los módulos se resuelven según west.yml de ese commit
```

Verificación:

```bash
cd ~/zephyrproject/zephyr
cat VERSION                    # 4.5.0, EXTRAVERSION = rc3
git describe --tags --always   # v4.5.0-rc3-10-gbc5d59d1b66
west list hal_espressif        # cc81db00233af6020f2fea9fbb0d0098fb3d0d86
```

Nota: `west list zephyr` muestra `HEAD` como revisión (el manifest repo no tiene rev fija); por eso
el commit se obtiene con `git rev-parse HEAD` dentro de `zephyr/`.

Otros módulos relevantes en este commit: mcuboot `725d63c9d5bc1bb572d802e7a768daf504186daf`,
mbedtls `c43f34b93797e81fd0257c30004dcd0ae332ae51`. La lista completa (70 proyectos) se obtiene con
`west list`; para guardarla fija: `west manifest --freeze`.

## 5. Blobs de Espressif (RF: WiFi/BLE)

No vienen en el repo del HAL, así que `west update` no los sincroniza: hay que repetir este paso tras
cada `west update`.

```bash
cd ~/zephyrproject
source .venv/bin/activate
west blobs fetch hal_espressif
west blobs list hal_espressif   # todos deben figurar con estado A (presente)
```

## 6. Sesión de trabajo (cada terminal nueva)

```bash
source ~/zephyrproject/.venv/bin/activate
source ~/zephyrproject/zephyr/zephyr-env.sh     # exporta ZEPHYR_BASE (necesario para apps fuera del workspace)
# equivalente mínimo: export ZEPHYR_BASE=~/zephyrproject/zephyr
```

Sin `ZEPHYR_BASE`, desde fuera del workspace west falla con
`west: unknown command "build"; do you need to run this inside a workspace?` (ver bitácora e00.6).

## 7. Compilar, correr y flashear

### native_sim (host)

```bash
cd ~/Documentos/codigos/learn-zephyr
west build -p always -b native_sim etapas/e00-entorno/hello_world
west build -t run
# *** Booting Zephyr OS build v4.5.0-rc3-10-gbc5d59d1b665 ***
# Hello World desde mi código! native_sim/native
```

### ESP32-S3-DevKitC (placa real)

```bash
west build -p always -b esp32s3_devkitc/esp32s3/procpu etapas/e00-entorno/hello_world
west flash
west espressif monitor -p /dev/ttyUSB0
# Hello World desde mi código! esp32s3_devkitc/esp32s3/procpu
```

Particularidades de la placa (evidencia 00.5):

- Hay que conectar **los dos puertos USB**: `USB` (Espressif USB-Serial/JTAG, `303a:1001`,
  `/dev/ttyACM0`) sirve para flashear (`west flash` lo autoselecciona); `UART` (Silicon Labs
  CP2102N, `10c4:ea60`, `/dev/ttyUSB0`) es la consola de la aplicación
  (`zephyr,console = &uart0`), por eso el monitor va con `-p /dev/ttyUSB0`.
- Los números `ttyACM*`/`ttyUSB*` pueden cambiar; `ls -l /dev/serial/by-id/` es estable.
- Los cables deben transmitir datos (un cable malo dio `error -71` en `journalctl -k`).
- `--reset-type watchdog-reset` **no** hizo falta.
- Salir del monitor: `Ctrl+]`.
- Al arrancar el bootloader imprime `SHA-256 comparison failed ... Attempting to boot anyway` y
  `Detected size(32768k) larger than the size in the binary image header(8192k)`; arranca igual
  (sin investigar, ver pendientes).

## 8. Permisos del host

Los puertos serie son `root:dialout` (`crw-rw----`). El usuario tiene que pertenecer al grupo
`dialout` (en este equipo `id -nG` incluye `dialout`):

```bash
sudo usermod -aG dialout $USER    # luego cerrar sesión y volver a entrar
```

(No hay evidencia de cuándo ni cómo se agregó este usuario al grupo; el comando es el estándar de
Ubuntu, no lo ejecuté acá.) No hizo falta ninguna regla udev adicional (no existen en `/etc/udev/rules.d`
reglas de Espressif/Zephyr).

## 9. Apéndice: `pip freeze` del venv

Resultado de `pip freeze` tras `pip install west` + `west packages pip --install` (179 paquetes).
Las dependencias de Zephyr no están pineadas con `==` en su `requirements`, por lo que una
instalación futura puede traer versiones distintas. Para igualar el entorno, guardar esta lista como
`requirements-freeze.txt` e instalar con `pip install -r`.

<details>
<summary>Ver lista completa</summary>

```text
anytree==2.13.0
appdirs==1.4.4
argparse-addons==0.12.0
arrow==1.4.0
asn1crypto==1.5.1
ast_serialize==0.13.1
astroid==4.3.4
attrs==26.1.0
base58==1.0.3
beartype==0.23.1
bincopy==20.1.1
bitarray==3.12.3
bitstring==4.4.0
boolean.py==5.0
canopen==2.4.1
capstone==5.0.9
cbor==1.0.0
cbor2==6.1.5
certifi==2026.10.23
cffi==2.1.1
chardet==5.2.0
charset-normalizer==3.5.2
clang-format==23.1.3
click==8.5.0
click-command-tree==1.2.0
click-option-group==0.5.9
cmsis-pack-manager==0.6.0
colorama==0.4.6
colorlog==6.12.0
coverage==7.17.0
crcmod-plus==2.3.6
cryptography==49.0.0
deepmerge==2.1.0
dill==0.4.1
docopt==0.6.2
ecdsa==0.19.2
esp-pylib==1.2.0
esptool==5.6.0
fastjsonschema==2.22.2
filelock==3.32.7
gcovr==8.7
gitdb==4.0.12
gitlint-core==0.19.1
GitPython==3.2.1
graphviz==0.21
grpcio==1.84.1
grpcio-tools==1.68.0
halo==0.0.31
hexdump==3.3
hidapi==0.15.0
html5rdf==1.2.1
humanfriendly==10.0
idna==3.20
ifaddr==0.2.0
imgtool==2.5.0
importlib_metadata==8.9.0
importlib_resources==7.1.0
iniconfig==2.3.1
intelhex==2.3.0
intervaltree==3.2.1
isort==9.0.2
Jinja2==3.1.6
json5==0.17.3
jsonschema==4.26.0
jsonschema-specifications==2025.9.1
junit2html==31.1.4
junitparser==5.0.3
lark==1.3.1
librt==0.16.0
libusb-package==1.0.30.0
libusbsio==2.2.1
libuuu==1.5.243
license-expression==30.4.4
log-symbols==0.0.14
lpc-checksum==3.0.0
lxml==6.1.3
markdown-it-py==4.2.0
MarkupSafe==3.0.4
mccabe==0.7.0
mdurl==0.1.2
mypy==2.4.0
mypy_extensions==1.1.0
narwhals==2.27.1
natsort==8.4.0
numpy==2.5.4
opencv-python==5.0.0.93
oscrypto==1.3.0
owlrl==7.6.2
packaging==26.3
pathspec==1.1.1
patool==4.1.0
pillow==13.0.0
platformdirs==4.11.15
plotly==7.1.0
pluggy==1.7.0
ply==3.11
polib==1.2.0
prettytable==3.18.0
protobuf==5.29.6
psutil==7.2.2
pyaes==1.6.1
pyasn1==0.6.4
pycparser==3.12
pycryptodome==4.2.0
pyDes==2.0.1
pyelftools==0.33
PyGithub==2.10.0
Pygments==2.21.0
PyJWT==2.15.1
pykwalify==1.8.0
pylink-square==1.7.0
pylint==4.1.3
PyNaCl==1.6.2
pyocd==0.46.0
pyparsing==3.3.3
pyserial==3.5
pyshacl==0.40.1
pytest==9.1.1
python-can==4.6.1
python-dateutil==3.0.0
python-debian==1.1.1
python-dotenv==1.2.4
python-magic==0.4.27
pyusb==1.3.1
PyYAML==6.0.3
rdflib==7.6.0
reedsolo==1.7.0
referencing==0.37.0
regex==2026.9.29
requests==2.34.2
reuse @ https://github.com/fsfe/reuse-tool/archive/f94ae0b1bb1d4d2ea91f49df88f3184c5d781a1d.tar.gz#sha256=aa155da6b54c41b0bb7a5652a571a9187c019d71f3b1bb63469acc738cf4e811
rich==15.0.0
rich-click==1.9.9
rpds-py==2026.9.1
ruamel.yaml==0.19.1
ruamel.yaml.clib==0.2.14
ruff==0.14.2
semantic-version==2.10.0
semver==3.1.0
setuptools==83.0.0
setuptools-scm==9.2.2
sh==2.4.0
six==1.17.0
sly==0.5
smmap==5.0.3
sortedcontainers==2.4.0
spdx-python-model==0.0.6
spdx-tools==0.8.5
spdx3-validate==0.0.7
sphinx-lint==1.0.2
spinners==0.0.24
spsdk==3.12.0
spsdk-pyocd==0.3.12
spsdk_mcu_link==0.6.18
sslcrypto==5.3
t61codec==1.0.1
tabulate==0.10.0
termcolor==3.3.0
tibs==0.5.7
tomlkit==0.15.2
tqdm==4.70.1
tree-sitter==0.25.2
tree-sitter-cmake==0.7.2.post1
typing_extensions==4.16.0
tzdata==2026.5
unidiff==1.0.1
uritools==6.1.3
urllib3==2.8.0
vermin==1.9.2
wasmtime==24.0.0
wcwidth==0.9.4
websockets==17.3
west==1.5.0
wrapt==1.17.3
x690==1.0.0.post1
xmltodict==1.0.4
yamllint==1.38.0
zeroconf==0.151.5
zipp==4.1.2
```

</details>
