/*
 * Sistema de transporte tipo SUBE (consola, archivos binarios)
 *
 * Funciones: alta de usuarios con beneficios, recargas con comprobante,
 * pago de pasajes, alta de choferes y unidades, y consultas.
 *
 * Archivos que genera: registros.dat, cuentas.dat, recargas.dat,
 * movimientos.dat, choferes.dat, transportes.dat, mayorchofer.dat
 * y un comprobante_N.txt por cada recarga.
 *
 * Compilar: gcc -Wall -Wextra -o sube sube-transport-system.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#define PASAJE 150.0f

/* Mismas cadenas en todo el programa, para que las comparaciones coincidan */
#define T_ESTUDIANTE   "Estudiante"
#define T_DISCAPACIDAD "Discapacidad"
#define T_MAYOR        "Mayor de edad"
#define T_EXCOMB       "Ex Combatiente"
#define T_SIN          "Sin beneficio"

#define TURNO_1 "primer turno"
#define TURNO_2 "segundo turno"

/* ------------------------------------------------------------------ */
/* Estructuras                                                          */
/* ------------------------------------------------------------------ */

struct fechayhora {
    int dia, mes, anio, hh, mm, ss;
};

struct registros {
    int id;                 /* numero de tarjeta */
    long dni, fecha;        /* fecha de nacimiento DDMMAAAA */
    char tele[20];
    char nomap[50];
    char direccion[50];
    char tipo[30];
};

struct cuentas {
    long dni;
    int id;
    float monto;            /* saldo */
    struct fechayhora fech; /* fecha de la ultima recarga */
    char tipos[30];
};

struct recarga {
    long dni;
    int nrocontrol;
    float monto;
    char pago[30];
    struct fechayhora fechora;
};

struct choferes {
    int id;
    long dni, fecha;
    char tele[20];
    char nomape[50], direccion[50], correo[50];
};

struct transporte {
    long dnic;              /* DNI del chofer */
    int nrounidad;
    char unidad[64];        /* "numero destino", ej: "5 Uruguay" */
    char turno[30];
    char origen[50];
    char destino[50];
};

struct movimiento {
    long numtarj, dni;
    char tel[20];
    struct fechayhora fech;
    float monto;
    char unidad[64], origen[50], destino[50], turno[30];
};

struct mayorchofer {        /* un registro por cada pasajero transportado */
    char nombredeunidad[64];
    long dnichofer;
    struct fechayhora fehc;
};

/* ------------------------------------------------------------------ */
/* Utilidades de entrada                                                */
/* ------------------------------------------------------------------ */

/* Lee una linea completa de forma segura (reemplaza gets y fflush(stdin)) */
static void leer_linea(char *dest, size_t n) {
    if (fgets(dest, (int)n, stdin) == NULL) {
        printf("\nFin de la entrada. Saliendo...\n");
        exit(0);
    }
    if (strchr(dest, '\n') == NULL) {   /* linea mas larga que el buffer */
        int c;
        while ((c = getchar()) != '\n' && c != EOF) {
        }
    }
    dest[strcspn(dest, "\r\n")] = '\0';
}

static long leer_long(const char *prompt) {
    char buf[64], *fin;
    long v;
    for (;;) {
        printf("%s", prompt);
        leer_linea(buf, sizeof buf);
        v = strtol(buf, &fin, 10);
        if (fin != buf) {
            while (*fin == ' ') fin++;
            if (*fin == '\0') return v;
        }
        printf("Valor invalido, ingresa un numero.\n");
    }
}

static float leer_float(const char *prompt) {
    char buf[64], *fin;
    double v;
    for (;;) {
        printf("%s", prompt);
        leer_linea(buf, sizeof buf);
        v = strtod(buf, &fin);
        if (fin != buf) {
            while (*fin == ' ') fin++;
            if (*fin == '\0') return (float)v;
        }
        printf("Valor invalido, ingresa un numero.\n");
    }
}

static void leer_texto(const char *prompt, char *dest, size_t n) {
    do {
        printf("%s", prompt);
        leer_linea(dest, n);
        if (dest[0] == '\0') printf("No puede quedar vacio.\n");
    } while (dest[0] == '\0');
}

static long leer_dni(void) {
    long dni;
    do {
        dni = leer_long("Ingresar numero de DNI: ");
        if (dni <= 0) printf("DNI invalido.\n");
    } while (dni <= 0);
    return dni;
}

