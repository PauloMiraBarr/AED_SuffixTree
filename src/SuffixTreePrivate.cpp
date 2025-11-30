#include "../include/SuffixTree.h"
#include <vector>
#include <limits>
#include <algorithm>
#include <iostream>

namespace aed::structure {

    using Node            = SuffixTree::Node;
    using MappedSubstring = SuffixTree::MappedSubstring;
    using Transition      = SuffixTree::Transition;
    using SinkNode        = SuffixTree::SinkNode;
    using Leaf            = SuffixTree::Leaf;
    using ReferencePoint  = SuffixTree::ReferencePoint;
    using Base            = SuffixTree::Base;
    using Index           = SuffixTree::Index;
    using ColorSet        = SuffixTree::ColorSet;

    // =====================================================
    //          IMPLEMENTACION PRINCIPAL PRIVATE
    // =====================================================

    /**
     * Convierte un substring a string legible (para debugging)
     */
    std::string SuffixTree::substring_to_string(const MappedSubstring& substr) const {
        auto it = haystack.find(substr.ref_str);
        if(it == haystack.end() or substr.empty()) {
            return "";
        }

        const std::string& str = it->second;
        std::string result;

        Index end = std::min(substr.r, static_cast<Index>(str.size() - 1));
        for (Index i = substr.l; i <= end; ++i) {
            result.push_back(str[i]);
        }

        return result;
    }


    /**
     * test_and_split - Verifica y divide aristas
     *
     * Dado un nodo n, un substring kp y un carácter t:
     * 1. Verifica si (n, kp) es un punto final (endpoint)
     * 2. Si estamos en un estado implícito (mitad de arista), crea un nodo nuevo
     *
     * @param n: nodo actual
     * @param kp: substring que representa la posición
     * @param t: carácter a verificar
     * @param w: string de referencia
     * @param r: [out] nodo resultante
     * @return true si es endpoint, false si se tuvo que dividir
     */
    bool SuffixTree::test_and_split(Node* n, MappedSubstring kp, char t, const std::string& w, Node** r) {
        Index delta = kp.r - kp.l;

        if(delta < 0) {
            *r = n;
            Transition t_trans = n->find_alpha_transition(t);

            if (t_trans.is_valid() and t_trans.tgt != nullptr && kp.ref_str > 0) {
                t_trans.tgt->mark_string(kp.ref_str);
            }

            return t_trans.is_valid();
        }

        char tk = w[kp.l];
        Transition tk_trans = n->find_alpha_transition(tk);
        MappedSubstring kp_prime = tk_trans.sub;

        auto str_it = haystack.find(kp_prime.ref_str);
        const std::string& str_prime = str_it->second;

        if (str_prime[kp_prime.l + delta + 1] == t) {
            *r = n;
            return true; // es endpoint
        }

        // Crear nuevo nodo intermedio
        *r = new Node();

        Transition new_trans = tk_trans;
        new_trans.sub.l += delta + 1;
        (*r)->g.insert({str_prime[new_trans.sub.l], new_trans});

        tk_trans.sub.r = tk_trans.sub.l + delta;
        tk_trans.tgt = *r;
        n->g[tk] = tk_trans;

        colors_computed = false;

        return false;
    }

    /**
     * update - Actualiza el árbol con un nuevo carácter
     *
     * Este es el corazón de cada iteración del algoritmo de Ukkonen.
     * Recorre el "border path" desde el punto activo hasta el endpoint
     * y agrega las transiciones necesarias para el i-ésimo carácter.
     *
     * @param n: nodo activo
     * @param ki: substring desde el punto activo hasta el carácter actual
     * @return punto de referencia actualizado (nuevo punto activo)
     */
    ReferencePoint SuffixTree::update(Node* n, MappedSubstring ki) {
        Node* oldr = &tree.root;
        Node* r = nullptr;
        bool is_endpoint = false;

        const std::string& w = haystack[ki.ref_str];
        MappedSubstring ki1 = ki;
        ki1.r = ki.r - 1; // Excluir el último carácter

        ReferencePoint sk(n, ki.ref_str, ki.l);

        // Probar y dividir en el punto actual
        is_endpoint = test_and_split(n, ki1, w[ki.r], w, &r);

        while (!is_endpoint) {
            // Crear nueva hoja
            Leaf* r_prime = new Leaf();

            // Agregar transición al nodo actual
            // El substring va desde ki.r hasta el infinito (representado con max)
            r->g.insert({w[ki.r],
                        Transition(MappedSubstring(ki.ref_str, ki.r,
                                  std::numeric_limits<Index>::max()),
                                  r_prime)});

            // Actualizar suffix links
            if (oldr != &tree.root) {
                oldr->suffix_link = r;
            }
            oldr = r;

            // Seguir el suffix link
            sk = canonize(sk.node->suffix_link, ki1);
            ki1.l = ki.l = sk.pos;

            // Probar y dividir en el nuevo punto
            is_endpoint = test_and_split(sk.node, ki1, w[ki.r], w, &r);
        }

        // Actualizar último suffix link
        if (oldr != &tree.root) {
            oldr->suffix_link = sk.node;
        }

        return sk;
    }


