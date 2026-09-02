#include "mount.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <regex>

void mount(std::vector<std::string> &parametros, EstadoGlobal &estado) {
    // parametros path y nombre
    std::string disk_path;
    std::string nombre;

    // leer los parametros
    for (size_t i = 1; i < parametros.size(); ++i) {
        std::string &command_data = parametros[i];

        std::vector<std::string> output_data(
            std::sregex_token_iterator(command_data.begin(), command_data.end(), igual, -1),
            std::sregex_token_iterator()
        );

        if (output_data.size() != 2) {
            std::cout << "[ERROR] Parametro invalido: " << command_data
                      << " (debe ser clave=valor, ej: -path=/disco.dk -name=part1)" << std::endl;
            return;
        }

        std::string nombre_parametro = output_data[0];
        std::string valor_parametro  = output_data[1];

        //pasar a minusculas
        std::transform(nombre_parametro.begin(), nombre_parametro.end(), nombre_parametro.begin(),
                       [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

        if (nombre_parametro == "path") {
            disk_path = valor_parametro;
        } else if (nombre_parametro == "name") {
            nombre = valor_parametro;
        } else {
            std::cout << "[ERROR] Parametro NO VALIDO - solo se aceptan path y name" << std::endl;
            return;
        }
    }

    // VALIDACIONES
    if (disk_path.empty() || nombre.empty()) {
        std::cout << "[ERROR] mount requiere los parametros path y name." << std::endl;
        return;
    }

    //  Buscar disco simulado (debe existir, creado con MKDISK)
    auto it_disco = estado.discos.find(disk_path);
    if (it_disco == estado.discos.end()) {
        std::cout << "[ERROR] EL DISCO NO EXISTE - revisar path (debe crearse antes con MKDISK)" << std::endl;
        return;
    }
    SimDisk &disco = it_disco->second;

    // valores default
    int posMBR    = -1;   // posicion en el MBR simulado (INDICE 0-3)
    int posLogica = -1;   // offset virtual del EBR logica
    int size      = 0;    // tamaño de la particion
    bool mount_ext = false;

    // busca la particion
    for (int i = 0; i < 4; ++i) {
        particion &p = disco.particiones[i];
        // si el nombre coincide
        if (p.part_name == nombre) {
            if (p.part_type != 'e') {
                // si no es extendida, establecer los datos
                posMBR = i;
                size   = p.part_s;
                break;
            } else {
                // no se pueden montar particiones extendidas directamente
                mount_ext = true;
                break;
            }
        }
    }

    if (mount_ext) {
        std::cout << "[ERROR] NO SE PUEDE MONTAR UNA PARTICION EXTENDIDA DIRECTAMENTE" << std::endl;
        return;
    }

    if (posMBR == -1) {
        // buscar la extendida en donde estan las logicas
        int inicioExt = -1;
        for (int i = 0; i < 4; ++i) {
            if (disco.particiones[i].part_type == 'e') {
                inicioExt = disco.particiones[i].part_start;
                break;
            }
        }

        if (inicioExt == -1) {
            std::cout << "[ERROR] LA PARTICION NO EXISTE" << std::endl;
            return;
        }

        int posEBR = inicioExt;
        auto it_ebr = disco.ebrs.find(posEBR);
        if (it_ebr == disco.ebrs.end()) {
            std::cout << "[ERROR] LA PARTICION NO EXISTE" << std::endl;
            return;
        }
        EBR ebr = it_ebr->second;

        bool l_particion = false;
        while (true) {
            if (std::string(ebr.part_name) == nombre) {
                l_particion = true;
                posLogica  = posEBR;      // ebr de la logica
                size       = ebr.part_s;  // size de la logica
                break;
            }
            // ya se busco en la lista de ebr y no hay mas
            if (ebr.part_next == -1) {
                break;
            }
            // siguiente ebr
            posEBR = ebr.part_next;
            auto it_next = disco.ebrs.find(posEBR);
            if (it_next == disco.ebrs.end()) {
                break;
            }
            ebr = it_next->second;
        }

        if (!l_particion) {
            std::cout << "[ERROR] LA PARTICION NO EXISTE" << std::endl;
            return;
        }
    }


    std::string discName = std::filesystem::path(disk_path).stem().string();
    if (discName.empty()) {
        discName = disk_path;
    }

    ::disco *diskEntry = nullptr;
    for (auto &d : estado.discosMontados) {
        if (d.nombre == discName) {
            diskEntry = &d;
            break;
        }
    }

    // si no esta se crea
    if (!diskEntry) {
        estado.discosMontados.emplace_back();
        ::disco &d = estado.discosMontados.back();
        d.nombre   = discName;
        d.ruta     = disk_path;
        d.contador = 1;
        diskEntry  = &d;
    }

    // verificar que la particion no este ya montada
    for (const auto &p : diskEntry->particiones) {
        if (p.nombre == nombre) {
            std::cout << "[ERROR] LA PARTICION " << nombre << " YA ESTA MONTADA" << std::endl;
            return;
        }
    }

    // Crea el MOUNT (id simulado)
    montada nueva;
    nueva.id     = "907" + std::to_string(diskEntry->contador) + discName;
    diskEntry->contador++;
    // si no es logica la posicion sera -1
    nueva.posEBR = posLogica;
    nueva.posMBR = posMBR;
    nueva.nombre = nombre;
    nueva.size   = size;

    diskEntry->particiones.push_back(nueva);

    if (posLogica == -1) {
        disco.particiones[posMBR].part_status = '1';
    } else {
        EBR &ebr = disco.ebrs[posLogica];
        ebr.part_status = '1';
    }

    std::cout << "[EXITOSO] PARTICION MONTADA (simulada)" << std::endl;
    std::cout << ">>>>>>>>>>> ID: " << nueva.id << std::endl;

    std::cout << "\nLISTADO DE PARTICIONES MONTADAS\n\n";

    for (const auto &d : estado.discosMontados) {
        if (d.particiones.empty()) {
            continue;
        }

        std::cout << d.nombre << ":\n";
        for (const auto &p : d.particiones) {
            std::cout << "- " << p.nombre << " (id=" << p.id << ")" << std::endl;
        }
    }
}
