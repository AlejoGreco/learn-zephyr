# Roadmap de aprendizaje de Zephyr RTOS
### Edición ESP32-S3-DevKitC-1 + native_sim · progresión no lineal

> **Rol del asistente:** docente / tutor técnico.
> **Rol del estudiante:** responsable de escribir, compilar, medir y defender cada entregable.

Este roadmap tiene **20 etapas**, y cada etapa está partida en **micro-pasos de 30–60 minutos**
con su propio checkpoint. Después del tronco inicial (etapas 0–3), el resto se abre en **cuatro
carriles paralelos** que podés recorrer en el orden que quieras: si un tema te aburre, saltás de
carril sin perder continuidad.

Cada etapa declara explícitamente:

- **Requiere:** qué etapas necesitás tener aprobadas antes (dependencia real, no sugerida).
- **Habilita:** qué se te desbloquea al terminarla.
- **Dónde:** `native_sim`, ESP32-S3, o ambos.
- **Mínimo para aprobar** vs **micro-pasos opcionales**, para que puedas cerrar una etapa sin
  agotar el tema y volver más tarde a profundizar.

---

## 0. Tu hardware: ESP32-S3-DevKitC-1

Todo el roadmap está calibrado para tu placa. Datos verificados contra la documentación oficial
de Zephyr para esta board:

| Dato | Valor |
| --- | --- |
| Nombre de board en Zephyr | `esp32s3_devkitc` |
| Target de build principal | `esp32s3_devkitc/esp32s3/procpu` |
| Segundo target (AMP) | `esp32s3_devkitc/esp32s3/appcpu` |
| Arquitectura | Xtensa LX7, doble núcleo hasta 240 MHz |
| Memoria por target | 416 KiB RAM, 8 MiB flash |
| Radio | Wi-Fi 802.11 b/g/n + Bluetooth LE 5.0 (**no** Bluetooth Classic) |
| Runner de flash | `esp32` (default) |
| Runner de debug | `openocd` (default), vía USB-JTAG integrado |
| Arranque por defecto | *Simple Boot* — imagen única, **sin** segundo bootloader |

### 0.1 Comandos que vas a usar todo el tiempo

```bash
# Blobs de RF: SIN ESTO Wi-Fi y BLE no funcionan. Correlo después de cada `west update`.
west blobs fetch hal_espressif

# Build para tu placa
west build -p always -b esp32s3_devkitc/esp32s3/procpu <ruta-app>

# Build para el host (mismo código, sin hardware)
west build -p always -b native_sim <ruta-app>
west build -t run

# Flash y monitor serie
west flash
west espressif monitor
```

### 0.2 Particularidades de la placa que te van a morder (leelas ahora)

1. **Los blobs de RF son obligatorios.** `west blobs fetch hal_espressif` no es opcional para
   Wi-Fi/BLE, y hay que repetirlo después de `west update`. Si BLE "no compila" o no arranca,
   esto es lo primero a revisar.

2. **No hay LED de usuario simple.** La DevKitC-1 tiene un **LED RGB direccionable** (tipo
   WS2812) manejado por un solo pin, no un LED común conectado a un GPIO. La documentación de
   Zephyr indica GPIO38, pero algunas revisiones de la placa lo traen en GPIO48: **verificá tu
   revisión**. Consecuencia práctica: para las etapas de GPIO conviene un **LED externo con
   resistencia en un pin del header**, que es más simple y más didáctico que el led-strip.
   Comprobá con `west build -t menuconfig` y leyendo `build/zephyr/zephyr.dts` si existe un alias
   `led0` en tu versión antes de asumirlo.

3. **Sí hay botón integrado.** La board define un nodo `gpio-keys` (el pulsador BOOT). Buscalo en
   `build/zephyr/zephyr.dts` y confirmá el nombre del alias antes de usarlo — no asumas `sw0`.

4. **Hay un sensor de temperatura interno al SoC** (`espressif,esp32-temp`). Esto te deja hacer
   toda la etapa de sensores **sin comprar nada**, y recién después pasar a un sensor I2C externo.

5. **Dos puertos USB-C, y no son equivalentes.** Uno es el puente USB-UART, el otro es el
   controlador USB Serial/JTAG nativo del SoC. Cuál de los dos es tu consola lo determina el nodo
   `zephyr,console` — descubrilo leyendo `build/zephyr/zephyr.dts` (es el primer ejercicio real de
   Devicetree que vas a hacer).

6. **Si tras `west flash` la placa no arranca sola**, el chip puede quedar en modo descarga.
   Solución documentada:
   ```bash
   west flash --reset-type watchdog-reset
   ```

7. **Ciclo de desarrollo lento por el flasheo.** Usá `west flash --esp-skip-flashed` (omite la
   escritura si el binario ya coincide, verificado por MD5 en el dispositivo). Existe también
   `--esp-diff`, que escribe solo las regiones distintas comparando contra una copia local — no lo
   uses si flasheaste con otra herramienta desde afuera.

8. **JTAG gratis.** El ESP32-S3 tiene USB-JTAG integrado: `west debug` funciona con solo un cable,
   sin sonda externa. Requiere el **OpenOCD parcheado de Espressif** (el que trae el Zephyr SDK
   puede no tener soporte ESP32) y, para ver threads de Zephyr en GDB,
   `CONFIG_DEBUG_THREAD_INFO=y`. Esta es la mayor ventaja de tu placa: aprovechala desde temprano.

9. **Snippets de variante.** Si tu módulo tiene otro tamaño de flash o PSRAM que el default de la
   board, se ajusta con snippets (`-S espressif-flash-8M`, `-S espressif-psram-4M`, etc.), no
   editando la board.

10. **MCUboot no está por defecto.** El arranque default es *Simple Boot*: una sola imagen, sin
    segundo bootloader, **sin OTA ni seguridad**. MCUboot es opt-in
    (`CONFIG_BOOTLOADER_MCUBOOT=y` o `--sysbuild`) y hay que flashearlo al menos una vez. Esto
    hace que la etapa de DFU sea un cambio de arquitectura de arranque, no un agregado: está bien
    dejarla para el final.

### 0.3 Por qué usar `native_sim` en paralelo, siempre

Tu placa es cómoda pero el ciclo build-flash-monitor es de decenas de segundos, y un crash de
Xtensa es más difícil de leer que un segfault del host. Regla del roadmap: **toda lógica que no
toque hardware se desarrolla y testea primero en `native_sim`**, donde el ciclo es de segundos, la
ejecución es determinista y podés usar `gdb`, `valgrind` y prints sin límite. Los carriles de
kernel y de observabilidad están diseñados para hacerse casi enteros en el host.

---

## 1. Cómo usar este roadmap

### 1.1 Repositorio de trabajo

```
zephyr-roadmap/
├── west.yml                  # tu manifiesto (recién en la Etapa 18)
├── etapas/
│   ├── e00-entorno/
│   ├── e01-app-layout/
│   └── ...
├── lib/                      # código reutilizable entre etapas (crece con vos)
├── tests/                    # suites Ztest (desde la Etapa 12)
├── docs/
│   ├── bitacora.md           # una entrada por micro-paso
│   └── evidencias/eNN/       # logs, mediciones, capturas
└── README.md                 # tabla de progreso
```

**Un commit por micro-paso**, con mensaje `eNN.M: <qué logré>`. El historial es parte del
entregable: es lo que te va a permitir retomar una etapa tres semanas después sin releerla toda.

### 1.2 Cómo elegir qué hacer hoy

1. Abrí la tabla de progreso (§4) y filtrá por lo que tenga sus **requisitos cumplidos**.
2. Elegí por ganas, no por orden. Si venís de tres días de Devicetree, cambiá de carril.
3. Elegí **un micro-paso**, no una etapa. La unidad de trabajo es el micro-paso.
4. Al terminar: commit, entrada en la bitácora, y marcá el checkbox.

No hay penalidad por dejar una etapa a medias: los micro-pasos son acumulativos y los "opcionales"
existen justamente para volver cuando el tema te interese de nuevo.

### 1.3 Ritmo sugerido

No hay tiempos por etapa a propósito: son distintos para cada uno y las estimaciones solo generan
culpa. En su lugar, dos referencias útiles: un micro-paso debería caber en una sesión de trabajo, y
si un micro-paso te lleva más de tres sesiones, está mal partido — avisame y lo dividimos.

---

## 2. Mapa de dependencias

### 2.1 Tronco obligatorio

```
E0 (entorno) ──► E1 (anatomía de la app) ──► E2 (Kconfig)
                                        └──► E3 (Devicetree)
```

Estas cuatro son secuenciales y no se saltean. Es la parte donde Zephyr parece hostil; después se
vuelve agradable. E2 y E3 se pueden hacer en cualquier orden entre sí.

### 2.2 Después del tronco: cuatro carriles paralelos

```
CARRIL K — Kernel (100 % en native_sim, cero hardware)
  E5 threads ──► E6 sincronización ──► E7 datos+workqueues
                              └──► E8 tiempo e interrupciones

CARRIL O — Observabilidad (casi todo en native_sim)
  E9 logging ──► E10 shell
  E12 tests+CI  (requiere algo de código: E5 o E7 o E11)
  E13 diagnóstico  (requiere E5)

CARRIL H — Hardware (ESP32-S3)
  E4 drivers+GPIO ──► E11 sensores (arranca con el sensor interno del SoC)

CARRIL P — Producto (ESP32-S3, el más pesado; dejalo para cuando K y O estén firmes)
  E14 storage ──► E17 MCUboot+DFU
  E15 consumo   (requiere E4 y E8)
  E16 conectividad BLE/Wi-Fi   (requiere E7)
  E18 módulos y drivers propios   (requiere E4)

E19 proyecto integrador — requiere el resto
```

### 2.3 Combinaciones que funcionan bien juntas

- **E5 + E13**: creás threads y acto seguido medís sus stacks reales. Muy satisfactorio.
- **E9 + E10**: logging y shell comparten el backend de consola; hacerlas juntas ahorra trabajo.
- **E4 + E11**: GPIO y sensores son la misma mecánica (DT → Kconfig → API) aplicada dos veces.
- **E3 + E4**: si el Devicetree te resultó abstracto, E4 le da sentido inmediato.
- **E12 en cualquier momento**: cada vez que termines una etapa con lógica, agregale tests. Es el
  mejor hábito que podés construir acá.

### 2.4 Si te frustrás

Orden de descarte: cambiá de carril → bajá a `native_sim` → volvé al micro-paso anterior y
verificá que sigue funcionando → pedí ayuda N1/N2 (§3). Frustrarse con Zephyr es normal; quedarse
tres días en el mismo error, no.

---

## 3. Pasos para aprobar una etapa

Una etapa está aprobada cuando cumplís los cinco pasos **sobre los micro-pasos mínimos**. Los
micro-pasos opcionales no son requisito.

1. **Lectura dirigida.** Leé la bibliografía. Antes de escribir código, anotá en la bitácora: qué
   problema resuelve esta feature, sus dos o tres APIs/opciones centrales, y una limitación que
   menciona la doc.
2. **Micro-pasos mínimos completos**, cada uno con su checkpoint verificado. Compila sin warnings
   nuevos.
3. **Evidencia** en `docs/evidencias/eNN/`: salida de consola, reporte de Twister, medición de
   ROM/RAM (`west build -t rom_report` / `ram_report`), captura. Sin evidencia, no cuenta.
