CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

COMUNES = analizador.cpp structs.cpp validaciones.cpp mkdisk.cpp rmdisk.cpp fdisk.cpp mount.cpp mkfs.cpp mkusr.cpp rmusr.cpp mkfile.cpp

.PHONY: all consola api run-api run-consola clean

all: consola api

# consola  (main.cpp) 
consola: analizador_ext2

analizador_ext2: main.cpp $(COMUNES)
	$(CXX) $(CXXFLAGS) main.cpp $(COMUNES) -o analizador_ext2

#  Servidor de la API para el frontend 
api: api_server

api_server: api_server_main.cpp http_server.cpp json_util.cpp $(COMUNES)
	$(CXX) $(CXXFLAGS) api_server_main.cpp http_server.cpp json_util.cpp $(COMUNES) -o api_server

run-consola: analizador_ext2
	./analizador_ext2

run-api: api_server
	./api_server

clean:
	rm -f analizador_ext2 api_server
