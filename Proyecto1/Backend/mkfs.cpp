#include "mkfs.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>

void mkfs(std::vector<std::string> &parametros, EstadoGlobal &estado) {
    std::string id;
    std::string tipo;

    auto to_lower_str = [](std::string &s) {
        std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        });
    };

    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() < 2) {
            std::cout << "[ERROR] El parametro " << command_data
                      << " no tiene la forma correcta. Ejemplo: -id=VALOR" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];
        to_lower_str(nombre_parametro);

        if (nombre_parametro == "id") {
            id = valor_parametro;
        } else if (nombre_parametro == "type") {
            tipo = valor_parametro;
            to_lower_str(tipo);
        } else {
            std::cout << "[ERROR] El parametro " << nombre_parametro << " no es valido para MKFS" << std::endl;
            return;
        }
    }

    //  VALIDACIONES
    if (id.empty()) {
        std::cout << "[ERROR] MKFS requiere el parametro obligatorio -id" << std::endl;
        return;
    }

    if (tipo.empty()) {
        tipo = "full"; // default
    }

    if (tipo != "full") {
        std::cout << "[ERROR] El parametro -type solo acepta el valor: Full" << std::endl;
        return;
    }

    //  busca la particion guardada con ese ID
    /*montada *encontrada = nullptr;
    for (auto &d : estado.discosMontados) {
        for (auto &p : d.particiones) {
            if (p.id == id) {
                encontrada = &p;
                break;
            }
        }
        if (encontrada) break;
    }

    if (!encontrada) {
        std::cout << "[ERROR] No existe ninguna particion montada con el id: " << id << std::endl;
        return;
    }*/

    //  FORMATEAR 
    estado.particionesFormateadas[id] = true;

    std::cout << "[EXITOSO] Particion con id " << id
              << " formateada correctamente como EXT2 " << std::endl;
}
