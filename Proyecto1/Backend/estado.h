#ifndef ESTADO_H
#define ESTADO_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "structs.h"

struct SimDisk {
    std::string path;                 // path "declarado" por el usuario (clave del registro)
    int tamano = 0;                   // tamaño total simulado, en bytes
    char fit = 'f';                   // fit por defecto del disco (f/b/w)
    time_t creado = 0;
    int signature = 0;

    particion particiones[4];         // "MBR" simulado: hasta 4 particiones primarias/extendida

    // Particiones logicas de una extendida: offset virtual -> EBR simulado.
    std::map<int, EBR> ebrs;
};

// Representa un archivo simulado creado con MKFILE
struct SimFile {
    int size = 0;              // tamaño en bytes simulado
    std::string origen;        // valor de -cont, si se proporciono
};

// Estado global de toda la simulacion, compartido entre todos los comandos.
struct EstadoGlobal {
    std::map<std::string, SimDisk> discos;                  // path -> disco simulado (MKDISK/RMDISK/FDISK)
    std::vector<disco> discosMontados;                       // discos con particiones montadas (MOUNT), con sus ids
    std::map<std::string, bool> particionesFormateadas;      // id de particion montada -> fue formateada con MKFS

    std::vector<usuario> usuarios;

    // Sistema de archivos simulado (para MKFILE): que archivos y que carpetas "existen" dentro de las particiones simuladas.
    std::map<std::string, SimFile> archivos;   // path de archivo -> datos simulados
    std::set<std::string> carpetas;            // paths de carpetas que "existen" en la simulacion
};

#endif