4. **Verificación de criterios**, uno por uno. Si uno no se cumple, la etapa sigue abierta.
5. **Defensa.** Respondé por escrito, sin mirar la doc, las tres preguntas de la etapa. Si no
   podés responder dos de tres, volvé al paso 1.

**Criterio de reprobación explícito:** si funciona pero no sabés por qué, o funciona solo con los
valores exactos del ejemplo, la etapa no está aprobada.

**Regla de los dos targets:** salvo donde se indique lo contrario, todo entregable debe compilar
para `native_sim` **y** para `esp32s3_devkitc/esp32s3/procpu`. Si algo solo compila para uno, hay
una dependencia de hardware escondida en tu código de aplicación — encontrala.

---

## 4. Cómo preguntarme tus dudas

El objetivo es que aprendas a navegar el ecosistema, que es enorme y cambia. Por eso mi modo por
defecto **no es darte la solución**, sino la pista y la fuente.

### 4.1 Escalera de ayuda (pedí el nivel explícitamente)

| Nivel | Pedilo así | Qué recibís |
| --- | --- | --- |
| **N1 — Orientación** | "Nivel 1: ¿por dónde busco?" | El nombre del subsistema, API u opción Kconfig relevante, y el término exacto para buscar. Sin código. |
| **N2 — Documentación dirigida** | "Nivel 2: ¿qué leo?" | Enlaces a las secciones que contienen la respuesta y qué mirar en cada una. Sin código. |
| **N3 — Diagnóstico socrático** | "Nivel 3: guiame con preguntas" | Preguntas que te hacen aislar la causa. Sin código. |
| **N4 — Esqueleto** | "Nivel 4: dame el esqueleto" | Pseudocódigo o estructura de archivos con huecos `/* TODO */`. |
| **N5 — Respuesta explícita** | "Nivel 5: dame la respuesta, ya intenté X, Y, Z" | Solución completa con explicación. **Solo con intento previo documentado.** |

Empezá en N1 o N2. Subí un nivel por vez.

### 4.2 Plantilla de pregunta

```
Etapa: eNN.M — <nombre del micro-paso>
Nivel de ayuda que pido: N<1-5>

Objetivo: qué quiero lograr (una frase).
Entorno: versión de Zephyr, target (-b ...), host OS.
Qué hice: comandos exactos ejecutados.
Qué esperaba: comportamiento esperado.
Qué pasó: error/log textual pegado, recortando lo irrelevante.
Qué ya descarté: hipótesis probadas y por qué.
Qué ya leí: enlaces consultados.
```

### 4.3 Mi contrato

- Sin nivel indicado, respondo en **N2**.
- **No entrego código de solución en N1–N3**, aunque insistas, salvo que invoques N5 con intento
  documentado.
- Ante un error, primero te pido el **log completo** y el **target**: en Zephyr el 80 % de los
  errores se identifica por el mensaje exacto de CMake, del Devicetree o del linker.
- Si pedís N5 sin haber intentado, respondo en N3 y te digo qué falta documentar.
- **Nunca invento** nombres de opciones Kconfig, propiedades de Devicetree ni funciones de API. Si
  no estoy seguro, te digo cómo verificarlo (buscar en el `Kconfig` del árbol, en `dts/bindings/`,
  o en la doc de API de tu versión).
- Para cosas específicas de ESP32-S3 te aviso cuando estoy en terreno menos documentado que el de
  las plataformas ARM mainstream, en lugar de afirmar con falsa seguridad.

### 4.4 Preguntas que conviene traer siempre

1. ¿Cuál es la opción Kconfig que habilita esto y de qué depende?
2. ¿Qué se decide en build time (DT/Kconfig) y qué en runtime?
3. ¿Cuánta RAM/flash agrega y cómo lo mido?
4. ¿Qué pasa si esto falla en producción y cómo me enteraría?

---

## 5. Tabla de progreso

Copiala a tu `README.md`. La columna **Requiere** te dice qué podés empezar hoy.

| # | Etapa | Carril | Requiere | Dónde | Estado |
| --- | --- | --- | --- | --- | --- |
| 0 | Entorno, west y toolchain | tronco | — | ambos | ⬜ |
| 1 | Anatomía de la app y build system | tronco | E0 | ambos | ⬜ |
| 2 | Kconfig | tronco | E1 | ambos | ⬜ |
| 3 | Devicetree | tronco | E1 | ambos | ⬜ |
| 4 | Modelo de drivers y GPIO | H | E3 | ESP32-S3 | ⬜ |
| 5 | Threads y scheduling | K | E1 | native_sim | ⬜ |
| 6 | Sincronización | K | E5 | native_sim | ⬜ |
| 7 | Paso de datos y workqueues | K | E6 | native_sim | ⬜ |
| 8 | Tiempo, timers e interrupciones | K | E5 | ambos | ⬜ |
| 9 | Logging | O | E2 | ambos | ⬜ |
| 10 | Shell | O | E9 | ESP32-S3 | ⬜ |
| 11 | Sensores (interno → I2C externo) | H | E4 | ESP32-S3 | ⬜ |
| 12 | Ztest, native_sim y Twister | O | E5 o E7 | native_sim | ⬜ |
| 13 | Diagnóstico y post-mortem | O | E5 | ambos | ⬜ |
| 14 | Almacenamiento no volátil | P | E3 | ESP32-S3 | ⬜ |
| 15 | Consumo y power management | P | E4, E8 | ESP32-S3 | ⬜ |
| 16 | Conectividad: BLE o Wi-Fi | P | E7 | ESP32-S3 | ⬜ |
| 17 | MCUboot, sysbuild y DFU | P | E14 | ESP32-S3 | ⬜ |
| 18 | Módulos y drivers out-of-tree | P | E4 | ambos | ⬜ |
| 19 | Proyecto integrador | — | casi todas | ambos | ⬜ |

---

# TRONCO

## Etapa 0 — Entorno, west y toolchain

**Requiere:** nada · **Habilita:** todo · **Dónde:** ambos

Zephyr no se instala: se *provisiona*. Su árbol es un conjunto de repositorios (Zephyr + HALs de
silicio + módulos como MCUboot) coordinados por `west`. Entender esto evita el error más común del
principiante: tratar a Zephyr como una biblioteca que se copia a un proyecto. En tu caso hay un
paso extra que no aparece en los tutoriales genéricos: los **blobs binarios de RF** de Espressif.

### Micro-pasos

- **0.1** Instalá `west`, el Zephyr SDK y las dependencias del host siguiendo el *Getting Started*.
  → *Checkpoint:* `west --version` y `cmake --version` responden.
- **0.2** Inicializá el workspace y corré `west update`.
  → *Checkpoint:* `west list | grep hal_espressif` muestra el HAL presente.
- **0.3** Corré `west blobs fetch hal_espressif`.
  → *Checkpoint:* `west blobs list hal_espressif` muestra los blobs como descargados.
- **0.4** Compilá y corré `samples/hello_world` en `native_sim`.
  → *Checkpoint:* `west build -t run` imprime el saludo en tu terminal.
- **0.5** Compilá y flasheá `samples/hello_world` en la placa; abrí `west espressif monitor`.
  → *Checkpoint:* ves el banner de Zephyr y el saludo. Si no arranca solo, probá
  `west flash --reset-type watchdog-reset` y anotá en la bitácora si te hizo falta.
- **0.6** Copiá el sample a `etapas/e00-entorno/`, cambiá el mensaje y compilá **desde tu
  directorio**, no desde el árbol de Zephyr.
  → *Checkpoint:* los dos targets muestran tu mensaje.
- **0.7** Escribí `docs/setup.md` con comandos exactos, versiones y el commit de Zephyr en uso
  (`west list zephyr`).
  → *Checkpoint:* otra persona podría reproducir tu entorno.
- **0.8 (opcional)** Configurá `west debug` con el OpenOCD de Espressif y poné un breakpoint en
  `main`. Es opcional hoy pero te va a ahorrar horas en E13.
  → *Checkpoint:* GDB se detiene en `main` y podés inspeccionar variables.

**Mínimo para aprobar:** 0.1 a 0.7.

### Entregable

`etapas/e00-entorno/` compilando para los dos targets + `docs/setup.md` + evidencia de ambas
ejecuciones.

### Criterios de aceptación

- [X] La misma app corre en `native_sim` y en la placa **sin modificar `src/main.c`**.
- [X] Los blobs de RF están descargados y sabés por qué son necesarios.
- [X] `docs/setup.md` permite reproducir el entorno desde cero.
- [X] Podés explicar qué hace `-p always` y cuándo es imprescindible.

### Defensa

1. ¿Qué diferencia hay entre el repo `zephyr` y el *workspace* de west?
2. ¿Por qué Espressif distribuye los blobs de RF por separado y qué implica para tu proyecto?
3. ¿Qué pasa si no borrás `build/` tras cambiar `prj.conf`?

### Bibliografía

- Getting Started Guide — https://docs.zephyrproject.org/latest/develop/getting_started/index.html
- West (meta-tool) — https://docs.zephyrproject.org/latest/develop/west/index.html
- **ESP32-S3-DevKitC en Zephyr** (tu referencia de cabecera) — https://docs.zephyrproject.org/latest/boards/espressif/esp32s3_devkitc/doc/index.html

---

## Etapa 1 — Anatomía de la aplicación y build system

**Requiere:** E0 · **Habilita:** E2, E3, E5, E9 · **Dónde:** ambos

Una aplicación Zephyr son cuatro piezas: `CMakeLists.txt` (qué se compila), `prj.conf` (qué se
habilita), `boards/` (ajustes por placa) y `src/`. La clave conceptual: **Zephyr se recompila para
tu aplicación**, no hay un kernel binario al que te enlazás. Esta etapa te enseña a leer
`build/` como documentación, porque lo es: `build/zephyr/.config` y `build/zephyr/zephyr.dts` son
la verdad final sobre tu firmware.

### Micro-pasos

- **1.1** Creá `etapas/e01-app-layout/` con **dos** archivos fuente y un header en `include/`.
  → *Checkpoint:* compila para los dos targets.
- **1.2** Agregá `boards/native_sim.conf` y `boards/esp32s3_devkitc_esp32s3_procpu.conf` con un
  valor distinto de la misma opción (por ejemplo el tamaño de stack del thread principal).
  → *Checkpoint:* los `.config` generados difieren, y podés señalar dónde.
- **1.3** Abrí `build/zephyr/.config` y encontrá el valor efectivo de esa opción en cada target.
  → *Checkpoint:* documentás el orden de precedencia que observaste.
- **1.4** Abrí `build/zephyr/zephyr.dts` de la placa y encontrá el nodo `chosen`. Identificá cuál
  es tu consola.
  → *Checkpoint:* sabés qué periférico y qué puerto USB estás usando para ver los logs.
- **1.5** Corré `west build -t rom_report` y `-t ram_report` en los dos targets.
  → *Checkpoint:* nombrás los tres consumidores principales de flash.
- **1.6** Hacé que `CMakeLists.txt` incluya un archivo fuente **condicionalmente**, según una
  opción de configuración.
  → *Checkpoint:* el archivo aparece/desaparece del binario y lo verificás en el `rom_report`.
- **1.7 (opcional)** Explorá `build/zephyr/include/generated/` y encontrá las macros generadas a
  partir del Devicetree. Es un adelanto de E3 que hace que E3 sea mucho más fácil.
  → *Checkpoint:* encontrás la macro correspondiente a un nodo concreto.

