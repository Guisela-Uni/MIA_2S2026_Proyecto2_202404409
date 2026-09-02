#include "mkfile.h"

#include <algorithm>
#include <cctype>
#include <iostream>
#include <regex>
#include <sstream>

namespace {

std::string to_lower_copy(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return s;
}

// Devuelve la ruta de la carpeta padre de un path
std::string carpeta_padre(const std::string &path) {
    size_t pos = path.find_last_of('/');
    if (pos == std::string::npos || pos == 0) {
        return "/";
    }
    return path.substr(0, pos);
}

// Genera todas las carpetas intermedias de una ruta (para -r):
std::vector<std::string> descomponer_carpetas(const std::string &carpeta) {
    std::vector<std::string> resultado;
    if (carpeta.empty() || carpeta == "/") {
        return resultado;
    }
    std::stringstream ss(carpeta);
    std::string parte;
    std::string acumulado;
    while (std::getline(ss, parte, '/')) {
        if (parte.empty()) continue;
        acumulado += "/" + parte;
        resultado.push_back(acumulado);
    }
    return resultado;
}

}  

void mkfile(std::vector<std::string> &parametros, EstadoGlobal &estado) {
    std::string path;
    std::string cont;
    bool tiene_r = false;
    bool tiene_size = false;
    long long size = 0;

    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        // El parametro -r no lleva "=valor", es un flag simple
        if (command_data.find('=') == std::string::npos) {
            if (to_lower_copy(command_data) == "r") {
                tiene_r = true;
                continue;
            }
            std::cout << "[ERROR] El parametro " << command_data << " no es valido para MKFILE" << std::endl;
            return;
        }

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() < 2) {
            std::cout << "[ERROR] El parametro " << command_data
                      << " no tiene la forma correcta. Ejemplo: -path=VALOR" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];
        std::string nombre_lower = to_lower_copy(nombre_parametro);

        if (nombre_lower == "path") {
            path = valor_parametro;
        } else if (nombre_lower == "size") {
            tiene_size = true;
            try {
                size = std::stoll(valor_parametro);
            } catch (...) {
                std::cout << "[ERROR] El parametro -size debe ser un valor numerico" << std::endl;
                return;
            }
        } else if (nombre_lower == "cont") {
            cont = valor_parametro;
        } else {
            std::cout << "[ERROR] El parametro " << nombre_parametro << " no es valido para MKFILE" << std::endl;
            return;
        }
    }

    //  VALIDACIONES
    if (path.empty()) {
        std::cout << "[ERROR] MKFILE requiere el parametro obligatorio -path" << std::endl;
        return;
    }

    if (tiene_size && size < 0) {
        std::cout << "[ERROR] El parametro -size no puede ser negativo" << std::endl;
        return;
    }
    if (!tiene_size) {
        size = 0; // por default
    }

    //  VALIDAR/CREAR CARPETAS (-r)
    std::string carpeta = carpeta_padre(path);
    if (carpeta != "/" && estado.carpetas.find(carpeta) == estado.carpetas.end()) {
        if (!tiene_r) {
            std::cout << "[ERROR] La carpeta " << carpeta
                      << " no existe. Use el parametro -r para crearla." << std::endl;
            return;
        }
        // crear todas las carpetas intermedias 
        for (const auto &c : descomponer_carpetas(carpeta)) {
            estado.carpetas.insert(c);
        }
    }

    // si ya existe pregunta si se sobreescribe
    auto it = estado.archivos.find(path);
    if (it != estado.archivos.end()) {
        std::cout << "[AVISO] El archivo " << path
                  << " ya existe. ¿Desea sobreescribirlo? (s/n): ";
        std::string respuesta;
        std::getline(std::cin, respuesta);
        respuesta = to_lower_copy(respuesta);
        if (respuesta != "s" && respuesta != "si") {
            std::cout << "[INFO] Operacion cancelada, no se sobreescribio el archivo." << std::endl;
            return;
        }
    }

    //  "CREAR" EL archivo (simulado, solo en memoria)
    SimFile archivo;
    archivo.size = static_cast<int>(size);
    archivo.origen = cont;
    estado.archivos[path] = archivo;

    std::cout << "[EXITOSO] Archivo simulado creado: " << path
              << " (" << size << " bytes"
              << (cont.empty() ? "" : (", contenido tomado de: " + cont))
              << ")" << std::endl;
}
