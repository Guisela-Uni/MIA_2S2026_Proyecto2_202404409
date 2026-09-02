#include "mkdisk.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

void mkdisk(std::vector<std::string> &parametros, EstadoGlobal &estado) {
    // ------------- DATA DEL DISCO (solo en memoria)
    int disk_size = 0;            // TAMAÑO
    std::string fit;              // ff/bf/wf
    char fit_char = '0';          // f/b/w
    std::string disk_unit;        // k/m
    std::string disk_path;        // PATH (clave logica, no se toca el filesystem real)

    // pasa a minuscula todo
    auto to_lower_str = [](std::string &s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    };

    // lee los parametros y establece el valor de cada variable size, fit, unit, path
    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() < 2) {
            std::cout << "[ERROR!] El parametro " << command_data
                      << " no cuenta con la forma correcta. EJEMPLO: -size=VALOR" << std::endl;
            return;
        }

        // nombre_parametro = fit, size, path, unit
        // valor_parametro = valor asignado al parametro
        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];

        to_lower_str(nombre_parametro);

        if (nombre_parametro == "size") {
            //prueba pasarlo a entero si no error
            try {
                disk_size = std::stoi(valor_parametro);
            } catch (...) {
                std::cout << "[ERROR!] El parametro size debe ser un entero" << std::endl;
                return;
            }
        } else if (nombre_parametro == "fit") {
            to_lower_str(valor_parametro);
            fit = valor_parametro;
        } else if (nombre_parametro == "unit") {
            to_lower_str(valor_parametro);
            disk_unit = valor_parametro;
        } else if (nombre_parametro == "path") {
            disk_path = valor_parametro;
        } else {
            std::cout << "[ERROR!] Parametro no valido: " << nombre_parametro << std::endl;
            return;
        }
    }

    //  VALIDACIONES
    //disk size > 0
    if (disk_size <= 0) {
        std::cout << "[ERROR!] El size debe ser mayor que 0" << std::endl;
        return;
    }
    // debe haber un path
    if (disk_path.empty()) {
        std::cout << "[ERROR!] MKDISK requiere el parametro path" << std::endl;
        return;
    }

    // si no viene fit se va un default
    if (fit.empty()) {
        fit = "ff";
    }
    // si no viene unit se va default
    if (disk_unit.empty()) {
        disk_unit = "m";
    }

    // pasar de string a char el fit
    if (fit == "ff") {
        fit_char = 'f';
    } else if (fit == "bf") {
        fit_char = 'b';
    } else if (fit == "wf") {
        fit_char = 'w';
    } else {
        std::cout << "[ERROR!] Tipo de Fit NO VALIDO - SOLO SE ACEPTA: ff, bf, wf" << std::endl;
        return;
    }

    // AJUSTAR TAMAÑO A BYTES
    int disk_size_bytes = 0;
    if (disk_unit == "m") {
        disk_size_bytes = disk_size * 1024 * 1024;
    } else if (disk_unit == "k") {
        disk_size_bytes = disk_size * 1024;
    } else {
        std::cout << "[ERROR!] Tipo de Unit NO VALIDO - SOLO SE ACEPTA: k, m" << std::endl;
        return;
    }

    // "CREACION" DEL DISCO (solo en el registro en memoria)
    // 1 VERIFICAR QUE NO EXISTA YA UN DISCO SIMULADO CON ESE PATH
    if (estado.discos.find(disk_path) != estado.discos.end()) {
        std::cout << "[ERROR!] Ya existe un disco simulado registrado en ese path: " << disk_path << std::endl;
        return;
    }

    // 2 REGISTRAR EL DISCO EN MEMORIA (equivalente al MBR inicial)
    SimDisk nuevo;
    nuevo.path = disk_path;
    nuevo.tamano = disk_size_bytes;
    nuevo.signature = std::rand() % 7777;
    nuevo.creado = std::time(nullptr);
    nuevo.fit = fit_char;

    for (int i = 0; i < 4; i++) {
        std::memset(nuevo.particiones[i].part_name, 0, sizeof(nuevo.particiones[i].part_name));
        nuevo.particiones[i].part_status = '0';
        nuevo.particiones[i].part_s = 0;
        nuevo.particiones[i].part_fit = fit_char;
        nuevo.particiones[i].part_start = -1;
        nuevo.particiones[i].part_type = '\0';
    }

    estado.discos[disk_path] = nuevo;

    std::cout << "[EXITOSO] Comando MKDISK valido. Disco simulado registrado: " << disk_path
              << " (" << disk_size_bytes << " bytes, fit=" << fit_char << ")" << std::endl;
}