**Mínimo para aprobar:** 1.1 a 1.6.

### Entregable

App multi-target + `docs/evidencias/e01/anatomia-build.md` con lo que encontraste en cada archivo
generado y las cifras de ROM/RAM de los dos targets.

### Criterios de aceptación

- [ ] Los dos targets compilan con `-p always` y sus `.config` difieren en lo esperado.
- [ ] Podés explicar por qué ganó el valor que ganó en `.config`.
- [ ] `CMakeLists.txt` no lista fuentes con `GLOB`.
- [ ] Sabés cuál de los dos puertos USB de tu placa es la consola, y por qué.

### Defensa

1. ¿En qué orden se mergean `prj.conf`, `boards/<target>.conf` y los defaults de la board?
2. ¿Por qué `zephyr.dts` no es igual al `.dts` de la board?
3. ¿Qué diferencia hay entre poner `CONFIG_X=y` en `prj.conf` y pasarlo como `-DCONFIG_X=y`?

### Bibliografía

- Application Development — https://docs.zephyrproject.org/latest/develop/application/index.html
- Build and Configuration Systems — https://docs.zephyrproject.org/latest/build/index.html

---

## Etapa 2 — Kconfig

**Requiere:** E1 · **Habilita:** E9, y hace más fácil todo lo demás · **Dónde:** ambos

Kconfig (heredado de Linux) es cómo Zephyr corre en 8 KB de RAM y también en un SoC con MMU: todo
es opcional y se resuelve en compilación. Dominarlo son dos habilidades: **encontrar** la opción
que necesitás entre miles, y **exponer** opciones propias. El fracaso típico es copiar `CONFIG_*`
de un blog y no entender por qué "no se aplica" — casi siempre una dependencia insatisfecha la
fuerza a `n`.

### Micro-pasos

- **2.1** Navegá `west build -t menuconfig` y encontrá tres opciones que ya venías usando. Leé su
  ayuda y sus dependencias.
  → *Checkpoint:* sabés usar la búsqueda del menuconfig.
- **2.2** Creá un `Kconfig` en tu app con un `bool` que habilita una feature.
  → *Checkpoint:* la opción aparece en menuconfig bajo tu menú.
- **2.3** Agregá un `int` con `range` y `default`, y un `choice` de tres opciones excluyentes.
  → *Checkpoint:* menuconfig rechaza valores fuera de rango.
- **2.4** Usá tus opciones en C con `IS_ENABLED()` y con `CONFIG_<algo>` numérico.
  → *Checkpoint:* cambiar el valor cambia el comportamiento sin editar código.
- **2.5** Hacé que una opción tuya **dependa** de un subsistema (`depends on`) y verificá el efecto.
  → *Checkpoint:* al deshabilitar la dependencia, tu opción desaparece del menú.
- **2.6** Provocá deliberadamente el caso "mi opción quedó en `n`" y diagnosticalo con método.
  → *Checkpoint:* escribís el procedimiento de diagnóstico en la bitácora.
- **2.7 (opcional)** Usá `select` en lugar de `depends on` y documentá por qué abusar de `select`
  es mala práctica.
  → *Checkpoint:* podés explicar la diferencia con un ejemplo propio.

**Mínimo para aprobar:** 2.1 a 2.6.

### Entregable

`Kconfig` propio + tres `prj.conf` variantes que produzcan tres binarios de tamaño distinto, con
los tamaños medidos.

### Criterios de aceptación

- [ ] Las tres variantes compilan y el `rom_report` muestra diferencias coherentes.
- [ ] Usás `IS_ENABLED()` donde `#ifdef` sería peor, y podés justificarlo.
- [ ] Tenés un método (no prueba y error) para "¿por qué mi `CONFIG_X=y` no tuvo efecto?".

### Defensa

1. ¿Cuál es la diferencia entre `default y` y `select`?
2. ¿Por qué `#ifdef CONFIG_FOO` puede ocultar errores que `IS_ENABLED()` no oculta?
3. ¿Dónde buscás todas las opciones disponibles de un subsistema?

### Bibliografía

- Kconfig — Configuration System — https://docs.zephyrproject.org/latest/build/kconfig/index.html
- Kconfig Tips and Best Practices — https://docs.zephyrproject.org/latest/build/kconfig/tips.html

---

## Etapa 3 — Devicetree

**Requiere:** E1 · **Habilita:** E4, E14, E18 · **Dónde:** ambos

Si Kconfig responde "qué software incluyo", el Devicetree responde "qué hardware existe y cómo
está conectado". Se procesa en build time y se convierte en macros de C: no hay parsing en runtime.
El salto mental difícil es que **no programás contra el DT, programás contra las macros que el DT
genera**. En el ESP32-S3 hay un componente extra muy visible: como casi cualquier función puede
salir por casi cualquier pin (matriz de GPIO), el **pinctrl** es protagonista, a diferencia de un
micro con pines fijos.

### Micro-pasos

- **3.1** Leé `build/zephyr/zephyr.dts` de tu placa y hacé un mapa de los nodos que te importan:
  consola, controladores GPIO, buses I2C/SPI, y el nodo `gpio-keys` del botón BOOT.
  → *Checkpoint:* `docs/evidencias/e03/dt-mapa.md` con el árbol relevante.
- **3.2** Encontrá el nodo `gpio-keys` y anotá el nombre real de su alias y su pin. **No asumas
  `sw0`.**
  → *Checkpoint:* citás la línea del `.dts` de la board donde está definido.
- **3.3** Escribí tu primer overlay: agregá un `alias` propio apuntando a un nodo existente.
  → *Checkpoint:* el alias aparece en `build/zephyr/zephyr.dts` (mostrá el diff).
- **3.4** Accedé desde C a una propiedad numérica y a un string del DT.
  → *Checkpoint:* imprimís valores que coinciden con el `.dts`.
- **3.5** Habilitá un bus I2C en el overlay y configurá sus pines vía `pinctrl`.
  → *Checkpoint:* el nodo del bus queda `status = "okay"` y con los pines que elegiste.
- **3.6** Agregá un `BUILD_ASSERT` o `#error` que dé un mensaje **tuyo y entendible** si el alias
  no existe.
  → *Checkpoint:* compilar para `native_sim` (que no tiene ese hardware) produce tu mensaje, no un
  error de macro indescifrable.
- **3.7 (opcional)** Comparé `DT_NODELABEL`, `DT_ALIAS` y `DT_PATH` sobre el mismo nodo y documentá
  cuándo usar cada uno.
- **3.8 (opcional)** Encontrá el archivo de *binding* de un nodo en `dts/bindings/` y leé qué
  propiedades son obligatorias.

**Mínimo para aprobar:** 3.1 a 3.6.

### Entregable

Overlay propio + módulo C que lee el DT + el mapa del árbol con el diff de tu overlay.

### Criterios de aceptación

- [ ] Tu overlay se refleja en `build/zephyr/zephyr.dts` (diff como evidencia).
- [ ] El código no tiene ni una dirección de periférico, número de pin o dirección I2C hardcodeada.
- [ ] Compilar sin el hardware produce **tu** mensaje de error.
- [ ] Podés explicar `status = "okay"` vs `"disabled"` en términos de qué código se genera.

### Defensa

1. ¿Por qué un alias es mejor que un node label para código portable?
2. ¿Qué rol cumple `pinctrl` en el ESP32-S3 y por qué es más central que en un STM32?
3. ¿Qué archivo determina qué propiedades son válidas y obligatorias para un nodo?

### Bibliografía

- Devicetree Guide — https://docs.zephyrproject.org/latest/build/dts/index.html
- Devicetree HOWTOs — https://docs.zephyrproject.org/latest/build/dts/howtos.html
- Devicetree Bindings — https://docs.zephyrproject.org/latest/build/dts/bindings.html

---

# CARRIL K — Kernel (todo en `native_sim`, sin hardware)

Este carril completo se hace en el host. Ciclo de segundos, ejecución determinista, `gdb` a mano.
Es el carril ideal para las sesiones cortas y para cuando no tenés la placa cerca.

## Etapa 5 — Threads y scheduling

**Requiere:** E1 · **Habilita:** E6, E8, E12, E13 · **Dónde:** native_sim

Recién acá entra el "RTOS". El scheduler es de prioridad fija con dos clases: **cooperativas**
(prioridades negativas, no las expulsa nadie salvo que cedan) y **preemptibles** (≥ 0). Esa
distinción más el *time slicing* opcional explican el 90 % de los comportamientos "raros" de
scheduling. También conocés la unidad de costo real del firmware: **el stack por thread**, que se
reserva estáticamente y que si subestimás produce corrupciones silenciosas.

Nota sobre tu placa: el ESP32-S3 es de doble núcleo, pero el modelo por defecto en Zephyr es AMP
(una aplicación por core, `procpu` y `appcpu` separados), no SMP con threads migrando. Todo este
carril razona sobre un core, que es lo correcto para `procpu`.

### Micro-pasos

- **5.1** Creá **un** thread con `K_THREAD_DEFINE` que imprima su nombre en loop con `k_sleep`.
  → *Checkpoint:* ves alternancia entre `main` y tu thread.
- **5.2** Agregá un segundo thread preemptible con prioridad distinta.
  → *Checkpoint:* podés explicar cuál corre primero y por qué.
- **5.3** Agregá un tercero, **cooperativo** (prioridad negativa), con un busy loop.
  → *Checkpoint:* observás que puede monopolizar la CPU.
- **5.4** **Antes de ejecutar**, escribí en la bitácora la secuencia de salida que predecís para
  los tres. Después ejecutá.
  → *Checkpoint:* predicción vs realidad documentada, con cada divergencia explicada.
- **5.5** Activá y desactivá *time slicing* y documentá el cambio.
  → *Checkpoint:* la diferencia es visible en la salida.
- **5.6** Creá un thread en runtime con `k_thread_create` + `K_THREAD_STACK_DEFINE`, hacelo
  terminar, y unite con `k_thread_join`.
  → *Checkpoint:* el join retorna y no se usa el stack después.
- **5.7 (opcional)** Ponele nombre a los threads (`k_thread_name_set`) — te va a servir en E10 y E13.
- **5.8 (opcional)** Corré el mismo programa en la placa y compará la salida con `native_sim`.
  Cualquier diferencia es información valiosa.

**Mínimo para aprobar:** 5.1 a 5.6.

### Entregable

Demo de scheduling + `docs/evidencias/e05/prediccion-vs-real.md`.

### Criterios de aceptación

- [ ] Tu predicción coincide con la ejecución, o la divergencia está explicada en términos de
      prioridad/preempción/cesión.
- [ ] El thread cooperativo demuestra que puede bloquear el sistema, y sabés cómo evitarlo.
- [ ] `k_thread_join` funciona correctamente.
- [ ] Podés justificar el tamaño de stack de cada thread (lo vas a medir en E13).

### Defensa

1. ¿Qué diferencia práctica hay entre `k_yield()`, `k_sleep(K_MSEC(0))` y `k_busy_wait()`?
2. ¿Por qué un thread cooperativo puede ser útil si puede monopolizar la CPU?
3. ¿Qué le pasa a tu programa si un stack es 32 bytes más chico de lo necesario?

### Bibliografía

