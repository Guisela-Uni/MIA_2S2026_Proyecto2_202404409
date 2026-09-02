#include "fdisk.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

void fdisk(std::vector<std::string> &parametros, EstadoGlobal &estado) {
    // VALIDACION

    int size = 0;                 // size
    std::string fit;              // ff/bf/wf
    std::string unidad;           // b/k/m
    std::string disk_path;        // path del disco
    std::string tipo;             // p/e/l
    std::string nombre;           // nombre de la partición
    char fit_char = 'f';          // f/b/w
    char tipo_char = 'p';         // p/e/l

    // PARA QUE SEA LOWECASE
    auto to_lower_str = [](std::string &s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    };

    //LEER LOS PARAMETROS Y ASIGNARLOS
    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() < 2) {
            std::cout << "[ERROR] El parametro " << command_data
                      << " no tiene la forma correcta. Ejemplo: -size=VALOR" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];

        to_lower_str(nombre_parametro);

        if (nombre_parametro == "size") {
            try {
                size = std::stoi(valor_parametro);
            } catch (...) {
                std::cout << "[ERROR] El size debe de ser un valor numerico." << std::endl;
                return;
            }
        } else if (nombre_parametro == "unit") {
            unidad = valor_parametro;
        } else if (nombre_parametro == "path") {
            disk_path = valor_parametro;
        } else if (nombre_parametro == "type") {
            tipo = valor_parametro;
        } else if (nombre_parametro == "fit") {
            fit = valor_parametro;
        } else if (nombre_parametro == "name") {
            nombre = valor_parametro;
        } else {
            std::cout << "[ERROR] El parametro " << nombre_parametro
                      << " no es valido para fdisk (solo crear particiones)." << std::endl;
            return;
        }
    }

    // -------------- VALIDACIONES
    if (disk_path.empty() || nombre.empty()) {
        std::cout << "[ERROR] EL COMANDO FDISK NECESITA PATH Y NAME" << std::endl;
        return;
    }

    if (size <= 0) {
        std::cout << "[ERROR] EL SIZE DEBE SER MAYOR A 0." << std::endl;
        return;
    }

    // normalizar a minusculas fit, unidad, tipo
    to_lower_str(fit);
    to_lower_str(unidad);
    to_lower_str(tipo);

    // DEFAULTS
    // FIT
    if (fit.empty()) {
        // si no vienen nada el default el ff
        fit = "ff";
    }

    if (fit == "ff") {
        fit_char = 'f';
    } else if (fit == "bf") {
        fit_char = 'b';
    } else if (fit == "wf") {
        fit_char = 'w';
    } else {
        std::cout << "[ERROR] EL TIPO DE FIT NO ES VALIDO, DEBE SER: FF, BF, WF" << std::endl;
        return;
    }

    // TIPO
    if (tipo.empty()) {
        tipo = "p";
    }

    if (tipo == "p") {
        tipo_char = 'p';
    } else if (tipo == "e") {
        tipo_char = 'e';
    } else if (tipo == "l") {
        tipo_char = 'l';
    } else {
        std::cout << "[ERROR] EL TIPO NO ES VALIDO, DEBE SER: p/e/l" << std::endl;
        return;
    }

    //unidad
    if (unidad.empty()) {
        unidad = "k";
    }

    int size_bytes = size;
    if (unidad == "m") {
        size_bytes = size * 1024 * 1024;
    } else if (unidad == "k") {
        size_bytes = size * 1024;
    } else if (unidad == "b") {
        size_bytes = size;  // ya viene en bytes
    } else {
        std::cout << "[ERROR] TIPO NO VALIDO, SOLO SE ACEPTA: B/K/M" << std::endl;
        return;
    }

    // ------- BUSCAR EL DISCO SIMULADO (debe haberse creado antes con MKDISK)
    auto it = estado.discos.find(disk_path);
    if (it == estado.discos.end()) {
        std::cout << "[ERROR] El disco no existe (debe crearse primero con MKDISK): " << disk_path << std::endl;
        return;
    }

    particionar(size_bytes, tipo_char, disk_path, nombre, fit_char, it->second);
}


