# e01 — Anatomía de la app y build system

**Zephyr:** v4.5.0-rc3-10-gbc5d59d1b665 · **SDK:** 1.0.1 · **App:** `etapas/e01-app-layout/`
**Targets:** `native_sim/native` y `esp32s3_devkitc/esp32s3/procpu`

Este documento consolida lo encontrado en los archivos generados por el build. El detalle
(salidas completas de cada comando) está en `01.1` a `01.6` de esta misma carpeta; acá solo se
resume y se enlaza.

---

## 1. Layout de la aplicación

```
etapas/e01-app-layout/
├── CMakeLists.txt        # qué se compila
├── Kconfig               # opción propia FEATURE_CONDITIONAL + source "Kconfig.zephyr"
├── prj.conf              # qué se habilita (común a ambos targets)
├── boards/
│   ├── native_sim.conf
│   └── esp32s3_devkitc_esp32s3_procpu.conf
├── include/              # saludos.h, feat_conditional.h
└── src/                  # main.c, saludos.c, feat_conditional.c (condicional)
```

`CMakeLists.txt` lista las fuentes **explícitamente**, sin `GLOB`:

```cmake
target_sources(app PRIVATE src/main.c src/saludos.c)
target_sources_ifdef(CONFIG_FEATURE_CONDITIONAL app PRIVATE src/feat_conditional.c)
```

Ambos targets compilan con `-p always` y `west build -t run` (native_sim) / `west flash`
(ESP32-S3) imprimen `Hola desde saludo_base()!` — evidencia en [01.1](01.1-crear-app.md).

---

## 2. `build/zephyr/.config` — qué valor ganó y por qué (1.2 y 1.3)

Opción usada: `CONFIG_MAIN_STACK_SIZE`.

| Configuración | `prj.conf` | `boards/<target>.conf` | `.config` native_sim | `.config` esp32s3 |
| --- | --- | --- | --- | --- |
| Normal | 3072 | 2048 / 1024 | **2048** | **1024** |
| Sin `boards/*.conf` (carpeta renombrada) | 3072 | — | **3072** | — |
| Sin opciones (`prj.conf` vacío, sin `boards/`) | — | — | **1024** | **4096** |

La tercera fila (verificada el 2026-11-18 con una copia de la app en un directorio aparte, `-p always`)
muestra el default de cada board: 1024 en native_sim y 4096 en ESP32-S3. El log solo muestra
`Loaded configuration '<board>_defconfig'` + `Merged configuration 'prj.conf'` (vacío); no investigué
en qué archivo de Kconfig está definido ese default.

Esto también explica la diferencia de `dram0_0_seg` entre [01.1](01.1-crear-app.md) (41 744 B, cuando
`prj.conf` solo tenía `CONFIG_PRINTK=y` → stack 4096) y el resto de las evidencias (38 672 B, stack 1024):
4096 − 1024 = 3072 B, igual a la diferencia observada.

Orden de merge observado en el log de CMake (idéntico en ambos targets, [01.3](01.3-conf-precedencia.md)):

1. `Loaded configuration '.../<board>_defconfig'` (defaults de la board)
2. `Merged configuration '.../prj.conf'`
3. `Merged configuration '.../boards/<board>.conf'`

Cada fuente posterior pisa a la anterior, por eso `boards/<board>.conf` gana sobre `prj.conf`.
Sin el archivo de board (tercera fila), el valor efectivo pasa a ser el de `prj.conf` (3072).

Diferencias entre ambos `.config` completos: [native.conf](native.conf), [esp32s3.conf](esp32s3.conf)
y el `diff` en [01.2](01.2-boards-conf.md). Las diferencias visibles son de
`CONFIG_DT_HAS_*_ENABLED` (el devicetree de cada board habilita periféricos distintos), además
del stack.

---

## 3. `build/zephyr/zephyr.dts` — consola del sistema (1.4)

Nodo `chosen` del ESP32-S3 DevKitC ([01.4](01.4-consola-dts.md)):

```dts
zephyr,console    = &uart0;   /* esp32s3_devkitc_procpu.dts:27 */
zephyr,shell-uart = &uart0;   /* esp32s3_devkitc_procpu.dts:28 */
```

| Nodo | `status` | Origen del status |
| --- | --- | --- |
| `uart0` | consola | `chosen` en `procpu.dts:27` |
| `usb_serial` (`uart@60038000`) | `disabled` | `procpu.dts:51` |
| `usb_otg` (`usb_otg@60080000`, pines GPIO19/20) | `okay` | `procpu.dts:161` |

**native_sim** (verificado el 2026-11-18): `zephyr,console = &uart0` y `zephyr,shell-uart = &uart0`
(`native_sim.dts:20-21`). Ahí `uart0` es un UART emulado que el runtime conecta a la terminal del host,
sin puerto físico.