/* Compara ignorando mayusculas/minusculas */
static int igual(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++;
        b++;
    }
    return *a == *b;
}

static struct fechayhora ahora(void) {
    time_t t = time(NULL);
    struct tm *l = localtime(&t);
    struct fechayhora f;
    f.dia = l->tm_mday;
    f.mes = l->tm_mon + 1;
    f.anio = l->tm_year + 1900;
    f.hh = l->tm_hour;
    f.mm = l->tm_min;
    f.ss = l->tm_sec;
    return f;
}

/* ------------------------------------------------------------------ */
/* Busquedas en archivos                                                */
/* ------------------------------------------------------------------ */

/* Busca una cuenta. Si la encuentra, el archivo queda posicionado justo
 * despues de ese registro (asi guardar_cuenta sabe donde escribir). */
static int buscar_cuenta(FILE *f, long dni, struct cuentas *out) {
    rewind(f);
    while (fread(out, sizeof *out, 1, f) == 1) {
        if (out->dni == dni) return 1;
    }
    return 0;
}

static void guardar_cuenta(FILE *f, const struct cuentas *c) {
    fseek(f, -(long)sizeof *c, SEEK_CUR);
    fwrite(c, sizeof *c, 1, f);
    fflush(f);
}

static int buscar_usuario(long dni, struct registros *out) {
    FILE *f = fopen("registros.dat", "rb");
    int ok = 0;
    if (f == NULL) return 0;
    while (fread(out, sizeof *out, 1, f) == 1) {
        if (out->dni == dni) {
            ok = 1;
            break;
        }
    }
    fclose(f);
    return ok;
}

static int buscar_chofer(long dni, struct choferes *out) {
    FILE *f = fopen("choferes.dat", "rb");
    int ok = 0;
    if (f == NULL) return 0;
    while (fread(out, sizeof *out, 1, f) == 1) {
        if (out->dni == dni) {
            ok = 1;
            break;
        }
    }
    fclose(f);
    return ok;
}

/* Chofer asignado a una unidad en un turno */
static int buscar_chofer_unidad(const char *unidad, const char *turno, struct transporte *out) {
    FILE *f = fopen("transportes.dat", "rb");
    int ok = 0;
    if (f == NULL) return 0;
    while (fread(out, sizeof *out, 1, f) == 1) {
        if (igual(unidad, out->unidad) && strcmp(turno, out->turno) == 0) {
            ok = 1;
            break;
        }
    }
    fclose(f);
    return ok;
}

/* Ultimo id usado (0 si el archivo no existe o esta vacio) */
static int ultimo_id_usuario(void) {
    FILE *f = fopen("registros.dat", "rb");
    struct registros r;
    int id = 0;
    if (f == NULL) return 0;
    while (fread(&r, sizeof r, 1, f) == 1) id = r.id;
    fclose(f);
    return id;
}

static int ultimo_id_chofer(void) {
    FILE *f = fopen("choferes.dat", "rb");
    struct choferes c;
    int id = 0;
    if (f == NULL) return 0;
    while (fread(&c, sizeof c, 1, f) == 1) id = c.id;
    fclose(f);
    return id;
}

static int ultimo_nro_control(void) {
    FILE *f = fopen("recargas.dat", "rb");
    struct recarga r;
    int nro = 0;
    if (f == NULL) return 0;
    while (fread(&r, sizeof r, 1, f) == 1) nro = r.nrocontrol;
    fclose(f);
    return nro;
}

/* ------------------------------------------------------------------ */
/* Altas                                                                */
/* ------------------------------------------------------------------ */