    /**
     * canonize - Obtiene el par canónico
     *
     * Dada una referencia (nodo, substring), retorna la representación
     * canónica de esa posición en el árbol.
     *
     * Si el substring es más largo que la primera arista, avanza por el árbol
     * hasta encontrar la posición correcta.
     *
     * @param n: nodo inicial
     * @param kp: substring a canonizar
     * @return punto de referencia canónico
     */
    ReferencePoint SuffixTree::canonize(Node* n, MappedSubstring kp) {
        if (kp.r < kp.l) {
            // Substring vacío, ya es canónico
            return ReferencePoint(n, kp.ref_str, kp.l);
        }

        const std::string& str = haystack[kp.ref_str];

        Transition tk_trans = n->find_alpha_transition(str[kp.l]);
        Index delta;

        // Mientras el substring sea más largo que la arista actual
        while ((delta = tk_trans.sub.r - tk_trans.sub.l) <= kp.r - kp.l) {
            kp.l += 1 + delta;
            n = tk_trans.tgt;

            if (kp.l <= kp.r) {
                tk_trans = n->find_alpha_transition(str[kp.l]);
            }
        }

        return ReferencePoint(n, kp.ref_str, kp.l);
    }

    /**
     * get_starting_node - Encuentra el nodo de inicio
     *
     * Camina por el árbol siguiendo los caracteres de s hasta que:
     * 1. s se agota completamente (coincidencia perfecta)
     * 2. s no coincide con el árbol (punto de divergencia)
     *
     * @param s: string a buscar
     * @param r: [in/out] punto de referencia (inicio/fin del recorrido)
     * @return índice donde diverge, o max si coincide completamente
     */
    Index SuffixTree::get_starting_node(const std::string &s, ReferencePoint* r) {
        Index k = r->pos;
        Index s_len = s.size();
        bool s_runout = false;

        while (!s_runout) {
            Node* r_node = r->node;

            if (k >= s_len) {
                s_runout = true;
                break;
            }

            // Buscar transición para el carácter actual
            Transition t = r_node->find_alpha_transition(s[k]);

            if (t.tgt != nullptr) {
                // Hay transición, verificar coincidencia carácter por carácter
                const std::string& ref_str = haystack[t.sub.ref_str];
                Index i;

                for (i = 1; i <= t.sub.r - t.sub.l; ++i) {
                    if (k + i >= s_len) {
                        s_runout = true;
                        break;
                    }

                    if (s[k + i] != ref_str[t.sub.l + i]) {
                        // Divergencia encontrada
                        r->pos = k;
                        return k + i;
                    }
                }

                if (!s_runout) {
                    // Toda la arista coincide, avanzar al siguiente nodo
                    r->node = t.tgt;
                    k += i;
                    r->pos = k;
                }
            } else {
                // No hay transición, este es el punto de divergencia
                return k;
            }
        }

        // String completamente encontrado
        r->pos = std::numeric_limits<Index>::max();
        return std::numeric_limits<Index>::max();
    }


    /**
     * deploy_suffixes - Despliega sufijos usando Ukkonen
     *
     * Implementa el algoritmo de Ukkonen para insertar todos los sufijos
     * de s en el árbol de manera incremental.
     *
     * @param s: string a insertar
     * @param sindex: ID del string
     * @return ID del string si tuvo éxito, -1 si falló
     */
    int SuffixTree::deploy_suffixes(const std::string& s, int sindex) {
        ReferencePoint active_point(&tree.root, sindex, 0);

        Index i = get_starting_node(s, &active_point);

        if (i == std::numeric_limits<Index>::max()) {
            return -1;
        }

        for (; i < s.size(); ++i) {
            MappedSubstring ki(sindex, active_point.pos, i);
            active_point = update(active_point.node, ki);
            ki.l = active_point.pos;
            active_point = canonize(active_point.node, ki);
        }

        return sindex;
    }


