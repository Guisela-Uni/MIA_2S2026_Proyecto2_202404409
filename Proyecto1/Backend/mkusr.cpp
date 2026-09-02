#include "mkusr.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>

void mkusr(std::vector<std::string> &parametros, EstadoGlobal &estado) {
    std::string user, pass, grp;

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
        std::string nombre_lower = nombre_parametro;
        to_lower_str(nombre_lower);

        if (nombre_lower == "user") {
            user = valor_parametro;
        } else if (nombre_lower == "pass") {
            pass = valor_parametro;
        } else if (nombre_lower == "grp") {
            grp = valor_parametro;
        } else {
            std::cout << "[ERROR] El parametro " << nombre_parametro << " no es valido para MKUSR" << std::endl;
            return;
        }
    }

    //  VALIDACIONES
    if (user.empty() || pass.empty() || grp.empty()) {
        std::cout << "[ERROR] MKUSR requiere los parametros obligatorios: -user, -pass y -grp" << std::endl;
        return;
    }

    // -max 10 caracteres cada uno
    if (user.size() > 10) {
        std::cout << "[ERROR] El nombre de usuario no puede tener mas de 10 caracteres" << std::endl;
        return;
    }
    if (pass.size() > 10) {
        std::cout << "[ERROR] La contraseña no puede tener mas de 10 caracteres" << std::endl;
        return;
    }
    if (grp.size() > 10) {
        std::cout << "[ERROR] El nombre del grupo no puede tener mas de 10 caracteres" << std::endl;
        return;
    }

    // no pueden haber 2 usuarios con el mismo nombre
    for (const auto &u : estado.usuarios) {
        if (u.user == user) {
            std::cout << "[ERROR] Ya existe un usuario con el nombre: " << user << std::endl;
            return;
        }
    }

    usuario nuevo;
    nuevo.user  = user;
    nuevo.pass  = pass;
    nuevo.grupo = grp;

    estado.usuarios.push_back(nuevo);

    std::cout << "[EXITOSO] Usuario '" << user << "' creado correctamente en el grupo '" << grp << "'" << std::endl;
}