static void pedir_tipo(char *tipo, size_t n) {
    long op, cumple;
    const char *nombre = T_SIN;

    printf("Ingresar tipo de usuario:\n");
    printf("1. Estudiante\n");
    printf("2. Discapacidad\n");
    printf("3. Mayor de edad\n");
    printf("4. Ex Combatiente de Malvinas\n");
    printf("5. Sin beneficio\n");
    do {
        op = leer_long("Opcion: ");
    } while (op < 1 || op > 5);

    switch (op) {
        case 1:
            printf("Requisitos:\n- Constancia de inscripcion BEEG\n- DNI original y copia\n");
            nombre = T_ESTUDIANTE;
            break;
        case 2:
            printf("Requisitos:\n- Certificado Unico de Discapacidad (CUD) original y copia\n- DNI original\n");
            nombre = T_DISCAPACIDAD;
            break;
        case 3:
            printf("Requisitos:\n");
            printf("- Domicilio en Posadas: DNI original y copia, tener 70 anios o mas\n");
            printf("- Domicilio en Garupa: DNI original y copia\n");
            printf("  - Mujeres: tener 60 anios / Varones: mas de 65 anios\n");
            printf("  - Carnet de la Tercera Edad emitido por la Municipalidad de Garupa (original y copia)\n");
            nombre = T_MAYOR;
            break;
        case 4:
            printf("Requisitos:\n- Certificado de Ex Combatiente\n- DNI original\n");
            nombre = T_EXCOMB;
            break;
        default:
            break;
    }

    if (op != 5) {
        do {
            cumple = leer_long("Ingresar 1 si cuenta con esos requisitos, 2 si no: ");
        } while (cumple != 1 && cumple != 2);
        if (cumple == 1) {
            printf("Beneficio aprobado: %s.\n", nombre);
        } else {
            nombre = T_SIN;
            printf("No podra tener el beneficio.\n");
        }
    }
    snprintf(tipo, n, "%s", nombre);
}

static void usuario(void) {
    struct registros us;
    struct cuentas cuen;
    FILE *reg, *cu;
    long dni = leer_dni();

    if (buscar_usuario(dni, &us)) {
        printf("El usuario ya se encuentra registrado.\n");
        return;
    }

    memset(&us, 0, sizeof us);
    memset(&cuen, 0, sizeof cuen);
    us.id = ultimo_id_usuario() + 1;
    us.dni = dni;
    leer_texto("Ingresar nombre y apellido: ", us.nomap, sizeof us.nomap);
    us.fecha = leer_long("Ingresar fecha de nacimiento (DDMMAAAA): ");
    leer_texto("Ingresar numero de telefono: ", us.tele, sizeof us.tele);
    leer_texto("Ingresar direccion: ", us.direccion, sizeof us.direccion);
    pedir_tipo(us.tipo, sizeof us.tipo);

    cuen.dni = us.dni;
    cuen.id = us.id;
    cuen.monto = 0;
    snprintf(cuen.tipos, sizeof cuen.tipos, "%s", us.tipo);

    reg = fopen("registros.dat", "ab");
    cu = fopen("cuentas.dat", "ab");
    if (reg == NULL || cu == NULL) {
        printf("Error al abrir el archivo\n");
    } else {
        fwrite(&us, sizeof us, 1, reg);
        fwrite(&cuen, sizeof cuen, 1, cu);
        printf("Usuario registrado con exito. Numero de tarjeta: %d\n", us.id);
    }
    if (reg != NULL) fclose(reg);
    if (cu != NULL) fclose(cu);
}

static void cargadatoschofer(void) {
    struct choferes cho;
    FILE *f;
    long dni = leer_dni();

    if (buscar_chofer(dni, &cho)) {
        printf("Chofer ya registrado.\n");
        return;
    }

    memset(&cho, 0, sizeof cho);
    cho.id = ultimo_id_chofer() + 1;
    cho.dni = dni;
    leer_texto("Nombre y apellido: ", cho.nomape, sizeof cho.nomape);
    cho.fecha = leer_long("Fecha de nacimiento (DDMMAAAA): ");
    leer_texto("Direccion: ", cho.direccion, sizeof cho.direccion);
    leer_texto("Telefono: ", cho.tele, sizeof cho.tele);
    leer_texto("Correo electronico: ", cho.correo, sizeof cho.correo);

    f = fopen("choferes.dat", "ab");
    if (f == NULL) {
        printf("Error al abrir el archivo 'choferes.dat'.\n");
        return;
    }
    fwrite(&cho, sizeof cho, 1, f);
    fclose(f);
    printf("Chofer registrado con exito.\n");
}