- Threads — https://docs.zephyrproject.org/latest/kernel/services/threads/index.html
- Scheduling — https://docs.zephyrproject.org/latest/kernel/services/scheduling/index.html

---

## Etapa 6 — Sincronización

**Requiere:** E5 · **Habilita:** E7 · **Dónde:** native_sim

Con más de un thread aparecen las carreras. Zephyr ofrece un menú —semáforos, mutex, condition
variables, events— y cada primitiva codifica una intención: un semáforo *señaliza*, un mutex
*protege*. Confundirlas produce los bugs de firmware que aparecen una vez por semana en campo.
Dos ideas críticas acá: **herencia de prioridad** (por qué un mutex no es un semáforo binario) y
**qué se puede usar desde una ISR** (casi nada que espere).

### Micro-pasos

- **6.1** Dos threads incrementan un contador compartido **sin** protección.
  → *Checkpoint:* el valor final es incorrecto de forma reproducible. Guardá los números.
- **6.2** Arreglalo con `k_mutex`.
  → *Checkpoint:* el valor final es exacto en N corridas.
- **6.3** Medí el impacto del mutex en throughput.
  → *Checkpoint:* tenés cifras antes/después.
- **6.4** Implementá un patrón *señal* con `k_sem` entre dos threads.
  → *Checkpoint:* el consumidor despierta solo cuando hay dato.
- **6.5 (opcional pero recomendado)** Provocá una **inversión de prioridad** y mostrá cómo el mutex
  la mitiga frente al semáforo binario.
  → *Checkpoint:* documentás la diferencia observada.
- **6.6 (opcional)** Reemplazá dos semáforos por un `k_event` y justificá cuándo conviene.

**Mínimo para aprobar:** 6.1 a 6.4.

### Entregable

Tres variantes del mismo programa (roto / mutex / semáforo) con las mediciones.

### Criterios de aceptación

- [ ] La versión sin protección falla de forma reproducible, con evidencia numérica.
- [ ] La versión con mutex es correcta y sabés justificar el orden de toma de locks.
- [ ] La señalización no usa APIs que puedan bloquear donde no corresponde.
- [ ] Explicás con tus palabras por qué "mutex ≠ semáforo binario".

### Defensa

1. ¿Qué pasa si llamás a `k_mutex_lock()` con timeout desde una ISR?
2. ¿Cuándo usarías `k_event` en lugar de varios semáforos?
3. ¿Cómo detectarías un deadlock en una placa sin debugger?

### Bibliografía

- Synchronization — https://docs.zephyrproject.org/latest/kernel/services/synchronization/index.html
- Kernel Services (índice) — https://docs.zephyrproject.org/latest/kernel/services/index.html

---

## Etapa 7 — Paso de datos y workqueues

**Requiere:** E6 · **Habilita:** E12, E16 · **Dónde:** native_sim

La forma robusta de comunicar threads no es memoria compartida con locks: es **pasar mensajes**.
Zephyr da message queues (tamaño fijo, con copia), FIFOs/LIFOs (punteros, sin copia), pipes y
mailboxes; elegir es una decisión de diseño con consecuencias en RAM y latencia. En paralelo
aparece la herramienta más usada del ecosistema: las **workqueues**, que resuelven "algo urgente
pasó en una ISR, procesalo después en contexto de thread" sin crear un thread por tarea.

### Micro-pasos

- **7.1** Productor y consumidor conectados por un `k_msgq`.
  → *Checkpoint:* los mensajes llegan completos y en orden.
- **7.2** Forzá **cola llena** y definí una política explícita (descartar viejo, descartar nuevo,
  bloquear).
  → *Checkpoint:* bajo saturación no se cuelga ni pierde datos en silencio; el log lo muestra.
- **7.3** Agregá un `k_work` a la system workqueue disparado por un evento simulado.
  → *Checkpoint:* el trabajo diferido se ejecuta en contexto de thread.
- **7.4** Encadená todo: evento → work item → `k_msgq` → consumidor que agrega/procesa.
  → *Checkpoint:* el pipeline completo funciona end-to-end.
- **7.5 (opcional)** Reimplementá con `k_fifo` y buffers propios; compará RAM y copias.
- **7.6 (opcional)** Creá una **workqueue dedicada** con su propia prioridad y explicá cuándo la
  system workqueue no alcanza.

**Mínimo para aprobar:** 7.1 a 7.4.

### Entregable

Pipeline funcionando + informe con el comportamiento bajo saturación y la política elegida.

### Criterios de aceptación

- [ ] Bajo saturación forzada la política es explícita y **observable en el log**.
- [ ] Ningún `k_work_submit` desde contexto de ISR usa argumentos que bloqueen.
- [ ] Podés explicar por qué usar la system workqueue para tareas largas es peligroso.

### Defensa

1. ¿Quién es dueño de la memoria de un item pasado por `k_fifo`?
2. ¿Qué pasa si un work item se re-submite mientras está corriendo?
3. ¿Cómo elegís entre `k_msgq`, `k_fifo` y memoria compartida con mutex?

### Bibliografía

- Data Passing — https://docs.zephyrproject.org/latest/kernel/services/data_passing/index.html
- Workqueue Threads — https://docs.zephyrproject.org/latest/kernel/services/threads/workqueue.html

---

## Etapa 8 — Tiempo, timers e interrupciones

**Requiere:** E5 · **Habilita:** E15 · **Dónde:** ambos (empezá en native_sim, cerrá en la placa)

El tiempo en Zephyr es *tickless* y se cuenta en ticks convertibles a ms/us/ciclos con redondeo
controlado. Importa porque `k_sleep(K_MSEC(1))` no significa "1 ms exacto", y confundirlo genera
drift acumulado en muestreo. Acá cerrás el modelo de ejecución: contexto de thread, contexto de
ISR, y el rol de `k_timer` frente a los timers de hardware (API `counter`, de la que tu SoC tiene
varias instancias).

Advertencia de tu placa: en `native_sim` el tiempo simulado corre desacoplado del tiempo real,
así que las mediciones de drift **solo tienen sentido en hardware**. Hacé la lógica en el host y
las mediciones en la placa.

### Micro-pasos

- **8.1** Muestreo periódico con `k_timer` a frecuencia fija.
  → *Checkpoint:* el callback se ejecuta al ritmo esperado.
- **8.2** La misma cosa con `k_sleep` en un loop.
  → *Checkpoint:* las dos versiones funcionan y podés compararlas.
- **8.3** **En la placa**, medí el drift de ambas durante varios minutos contra `k_uptime_get()`.
  → *Checkpoint:* drift reportado en ms/minuto o ppm, con metodología escrita.
- **8.4** Medí una duración con `k_cycle_get_32()` y convertila correctamente a microsegundos con
  `sys_clock_hw_cycles_per_sec()`.
  → *Checkpoint:* el número es plausible y sabés de dónde sale cada factor.
- **8.5** Registrá una ISR (o usá el callback de GPIO de E4 si ya la hiciste) y medí la latencia
  evento → handler.
  → *Checkpoint:* latencia en ciclos y en microsegundos.
- **8.6 (opcional)** Cambiá `CONFIG_SYS_CLOCK_TICKS_PER_SEC` y documentá qué le pasó a tus
  mediciones.
- **8.7 (opcional)** Usá la API `counter` con uno de los timers de hardware del ESP32-S3 y compará
  con `k_timer`.

**Mínimo para aprobar:** 8.1 a 8.5.

### Entregable

Programa de muestreo con drift medido en hardware + informe de latencia con método.

### Criterios de aceptación

- [ ] El drift está reportado con unidades y metodología, medido **en la placa**.
- [ ] La versión con timer periódico tiene menos drift que el loop con sleep, y explicás por qué.
- [ ] La latencia está medida en ciclos y convertida correctamente.
- [ ] El callback de `k_timer` respeta las restricciones de su contexto.

### Defensa

1. ¿En qué contexto corre el callback de un `k_timer`?
2. ¿Por qué existen variantes `_floor`, `_ceil` y `_near` en las conversiones de tiempo?
3. ¿Por qué medir drift en `native_sim` no tiene sentido?

### Bibliografía

- Kernel Timing — https://docs.zephyrproject.org/latest/kernel/services/timing/clocks.html
- Interrupts — https://docs.zephyrproject.org/latest/kernel/services/interrupts.html

---

# CARRIL O — Observabilidad

## Etapa 9 — Logging

**Requiere:** E2 · **Habilita:** E10, E13 · **Dónde:** ambos

`printk` no es una estrategia de observabilidad. El subsistema de logging da niveles por módulo,
filtrado en compile time y runtime, modo diferido (los mensajes se procesan en una workqueue para
no penalizar el camino crítico), backends múltiples y logging desde ISR. El precio es flash y RAM,
así que la habilidad real es **calibrar**: logs útiles en desarrollo, binario razonable en
producción, sin mantener dos códigos.

### Micro-pasos

- **9.1** Convertí un módulo a `LOG_MODULE_REGISTER` + `LOG_INF/WRN/ERR`.
  → *Checkpoint:* la salida muestra el prefijo del módulo y el nivel.
- **9.2** Llevalo a tres módulos con niveles distintos por módulo desde configuración.
  → *Checkpoint:* cambiás verbosidad sin editar código.
- **9.3** Compará `CONFIG_LOG_MODE_DEFERRED` vs `CONFIG_LOG_MODE_IMMEDIATE`.
  → *Checkpoint:* documentás la diferencia de comportamiento observada.
- **9.4** Saturá el logger hasta provocar pérdida de mensajes; ajustá el buffer.
  → *Checkpoint:* viste el aviso de mensajes perdidos y documentaste el trade-off.
- **9.5** Generá dos builds, *debug* y *release*, y compará flash/RAM.
  → *Checkpoint:* cifras de ambos en la bitácora.
- **9.6 (opcional)** Si ya hiciste E8, medí cuánto le agrega el modo immediate a la latencia de tu
  ISR. Es el número que justifica todo este subsistema.
- **9.7 (opcional)** Probá `LOG_HEXDUMP` con un buffer y observá su costo.

**Mínimo para aprobar:** 9.1 a 9.5.

### Entregable

App instrumentada + informe con la comparación deferred/immediate, el caso de saturación y las
cifras de los dos builds.

### Criterios de aceptación

- [ ] Cero `printk` en código de aplicación, salvo justificaciones documentadas.
- [ ] Los niveles se controlan por módulo desde configuración.
- [ ] El build release reduce flash de forma medible y sigue siendo diagnosticable.

### Defensa

1. ¿Por qué el modo deferred puede perder mensajes y cómo lo detectás?
2. ¿Qué diferencia hay entre el `LOG_LEVEL` de compile time y el filtrado en runtime?
3. ¿Qué precauciones tiene `LOG_HEXDUMP` con datos grandes?

### Bibliografía

- Logging — https://docs.zephyrproject.org/latest/services/logging/index.html

---

## Etapa 10 — Shell

**Requiere:** E9 · **Habilita:** E13, E14 · **Dónde:** ESP32-S3 (también funciona en native_sim)

El shell convierte tu firmware en algo interrogable: comandos jerárquicos con autocompletado, más
módulos listos (`kernel threads`, `kernel stacks`, `device list`, `i2c scan`, `log`, `flash`). Es
la herramienta de diagnóstico con mejor relación costo/beneficio del ecosistema, y la puerta de
entrada al testing manual y a producción (calibración, self-test). Diseñar buenos comandos es
diseñar la interfaz de soporte de tu producto.