**Conclusión (ESP32-S3):** la consola es `uart0`, que sale por el puente UART→USB de la placa. El puerto USB
nativo (GPIO19/20) está asignado a `usb_otg`, no a la consola. En la práctica el monitor se abrió
sobre `/dev/ttyUSB0` ([01.1](01.1-crear-app.md)), que es el puerto del puente UART.

---

## 4. Reportes de memoria (1.5)

### ESP32-S3 (`esp32s3_devkitc/esp32s3/procpu`) — build final sin opciones extra

Tabla de regiones del linker ([01.6](01.6-compilacion-condicional.md)):

| Región | Usado | Tamaño región | % |
| --- | --- | --- | --- |
| FLASH | 135 028 B | 8 388 352 B | 1.61 % |
| iram0_0_seg | 38 904 B | 415 492 B | 9.36 % |
| dram0_0_seg | 38 672 B | 399 108 B | 9.69 % |
| irom0_0_seg | 14 976 B | 32 MB | 0.04 % |
| drom0_0_seg | 69 492 B | 32 MB | 0.21 % |

`rom_report` — total 58 292 B ([01.5](01.5-reports.md)):

| # | Consumidor | Bytes | % |
| --- | --- | --- | --- |
| 1 | `WORKSPACE` (principalmente HAL de Espressif) | 34 288 | 58.82 % |
| 2 | `ZEPHYR_BASE` (kernel, drivers, lib) | 14 754 | 25.31 % |
| 3 | `(hidden)` | 6 998 | 12.01 % |

Dentro de esos grupos: `esp_hw_support` 9 808 B, `spi_flash` 7 956 B, `ZEPHYR_BASE/drivers/*`
6 812 B. Código de la app: `main` + `saludo_base` ≈ 27 B (0.05 %).

`ram_report` — total 123 160 B:

| # | Consumidor | Bytes | % |
| --- | --- | --- | --- |
| 1 | `(hidden)` | 111 224 | 90.31 % |
| 2 | `ZEPHYR_BASE` | 9 019 | 7.32 % |
| 3 | `WORKSPACE` (HAL) | 2 275 | 1.85 % |

Dentro de `ZEPHYR_BASE`: `kheap__system_heap` 4 164 B, `z_interrupt_stacks` 2 048 B,
`z_main_stack` 1 024 B (= `CONFIG_MAIN_STACK_SIZE` efectivo).

### native_sim

| Reporte | Total | Observación |
| --- | --- | --- |
| `rom_report` | 10 642 B (94 % `(hidden)`) | **No comparable** con el ESP32-S3 |
| `ram_report` | 5 557 B (89 % `(hidden)`) | **No comparable** con el ESP32-S3 |

`native_sim` compila a un ejecutable de Linux: no hay regiones de flash/RAM de firmware, el
runtime (libc, pthreads) es del host y Zephyr queda como objeto pre-enlazado, por eso casi todo
cae en `(hidden)`. Estas cifras no representan lo que ocuparía la app en un microcontrolador.

---

Verificación final (2026-11-18): build limpio (`-p always`) de la app completa en ambos targets y
`rom_report`/`ram_report` de cada uno. Todas las cifras de esta sección se reprodujeron idénticas
(FLASH 135 028 B, dram0 38 672 B, ROM 58 292 B, RAM 123 160 B, native 10 642 B / 5 557 B), con
`CONFIG_MAIN_STACK_SIZE` = 2048 (native_sim) y 1024 (ESP32-S3) en el `.config`. Además
`zephyr.exe` imprime `Hola desde saludo_base()!`.

---

## 5. Compilación condicional (1.6)

`FEATURE_CONDITIONAL` (Kconfig propio, `default n`), activada con
`-- -DCONFIG_FEATURE_CONDITIONAL=y` ([01.6](01.6-compilacion-condicional.md)). Target: ESP32-S3.

| Opción | FLASH | `feat_conditional.c` en `rom_report` |
| --- | --- | --- |
| `n` (default) | 135 028 B | ausente |
| `y` | 135 060 B (+32 B) | presente: `feature_conditional` 14 B |

---

## 6. Puntos abiertos (no verificados)

- **ROM del reporte vs FLASH del linker:** `rom_report` suma 58 292 B y la tabla del linker dice
  `FLASH` 135 028 B. No se investigó el origen de la diferencia, por lo que las dos cifras no deben
  mezclarse como si midieran lo mismo.
- **`(hidden)` de 111 KB en `ram_report`:** su contenido no fue identificado.
- **JTAG con `usb_otg` activo:** no se probó si el puerto nativo sigue sirviendo para depurar.
