# SUBE-Style Public Transport System (C)

[Versión en español más abajo](#versión-en-español)

A console application written in C that simulates a public bus payment system similar to Argentina's SUBE card: user registration with discounted fare categories, balance top-ups, fare payments, driver and bus management, and reports. All data is stored in binary files.

Team project (2023), later reviewed, debugged and improved.

## Features

- **User registration** with fare categories: Student, Disability, Senior, War Veteran, or No benefit. Each user gets a card number and an account with a balance.
- **Balance top-ups** through a payment point (Pago Fácil / Rapipago), generating a text receipt (`comprobante_N.txt`) with a control number.
- **Fare payment** by SUBE card or virtual wallet. Users with a benefit ride for free; others are charged a fixed fare. The payment is only charged if the bus has a registered driver for the current shift.
- **Driver and bus management**: register drivers and assign each one to a bus and a shift (morning / afternoon). A driver can only be on one bus, and a bus can only have one driver per shift.
- **Reports**:
  - List of all users
  - Account balance and category by DNI
  - Users with benefits (with total count)
  - Transaction history for a given user
  - Driver with the most passengers in a given month
  - Top-up history for a given user

## Build and run

Requires a C compiler such as GCC (Linux, macOS, or MinGW on Windows).

```bash
gcc -Wall -Wextra -o sube sube-transport-system.c
./sube
```

The program creates its data files in the current directory the first time they are needed.

## Data files

| File | Content |
|---|---|
| `registros.dat` | Registered users |
| `cuentas.dat` | Accounts (balance and category) |
| `recargas.dat` | Top-up records |
| `movimientos.dat` | Transactions (top-ups and rides) |
| `choferes.dat` | Registered drivers |
| `transportes.dat` | Driver ↔ bus ↔ shift assignments |
| `mayorchofer.dat` | One record per passenger carried (used for the monthly ranking) |
| `comprobante_N.txt` | Text receipt for top-up number N |

## Concepts practiced

- Structs and fixed-size records
- Binary file I/O (`fopen`, `fread`, `fwrite`, `fseek`, in-place record updates)
- Text file output (receipts)
- Menu-driven console programs
- Safe input handling (line-based reading and validation instead of raw `scanf`)
- Working with dates and times (`time.h`)

## Testing

The program was tested manually with scripted sessions covering user registration (including duplicates), top-ups, payments with and without benefits, insufficient balance, unknown buses, driver/bus assignment rules, all reports, invalid input, and data persistence across runs. It compiles cleanly with `-Wall -Wextra -pedantic` and ran without errors under AddressSanitizer and UndefinedBehaviorSanitizer.

## Known limitations

- Users cannot be edited or deleted once registered (only balances change).
- The fare is a fixed constant (`PASAJE` in `sube-transport-system.c`).
- No authentication; this is a learning project, not production software.
- Binary data files are not portable between different compilers/platforms (struct layout may differ).
- Records are searched linearly, which is fine for small data sets but would not scale.
- Text is ASCII only (no accented characters).

## Authors

- Carlos Damián Padilla
- Juan Agustín Nobs
- Lautaro Zado

---

# Versión en español

## Sistema de transporte público tipo SUBE (C)

Aplicación de consola escrita en C que simula un sistema de pago de colectivos similar a la tarjeta SUBE de Argentina: registro de usuarios con categorías de tarifa con beneficio, recargas de saldo, pago de pasajes, gestión de choferes y unidades, y listados. Todos los datos se guardan en archivos binarios.

Proyecto en equipo (2023), luego revisado, depurado y mejorado.

### Funcionalidades

- **Registro de usuarios** con categorías: Estudiante, Discapacidad, Mayor de edad, Ex Combatiente de Malvinas o Sin beneficio. Cada usuario recibe un número de tarjeta y una cuenta con saldo.
- **Recargas de saldo** por boca de pago (Pago Fácil / Rapipago), con un comprobante de texto (`comprobante_N.txt`) y número de control.
- **Pago de pasajes** con tarjeta SUBE o billetera virtual. Los usuarios con beneficio viajan gratis; el resto paga una tarifa fija. El pago solo se cobra si la unidad tiene un chofer registrado en el turno actual.
- **Gestión de choferes y unidades**: alta de choferes y asignación de cada uno a una unidad y un turno (mañana / tarde). Un chofer solo puede estar en una unidad, y una unidad solo puede tener un chofer por turno.
- **Listados y consultas**:
  - Listado de usuarios
  - Saldo y categoría de una cuenta por DNI
  - Usuarios con beneficios (con cantidad total)
  - Movimientos de un usuario en particular
  - Chofer con más pasajeros en un mes determinado
  - Recargas realizadas por un usuario

### Compilar y ejecutar

Requiere un compilador de C como GCC (Linux, macOS o MinGW en Windows).

```bash
gcc -Wall -Wextra -o sube sube-transport-system.c
./sube
```

El programa crea sus archivos de datos en la carpeta actual la primera vez que los necesita.

### Archivos de datos

| Archivo | Contenido |
|---|---|
| `registros.dat` | Usuarios registrados |
| `cuentas.dat` | Cuentas (saldo y categoría) |
| `recargas.dat` | Registro de recargas |
| `movimientos.dat` | Movimientos (recargas y viajes) |
| `choferes.dat` | Choferes registrados |
| `transportes.dat` | Asignaciones chofer ↔ unidad ↔ turno |
| `mayorchofer.dat` | Un registro por cada pasajero transportado (se usa para el ranking mensual) |
| `comprobante_N.txt` | Comprobante de texto de la recarga número N |

### Conceptos practicados

- Estructuras (`struct`) y registros de tamaño fijo
- Manejo de archivos binarios (`fopen`, `fread`, `fwrite`, `fseek`, actualización de registros en el lugar)
- Salida a archivos de texto (comprobantes)
- Programas de consola con menús
- Lectura segura de datos (lectura por línea y validación en lugar de `scanf` directo)
- Manejo de fechas y horas (`time.h`)

### Pruebas

El programa se probó manualmente con sesiones guionadas que cubren el registro de usuarios (incluyendo duplicados), recargas, pagos con y sin beneficio, saldo insuficiente, unidades inexistentes, reglas de asignación de choferes y unidades, todos los listados, entradas inválidas y persistencia de datos entre ejecuciones. Compila sin avisos con `-Wall -Wextra -pedantic` y se ejecutó sin errores con AddressSanitizer y UndefinedBehaviorSanitizer.

### Limitaciones conocidas

- Los usuarios no se pueden editar ni eliminar una vez registrados (solo cambian los saldos).
- La tarifa es una constante fija (`PASAJE` en `sube-transport-system.c`).
- No hay autenticación; es un proyecto de aprendizaje, no software de producción.
- Los archivos binarios no son portables entre distintos compiladores o plataformas (el formato de las estructuras puede variar).
- Los registros se buscan de forma lineal: sirve para pocos datos, pero no escalaría.
- El texto es solo ASCII (sin caracteres acentuados).

### Autores

- Carlos Damián Padilla
- Juan Agustín Nobs
- Lautaro Zado
