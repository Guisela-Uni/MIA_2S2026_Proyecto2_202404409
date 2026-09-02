#include "rmdisk.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

void rmdisk(std::vector<std::string> &parametros, EstadoGlobal &estado) {
    std::string disk_path;

    // para pasar a minuscula
    auto to_lower_str = [](std::string &s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    };

    // leer los parametros y establecer el valor de la variable
    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() < 2) {
            std::cout << "[ERROR!] El parametro " << command_data
                      << " no cuenta con la forma correcta. EJEMPLO: -path=VALOR" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];

        to_lower_str(nombre_parametro);

        if (nombre_parametro == "path") {
            disk_path = valor_parametro;
        } else {
            std::cout << "[ERROR!] EL PARAMETRO " << nombre_parametro << " NO ES VALIDO EN RMDISK" << std::endl;
            return;
        }
    }

    //  VALIDACIONES
    // obligatorio path
    if (disk_path.empty()) {
        std::cout << "[ERROR!] RMDISK necesita el parametro obligatorio path" << std::endl;
        return;
    }

    // verificar que el disco simulado exista en el registro en memoria
    auto it = estado.discos.find(disk_path);
    if (it == estado.discos.end()) {
        std::cout << "[ERROR!] EL DISCO NO EXISTE, REVISAR LA RUTA " << disk_path << std::endl;
        return;
    }

    // "ELIMINAR" EL DISCO -> solo se quita del registro en memoria
    estado.discos.erase(it);

    std::cout << "[EXITOSO] Disco simulado eliminado del registro -> " << disk_path << std::endl;
}
