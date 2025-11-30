#include "../include/SuffixTree.h"
#include <list>

namespace aed::structure {

    using ST              = SuffixTree;
    using Node            = ST::Node;
    using MappedSubstring = ST::MappedSubstring;
    using Transition      = ST::Transition;
    using SinkNode        = ST::SinkNode;
    using Leaf            = ST::Leaf;
    using ReferencePoint  = ST::ReferencePoint;
    using Base            = ST::Base;
    using Index           = ST::Index;


    /**
     * MappedSubstring - Representa un substring referenciado
     *
     * En lugar de almacenar el substring completo, guardamos:
     * - ref_str: ID del string al que pertenece
     * - l: índice izquierdo (inicio del substring)
     * - r: índice derecho (fin del substring)
     *
     * Esto ahorra memoria al no duplicar caracteres.
     */

    MappedSubstring::MappedSubstring() : ref_str(0), l(0), r(0) {}

    MappedSubstring::MappedSubstring(int ref, Index left, Index right)
        : ref_str(ref), l(left), r(right) {}

    bool MappedSubstring::empty() const {
        return (l > r);
    }

    int MappedSubstring::lenght() const {
        return empty()? 0 : (r - l + 1);
    }



    /**
     * Transition - Representa una arista en el árbol
     *
     * Cada transición tiene:
     * - sub: el substring que etiqueta la arista
     * - tgt: puntero al nodo destino
     */

    Transition::Transition() : sub(), tgt(nullptr) {}

    Transition::Transition(MappedSubstring s, Node* t): sub(s), tgt(t) {}

    bool Transition::is_valid() const {
        return (tgt != nullptr);
    }



    /**
     * Node - Nodo del árbol con coloreo
     *
     * Contiene:
     * - g: mapa de transiciones (hashtable indexado por primer carácter)
     * - suffix_link: enlace de sufijo a otro nodo (usado por Ukkonen)
     * - ColorSet: Un bitset que indica que string de conjunto pasan por
     *   este nodo.
     */

    Node::Node() : suffix_link(nullptr) {
        colors.reset(); // Iniciar los bits en 0
    }

    Node::~Node() {}

    /**
     * Busca la transición que comienza con el carácter alpha
     * Retorna una transición inválida si no existe
     */
    Transition Node::find_alpha_transition(char alpha) {
        auto it = g.find(alpha);
        if (it == g.end())
            return Transition(MappedSubstring(0, 0, -1), nullptr);
        return it->second;
    }

    void Node::mark_string(int string_id) {
        if(string_id > 0 and string_id <= ST::MAX_STRINGS) {
            colors.set(string_id - 1);
        }
    }

    bool Node::has_single_string() const {
        return colors.count() == 1;
    }

    int Node::get_single_string_id() const {
        if(!has_single_string()) return -1;
        for(size_t i = 0; i < ST::MAX_STRINGS; ++i)
            if(colors.test(i))
                return i + 1;

        return -1;
    }

    void Node::merge_colors(const ST::ColorSet& other) {
        colors |= other;
    }


    /**
    * SinkNode - Nodo especial "sumidero"
    *
    * Este nodo auxiliar tiene una transición implícita para CUALQUIER carácter
    * que apunta de vuelta a la raíz. Se usa en el algoritmo de Ukkonen.
    */

    //    Especializacion de SinkNode
    Transition SinkNode::find_alpha_transition(char alpha) {
        return Transition(MappedSubstring(0, 0, 0), this->suffix_link);
    }


    /**
     * ReferencePoint - Punto de referencia en el árbol
     *
     * Representa una posición en el árbol que puede estar:
     * - En un nodo explícito (cuando substring está vacío)
     * - En medio de una arista (cuando substring no está vacío)
     *
     * Componentes:
     * - node: nodo desde donde parte
     * - ref_str: ID del string
     * - pos: posición actual en el string
     */

    ReferencePoint::ReferencePoint(Node* n, int ref, Index p) : node(n), ref_str(ref), pos(p) {}



    /**
     * Base - Estructura base del árbol
     *
     * Maneja la raíz, el nodo sumidero y la limpieza de memoria.
     * Los enlaces de sufijo iniciales son:
     * - root -> sink
     * - sink -> root
     */

    Base::Base() : sink(), root() {
        root.suffix_link = &sink;
        sink.suffix_link = &root;
    }

    Base::~Base() {
        clean();
    }

    /**
     * Limpia todos los nodos del árbol usando BFS
     * No elimina root ni sink (son miembros de la clase)
     */
    void Base::clean() {
        std::list<Node*> del_list{&root};
        while (!del_list.empty()) {
            Node* current = del_list.front();
            del_list.pop_front();

            for (auto& pair : current->g) {
                del_list.push_back(pair.second.tgt);
            }

            if (current != &root) {
                delete current;
            }
        }
    }



} // namespace aed::structure