### Micro-pasos

- **10.1** Habilitá el shell y conectate por el monitor serie.
  → *Checkpoint:* tenés prompt y `help` responde.
- **10.2** Explorá los built-in: `kernel threads`, `kernel stacks`, `device list`.
  → *Checkpoint:* guardá la salida de `kernel stacks` — la usás en E13.
- **10.3** Creá **un** comando propio (`app status`).
  → *Checkpoint:* aparece en `help` y ejecuta.
- **10.4** Armá un árbol de subcomandos con `SHELL_STATIC_SUBCMD_SET_CREATE`
  (`app led on|off`, `app cfg get|set`).
  → *Checkpoint:* el autocompletado navega tu árbol.
- **10.5** Validá argumentos: faltantes, de más, inválidos.
  → *Checkpoint:* error legible y `retval` distinto de 0, nunca un crash.
- **10.6** Verificá que el shell se pueda deshabilitar por Kconfig sin romper el build.
  → *Checkpoint:* el build sin shell compila y funciona.
- **10.7 (opcional)** Autocompletado dinámico para un argumento.
- **10.8 (opcional)** Shell sobre el USB Serial/JTAG en lugar del puente UART, para liberar la UART.

**Mínimo para aprobar:** 10.1 a 10.6.

### Entregable

`shell_cmds.c` con al menos 5 comandos útiles + transcripción de una sesión.

### Criterios de aceptación

- [ ] `help` documenta tus comandos de forma comprensible para quien no escribió el código.
- [ ] Argumentos inválidos producen error claro, no crash.
- [ ] `kernel stacks` te muestra el uso real de stack de tus threads.
- [ ] El shell es opcional por configuración.

### Defensa

1. ¿En qué thread corren tus handlers y qué implica para operaciones lentas?
2. ¿Qué riesgos de seguridad tiene dejar el shell habilitado en producción?
3. ¿Cómo harías un comando que tarda 5 segundos sin bloquear el shell?

### Bibliografía

- Shell — https://docs.zephyrproject.org/latest/services/shell/index.html

---

## Etapa 12 — Ztest, `native_sim` y Twister

**Requiere:** E5 o E7 (algo de lógica que testear) · **Habilita:** buenos hábitos para siempre · **Dónde:** native_sim

El firmware sin tests automatizados no escala. Zephyr trae el trío: **Ztest** (suites, fixtures,
mocks), **`native_sim`** (tu app como ejecutable del host, determinista y rapidísimo) y **Twister**
(compila y ejecuta N escenarios sobre M plataformas con reportes). La habilidad clave es **diseñar
para testeabilidad**: separar lógica pura de acceso a hardware.

Nota: existen dos targets de test distintos y conviene no confundirlos. `unit_testing` compila y
enlaza **solo** tus fuentes con el harness de Ztest, sin kernel, sin scheduler, sin devicetree;
todo lo que tu módulo necesite lo tenés que proveer como stub. `native_sim` sí arranca el kernel
completo sobre el host.

### Micro-pasos

- **12.1** Escribí tu primera suite con `ZTEST_SUITE` y **un** test trivial que pase.
  → *Checkpoint:* corre en `native_sim`.
- **12.2** Refactorizá una función de una etapa previa para que sea **pura** (sin `struct device`,
  sin `k_sleep`).
  → *Checkpoint:* la función se puede llamar desde un test sin inicializar nada.
- **12.3** Escribí 8 casos, incluyendo límites y errores.
  → *Checkpoint:* todos pasan y cubren algo real.
- **12.4** Agregá `setup`/`teardown` con fixture.
  → *Checkpoint:* el estado se reinicia entre tests.
- **12.5** Corré la suite también en el target `unit_testing` y documentá la diferencia.
  → *Checkpoint:* entendés qué tuviste que stubear y por qué.
- **12.6** Configurá `testcase.yaml` y corré `twister -T tests/`.
  → *Checkpoint:* reporte de Twister en verde, guardado como evidencia.
- **12.7** Introducí un bug a propósito y verificá que un test lo detecta; revertilo.
  → *Checkpoint:* commit con el fallo y commit con la reversión.
- **12.8 (opcional)** CI en GitHub Actions que corra Twister en cada push.
- **12.9 (opcional)** Corré Twister sobre la placa real con hardware map. Es notablemente más
  incómodo que en el host: hacelo una vez para entender por qué el host es tu lugar de trabajo.

**Mínimo para aprobar:** 12.1 a 12.7.

### Entregable

`tests/` con suites, configuración de Twister y reporte de ejecución.

### Criterios de aceptación

- [ ] `twister -T tests/` pasa en verde y el reporte está guardado.
- [ ] Al menos un test falla ante un bug deliberado (demostrado con commits).
- [ ] La lógica testeada no depende de `struct device` ni de temporización real.

### Defensa

1. ¿Qué diferencia hay entre el target `unit_testing` y `native_sim`?
2. ¿Cómo testeás código que sí depende de un driver, sin la placa?
3. ¿Para qué sirven los `filter` de `testcase.yaml`?

### Bibliografía

- Test Framework (Ztest) — https://docs.zephyrproject.org/latest/develop/test/ztest.html
- Test Runner (Twister) — https://docs.zephyrproject.org/latest/develop/twister/index.html
- native_sim — https://docs.zephyrproject.org/latest/boards/native/native_sim/doc/index.html

---

## Etapa 13 — Diagnóstico y análisis post-mortem

**Requiere:** E5 · **Recomendado:** E9, E10 · **Dónde:** ambos

Tarde o temprano algo se cuelga y no hay debugger conectado. Esta etapa te da el equipamiento:
**thread analyzer** (uso real de stack y CPU), protecciones de stack, lectura del volcado de
*fatal error* y reconstrucción del `PC` con `addr2line`. Y es donde por fin dimensionás los stacks
que venís eligiendo a ojo desde E5.

Dos aclaraciones para Xtensa/ESP32-S3, donde el terreno está menos pisado que en ARM Cortex-M:

- El **volcado de pánico y el backtrace** son tu herramienta principal, y funcionan bien. El
  prefijo del toolchain para las herramientas es del estilo
  `xtensa-espressif_esp32s3_zephyr-elf-` — confirmá el nombre exacto en tu instalación del SDK.
- **Coredump y protección de stack por hardware**: verificá el soporte para tu target antes de
  planificar sobre ellos (buscá las opciones en `menuconfig`; si no aparecen, no están
  disponibles). Si no están, el sustituto es superior de todos modos: tu placa tiene **JTAG por
  USB integrado**, así que podés hacer post-mortem en vivo con `west debug` y
  `CONFIG_DEBUG_THREAD_INFO=y`.

### Micro-pasos

- **13.1** Habilitá el thread analyzer y obtené el uso de stack y CPU por thread bajo carga.
  → *Checkpoint:* tabla con pico de uso por thread.
- **13.2** Ajustá **todos** tus stacks a `pico + margen justificado`.
  → *Checkpoint:* tabla antes/después con el criterio de margen escrito.
- **13.3** Provocá un desbordamiento de stack en una rama aparte y capturá el volcado.
  → *Checkpoint:* volcado guardado.
- **13.4** Provocá una desreferencia de puntero nulo. Capturá el volcado.
  → *Checkpoint:* volcado guardado.
- **13.5** Provocá un assert fallido. Capturá el volcado.
  → *Checkpoint:* volcado guardado.
- **13.6** Traducí el `PC` de cada uno de los tres a **línea de código** con `addr2line`.
  → *Checkpoint:* tres líneas exactas identificadas.
- **13.7** Reproducí uno de los tres en `native_sim` y compará qué información te da cada
  plataforma.
  → *Checkpoint:* documentás qué es más fácil de diagnosticar y dónde.
- **13.8 (opcional, alto valor)** Sesión de `west debug` con thread awareness: `info threads` y
  backtrace de un thread que no es el actual.
- **13.9 (opcional)** Verificá disponibilidad de coredump y de protección de stack por hardware en
  tu target, y documentá el resultado (positivo o negativo).

**Mínimo para aprobar:** 13.1 a 13.7.

### Entregable

`docs/evidencias/e13/postmortem.md`: los tres crashes con sus volcados, la línea exacta de cada
uno, y la tabla de dimensionamiento de stacks antes/después.

### Criterios de aceptación

- [ ] Cada uno de los tres crashes está resuelto hasta la **línea exacta**.
- [ ] Los tamaños de stack están justificados con mediciones, no con números redondos.
- [ ] Documentás qué herramientas de post-mortem están efectivamente disponibles en tu target.

### Defensa

1. ¿Por qué un desbordamiento de stack puede manifestarse como un bug en otro thread?
2. ¿Qué información del volcado es imprescindible y qué se pierde?
3. ¿Qué costo tiene dejar el thread analyzer habilitado en producción?

### Bibliografía

- Thread analyzer — https://docs.zephyrproject.org/latest/services/debugging/thread-analyzer.html
- Debugging (índice) — https://docs.zephyrproject.org/latest/develop/debug/index.html
- Core Dump — https://docs.zephyrproject.org/latest/services/debugging/coredump.html

---

# CARRIL H — Hardware

## Etapa 4 — Modelo de drivers y GPIO

**Requiere:** E3 · **Habilita:** E11, E15, E18 · **Dónde:** ESP32-S3

Zephyr tiene un modelo de dispositivos uniforme: cada driver expone un `struct device` con una
tabla de punteros a función, se instancia en build time desde el Devicetree, y se inicializa en un
orden calculado por dependencias. Como consumidor tu contrato es siempre el mismo: obtenés un
`const struct device *`, verificás `device_is_ready()`, y llamás a la API genérica. GPIO es el
mejor primer subsistema: simple, pero ya te obliga a manejar especificadores de DT, flags y
callbacks de interrupción.

**Importante para tu placa:** la DevKitC-1 **no tiene un LED de usuario simple**. Su LED RGB es
direccionable (un protocolo serie, no un GPIO on/off). Por eso esta etapa usa un **LED externo con
resistencia** en un pin del header: dos componentes, cinco minutos, y el ejercicio queda limpio.
El botón sí es integrado (nodo `gpio-keys`, el pulsador BOOT).

### Micro-pasos

- **4.1** Conectá un LED con su resistencia a un GPIO libre del header y describilo en tu overlay
  como nodo `gpio-leds` con alias.
  → *Checkpoint:* el nodo aparece en `zephyr.dts` con el pin que elegiste.
- **4.2** Encendelo y apagalo desde `main` con `gpio_pin_configure_dt` y `gpio_pin_set_dt`.
  → *Checkpoint:* el LED responde. Primer hardware propio funcionando.
- **4.3** Envolvé eso en un módulo `led.c` con API propia (`app_led_toggle()`), sin exponer nada de
  GPIO hacia afuera.
  → *Checkpoint:* `main.c` no incluye `<zephyr/drivers/gpio.h>`.
- **4.4** Leé el botón BOOT por **polling** desde el nodo `gpio-keys` de la board.
  → *Checkpoint:* detectás las pulsaciones.
- **4.5** Pasá el botón a **interrupción** con `gpio_add_callback`.
  → *Checkpoint:* el callback se ejecuta y no hace nada bloqueante.
- **4.6** Agregá antirrebote sin bloquear en el callback (una `k_work` con delay es el patrón).
  → *Checkpoint:* pulsaciones rápidas no generan eventos espurios en el log.
