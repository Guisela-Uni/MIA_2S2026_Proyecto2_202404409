import { useState } from 'react'
import { procesarArchivo } from './api.js'

const EJEMPLO = `# ARCHIVO DE EJEMPLO

#----------------- MKDISK
# CASO 01 - valido
mkdisk -size=20 -unit=M -fit=FF -path=/home/calificacion/Disco1.mia

# CASO 02 - valido (mayusculas, valor con espacios entre comillas)
MKDISK -PATH="/home/misdiscos/Disco Dos.mia" -FIT=BF -SIZE=3000 -UNIT=K

# CASO 03 - ERROR: nombre alterado (lexico)
mkd1sk -size=10 -path=/home/calificacion/error1.mia

# CASO 04 - ERROR: caracter invalido en el parametro (lexico)
mkdisk -s!ze=10 -path=/home/calificacion/error2.mia
`

function App() {
  const [comandos, setComandos] = useState('')
  const [resultados, setResultados] = useState('')
  const [procesando, setProcesando] = useState(false)

  async function manejarProcesar() {
    if (!comandos.trim() || procesando) return

    setProcesando(true)
    setResultados('Procesando...')

    try {
      const reporte = await procesarArchivo(comandos)
      setResultados(reporte)
    } catch (error) {
      setResultados(
        `[ERROR DE CONEXION] ${error.message}\n\n¿Esta corriendo el servidor de la API? (api_server, puerto 8080)`
      )
    } finally {
      setProcesando(false)
    }
  }

  function cargarEjemplo() {
    setComandos(EJEMPLO)
  }

  return (
    <div className="app">
      <header className="encabezado">
        <h1>
          Analizador de comandos
        </h1>
        <p className="subtitulo">
          Analisis lexico y sintactico de comandos EXT2 
        </p>
      </header>

      <main className="contenido">
        {/* ---------- Comandos ---------- */}
        <section className="columna">
          <div className="columna-titulo">
            <h2>Comandos</h2>
            <button className="boton-secundario" onClick={cargarEjemplo} type="button">
              Cargar ejemplo
            </button>
          </div>
          <textarea
            className="textarea-comandos"
            value={comandos}
            onChange={(evento) => setComandos(evento.target.value)}
            placeholder={'# escribe uno o varios comandos, uno por linea\nmkdisk -size=5 -unit=M -path=/home/user/Disco1.mia'}
            spellCheck={false}
          />
          <button className="boton-procesar" onClick={manejarProcesar} disabled={procesando}>
            {procesando ? 'Procesando...' : 'Procesar'}
          </button>
        </section>

        {/* ---------- Resultados ---------- */}
        <section className="columna">
          <div className="columna-titulo">
            <h2>Resultados</h2>
          </div>
          <textarea
            className="textarea-resultados"
            value={resultados}
            readOnly
            placeholder="Resultados."
            spellCheck={false}
          />
        </section>
      </main>
    </div>
  )
}

export default App
