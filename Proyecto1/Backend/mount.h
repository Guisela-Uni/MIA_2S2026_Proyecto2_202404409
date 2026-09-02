#ifndef MOUNT
#define MOUNT

//Locales
#include "structs.h"
#include "estado.h"

//Librerias
#include <iostream>
#include <string>
#include <vector>

// Monta una particion simulada y le asigna un id interno (en memoria).
void mount(std::vector<std::string> &parametros, EstadoGlobal &estado);

#endif