void particionar(int size, char tipo, const std::string &ruta, const std::string &nombre, char fit, SimDisk &disco) {
    (void)ruta; // ya no se usa para abrir archivos, se deja por compatibilidad de firma/documentacion

    // DEFAULT -1, es que no hay extendida
    int posicion = -1;

    // ************** ELEGIR ESPACIO SEGUN EL FIT
    auto fitting_fit = [&](auto &espacios, int espacio_requerido) -> int {
        //espacios=vector de las sturcts libre, cada uno tiene un campo size
        //espacio_requerido=cuanto espacio necesita

        //DEVUELVE EL INDICE DEL ESPACIO EN DONDE CABE Y -1 SI NO CABE

        // SI EL ESPACIO DA MENOS UNO MARCA QUE NO HAY ESPACIO DISPONIBLE
        if (espacios.empty()) {
            return -1;
            // si no hay espacio
        }

        if (fit == 'f' || fit == 'b') {
            //ordena los bloques de menor a mayor y luego ya recorre y queda donde primero quepa
            if (fit == 'b') {
                std::sort(espacios.begin(), espacios.end());
            }
            //recorre en orden y devuelve el primer indice en donde el size cabe
            for (int i = 0; i < static_cast<int>(espacios.size()); ++i) {
                if (espacio_requerido <= espacios[i].size) {
                    return i;
                }
            }
            return -1;
        }

        if (fit == 'w') {
            //ordena de menor a mayor y elige el mayoir back()
            std::sort(espacios.begin(), espacios.end());
            //si cabe en el del final devuelve ese indice, por eso se le resta 1
            if (espacio_requerido <= espacios.back().size) {
                return static_cast<int>(espacios.size()) - 1;
            }
            return -1;
        }

        return -1;
    };


    if (tipo == 'l') {
        int inicioExt;
        int finExt;
        EBR ebr;
        bool existe_particion = false;

        std::vector<libreL> espacios;
        bool header_visitada = false;
        bool keep_going = true;

        // 1. PRIMERO SE DEBE BUSCAR UNA EXTENDIDA EN EL MBR SIMULADO
        for (int i = 0; i < 4; i++) {
            if (disco.particiones[i].part_type == 'e') {
                posicion = i;
                break;
            }
        }

        if (posicion == -1) {
            std::cout << "[ERROR] NO EXISTE UNA EXTENDIDA - NO SE PUEDE CREAR LOGICA." << std::endl;
            return;
        }

        // SEGUNDO LEER EL HEADER DE LA EXTENDIDA - el primer ebr
        inicioExt = disco.particiones[posicion].part_start;
        finExt    = inicioExt + disco.particiones[posicion].part_s;

        auto it_ebr = disco.ebrs.find(inicioExt);
        if (it_ebr == disco.ebrs.end()) {
            std::cout << "[ERROR] Estructura interna invalida: no se encontro el header de la extendida." << std::endl;
            return;
        }
        ebr = it_ebr->second;

        // TERCERO RECORRER LOS EBR para ver el espacio libre y los nombre repetidos
        while (keep_going) {
            // revisa si ya existe una particion con ese nombre
            if (std::string(ebr.part_name) == nombre) {
                existe_particion = true;
                break;
            }

            //  estructura libre: lista de espacios libres
            libreL esp;
            // caso 1= da una vuelta y el header esta vacio ==0
            // ES DECIR estamos en el primer ebr y no tiene particiones logicas
            if (!header_visitada && ebr.part_s == 0) {
                // se marca que partene al header
                // y las posiciones
                esp.header = true;
                esp.inicioEBR = inicioExt;
                esp.finLogica = ebr.part_start;

                if (ebr.part_next == -1) {
                    // no hay mas ebr, entonces el espacio libre es hasta el final de la extendida
                    esp.size = finExt - esp.finLogica;
                } else {
                    // hay mas ebr, entonces el espacio libre es hasta el siguiente ebr
                    esp.size = ebr.part_next - esp.finLogica;
                }
            } else {
                // caso 2 = ya no es el header o ya tiene datos. EL header ya tiene particion
                esp.header = false;
                // aca inicioExt se va reasignando dentro del loop (posicion del ebr actual)
                esp.inicioEBR = inicioExt;
                //ultimo byte ocupado por la logica actual
                esp.finLogica = ebr.part_start + ebr.part_s - 1;

                //el espacio libre empieza desde finLogica +1 hasta el siguiente ebr o el final de la extendida

                if (ebr.part_next == -1) {
                    //si no hay mas ebr, el espacio libre desde el fin de la logica hasta el fin de la extendida
                    esp.size = finExt - (esp.finLogica + 1);
                } else {
                    //si hay mas ebr, el espacio libre desde el fin de la logica hasta el siguiente ebr
                    esp.size = ebr.part_next - (esp.finLogica + 1);
                }
            }
            //si es mayor a 0(es decir si tiene tamaño) se añade a los espacios libres
            if (esp.size > 0) {
                espacios.push_back(esp);
            }

            // moverse al siguiente ebr si existe
            header_visitada = true;

            if (ebr.part_next == -1) {
                // si ya no hay mas no se sigue
                keep_going = false;
            } else {
                // si aun hay mas sigue el loop.
                // EL INICIO ahora es el puntero next
                inicioExt = ebr.part_next;
                auto it_next = disco.ebrs.find(inicioExt);
                if (it_next == disco.ebrs.end()) {
                    std::cout << "[ERROR] Estructura interna invalida: encadenamiento de EBR roto." << std::endl;
                    return;
                }
                ebr = it_next->second;
            }
        }

        //  VALIDACIONES LOGICAS
        if (existe_particion) {
            std::cout << "[ERROR] YA EXISTE UNA PARTICION LOGICA CON ESE NOMBRE" << std::endl;
            return;
        }

        if (espacios.empty()) {
            std::cout << "[ERROR] LA PARTICIÓN EXTENDIDA NO TIENE ESPACIO LIBRE." << std::endl;
            return;
        }

        //  ELEGIR EL ESPACIO SEGUN EL FIT
        int espacio_requerido = size + static_cast<int>(sizeof(EBR));
        //posEspacio es el indice en donde cabe
        int posEspacio = fitting_fit(espacios, espacio_requerido);

        if (posEspacio == -1) {
            std::cout << "[ERROR] NO HAY ESPACIO DISPONIBLE EN LA PARTICIÓN EXTENDIDA." << std::endl;
            return;
        }

        // CREAR LA PARTICION LOGICA (simulada)

        // caso 1: si el espacio libre es el header
        if (espacios[posEspacio].header) {
            // REESCRIBIR el header (en memoria)
            EBR &header = disco.ebrs[espacios[posEspacio].inicioEBR];

            std::memset(header.part_name, 0, sizeof(header.part_name));
            std::strncpy(header.part_name, nombre.c_str(), sizeof(header.part_name) - 1);
            header.part_fit = fit;
            header.part_s = size;
            header.part_status = '0'; //aun no esta montada

            std::cout << "[EXITOSO] PARTICION LOGICA CREADA CORRECTAMENTE" << std::endl;
        } else {
            // caso 2: si el espacio libre esta entre dos ebr o al final o no es la header
            //posEBR es el inicio (virtual) del nuevo ebr, justo luego de la logica anterior
            int posEBR = espacios[posEspacio].finLogica + 1;
            int next;

            // Actualizar EBR padre para que el next ahora apunte al inicio del nuevo EBR
            EBR &padre = disco.ebrs[espacios[posEspacio].inicioEBR];
            // NEXT ES A LO QUE ANTES APUNTABA
            next = padre.part_next;
            padre.part_next = posEBR;

            // Crear nuevo EBR (simulado)
            EBR nuevo{};
            nuevo.part_fit = fit;
            std::memset(nuevo.part_name, 0, sizeof(nuevo.part_name));
            std::strncpy(nuevo.part_name, nombre.c_str(), sizeof(nuevo.part_name) - 1);
            nuevo.part_status = '0';
            nuevo.part_s = size;
            // LOS DATOS Empiezan luego del BER (offset virtual)
            nuevo.part_start = posEBR + static_cast<int>(sizeof(EBR));
            //el nuevo apunta a lo que antes apuntaba el padre
            nuevo.part_next = next;

            disco.ebrs[posEBR] = nuevo;

            std::cout << "[EXITOSO] PARTICION LOGICA CREADA CORRECTAMENTE " << std::endl;
        }

        return;
    }

    // PRIMARIA O EXTENDIDA
    /*
    SOLO PUEDE EXISTIR UNA EXTENDIDA
    */
    if (tipo == 'p' || tipo == 'e') {
        bool extendedExist = false;
        bool existe_particion = false;
        // POSICIONES DE LAS PARTICIONES NO VACIAS
        std::vector<position> posiciones;
        std::vector<libre> espacios;
        int posEspacio = -1;

        // ------ 1. Buscar slot libre en el MBR simulado y detectar si ya hay extendida
        for (int i = 0; i < 4; i++) {
            if (disco.particiones[i].part_name[0] == '\0') {
                //si encuentra una parte libre toma esa posicion
                posicion = i;
            }

            if (disco.particiones[i].part_type == 'e') {
                //revisar si hay una extendida
                extendedExist = true;
            }

            if (posicion != -1) {
                // si no hay entradas, las 4 estan usadas entonces para
                break;
            }
        }

        // validaciones, SOLO UNA EXTENDIDA y NO HAY ESPACIO
        if (extendedExist && tipo == 'e') {
            std::cout << "[ERROR] SOlo puede existir una particion extendida" << std::endl;
            return;
        }

        if (posicion == -1) {
            std::cout << "[ERROR] No hay espacio, ya hay 4 particiones" << std::endl;
            return;
        }

        // cargar las particiones existentes y verificar los nombres
        for (int i = 0; i < 4; i++) {
            if (disco.particiones[i].part_name[0] != '\0') {
                position pos;
                pos.inicio = disco.particiones[i].part_start;
                pos.fin = disco.particiones[i].part_start + disco.particiones[i].part_s - 1;
                pos.nombre = disco.particiones[i].part_name;
                //POSICIONES ALMACENA EL INICIO Y FIN DE CADA PARTICION
                posiciones.push_back(pos);

                if (nombre == disco.particiones[i].part_name) {
                    existe_particion = true;
                }
            }
        }

        if (existe_particion) {
            std::cout << "[ERROR] Ya existe una particion con ese nombre" << std::endl;
            return;
        }

        // ordenar las posiciones si no esta vacio
        if (!posiciones.empty()) {
            std::sort(posiciones.begin(), posiciones.end());
        }

        // si no hay particiones - Construir los espacios libres
        // si no todo el espacio desde endMBR hasta el final del disco esta libre
        if (posiciones.empty()) {
            libre esp;
            esp.inicio = EndMBR;
            esp.size = disco.tamano - EndMBR;
            espacios.push_back(esp);
        } else {
            // si si hay particiones se debe buscar los espacios libres entre ellas
            for (int i = 0; i < static_cast<int>(posiciones.size()); i++) {
                // esp = struct libre
                libre esp;
                // x = posicion actual
                position &x = posiciones[i];
                //tamaño del espacio
                int espacio_libre = 0;

                if (i == 0 && i != (static_cast<int>(posiciones.size()) - 1)) {
                    // HAY MAS DE UNA Y ESTAMOS EN LA PRIMERA PARTICION

                    // 1 espacio entre el final del mbr y el inicio de la primera particion
                    espacio_libre = x.inicio - EndMBR;
                    if (espacio_libre > 0) {
                        esp.inicio = EndMBR;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                    // 2 espacio entre la primera y la segunda particion
                    position &y = posiciones[i + 1];
                    // espacio justo despues de la primera particion
                    espacio_libre = y.inicio - (x.fin + 1);
                    if (espacio_libre > 0) {
                        esp.inicio = x.fin + 1;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                } else if (i == 0 && i == (static_cast<int>(posiciones.size()) - 1)) {
                    //2. SOLO HAY UNA PARTICION

                    //espacio entre el MBR y la particion
                    espacio_libre = x.inicio - EndMBR;
                    if (espacio_libre > 0) {
                        esp.inicio = EndMBR;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                    // espacio entre esa particion y el final del disco
                    espacio_libre = disco.tamano - (x.fin + 1);
                    if (espacio_libre > 0) {
                        esp.inicio = x.fin + 1;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                } else if (i != (static_cast<int>(posiciones.size()) - 1)) {
                    // 3. PARTICION INTERMEDIA

                    // espacio entre la particion actual y la siguiente
                    position &y = posiciones[i + 1];
                    espacio_libre = y.inicio - (x.fin + 1);
                    if (espacio_libre > 0) {
                        esp.inicio = x.fin + 1;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                } else {
                    // 4. ULTIMA PARTICION
                    // espacio entre la ultima particion y el final del disco
                    espacio_libre = disco.tamano - (x.fin + 1);
                    if (espacio_libre > 0) {
                        esp.inicio = x.fin + 1;
                        esp.size = espacio_libre;
                        espacios.push_back(esp);
                    }
                }
            }
        }

        if (espacios.empty()) {
            std::cout << "[ERROR] NO HAY ESPACIO EN EL DISCO" << std::endl;
            return;
        }

        // elegir el espacio segun el fit
        posEspacio = fitting_fit(espacios, size);

        if (posEspacio == -1) {
            std::cout << "[ERROR] NO HAY ESPACIO EN EL DISCO PARA LA PARTICION" << std::endl;
            return;
        }

        //CREAR LA PARTICION (simulada)
        disco.particiones[posicion].part_fit   = fit;
        std::memset(disco.particiones[posicion].part_name, 0, sizeof(disco.particiones[posicion].part_name));
        std::strncpy(disco.particiones[posicion].part_name, nombre.c_str(), sizeof(disco.particiones[posicion].part_name) - 1);
        disco.particiones[posicion].part_status = '0'; // no montada
        disco.particiones[posicion].part_s      = size;
        disco.particiones[posicion].part_type   = tipo; // p/e

        //inicio del bloque libre elegido
        disco.particiones[posicion].part_start  = espacios[posEspacio].inicio;

        // SI ES EXT SE DEBE CREAR EL EBR INICIAL (header, simulado)
        // el ebr inicial es la header que luego se usa con las particiones logicas
        if (tipo == 'e') {
            EBR ebr{};
            //nombre vacio, part_next -1 (no hay logicas)
            std::memset(ebr.part_name, 0, sizeof(ebr.part_name));
            ebr.part_next  = -1;
            // inicio de la extendida + tamaño del ebr (donde inician los datos)
            ebr.part_start = espacios[posEspacio].inicio + static_cast<int>(sizeof(EBR)) + 1;
            ebr.part_s     = 0;
            ebr.part_status = '0';

            disco.ebrs[espacios[posEspacio].inicio] = ebr;
        }

        std::cout << "[EXITOSO] LA PARTICION " << (tipo == 'e' ? "EXTENDIDA" : "PRIMARIA")
                  << " FUE CREADA CORRECTAMENTE (simulada)" << std::endl;
    }
}