/* Asigna un chofer a una unidad y turno */
static void transporte(void) {
    struct choferes cho;
    struct transporte nuevo, tr;
    FILE *f;
    long dnic, op;
    int ocupado = 0;

    dnic = leer_dni();
    if (!buscar_chofer(dnic, &cho)) {
        printf("Chofer no registrado.\n");
        return;
    }

    /* el chofer no puede estar en dos unidades */
    f = fopen("transportes.dat", "rb");
    if (f != NULL) {
        while (fread(&tr, sizeof tr, 1, f) == 1) {
            if (tr.dnic == dnic) ocupado = 1;
        }
        fclose(f);
    }
    if (ocupado) {
        printf("El chofer ya se encuentra en una unidad.\n");
        return;
    }

    memset(&nuevo, 0, sizeof nuevo);
    nuevo.dnic = dnic;
    printf("Datos de transporte:\n");
    nuevo.nrounidad = (int)leer_long("Numero de unidad: ");
    leer_texto("Origen: ", nuevo.origen, sizeof nuevo.origen);
    leer_texto("Destino: ", nuevo.destino, sizeof nuevo.destino);
    do {
        op = leer_long("Turno (1 = primer turno, 2 = segundo turno): ");
    } while (op != 1 && op != 2);
    snprintf(nuevo.turno, sizeof nuevo.turno, "%s", op == 1 ? TURNO_1 : TURNO_2);
    snprintf(nuevo.unidad, sizeof nuevo.unidad, "%d %s", nuevo.nrounidad, nuevo.destino);

    if (buscar_chofer_unidad(nuevo.unidad, nuevo.turno, &tr)) {
        printf("Esa unidad ya tiene chofer asignado en ese turno.\n");
        return;
    }

    f = fopen("transportes.dat", "ab");
    if (f == NULL) {
        printf("Error al abrir el archivo 'transportes.dat'.\n");
        return;
    }
    fwrite(&nuevo, sizeof nuevo, 1, f);
    fclose(f);
    printf("Transporte asignado con exito.\n");
}

/* ------------------------------------------------------------------ */
/* Recargas y pagos                                                     */
/* ------------------------------------------------------------------ */

static void realizarRecarga(void) {
    struct cuentas cuen;
    struct recarga car;
    struct movimiento mov;
    struct registros us;
    FILE *fcu, *frec = NULL, *fmov = NULL, *comp;
    char nombreArchivo[64];
    float monto;
    long dni, op;

    fcu = fopen("cuentas.dat", "r+b");
    if (fcu == NULL) {
        printf("Todavia no hay cuentas registradas.\n");
        return;
    }

    dni = leer_dni();
    if (!buscar_cuenta(fcu, dni, &cuen)) {
        printf("El usuario no existe en el archivo de cuentas.\n");
        goto fin;
    }

    do {
        monto = leer_float("Ingrese el monto de la recarga: ");
        if (monto <= 0) printf("El monto debe ser mayor a 0.\n");
    } while (monto <= 0);
    do {
        op = leer_long("Boca de pago (pago facil(1) o rapipago (2)): ");
    } while (op != 1 && op != 2);

    frec = fopen("recargas.dat", "ab");
    fmov = fopen("movimientos.dat", "ab");
    if (frec == NULL || fmov == NULL) {
        printf("Error de apertura de archivo\n");
        goto fin;
    }

    memset(&car, 0, sizeof car);
    car.dni = dni;
    car.monto = monto;
    car.nrocontrol = ultimo_nro_control() + 1;
    snprintf(car.pago, sizeof car.pago, "%s", op == 1 ? "pago facil" : "rapipago");
    car.fechora = ahora();

    /* actualizar el saldo de ESTA cuenta */
    cuen.monto += monto;
    cuen.fech = car.fechora;
    guardar_cuenta(fcu, &cuen);

    fwrite(&car, sizeof car, 1, frec);

    /* comprobante en texto */
    snprintf(nombreArchivo, sizeof nombreArchivo, "comprobante_%d.txt", car.nrocontrol);
    comp = fopen(nombreArchivo, "w");
    if (comp != NULL) {
        fprintf(comp, "DNI: %ld\n", car.dni);
        fprintf(comp, "Monto de la recarga: %.2f\n", car.monto);
        fprintf(comp, "Boca de pago: %s\n", car.pago);
        fprintf(comp, "Fecha de la recarga: %02d/%02d/%d\n", car.fechora.dia, car.fechora.mes, car.fechora.anio);
        fprintf(comp, "Hora de la recarga: %02d:%02d:%02d\n", car.fechora.hh, car.fechora.mm, car.fechora.ss);
        fclose(comp);
    } else {
        printf("No se pudo crear el comprobante.\n");
    }

    /* movimiento */
    memset(&mov, 0, sizeof mov);
    mov.dni = dni;
    mov.numtarj = cuen.id;
    mov.monto = monto;
    mov.fech = car.fechora;
    snprintf(mov.origen, sizeof mov.origen, "RECARGA");
    snprintf(mov.destino, sizeof mov.destino, "%s", car.pago);
    snprintf(mov.unidad, sizeof mov.unidad, "-");
    snprintf(mov.turno, sizeof mov.turno, "-");
    if (buscar_usuario(dni, &us)) snprintf(mov.tel, sizeof mov.tel, "%s", us.tele);
    fwrite(&mov, sizeof mov, 1, fmov);

    printf("Recarga realizada con exito. Comprobante: %s. Nuevo saldo: %.2f\n", nombreArchivo, cuen.monto);

fin:
    if (fcu != NULL) fclose(fcu);
    if (frec != NULL) fclose(frec);
    if (fmov != NULL) fclose(fmov);
}

