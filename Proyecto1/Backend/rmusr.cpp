#include "rmusr.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>

void rmusr(std::vector<std::string> &parametros, EstadoGlobal &estado) {
    std::string user;

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
                      << " no tiene la forma correcta. Ejemplo: -user=VALOR" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];
        to_lower_str(nombre_parametro);

        if (nombre_parametro == "user") {
            user = valor_parametro;
        } else {
            std::cout << "[ERROR] El parametro " << nombre_parametro << " no es valido para RMUSR" << std::endl;
            return;
        }
    }

    if (user.empty()) {
        std::cout << "[ERROR] RMUSR requiere el parametro obligatorio -user" << std::endl;
        return;
    }

    //  BUSCAR Y ELIMINAR
    auto it = std::find_if(estado.usuarios.begin(), estado.usuarios.end(),
                            [&](const usuario &u) { return u.user == user; });

    if (it == estado.usuarios.end()) {
        std::cout << "[ERROR] El usuario " << user << " no existe" << std::endl;
        return;
    }

    estado.usuarios.erase(it);

    std::cout << "[EXITOSO] Usuario '" << user << "' eliminado correctamente" << std::endl;
}
