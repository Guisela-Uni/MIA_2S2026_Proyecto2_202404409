#ifndef RMDISK
#define RMDISK

//Locales
#include "structs.h"
#include "estado.h"
#include <string>
#include <vector>

// recibe parametros:"rmdisk" y solo elimina la entrada del registro simulado en memoria,

void rmdisk(std::vector<std::string> &parametros, EstadoGlobal &estado);

#endif