static void efectuarpago(void) {
    struct cuentas cuen;
    struct registros us;
    struct transporte tr;
    struct movimiento mov;
    struct mayorchofer may;
    struct fechayhora fh = ahora();
    FILE *fcu, *f;
    char partida[50], destino[50], nombre[50], unidad[64], turno[30];
    long modo, dni;
    int numuni;
    float monto;

    printf("Ingresar modo de pago\n");
    printf("Ingresar |1| si quiere pagar con tarjeta SUBE\n");
    printf("Ingresar |2| si quiere pagar con billetera virtual\n");
    modo = leer_long("Opcion: ");
    if (modo != 1 && modo != 2) {
        printf("Opcion incorrecta.\n");
        return;
    }

    fcu = fopen("cuentas.dat", "r+b");
    if (fcu == NULL) {
        printf("Todavia no hay cuentas registradas.\n");
        return;
    }

    dni = leer_dni();
    if (!buscar_cuenta(fcu, dni, &cuen)) {
        printf("No se encontro la cuenta con el DNI especificado.\n");
        fclose(fcu);
        return;
    }

    leer_texto("Ingresar lugar de partida: ", partida, sizeof partida);
    leer_texto("Ingresar destino: ", destino, sizeof destino);

    /* los beneficiarios viajan gratis; los demas pagan el pasaje */
    monto = (strcmp(cuen.tipos, T_SIN) == 0) ? PASAJE : 0.0f;
    if (cuen.monto < monto) {
        printf("Saldo insuficiente (saldo: %.2f, pasaje: %.2f).\n", cuen.monto, monto);
        fclose(fcu);
        return;
    }

    printf("\nIngresar numero y destino de la unidad (por ejemplo, 01 Uruguay):\n");
    numuni = (int)leer_long("Ingrese numero de unidad: ");
    leer_texto("Ingresar destino de la unidad: ", nombre, sizeof nombre);
    snprintf(unidad, sizeof unidad, "%d %s", numuni, nombre);
    snprintf(turno, sizeof turno, "%s", fh.hh < 12 ? TURNO_1 : TURNO_2);

    if (!buscar_chofer_unidad(unidad, turno, &tr)) {
        printf("No hay chofer registrado en la unidad '%s' para el %s. No se cobro el pasaje.\n", unidad, turno);
        fclose(fcu);
        return;
    }

    /* todo validado: recien ahora se cobra y se guarda el saldo */
    cuen.monto -= monto;
    guardar_cuenta(fcu, &cuen);
    fclose(fcu);

    memset(&mov, 0, sizeof mov);
    mov.dni = dni;
    mov.numtarj = cuen.id;
    mov.monto = monto;
    snprintf(mov.unidad, sizeof mov.unidad, "%s", unidad);
    snprintf(mov.origen, sizeof mov.origen, "%s", partida);
    snprintf(mov.destino, sizeof mov.destino, "%s", destino);
    snprintf(mov.turno, sizeof mov.turno, "%s", turno);
    mov.fech = fh;
    if (buscar_usuario(dni, &us)) snprintf(mov.tel, sizeof mov.tel, "%s", us.tele);

    f = fopen("movimientos.dat", "ab");
    if (f == NULL) {
        printf("Error al abrir el archivo de movimientos\n");
    } else {
        fwrite(&mov, sizeof mov, 1, f);
        fclose(f);
    }

    memset(&may, 0, sizeof may);
    snprintf(may.nombredeunidad, sizeof may.nombredeunidad, "%s", unidad);
    may.dnichofer = tr.dnic;
    may.fehc = fh;
    f = fopen("mayorchofer.dat", "ab");
    if (f == NULL) {
        printf("Error al abrir el archivo de pasajeros\n");
    } else {
        fwrite(&may, sizeof may, 1, f);
        fclose(f);
    }

    printf("Pago efectuado con exito (%s). Monto: %.2f. Saldo restante: %.2f\n",
           modo == 1 ? "tarjeta SUBE" : "billetera virtual", monto, cuen.monto);
}