- **4.7** Verificá `device_is_ready()` y reportá el fallo explícitamente.
  → *Checkpoint:* si el device no está listo, el log lo dice claramente.
- **4.8 (opcional)** Compilá para `native_sim`: debería fallar con **tu** mensaje de E3.6, no con
  un error de macro.
- **4.9 (opcional)** Manejá el LED RGB direccionable con la API `led-strip`. Es notablemente más
  complejo y por eso está al final: hacelo cuando quieras el desafío.
- **4.10 (opcional)** Generá PWM con el LEDC del ESP32-S3 para atenuar el LED.

**Mínimo para aprobar:** 4.1 a 4.7.

### Entregable

Módulos `led`/`button` reutilizables en `lib/` + demo con interrupción y antirrebote.

### Criterios de aceptación

- [ ] `device_is_ready()` se verifica y el fallo no se ignora.
- [ ] El callback de GPIO no contiene lógica bloqueante ni prints largos, y sabés explicar por qué.
- [ ] El antirrebote funciona, demostrado con pulsaciones rápidas en el log.
- [ ] Cambiar el pin del LED requiere editar **solo** el overlay.

### Defensa

1. ¿En qué contexto corre un callback de GPIO y qué APIs de kernel están prohibidas ahí?
2. ¿Qué hace `GPIO_ACTIVE_LOW` y por qué es propiedad del hardware y no de tu código?
3. ¿Qué garantiza el orden de inicialización de devices y cómo lo influís?

### Bibliografía

- Device Driver Model — https://docs.zephyrproject.org/latest/kernel/drivers/index.html
- GPIO API — https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html

---

## Etapa 11 — Sensores

**Requiere:** E4 · **Habilita:** E19 · **Dónde:** ESP32-S3

Acá se junta todo. La API de sensores expone cientos de drivers detrás de la misma interfaz:
`sensor_sample_fetch()` + `sensor_channel_get()`, con canales, atributos y triggers. El patrón a
internalizar: **el Devicetree describe el hardware, Kconfig habilita el driver, la API lee los
datos**. Después de eso, agregar un sensor nuevo es un overlay y una línea de Kconfig.

**Ventaja de tu SoC:** el ESP32-S3 trae un **sensor de temperatura interno**
(`espressif,esp32-temp`) expuesto por la misma API de sensores. Podés hacer los primeros
micro-pasos sin comprar ni cablear nada, y recién después pasar a I2C externo. Empezá por ahí: te
saca la variable "¿está mal el cableado?" del problema.

### Micro-pasos

- **11.1** Habilitá el sensor de temperatura interno del SoC y leé su canal.
  → *Checkpoint:* obtenés un valor plausible, sin hardware externo.
- **11.2** Convertí `sensor_value` a unidades legibles **sin float** en el camino crítico.
  → *Checkpoint:* imprimís grados con decimales usando enteros.
- **11.3** Envolvé la lectura en un módulo con API propia que no exponga la API de sensores.
  → *Checkpoint:* `main.c` pide "temperatura", no "canal 13 del device X".
- **11.4** Conectá un sensor I2C externo (BME280, SHT3x, LIS2DH...) y usá `i2c scan` del shell para
  confirmar su dirección **antes** de escribir código.
  → *Checkpoint:* el scan lo encuentra.
- **11.5** Describilo en el overlay (compatible, `reg`, bus con su `pinctrl`) y habilitá el driver
  por Kconfig.
  → *Checkpoint:* el nodo queda `okay` y `device_is_ready()` da true.
- **11.6** Leé dos canales periódicamente y verificá que reaccionan a un estímulo real (soplar,
  calentar, mover).
  → *Checkpoint:* los valores cambian de forma coherente con el estímulo.
- **11.7** Manejá el error de bus: desconectá el sensor en caliente.
  → *Checkpoint:* la app reporta el error y **no** se cuelga ni reinicia.
- **11.8 (opcional)** Implementá un **trigger** (data ready o threshold) en lugar de polling, si el
  sensor lo soporta.
- **11.9 (opcional)** Cambiá de sensor modificando **solo** overlay + Kconfig, sin tocar `src/`.
  Es la prueba de fuego de que entendiste el patrón.

**Mínimo para aprobar:** 11.1 a 11.7.

### Entregable

App de adquisición con el sensor interno y el externo, con su overlay y evidencia de lecturas
plausibles bajo estímulo.

### Criterios de aceptación

- [ ] Las lecturas son físicamente plausibles y reaccionan a un estímulo real.
- [ ] `src/` no contiene la dirección I2C ni el modelo del sensor.
- [ ] Desconectar el sensor produce error reportado, no cuelgue.
- [ ] Podés explicar cómo encontrar el `compatible` correcto de un chip cualquiera.

### Defensa

1. ¿Qué diferencia hay entre un canal, un atributo y un trigger?
2. ¿Por qué `sensor_value` usa `val1`/`val2` en lugar de un `double`?
3. ¿Por qué el sensor interno del SoC usa la misma API que uno externo por I2C?

### Bibliografía

- Sensors — https://docs.zephyrproject.org/latest/hardware/peripherals/sensor/index.html
- I2C API — https://docs.zephyrproject.org/latest/hardware/peripherals/i2c.html

---

# CARRIL P — Producto

Este carril es el más pesado y el que más se apoya en los anteriores. Recomendación: no lo empieces
hasta tener los carriles K y O firmes, porque acá los errores son más difíciles de diagnosticar y
querés llegar con buenas herramientas.

## Etapa 14 — Almacenamiento no volátil

**Requiere:** E3 · **Recomendado:** E10 · **Habilita:** E17 · **Dónde:** ESP32-S3

Todo producto real necesita recordar algo entre reinicios. Zephyr lo resuelve en capas:
**particiones** en el Devicetree (dónde), **NVS** (clave-valor con wear leveling para flash NOR), y
**Settings** (configuración jerárquica sobre un backend). La sutileza que distingue al profesional:
la flash tiene ciclos de borrado finitos y escrituras no atómicas, así que el diseño debe ser
**resistente a corte de energía**.

Tu placa tiene 8 MiB de flash, así que el espacio no es la restricción: la disciplina sí.

### Micro-pasos

- **14.1** Encontrá el mapa de particiones actual en `zephyr.dts` y dibujalo.
  → *Checkpoint:* diagrama con offsets y tamaños en `docs/evidencias/e14/`.
- **14.2** Definí (o identificá) tu partición de storage y explicá por qué está donde está.
  → *Checkpoint:* la partición aparece y no se solapa con nada.
- **14.3** Escribí y leé **una** clave con la API Settings.
  → *Checkpoint:* el valor sobrevive a un reset.
- **14.4** Llevalo a tres claves de tipos distintos con handlers `set`/`get`/`export`.
  → *Checkpoint:* las tres persisten correctamente.
- **14.5** Definí defaults sensatos para el primer arranque con flash virgen.
  → *Checkpoint:* borrás la partición y el sistema arranca con defaults, logueándolo.
- **14.6** Integralo con tu shell (`app cfg get/set`) y con el comando `settings` built-in.
  → *Checkpoint:* podés cambiar configuración en vivo y verla persistir.
- **14.7** **Prueba de corte de energía:** escribí en loop y cortá la alimentación en momentos
  aleatorios, al menos 20 veces.
  → *Checkpoint:* el sistema **siempre** arranca con configuración válida, o detecta la corrupción
  y cae a defaults de forma logueada.
- **14.8 (opcional)** Estimá la vida útil en escrituras de tu diseño (ciclos de borrado × tamaño de
  sector ÷ frecuencia de escritura) y documentá el cálculo.
- **14.9 (opcional)** Usá NVS directamente y compará con Settings.

**Mínimo para aprobar:** 14.1 a 14.7.

### Entregable

Módulo de configuración persistente + informe del test de corte de energía con los 20+ ciclos.

### Criterios de aceptación

- [ ] Los valores sobreviven a reset y a power cycle.
- [ ] Tras los cortes de energía nunca arranca con configuración corrupta silenciosa.
- [ ] Hay defaults sensatos para flash virgen.
- [ ] Sabés dónde está tu partición y que no colisiona con nada.

### Defensa

1. ¿Por qué NVS necesita más espacio que la suma de tus datos?
2. ¿Qué diferencia hay entre usar NVS directo y Settings sobre NVS?
3. ¿Qué pasaría si tu partición se solapara con la del bootloader?

### Bibliografía

- Non-Volatile Storage (NVS) — https://docs.zephyrproject.org/latest/services/storage/nvs/nvs.html
- Settings — https://docs.zephyrproject.org/latest/services/storage/settings/settings.html

---

## Etapa 15 — Consumo y power management

**Requiere:** E4, E8 · **Dónde:** ESP32-S3

En un producto a batería el consumo es un requisito funcional. Zephyr separa **system PM** (el
kernel, al quedar idle, entra en un estado de bajo consumo según una política y el próximo timeout
conocido) de **device PM** (cada periférico se suspende, con o sin runtime PM por referencia
contada).

**Ajuste honesto para tu setup, y es importante.** Esta etapa está deliberadamente reformulada
respecto de lo que pediría en una placa de bajo consumo, por dos razones:

1. **La DevKitC-1 no está diseñada para medir consumo.** No tiene jumper de shunt, y el regulador
   más el puente USB-UART consumen bastante y quedan alimentados mientras el USB esté conectado.
   Cualquier medición que hagas es **de la placa completa**, no del SoC, y el piso lo pone la
   placa, no tu firmware. Para medir con algún sentido: alimentá por el pin 3V3 o 5V **con el USB
   desconectado** y el multímetro en serie.
2. **El soporte de PM para ESP32-S3 en Zephyr es menos maduro** que en las plataformas ARM que la
   documentación usa como ejemplo. Antes de planificar, verificá en `menuconfig` qué opciones de
   PM existen realmente para tu target. Si algo no aparece, no está disponible: no es que lo estés
   configurando mal.

Por eso el criterio de aceptación acá es **reducción relativa medida y atribuida**, no un número
absoluto de µA. El objetivo pedagógico —entender qué te mantiene despierto y por qué— se cumple
igual, y de hecho mejor.

### Micro-pasos

- **15.1** Medí el consumo base de tu app actual, con el método de arriba. Registrá el número
  **antes** de tocar nada.
  → *Checkpoint:* una cifra reproducible (±10 % entre corridas) y el método documentado.
- **15.2** Inventariá tus fuentes de actividad: threads que hacen polling, timers frecuentes,
  periféricos habilitados sin uso, logging activo, radio encendida.
  → *Checkpoint:* lista escrita con una hipótesis de costo para cada una.
- **15.3** Eliminá la primera fuente y medí. Atribuí la diferencia.
  → *Checkpoint:* cifra antes/después de **esa** causa.
- **15.4** Repetí con una segunda y una tercera fuente, midiendo de a una.
  → *Checkpoint:* tres reducciones atribuidas individualmente. Esto es lo central de la etapa:
  medir de a una es lo que separa optimizar de adivinar.
- **15.5** Averiguá qué opciones de PM soporta tu target y habilitá lo que exista.
  → *Checkpoint:* documentás qué encontraste, incluso si es "menos de lo que esperaba".
- **15.6** Implementá un wake-up por GPIO (usá el botón de E4) y verificá que la funcionalidad
  sobrevive al ciclo dormir/despertar.
  → *Checkpoint:* el sistema despierta por el evento previsto y sigue operando bien.
