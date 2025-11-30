#include "SuffixTree.h"
#include <vector>
#include <limits>
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
    //          IMPLEMENTACION PRINCIPAL PUBLIC
    // =====================================================

    SuffixTree::SuffixTree() : last_index(0), colors_computed(false) {}

    int SuffixTree::add_string(const std::string &str) {
        if (contains_end_token(str)) {
            std::cerr << "Error: El string contiene el token terminal '"
                     << END_TOKEN << "'" << std::endl;
            return -1;
        }

        if (last_index >= MAX_STRINGS) {
            std::cerr << "Error: Límite de strings alcanzado ("
                     << MAX_STRINGS << ")" << std::endl;
            return -1;
        }

        ++last_index;

        // Agregar token terminal
        std::string s = str + END_TOKEN;
        haystack[last_index] = s;

        // Desplegar sufijos
        if (deploy_suffixes(s, last_index) < 0) {
            haystack.erase(last_index);
            --last_index;
            return -1;
        }

        colors_computed = false;  // Invalidar colores

        return last_index;
    }



    /**
     * compute_colors - Calcula los colores de todos los nodos
     *
     * Debe llamarse después de agregar todos los strings y antes de
     * buscar distinguishing substrings.
     */
    void SuffixTree::compute_colors() {
        if (colors_computed) {
            return;  // Ya están calculados
        }

        // Primero marcar todas las hojas
        for (int i = 1; i <= last_index; ++i) {
            mark_leaves_for_string(i);
        }

        // Luego propagar colores desde las hojas hacia arriba
        compute_colors_dfs(&tree.root);

        colors_computed = true;
    }





    std::unordered_map<ColorSet, std::vector<std::string>> SuffixTree::get_all_strings(Node* node) {
        // Asegurar que los colores estén computados
        if (!colors_computed) {
            compute_colors();
        }
        
        std::unordered_map<ColorSet, std::vector<std::string>> result;
        std::string current_path = "";
        
        // Hacer DFS desde el nodo dado
        get_all_strings_dfs(node, current_path, result);
        
        return result;
    }







    bool SuffixTree::is_suffix(const std::string& str) {
        if (contains_end_token(str)) {
            return false;
        }

        std::string s = str + END_TOKEN;
        ReferencePoint root_point(&tree.root, -1, 0);

        return (get_starting_node(s, &root_point) == std::numeric_limits<Index>::max());
    }

    bool SuffixTree::is_substring(const std::string& str) {
        if (contains_end_token(str)) {
            return false;
        }

        ReferencePoint root_point(&tree.root, -1, 0);

        return (get_starting_node(str, &root_point) == std::numeric_limits<Index>::max());
    }

    int SuffixTree::get_string_count() const {
        return last_index;
    }

    std::string SuffixTree::get_string(int id) const {
        auto it = haystack.find(id);
        if (it != haystack.end()) {
            std::string result = it->second;
            if (!result.empty() && result.back() == END_TOKEN) {
                result.pop_back();
            }
            return result;
        }
        return "";
    }



} // namespace aed::structure