/* ------------------------------------------------------------------ */
/* Consultas                                                            */
/* ------------------------------------------------------------------ */

static void mostrarus(void) {
    FILE *f = fopen("registros.dat", "rb");
    struct registros us;
    int n = 0;

    if (f == NULL) {
        printf("No hay usuarios registrados.\n");
        return;
    }
    while (fread(&us, sizeof us, 1, f) == 1) {
        printf("Tarjeta N: %d\n", us.id);
        printf("DNI: %ld\n", us.dni);
        printf("Nombre y apellido: %s\n", us.nomap);
        printf("Direccion: %s\n", us.direccion);
        printf("Fecha de nacimiento: %08ld\n", us.fecha);
        printf("Telefono: %s\n", us.tele);
        printf("Tipo: %s\n", us.tipo);
        printf("-------------------------------\n");
        n++;
    }
    fclose(f);
    printf("Total de usuarios: %d\n", n);
}

static void cuenta(void) {
    FILE *f = fopen("cuentas.dat", "rb");
    struct cuentas cuen;
    long dni;

    if (f == NULL) {
        printf("No hay cuentas registradas.\n");
        return;
    }
    dni = leer_dni();
    if (buscar_cuenta(f, dni, &cuen)) {
        printf("Tarjeta N: %d\n", cuen.id);
        printf("Saldo: %.2f\n", cuen.monto);
        printf("Tipo: %s\n", cuen.tipos);
    } else {
        printf("No se encontro la cuenta.\n");
    }
    fclose(f);
}

static void listadobeneficio(void) {
    FILE *f = fopen("registros.dat", "rb");
    struct registros us;
    int n = 0;

    if (f == NULL) {
        printf("Error al acceder al archivo registros.dat\n");
        return;
    }
    printf("Usuarios con beneficios:\n");
    while (fread(&us, sizeof us, 1, f) == 1) {
        if (strcmp(us.tipo, T_SIN) != 0) {
            printf("Nombre y apellido: %s\nDNI: %ld\nTelefono: %s\nDireccion: %s\nTipo: %s\n",
                   us.nomap, us.dni, us.tele, us.direccion, us.tipo);
            printf("-------------------------------\n");
            n++;
        }
    }
    fclose(f);
    printf("Cantidad de usuarios con beneficios: %d\n", n);
}

static void listmov(void) {
    FILE *f;
    struct registros us;
    struct movimiento mov;
    long dni = leer_dni();
    int n = 0;

    if (!buscar_usuario(dni, &us)) {
        printf("Usuario no registrado.\n");
        return;
    }
    f = fopen("movimientos.dat", "rb");
    if (f == NULL) {
        printf("Todavia no hay movimientos.\n");
        return;
    }
    printf("Movimientos de %s:\n", us.nomap);
    while (fread(&mov, sizeof mov, 1, f) == 1) {
        if (mov.dni == dni) {
            printf("Fecha: %02d/%02d/%d %02d:%02d\n", mov.fech.dia, mov.fech.mes, mov.fech.anio, mov.fech.hh, mov.fech.mm);
            printf("Monto: %.2f\n", mov.monto);
            printf("Origen: %s\n", mov.origen);
            printf("Destino: %s\n", mov.destino);
            printf("Unidad: %s (%s)\n", mov.unidad, mov.turno);
            printf("Telefono: %s\n", mov.tel);
            printf("Numero de tarjeta: %ld\n", mov.numtarj);
            printf("-------------------------------\n");
            n++;
        }
    }
    fclose(f);
    if (n == 0) printf("El usuario no tiene movimientos.\n");
}

static void listarecargausuario(void) {
    FILE *f = fopen("recargas.dat", "rb");
    struct recarga car;
    long dni;
    int n = 0;

    if (f == NULL) {
        printf("Todavia no hay recargas.\n");
        return;
    }
    dni = leer_dni();
    while (fread(&car, sizeof car, 1, f) == 1) {
        if (car.dni == dni) {
            printf("DNI: %ld\n", car.dni);
            printf("Fecha: %02d/%02d/%d\n", car.fechora.dia, car.fechora.mes, car.fechora.anio);
            printf("Monto: %.2f\n", car.monto);
            printf("Boca de pago: %s\n", car.pago);
            printf("Numero de control: %d\n", car.nrocontrol);
            printf("-------------------------------\n");
            n++;
        }
    }
    fclose(f);
    if (n == 0) printf("El usuario no tiene recargas.\n");
}

