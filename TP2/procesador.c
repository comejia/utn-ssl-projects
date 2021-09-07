#include "procesador.h"

static char *obtenerTablaDePagina(FILE *fd);
static void generarReporte(t_tabla *tabla, REPORTE reporte, FORMATO tipo);
static t_tabla *parsearTabla(char *pagina);
static void listarEnConsola(t_tabla *tabla);
static void generarHTML(t_tabla *tabla, char *ruta);
static void generarCSV(t_tabla *tabla, char *ruta);
static void cargarDatosDeArchivo(char *buffer, FILE *fd);
static float obtenerNumero(char *numero);


void listarEspeciesEnNegativo(FILE *fd, FORMATO formato) {
    char *tablaPagina = obtenerTablaDePagina(fd);
    t_tabla *tabla = parsearTabla(tablaPagina);
    generarReporte(tabla, ESPECIES_EN_NEGATIVO, formato);
    free(tablaPagina);
    eliminarTabla(tabla);
}

void generarCotizacionesCompraYVenta(FILE *fd, FORMATO formato) {
    char *tablaPagina = obtenerTablaDePagina(fd);
    t_tabla *tabla = parsearTabla(tablaPagina);
    generarReporte(tabla, COTIZACION_COMPRA_VENTA, formato);
    free(tablaPagina);
    eliminarTabla(tabla);
}

static void generarReporte(t_tabla *tabla, REPORTE reporte, FORMATO salida) {
    switch(reporte) {
        case ESPECIES_EN_NEGATIVO:
            if(salida == CONSOLA) {
                listarEnConsola(tabla);
            } else if(salida == HTML) {
                generarHTML(tabla, "index.html");
            }
            break;
        case COTIZACION_COMPRA_VENTA:
            if(salida == CSV) {
                generarCSV(tabla, "compra_venta.csv");
            }
            break;
    }
}

static char *obtenerTablaDePagina(FILE *fd) {
    char *pagina = (char *)malloc(sizeof(char) * 20480);
    memset(pagina, (char)'\0', 20480);
    char line[2048];
    while(fgets(line, sizeof(line), fd) != NULL) {
        if(strstr(line, "<tbody>") == NULL) {
            continue;
        }
        break;
    }
    strcat(pagina, line);
    while(fgets(line, sizeof(line), fd) != NULL) {
        if(strstr(line, "</tbody>") != NULL) {
            break;
        }
        if(strstr(line, ">48hs<")) {
            strcat(pagina, line);
        }
        
    }
    strcat(pagina, line);
    return pagina;
}

static t_tabla *parsearTabla(char *pagina) {
    t_tabla *tabla = crearTabla();
    char *aux = pagina;
    char *registro;
    
    char dato[MAX_DATOS];
    int iDato = 0;
    int i, j;

    while ((registro = strstr(aux, INICIO_TD)) != NULL) {
        agregarRegistro(tabla);
        iDato = 0;
        while ((registro = strstr(aux, INICIO_TD)) != NULL && iDato < MAX_DATOS) {
            if(iDato != 7) {
                while (*registro++ != INICIO_TAG);
                for(i = 0, j = 0; registro[i] != FIN_TAG; i++) {
                    if(registro[i] == '.') {
                        continue;
                    }
                    if(registro[i] == ',') {
                        registro[i] = '.';
                    }
                    dato[j] = registro[i];
                    j++;
                }
            }
            else {
                while (*registro++ != INICIO_TAG);
                while (*registro++ != INICIO_TAG);
                for(i = 0, j = 0; registro[i] != '%'; i++) {
                    if(registro[i] == '.') {
                        continue;
                    }
                    if(registro[i] == ',') {
                        registro[i] = '.';
                    }
                    dato[j] = registro[i];
                    j++;
                }
            }
            dato[j] = '\0';
            guardarDatoEnRegistro(&(tabla->regs[(tabla->filas)-1]), iDato, dato);
            aux = registro;
            iDato++;
        }
    }

    return tabla;
}

static void listarEnConsola(t_tabla *tabla) {
    printf("\n\tEspecies en negativo\n");
    printf(HEADER_CONSOLA);
    for(int i = 0; i < tabla->filas; i++) {            
        if(obtenerNumero(tabla->regs[i].variacion) < 0) {
            printf(FORMATO_FILA_CONSOLA, 
                tabla->regs[i].especie, tabla->regs[i].vencimiento, tabla->regs[i].cantNominalCompra, tabla->regs[i].precioCompra, tabla->regs[i].precioVenta, tabla->regs[i].cantNominalVenta, tabla->regs[i].ultimo, tabla->regs[i].variacion,
                tabla->regs[i].apertura, tabla->regs[i].min, tabla->regs[i].max, tabla->regs[i].cierreAnterior, tabla->regs[i].volumen, tabla->regs[i].monto, tabla->regs[i].operacion, tabla->regs[i].hora
            );
        }
    }
}

static void generarCSV(t_tabla *tabla, char *ruta) {
    FILE *csv = fopen(ruta, "w");

    fprintf(csv, HEADER_CSV);
    for (int i = 0; i < tabla->filas; i++) {
        fprintf(csv, FORMATO_FILA_CSV,
            tabla->regs[i].especie, tabla->regs[i].precioCompra, tabla->regs[i].precioVenta, tabla->regs[i].apertura, tabla->regs[i].min, tabla->regs[i].max
        );
    }

    fclose(csv);
}

static void generarHTML(t_tabla *tabla, char *ruta) {
    FILE *template = fopen("template.html", "r");
    FILE *html = fopen(ruta, "w");
    char salida[8192] = {0};
    char filasTabla[8192] = {0};

    cargarDatosDeArchivo(salida, template);

    for (int i = 0; i < tabla->filas; i++) {
        char fila[1024] = {0};          
        if(obtenerNumero(tabla->regs[i].variacion) < 0) {
            sprintf(fila, DATOS_TD_HTML,
                tabla->regs[i].especie, tabla->regs[i].vencimiento, tabla->regs[i].cantNominalCompra, tabla->regs[i].precioCompra, tabla->regs[i].precioVenta, tabla->regs[i].cantNominalVenta, tabla->regs[i].ultimo, tabla->regs[i].variacion,
                tabla->regs[i].apertura, tabla->regs[i].min, tabla->regs[i].max, tabla->regs[i].cierreAnterior, tabla->regs[i].volumen, tabla->regs[i].monto, tabla->regs[i].operacion, tabla->regs[i].hora
            );
            if(obtenerNumero(tabla->regs[i].precioCompra) < obtenerNumero(tabla->regs[i].apertura) &&
                    obtenerNumero(tabla->regs[i].precioVenta) < obtenerNumero(tabla->regs[i].apertura)) {
                strcat(filasTabla, INICIO_TR_HTML_COLOR);
            } else {
                strcat(filasTabla, INICIO_TR_HTML);
            }
            strcat(filasTabla, fila);
            strcat(filasTabla, FIN_TR_HTML);
        }
    }
    fprintf(html, salida, filasTabla);

    fclose(template);
    fclose(html);
}

static void cargarDatosDeArchivo(char *buffer, FILE *fd) {
    char linea[512] = {0};
    while(fgets(linea, sizeof(linea), fd) != NULL) {
        strcat(buffer, linea);
    }
}

static float obtenerNumero(char *numero) {
    return strtof(numero, NULL);
}