    /**
     * compute_colors_dfs - Calcula colores de nodos usando DFS
     *
     * Realiza un recorrido post-order del árbol para propagar los colores
     * desde las hojas hacia la raíz. Cada nodo acumula los colores de todos
     * sus descendientes.
     *
     * @param node: nodo actual
     * @return ColorSet con los colores acumulados del subárbol
     */
    ColorSet SuffixTree::compute_colors_dfs(Node* node) {
        if (!node) {
            return ColorSet();
        }

        // Si es una hoja, determinar a qué string pertenece
        if (node->g.empty()) {
            // Las hojas se crean durante deploy_suffixes
            // Necesitamos rastrear qué string creó cada hoja
            return node->colors;
        }

        // Nodo interno: acumular colores de todos los hijos
        ColorSet accumulated;
        for (auto& pair : node->g) {
            Transition& trans = pair.second;

            // Marcar el color del string al que pertenece esta arista
            if (trans.sub.ref_str > 0 && trans.sub.ref_str <= MAX_STRINGS) {
                accumulated.set(trans.sub.ref_str - 1);
            }

            // Recursión: obtener colores del subárbol hijo
            ColorSet child_colors = compute_colors_dfs(trans.tgt);
            accumulated |= child_colors;
        }

        // Actualizar colores de este nodo
        node->colors = accumulated;
        return accumulated;
    }

    /**
     * mark_leaves_for_string - Marca las hojas del string dado
     *
     * Recorre el árbol siguiendo los sufijos del string y marca todas
     * las hojas correspondientes con el ID del string.
     */
    void SuffixTree::mark_leaves_for_string(int string_id) {
        const std::string& str = haystack[string_id];

        // Para cada sufijo del string
        for (Index start = 0; start < str.size(); ++start) {
            Node* current = &tree.root;
            Index pos = start;

            // Seguir el camino del sufijo
            while (pos < str.size()) {
                Transition trans = current->find_alpha_transition(str[pos]);

                if (!trans.is_valid()) {
                    break;
                }

                // Calcular cuántos caracteres coinciden en esta arista
                const std::string& edge_str = haystack[trans.sub.ref_str];
                Index edge_len = trans.sub.r - trans.sub.l + 1;
                Index match_len = 0;

                for (Index i = 0; i < edge_len && pos + i < str.size(); ++i) {
                    if (str[pos + i] != edge_str[trans.sub.l + i]) {
                        break;
                    }
                    match_len++;
                }

                pos += match_len;
                current = trans.tgt;

                // Si llegamos a una hoja, marcarla
                if (current->g.empty()) {
                    current->mark_string(string_id);
                    break;
                }
            }
        }
    }


    bool SuffixTree::contains_end_token(const std::string& str) const {
        return str.find(END_TOKEN) != std::string::npos;
    }

    void SuffixTree::get_all_strings_dfs(Node* node, std::string& current_path, std::unordered_map<ColorSet, std::vector<std::string>>& result) {
        if (node == nullptr) {
            return;
        }

        // Verificar los colores del nodo actual
        ColorSet node_colors = node->colors;
        int color_count = node_colors.count();

        // Si el nodo tiene colores, procesarlo
        if (color_count > 0) {
            std::string to_store;
            
            if (color_count == 1) {
                // Solo un color: guardar solo el primer carácter del path completo
                if (!current_path.empty()) {
                    to_store = std::string(1, current_path[0]);
                } else {
                    to_store = "";  // Nodo raíz sin transición
                }
            } else {
                // Múltiples colores: guardar el string completo
                to_store = current_path;
                // Eliminar END_TOKEN si está presente
                if (!to_store.empty() && to_store.back() == END_TOKEN) {
                    to_store.pop_back();
                }
            }
            
            // Guardar en el resultado
            if (!to_store.empty() || color_count > 1) {
                result[node_colors].push_back(to_store);
            }
        }

        // Recorrer todas las transiciones (hijos)
        for (auto& pair : node->g) {
            Transition& trans = pair.second;
            if (trans.tgt != nullptr) {
                // Obtener el string de la transición
                std::string edge_str = substring_to_string(trans.sub);
                
                // Agregar al path actual
                current_path += edge_str;
                
                // Recursión
                get_all_strings_dfs(trans.tgt, current_path, result);
                
                // Backtrack: remover del path
                current_path.erase(current_path.length() - edge_str.length());
            }
        }
    }








} // namespace aed::structure