static int contar_viajes(long dnichofer, int mes, int anio) {
    FILE *f = fopen("mayorchofer.dat", "rb");
    struct mayorchofer may;
    int n = 0;

    if (f == NULL) return 0;
    while (fread(&may, sizeof may, 1, f) == 1) {
        if (may.dnichofer == dnichofer && may.fehc.mes == mes && may.fehc.anio == anio) n++;
    }
    fclose(f);
    return n;
}

static void chofer_mas_pasajeros(void) {
    FILE *f = fopen("choferes.dat", "rb");
    struct choferes cho;
    int mes, anio, max = 0, n;

    if (f == NULL) {
        printf("No hay choferes registrados.\n");
        return;
    }
    do {
        mes = (int)leer_long("Mes (1-12): ");
    } while (mes < 1 || mes > 12);
    anio = (int)leer_long("Anio (ej. 2026): ");

    while (fread(&cho, sizeof cho, 1, f) == 1) {
        n = contar_viajes(cho.dni, mes, anio);
        if (n > max) max = n;
    }
    if (max == 0) {
        printf("No hay pasajeros registrados en %02d/%d.\n", mes, anio);
        fclose(f);
        return;
    }
    printf("Chofer/es con mas pasajeros en %02d/%d (%d pasajeros):\n", mes, anio, max);
    rewind(f);
    while (fread(&cho, sizeof cho, 1, f) == 1) {
        if (contar_viajes(cho.dni, mes, anio) == max) {
            printf("- %s (DNI %ld)\n", cho.nomape, cho.dni);
        }
    }
    fclose(f);
}

/* ------------------------------------------------------------------ */
/* Menus                                                                */
/* ------------------------------------------------------------------ */

static void menu(void) {
    long op;
    printf("--> MENU DE REGISTROS Y ALTAS <--\n");
    printf("|1| registrar usuarios\n");
    printf("|2| realizar una recarga\n");
    printf("|3| efectuar pago\n");
    printf("|4| registrar choferes\n");
    printf("|5| registrar unidades\n");
    printf("|-1| volver\n");
    op = leer_long("Opcion: ");
    printf("--------------------------------------\n");

    switch (op) {
        case 1: usuario(); break;
        case 2: realizarRecarga(); break;
        case 3: efectuarpago(); break;
        case 4: cargadatoschofer(); break;
        case 5: transporte(); break;
        case -1: break;
        default: printf("Opcion invalida.\n");
    }
}

static void consultas(void) {
    long op;
    printf("                 ----> MENU DE CONSULTAS <----\n");
    printf("--> |1| listado de usuarios\n");
    printf("--> |2| ver cuentas\n");
    printf("--> |3| listado de usuarios con beneficios\n");
    printf("--> |4| movimientos realizados por un usuario en particular\n");
    printf("--> |5| chofer con mas pasajeros en un mes especificado\n");
    printf("--> |6| listado de recargas efectuadas por un usuario\n");
    printf("--> |-1| volver\n");
    printf("---------------------------------------------------------------------------------\n");
    op = leer_long("Opcion: ");

    switch (op) {
        case 1: mostrarus(); break;
        case 2: cuenta(); break;
        case 3: listadobeneficio(); break;
        case 4: listmov(); break;
        case 5: chofer_mas_pasajeros(); break;
        case 6: listarecargausuario(); break;
        case -1: break;
        default: printf("Opcion invalida.\n");
    }
}

int main(void) {
    long opcion;
    do {
        printf("\n                                   ----------------------> MENU  <----------------------\n");
        printf("                                   |1| Altas/ modificaciones de archivos\n");
        printf("                                   |2| Listados/consultas\n");
        printf("                                   |-1| para salir\n");
        printf("                                  ------------------------------------------------------\n");
        opcion = leer_long("Opcion: ");

        switch (opcion) {
            case 1: menu(); break;
            case 2: consultas(); break;
            case -1: break;
            default: printf("Opcion invalida.\n");
        }
    } while (opcion != -1);
    return 0;
}