- **15.7** Escribí el presupuesto energético: consumo por estado, tiempo estimado en cada estado,
  autonomía con sus supuestos explícitos.
  → *Checkpoint:* `docs/evidencias/e15/energia.md` con la fórmula, no solo el resultado.
- **15.8 (opcional)** Bajá la frecuencia de CPU y medí el efecto.
- **15.9 (opcional)** Si hiciste E16, medí cuánto cuesta la radio encendida. Suele ser el número
  más grande de todos, y ver eso cambia cómo diseñás.

**Mínimo para aprobar:** 15.1 a 15.7.

### Entregable

`docs/evidencias/e15/energia.md` con mediciones antes/después, las tres optimizaciones con su
impacto **individual**, el presupuesto energético y las limitaciones del método reconocidas.

### Criterios de aceptación

- [ ] Hay reducción medida y **atribuida causa por causa**, no "bajó, no sé por qué".
- [ ] El dispositivo despierta por el evento previsto y la funcionalidad queda intacta.
- [ ] Documentás explícitamente las limitaciones de medir consumo en esta placa.
- [ ] La estimación de autonomía indica sus supuestos.

### Defensa

1. ¿Cómo sabe el kernel cuánto tiempo puede dormir?
2. ¿Por qué el logging puede arruinar el consumo incluso sin nadie leyendo la UART?
3. ¿Qué medirías distinto en una placa diseñada para bajo consumo, y por qué?

### Bibliografía

- Power Management (índice) — https://docs.zephyrproject.org/latest/services/pm/index.html
- System Power Management — https://docs.zephyrproject.org/latest/services/pm/system.html

---

## Etapa 16 — Conectividad: BLE o Wi-Fi

**Requiere:** E7 · **Dónde:** ESP32-S3 · **Prerrequisito duro:** `west blobs fetch hal_espressif`

Tu SoC tiene las dos radios, así que podés elegir. Elegí **un** camino y hacelo completo; el otro
queda para después.

- **Camino A — Bluetooth LE 5.0.** GAP (advertising, conexión) y GATT (servicios y
  características). Notá que el ESP32-S3 hace **solo BLE**, no Bluetooth Classic: no busques SPP ni
  A2DP.
- **Camino B — Wi-Fi + IP.** Stack IPv4/IPv6 con BSD sockets sobre el Wi-Fi del SoC. Camino natural
  si apuntás a IoT (MQTT, CoAP).

La lección transversal es la misma en ambos: **la conectividad es asíncrona y falla**. Tu
arquitectura debe tratar la desconexión como estado normal, no como excepción.

### Micro-pasos — Camino A (BLE)

- **16A.1** Levantá advertising con un nombre propio.
  → *Checkpoint:* una app de scanner en el celular ve tu dispositivo.
- **16A.2** Agregá datos propios al advertising payload.
  → *Checkpoint:* el scanner los muestra.
- **16A.3** Aceptá conexiones y logueá connect/disconnect con callbacks.
  → *Checkpoint:* el log refleja el ciclo completo.
- **16A.4** Volvé a advertising automáticamente tras una desconexión.
  → *Checkpoint:* podés reconectar sin resetear la placa.
- **16A.5** Definí un servicio GATT propio con UUID de 128 bits y una característica de **lectura**.
  → *Checkpoint:* el scanner lee el valor.
- **16A.6** Agregá una característica de **escritura**.
  → *Checkpoint:* escribís desde el celular y el firmware reacciona.
- **16A.7** Agregá **notify** y conectale los datos de tu sensor de E11.
  → *Checkpoint:* el celular recibe actualizaciones sin pedirlas.
- **16A.8** Aislá todo detrás de una interfaz propia: `main.c` no debe incluir headers de Bluetooth.
  → *Checkpoint:* podrías reemplazar BLE por Wi-Fi tocando un solo módulo.

### Micro-pasos — Camino B (Wi-Fi)

- **16B.1** Levantá la interfaz y conectate a tu red.
  → *Checkpoint:* `net iface` muestra IP asignada.
- **16B.2** `net ping` desde el shell hacia el gateway.
  → *Checkpoint:* responde.
- **16B.3** Cliente UDP que envía un mensaje fijo a un servidor tuyo en la PC.
  → *Checkpoint:* lo ves llegar (`nc -lu` o `tcpdump`).
- **16B.4** Pasá los datos reales del sensor de E11.
  → *Checkpoint:* el servidor recibe lecturas plausibles.
- **16B.5** Migrá a TCP y manejá el cierre de conexión del otro extremo.
  → *Checkpoint:* no se cuelga cuando matás el servidor.
- **16B.6** Reconexión con backoff exponencial **con jitter**.
  → *Checkpoint:* el log muestra los intentos espaciándose.
- **16B.7** Manejá "sin red al arrancar" sin bloquear el resto de la app.
  → *Checkpoint:* el resto del firmware funciona mientras la red está caída.
- **16B.8** Aislá todo detrás de una interfaz propia.
  → *Checkpoint:* `main.c` no incluye headers de red.

**Mínimo para aprobar:** los pasos .1 a .8 del camino que elegiste (sin los opcionales del otro).

### Entregable

Aplicación conectada con evidencia externa (captura del scanner BLE o log del servidor) y el manejo
de fallos demostrado.

### Criterios de aceptación

- [ ] Un cliente externo recibe datos reales de tu sensor.
- [ ] Apagar el otro extremo no cuelga ni reinicia el dispositivo: se recupera solo, y se ve en el log.
- [ ] El código de conectividad está aislado detrás de una interfaz propia.
- [ ] Documentás el costo en RAM/flash de habilitar el stack, comparado con el build de E11.

### Defensa

**A:** 1) ¿Diferencia entre notify e indicate? 2) ¿Por qué UUID de 128 bits para servicios propios?
3) ¿Qué relación hay entre connection interval y consumo?
**B:** 1) ¿En qué thread corren los callbacks de red? 2) ¿Por qué el backoff necesita jitter?
3) ¿Cómo dimensionás los buffers de red?

### Bibliografía

- Bluetooth — https://docs.zephyrproject.org/latest/connectivity/bluetooth/index.html
- Networking — https://docs.zephyrproject.org/latest/connectivity/networking/index.html

---

## Etapa 17 — MCUboot, sysbuild y DFU

**Requiere:** E14 · **Recomendado:** E16 · **Dónde:** ESP32-S3

Un producto que no se puede actualizar en campo es un pasivo. Acá armás la cadena: **MCUboot**
(bootloader con imágenes firmadas, slots primario/secundario, swap y rollback), **sysbuild** (para
construir bootloader + aplicación en una sola invocación con particiones coherentes), y el
transporte de actualización.

**Contexto específico de tu placa, y es la razón por la que esta etapa va al final.** El arranque
por defecto del `esp32s3_devkitc` es *Simple Boot*: **una sola imagen, sin segundo bootloader, sin
OTA ni seguridad**. Pasar a MCUboot no es agregar una feature: es **cambiar la arquitectura de
arranque** de tu firmware. Hay que construirlo y flashearlo al menos una vez, y el mapa de
particiones cambia. Si el mapa o las firmas no cuadran, la placa no arranca y el mensaje de error
es escueto. Es la etapa con más piezas móviles del roadmap y la que más recompensa la disciplina.

Con `--sysbuild`, el directorio de build se reorganiza por dominios (`build/<app>/zephyr/` y
`build/mcuboot/zephyr/`): no te asustes cuando no encuentres `build/zephyr/zephyr.bin` donde
siempre estuvo.

### Micro-pasos

- **17.1** Antes de tocar nada, guardá el mapa de particiones de *Simple Boot* como línea de base.
  → *Checkpoint:* diagrama "antes" en la bitácora.
- **17.2** Construí `hello_world` con `--sysbuild` y observá la estructura de dominios del build.
  → *Checkpoint:* encontrás los dos `zephyr.bin`, del bootloader y de la app.
- **17.3** Flasheá y verificá en el log de arranque que **primero** bootea MCUboot y después tu app.
  → *Checkpoint:* dos banners en el log.
- **17.4** Documentá el mapa de particiones nuevo y cómo convive con tu partición de storage de E14.
  → *Checkpoint:* diagrama "después", sin solapamientos.
- **17.5** Ahora sí, construí **tu** aplicación con sysbuild.
  → *Checkpoint:* tu app arranca bajo MCUboot y la config de E14 sigue viva.
- **17.6** Generá tus **propias claves de firma** (no las de ejemplo del árbol) y firmá.
  → *Checkpoint:* la imagen firmada con tu clave arranca.
- **17.7** Probá el rechazo: firmá una imagen con la clave incorrecta.
  → *Checkpoint:* el bootloader la rechaza, y lo ves en el log.
- **17.8** Actualización end-to-end: subí una imagen v2 al slot secundario, marcala, reiniciá.
  → *Checkpoint:* corre v2 y el log muestra la versión correcta.
- **17.9** Probá el **rollback**: una imagen que no se confirma a sí misma.
  → *Checkpoint:* MCUboot vuelve a la anterior y el dispositivo queda operativo.
- **17.10** Verificá que la configuración persistente de E14 sobrevivió a todo el proceso.
  → *Checkpoint:* las claves siguen ahí después del upgrade.
- **17.11 (opcional)** Si hiciste E16 camino A, montá DFU sobre BLE. Es el flujo más parecido a un
  producto real.

**Mínimo para aprobar:** 17.1 a 17.10.

### Entregable

Build con sysbuild + MCUboot, los dos mapas de particiones (antes/después) y `docs/dfu.md` con el
procedimiento reproducible, más evidencia de los tres escenarios: upgrade OK, rollback, firma
inválida.

### Criterios de aceptación

- [ ] La actualización v1→v2 funciona y el log muestra la versión corriendo.
- [ ] El rollback se dispara y el dispositivo queda operativo con la imagen anterior.
- [ ] La imagen con firma inválida es rechazada, con evidencia en el log.
- [ ] Las claves de ejemplo del repositorio **no** están en uso, y documentás dónde deberían
      custodiarse las reales.
- [ ] La configuración persistente sobrevive a la actualización.

### Defensa

1. ¿Qué diferencia hay entre `swap` y `overwrite-only`, y qué se pierde con cada uno?
2. ¿Qué significa exactamente "confirmar" una imagen y qué pasa si nunca se confirma?
3. La firma de MCUboot, ¿da confidencialidad, integridad, autenticidad, o cuáles de las tres?

### Bibliografía

- Sysbuild (System build) — https://docs.zephyrproject.org/latest/build/sysbuild/index.html
- Building and using MCUboot with Zephyr — https://docs.mcuboot.com/readme-zephyr.html
- Device Management (mcumgr / DFU) — https://docs.zephyrproject.org/latest/services/device_mgmt/index.html
- **Sección MCUboot / Sysbuild de tu board** — https://docs.zephyrproject.org/latest/boards/espressif/esp32s3_devkitc/doc/index.html

---

## Etapa 18 — Módulos y drivers out-of-tree

**Requiere:** E4 · **Recomendado:** E12 · **Dónde:** ambos

Hasta acá fuiste consumidor del ecosistema; ahora lo extendés. Un **módulo west** es un repositorio
propio (con `zephyr/module.yml`) que el build system integra como si fuera parte de Zephyr: puede
aportar drivers, bindings, opciones Kconfig y boards. Escribir un driver te fuerza a entender el
modelo de dispositivos desde el otro lado: macros de instanciación por Devicetree, separación de
datos constantes y mutables, niveles de inicialización. Es el paso previo a contribuir upstream.

