#ifndef AED_SUFFIX_TREE
#define AED_SUFFIX_TREE


#include <iostream>
#include <unordered_map>
#include <string>
#include <bitset>
#include <algorithm>
#include <vector>


namespace aed::structure {


/**
 * Clase SuffixTree - Implementacion del algoritmo de Ukkonen para GST y dsus
 *
 * Genera un Suffix Tree Generalizado (GST) que puede contener multiples strings.
 * Cada string se identifica con un ID unico.
 *
 * Caracteriticas adicionales:
 *  - Coloreo de nodos bitsets para identificar string por nodo.
 *  - Busqueda de Distinguidhing Substrings (DSus)
 *
 * Parametros:
 *  - MAX_STRINGS: Maximo numero de substrings que puede contener (por defecto 64, modificable)
 *  - END_TOKEN: Caracter especial que marca el final de cada string (por defecto '$')
*/
class SuffixTree {


public:
    // Cantidad de string que haremos coloreo
    static constexpr std::size_t MAX_STRINGS = 64;
    // Caracter terminal que marca el final de cada string
    static constexpr char END_TOKEN = '$';

    // Tip lexico para indices en los strings (simple formato)
    using Index = int;
    // Tip lexico para coloreo de nodos (simple formato)
    using ColorSet = std::bitset<MAX_STRINGS>;


// private:

    // CLASES INTERNAS
    struct Node;

    struct MappedSubstring {
        int ref_str;
        Index l;
        Index r;
        MappedSubstring();
        MappedSubstring(int ref, Index left, Index right);
        bool empty() const;
        int lenght() const;
    };

    struct Transition {
        MappedSubstring sub;
        Node* tgt;
        Transition();
        Transition(MappedSubstring s, Node* t);
        bool is_valid() const;
    };

    struct Node {
        std::unordered_map<char, Transition> g;
        Node* suffix_link;
        ColorSet colors;
        Node();
        virtual ~Node();
        virtual Transition find_alpha_transition(char alpha);
        void mark_string(int string_id);
        bool has_single_string() const;
        int get_single_string_id() const;
        void merge_colors(const ColorSet& other);
    };

    struct SinkNode : public Node {
        virtual Transition find_alpha_transition(char aplha) override;
    };

    struct Leaf : public Node {
        // Las hojas implican sufijos completos
    };

    struct ReferencePoint {
        Node* node;
        int ref_str;
        Index pos;
        ReferencePoint(Node* n, int ref, Index p);
    };

    struct Base {
        SinkNode sink;
        Node root;
        Base();
        ~Base();
        void clean();
    };


    // MIEMBROS DE CLASE

    Base tree;
    std::unordered_map<int, std::string> haystack;
    int last_index;
    bool colors_computed;

    // METODOS AUXILIARES
    std::string substring_to_string(const MappedSubstring& substr) const;
    bool test_and_split(Node* n, MappedSubstring kp, char t, const std::string& w, Node** r);
    ReferencePoint update(Node* n, MappedSubstring ki);
    ReferencePoint canonize(Node* n, MappedSubstring kp);
    Index get_starting_node(const std::string &s, ReferencePoint* r);
    int deploy_suffixes(const std::string& s, int sindex);
    bool contains_end_token(const std::string& str) const;

    ColorSet compute_colors_dfs(Node* node);
    void mark_leaves_for_string(int string_id);
    void get_all_strings_dfs(Node* node, std::string& current_path, std::unordered_map<ColorSet, std::vector<std::string>>& result);

// public:

    SuffixTree();
    int add_string(const std::string &str);
    bool is_suffix(const std::string& str);
    bool is_substring(const std::string& str);
    int get_string_count() const;
    std::string get_string(int id) const;
    void compute_colors();

    std::unordered_map<ColorSet, std::vector<std::string>> get_all_strings(Node* node);
};


}


#endif // AED_SUFFIX_TREE

