#include <iostream>
#include <sstream>

#include "analizador.h"
#include "estado.h"
#include "http_server.h"
#include "json_util.h"

namespace {
RespuestaHttp manejarComando(const PeticionHttp &peticion) {
    std::string comando = extraer_campo_json(peticion.cuerpo, "comando");

    if (comando.empty()) {
        RespuestaHttp respuesta;
        respuesta.codigo = 400;
        respuesta.cuerpo = R"({"error":"falta el campo 'comando' en el JSON"})";
        return respuesta;
    }

    // Aca se conecta con TODO el backend que ya existia: el mismo lexer/analizador que usa la consola.
    std::string resultado = ejecutar_comando_texto(comando);

    RespuestaHttp respuesta;
    respuesta.codigo = 200;
    respuesta.cuerpo = "{\"resultado\":\"" + escapar_json(resultado) + "\"}";
    return respuesta;
}


RespuestaHttp manejarProcesarArchivo(const PeticionHttp &peticion) {
    std::string contenido = extraer_campo_json(peticion.cuerpo, "contenido");

    if (contenido.empty()) {
        RespuestaHttp respuesta;
        respuesta.codigo = 400;
        respuesta.cuerpo = R"({"error":"falta el campo 'contenido' en el JSON"})";
        return respuesta;
    }

    std::string reporte = procesar_archivo_comandos(contenido);

    RespuestaHttp respuesta;
    respuesta.codigo = 200;
    respuesta.cuerpo = "{\"reporte\":\"" + escapar_json(reporte) + "\"}";
    return respuesta;
}

RespuestaHttp manejarEstado(const PeticionHttp & /*peticion*/) {
    EstadoGlobal &estado = obtener_estado();
    std::ostringstream json;

    json << "{";

    //  discos simulados (creados con MKDISK, aunque no esten montados)
    json << "\"discos\":[";
    bool primerDisco = true;
    for (const auto &[path, disco] : estado.discos) {
        if (!primerDisco) json << ",";
        primerDisco = false;
        json << "{\"path\":\"" << escapar_json(path) << "\","
             << "\"tamano\":" << disco.tamano << ","
             << "\"fit\":\"" << disco.fit << "\"}";
    }
    json << "],";

    //  particiones montadas (con su id, generado por MOUNT)
    json << "\"montadas\":[";
    bool primeraMontada = true;
    for (const auto &d : estado.discosMontados) {
        for (const auto &p : d.particiones) {
            if (!primeraMontada) json << ",";
            primeraMontada = false;
            json << "{\"id\":\"" << escapar_json(p.id) << "\","
                 << "\"disco\":\"" << escapar_json(d.nombre) << "\","
                 << "\"particion\":\"" << escapar_json(p.nombre) << "\","
                 << "\"size\":" << p.size << ","
                 << "\"formateada\":" << (estado.particionesFormateadas.count(p.id) ? "true" : "false")
                 << "}";
        }
    }
    json << "],";

    //  usuarios simulados (MKUSR/RMUSR)
    json << "\"usuarios\":[";
    bool primerUsuario = true;
    for (const auto &u : estado.usuarios) {
        if (!primerUsuario) json << ",";
        primerUsuario = false;
        json << "{\"user\":\"" << escapar_json(u.user) << "\","
             << "\"grupo\":\"" << escapar_json(u.grupo) << "\"}";
    }
    json << "],";

    //  archivos simulados (MKFILE)
    json << "\"archivos\":[";
    bool primerArchivo = true;
    for (const auto &[path, archivo] : estado.archivos) {
        if (!primerArchivo) json << ",";
        primerArchivo = false;
        json << "{\"path\":\"" << escapar_json(path) << "\","
             << "\"size\":" << archivo.size << "}";
    }
    json << "]";

    json << "}";

    RespuestaHttp respuesta;
    respuesta.codigo = 200;
    respuesta.cuerpo = json.str();
    return respuesta;
}

}  // namespace

int main() {
    const int PUERTO = 8080;

    ServidorHttp servidor;
    servidor.agregarRuta("POST", "/api/comando", manejarComando);
    servidor.agregarRuta("POST", "/api/procesar", manejarProcesarArchivo);
    servidor.agregarRuta("GET", "/api/estado", manejarEstado);

    std::cout << "======================================================\n";
    std::cout << " Analizador EXT2 - API REST (backend para el frontend)\n";
    std::cout << "======================================================\n";
    std::cout << "POST http://localhost:" << PUERTO << "/api/comando   {\"comando\": \"...\"}\n";
    std::cout << "POST http://localhost:" << PUERTO << "/api/procesar  {\"contenido\": \"varias\\nlineas\"}\n";
    std::cout << "GET  http://localhost:" << PUERTO << "/api/estado\n\n";

    servidor.escuchar(PUERTO);  // se queda corriendo para siempre
    return 0;
}
