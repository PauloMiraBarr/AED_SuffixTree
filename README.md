# Suffix Tree & Generalized Suffix Tree (GST) – Hito 2 (Curso AED)

Implementación en **C++20** de un **Suffix Tree** y su extensión a **Generalized Suffix Tree (GST)**, con un visualizador gráfico desarrollado usando **SFML 2.6**.  
El propósito del proyecto es experimentar y visualizar la construcción de GSTs y ofrecer utilidades para comprobar sufijos y substrings, aplicable a problemas como identificación de patrones en genomas.

---

## Especificaciones técnicas
- Lenguaje: **C++20**
- Motor gráfico: **SFML 2.6**
- Build: **CMake** (CMakeLists.txt)
- Fichero terminador por defecto: `END_TOKEN = '$'` (editable)
- Capacidad por defecto del bitset de coloreo: `MAX_STRINGS = 64` (editable)
- Coloreo por bitset: cada nodo guarda un `std::bitset<MAX_STRINGS>` para identificar qué cadenas pasan por ese nodo.

---

## Funcionalidades principales de la librería
- Implementación basada en **Ukkonen's Algorithm**.
- Soporte para **múltiples cadenas** (GST).
- API pública (ejemplos):
  - `add_string(std::string s)` — agrega la cadena `s` al GST.
  - `bool is_suffix(std::string s)` — devuelve `true` si `s` es sufijo en el GST.
  - `bool is_substring(std::string s)` — devuelve `true` si `s` aparece como substring en el GST.
- Carácter terminador por cadena: por defecto `$` (variable `END_TOKEN`), se debe asegurar que ninguna cadena de entrada contenga este token.

---

## Estructura del proyecto

```
CMakeLists.txt
main.cpp

include/
├─ SuffixTree.h
└─ Visualizer.h

src/
├─ InternalEstructusImplementation.cpp   # estructuras internas del ST/GST
├─ SuffixTreePrivate.cpp                 # métodos privados e implementación interna
└─ SuffixTreePublic.cpp                  # métodos y API pública
```

**Razonamiento de diseño:** la implementación está separada en tres capas para mantener claridad: estructuras internas (nodos, aristas, estados), implementación privada (funciones auxiliares y mantenimiento) y métodos públicos (API para el usuario). Esto facilita mantenimiento y pruebas unitarias.

---

## Visualizador (SFML 2.6) — Manual de uso

El visualizador está pensado para correr desde `main.cpp` y permite observar la construcción del GST de forma interactiva.

### Definición de las cadenas
- En `main.cpp` se define la lista de cadenas que se quieren agregar al GST.
- Puede ser **una sola cadena** o **varias**. El programa las tomará en el orden definido y empezará la construcción del GST.

Ejemplo mínimo en `main.cpp`:
```cpp
std::vector<std::string> inputs = {
    "ACGT",
    "GGT",
    "TACG"
};
```
> Nota: si usas `END_TOKEN` distinto de `$`, asegúrate de añadirlo manualmente o de que `add_string` lo haga.

### Interacción en tiempo de ejecución
Mientras el visualizador está abierto, puedes controlar la construcción y pruebas de la siguiente forma:

- **Intro / Enter**: al presionar Enter, el visualizador avanza **paso a paso** en la construcción del GST. Cada pulsación ejecuta el siguiente paso significativo del algoritmo (por ejemplo, procesar el siguiente carácter, crear nodos, romper aristas, etc.), actualizando la vista gráfica para reflejar el estado actual del árbol.
- **'t' (tecla t)**: abre un **prompt** dentro de la ventana (o en la consola si así está implementado) para introducir una cadena de prueba. Al ingresar la cadena, el visualizador preguntará si deseas comprobar:
  - Si es **sufijo** (`is_suffix`)
  - Si es **substring** (`is_substring`)
  Según la opción, mostrará el resultado (true/false) y, si aplica, resaltará en el árbol el camino correspondiente usando el coloreo por bitset o un resaltado temporal.

### Qué se muestra
- Nodos y aristas en una vista jerárquica legible.
- Información por nodo: índice, intervalos de borde (start, end), y bitset de coloreo (visualizado como pequeñas marcas o leyenda).
- Resaltado temporal del camino evaluado para `is_suffix` / `is_substring`.

---

## Variables editables importantes
- `END_TOKEN` — carácter que identifica el final de una cadena en el GST (por defecto `$`). Cambiarlo requiere asegurar que no exista dentro de las cadenas de entrada.
- `MAX_STRINGS` — tamaño del `bitset` que define cuántas cadenas pueden ser identificadas por nodo; por defecto **64**. Si necesitas más, cambia `MAX_STRINGS` y recompila; recuerda el coste en memoria.

---

## Ejemplo de flujo de trabajo
1. Editar `main.cpp` para definir las cadenas que deseas analizar.
2. (Opcional) Ajustar `END_TOKEN` y `MAX_STRINGS` en los headers/constantes.
3. Compilar:
```bash
mkdir build
cd build
cmake ..
make
```
4. Ejecutar el visualizador:
```bash
./gst_visualizer
```
5. Interactuar:
   - Presiona **Enter** repetidamente para ver la construcción paso a paso.
   - Presiona **t** y escribe la cadena para comprobar si es sufijo o substring.

---

## Recursos y lectura recomendada

- Ukkonen's Algorithm — video explicativo: https://www.youtube.com/watch?v=aPRqocoBsFQ  
- Animación del algoritmo: https://brenden.github.io/ukkonen-animation/  
- Generalized suffix tree — matching de dos cadenas (video): https://www.youtube.com/watch?v=Y9dXDcjxkB4&t=1480s  
- Implementaciones de referencia: https://github.com/Rerito/suffix-tree/tree/master

---

## Contenido del repositorio
- Código fuente (include/ + src/)
- `main.cpp` con ejemplo de definición de cadenas y control del visualizador.
- `CMakeLists.txt`
- PDF de la investigación
- README
- Licencia

---

## Licencia
Este proyecto está bajo la licencia MIT. Consulta el archivo LICENSE para más detalles.

---