Buena noticia: esta etapa se puede desarrollar y testear casi entera en `native_sim`, mockeando el
bus. Es el mejor ejemplo de por qué venimos insistiendo con el host.

### Micro-pasos

- **18.1** Creá un repo aparte con `zephyr/module.yml` y agregalo a tu `west.yml`.
  → *Checkpoint:* `west list` muestra tu módulo.
- **18.2** Movele un archivo trivial y hacé que tu app lo compile desde el módulo.
  → *Checkpoint:* el build lo toma sin rutas absolutas.
- **18.3** Escribí un **binding** propio en `dts/bindings/` con propiedades requeridas y opcionales.
  → *Checkpoint:* el binding es reconocido y `menuconfig`/DT no protestan.
- **18.4** Escribí el `Kconfig` del driver con sus dependencias correctas.
  → *Checkpoint:* tu opción aparece y depende de lo que debe.
- **18.5** Implementá el driver exponiendo una API de subsistema existente, con una sola instancia.
  → *Checkpoint:* `device_is_ready()` da true y la API responde.
- **18.6** Pasalo a `DT_INST_FOREACH_STATUS_OKAY` e instanciá **dos** dispositivos en un overlay.
  → *Checkpoint:* las dos instancias funcionan de forma independiente.
- **18.7** Hacé que falte una propiedad requerida y verificá el error de build.
  → *Checkpoint:* el mensaje es claro y accionable.
- **18.8** Tests Ztest del driver corriendo en `native_sim`, mockeando el bus.
  → *Checkpoint:* pasan en verde sin hardware.
- **18.9** `README.md` que explique cómo integrar tu módulo en otro proyecto.
  → *Checkpoint:* otro proyecto lo consume agregando una entrada en `west.yml`.
- **18.10 (opcional)** Pasá `checkpatch.pl` o el formateador del proyecto sobre tu código.
- **18.11 (opcional)** Definí un *shield* o una board propia derivada de la DevKitC-1 con tu
  cableado fijo, para dejar de arrastrar overlays entre etapas.

**Mínimo para aprobar:** 18.1 a 18.9.

### Entregable

Módulo west con driver, binding, Kconfig, tests y README de integración.

### Criterios de aceptación

- [ ] Otro proyecto consume tu módulo agregando solo una entrada en `west.yml`.
- [ ] Dos instancias coexisten y funcionan independientemente.
- [ ] Instanciar sin una propiedad requerida produce un error de build claro.
- [ ] Los tests del driver corren sin hardware.

### Defensa

1. ¿Por qué la config de un device es `const` y los datos son mutables? ¿Dónde vive cada uno?
2. ¿Qué determina el nivel de inicialización de tu driver y qué pasa si lo elegís mal?
3. ¿Cuándo podés usar `DT_INST_*` en lugar de `DT_*`?

### Bibliografía

- Modules (External projects) — https://docs.zephyrproject.org/latest/develop/modules.html
- Devicetree Bindings — https://docs.zephyrproject.org/latest/build/dts/bindings.html
- Board Porting Guide — https://docs.zephyrproject.org/latest/hardware/porting/board_porting.html

---

# Etapa 19 — Proyecto integrador

**Requiere:** casi todo · **Dónde:** ambos

No introduce tecnología nueva: demuestra que sabés **combinarla con criterio**. El objetivo es un
dispositivo completo y defendible, del tipo que se entregaría a un cliente. Lo que se evalúa no es
la ambición del alcance sino la coherencia de las decisiones y la calidad de la evidencia.

Consejo de secuencia: armalo **por capas verticales delgadas**, no por módulos completos. Primero
un camino end-to-end mínimo (sensor → cola → salida), y después le agregás persistencia,
conectividad, DFU. Así siempre tenés algo que funciona, que es exactamente el hábito que este
roadmap intentó construir.

### Ejercicio

Un **datalogger conectado** sobre ESP32-S3 con:

1. Adquisición periódica del sensor de E11 con drift acotado y medido.
2. Pipeline con workqueue + queue resistente a saturación.
3. Configuración persistente (intervalo, umbrales, identificador) modificable por shell y por el
   canal remoto.
4. Transmisión por el camino de E16, con reconexión y buffering local cuando no hay enlace.
5. Reducción de consumo medida y atribuida entre muestras.
6. Actualización de firmware con imágenes firmadas y rollback probado.
7. Shell de diagnóstico con `status`, estadísticas de recursos y auto-test.
8. Suite de tests en CI cubriendo la lógica de negocio, corriendo en `native_sim`.
9. `README.md` de nivel producto: arquitectura, mapa de flash, presupuesto de RAM/flash/energía,
   procedimiento de actualización, y **decisiones de diseño con sus alternativas descartadas**.

### Micro-pasos sugeridos

- **19.1** Capa vertical mínima: sensor → cola → log. Nada más.
- **19.2** Agregá persistencia de un solo parámetro.
- **19.3** Agregá el canal de transmisión, sin manejo de fallos.
- **19.4** Agregá manejo de fallos y buffering.
- **19.5** Agregá shell de diagnóstico.
- **19.6** Agregá tests de la lógica de negocio y CI.
- **19.7** Agregá MCUboot y el flujo de actualización.
- **19.8** Optimizá consumo, midiendo.
- **19.9** Escribí el README de producto.
- **19.10** Corrida de estabilidad de 72 horas.

### Entregable

Repositorio completo, historial de commits legible, CI en verde, demo reproducible por un tercero
siguiendo el README.

### Criterios de aceptación

- [ ] Un tercero puede clonar, compilar, flashear y ver el sistema funcionando siguiendo solo el
      README (incluido `west blobs fetch hal_espressif`, que se olvida siempre).
- [ ] Corre **72 horas continuas** sin reinicios no planificados ni fugas de recursos: evidencia
      con uptime y estadísticas al inicio y al final.
- [ ] Pierde el enlace y lo recupera sin intervención, sin perder datos ya adquiridos.
- [ ] La actualización de firmware funciona sobre el dispositivo en marcha, sin perder configuración.
- [ ] Cada decisión de diseño relevante tiene justificación escrita y alternativa descartada.

### Defensa

1. ¿Cuál es el punto único de falla más probable de tu diseño y cómo lo mitigarías con más tiempo?
2. Si tuvieras que reducir el consumo a la mitad, ¿qué sacrificarías?
3. ¿Qué parte de tu firmware **no** está cubierta por tests y qué riesgo aceptaste con eso?

### Bibliografía

- Zephyr Project Documentation — https://docs.zephyrproject.org/latest/index.html
- Samples and Demos — https://docs.zephyrproject.org/latest/samples/index.html
- Contribution Guidelines — https://docs.zephyrproject.org/latest/contribute/index.html

---

## Anexo A — Recursos transversales

- Índice general de la doc — https://docs.zephyrproject.org/latest/index.html
- **Samples and Demos** — el mejor recurso de aprendizaje del proyecto: para casi cualquier feature
  hay un sample mínimo — https://docs.zephyrproject.org/latest/samples/index.html
- API Reference (doxygen) — https://docs.zephyrproject.org/apidoc/latest/index.html
- Índice de opciones Kconfig — https://docs.zephyrproject.org/latest/kconfig.html
- Índice de bindings de Devicetree — https://docs.zephyrproject.org/latest/build/dts/api/bindings.html
- Release Notes y Migration Guides (imprescindibles al subir de versión) — https://docs.zephyrproject.org/latest/releases/index.html
- Repositorio principal — https://github.com/zephyrproject-rtos/zephyr

**Específicos de tu hardware:**

- Board ESP32-S3-DevKitC en Zephyr — https://docs.zephyrproject.org/latest/boards/espressif/esp32s3_devkitc/doc/index.html
- Requisitos de sistema de las boards Espressif — https://docs.zephyrproject.org/latest/boards/espressif/common/system-requirements.html
- Guía de usuario oficial de la DevKitC-1 (pinout, revisiones de placa) — https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/hw-reference/esp32s3/user-guide-devkitc-1.html
- OpenOCD para ESP32 (necesario para `west debug`) — https://github.com/espressif/openocd-esp32/releases
- JTAG debugging de Espressif — https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/jtag-debugging/index.html

Nota sobre las rutas: apuntan a `docs.zephyrproject.org/latest` y la doc se reorganiza entre
releases. Fijá una versión de Zephyr al empezar y usá la doc de esa versión para detalles de API;
si un enlace se rompe, buscá el título de la página en el buscador de la doc.

Fuentes de apoyo no oficiales pero de calidad: las charlas del **Zephyr Developer Summit**
publicadas por la Linux Foundation, y el foro de Espressif para temas específicos del SoC. Ante
discrepancia, manda la documentación oficial de tu versión.

## Anexo B — Trampas frecuentes

**Genéricas de Zephyr:**

1. **Cambiaste `prj.conf` y no pasó nada** → falta `-p always`, o una dependencia insatisfecha
   fuerza la opción a `n`. Verificá en `build/zephyr/.config`, no en tu `prj.conf`.
2. **"Unknown compatible" o el driver no se instancia** → el nodo está `disabled`, el `compatible`
   no coincide con ningún binding, o falta el `CONFIG_` del driver.
3. **`device_is_ready()` devuelve false** → el driver falló en su init. Subí el nivel de log del
   driver.
4. **Funciona con logs y falla sin logs** → race condition o dependencia de timing. Sospechá de la
   sincronización, no del logger.
5. **Crash aleatorio tras minutos u horas** → stack subdimensionado o fragmentación de heap. E13.
6. **Un ejemplo de internet no compila** → casi siempre está escrito para otra versión de Zephyr.
   Cruzá contra la doc de *tu* versión antes de investigar más.

**Específicas de tu placa:**

7. **Wi-Fi o BLE no compilan o no arrancan** → `west blobs fetch hal_espressif`, y repetilo después
   de cada `west update`.
8. **Flasheaste y la placa no arranca sola** → puede haber quedado en modo descarga. Probá
   `west flash --reset-type watchdog-reset`.
9. **No encontrás `led0`** → esta placa no tiene LED de usuario simple; su LED RGB es
   direccionable. Usá un LED externo (E4) o la API `led-strip`.
10. **No ves nada en el monitor serie** → puede que estés en el puerto USB equivocado. Son dos
    puertos distintos: puente USB-UART y USB Serial/JTAG nativo. Confirmá cuál es tu consola en
    `zephyr.dts`.
11. **`west debug` falla** → el OpenOCD del Zephyr SDK puede no tener soporte ESP32. Usá el de
    Espressif y pasá `-DOPENOCD=` y `-DOPENOCD_DEFAULT_PATH=`.
12. **Tras `--sysbuild` no encontrás `build/zephyr/zephyr.bin`** → el build se reorganizó por
    dominios: mirá `build/<app>/zephyr/` y `build/mcuboot/zephyr/`.
13. **El tamaño de flash o PSRAM no coincide con tu módulo** → se ajusta con snippets
    (`-S espressif-flash-8M`, `-S espressif-psram-4M`), no editando la board.
14. **Mediste consumo y el número es altísimo** → estás midiendo la placa completa, con regulador y
    puente USB-UART. Es esperable: leé la nota de E15.
