#include "TreeVisualizer.h"
#include <cmath>
#include <sstream>

TreeVisualizer::TreeVisualizer() 
    : offsetX(50), offsetY(50), nodeRadius(25), 
      horizontalSpacing(150), verticalSpacing(100), fontLoaded(false) {
    // Intentar cargar fuente
    if (!font.loadFromFile("../resources/arial.ttf")) {
        fontLoaded = false;
    } else {
        fontLoaded = true;
    }
}

float TreeVisualizer::computeSubtreeWidth(
    aed::structure::SuffixTree::Node* node,
    std::unordered_map<aed::structure::SuffixTree::Node*, float>& widths)
{
    if (node->g.empty()) {
        widths[node] = nodeRadius * 2;   // ancho mínimo
        return widths[node];
    }

    float total = 0;
    for (const auto& p : node->g) {
        if (p.second.tgt) {
            total += computeSubtreeWidth(p.second.tgt, widths);
        }
    }

    // Separación levemente mayor entre subárboles
    total += (node->g.size() - 1) * (horizontalSpacing / 2.0f);

    widths[node] = total;
    return total;
}

void TreeVisualizer::assignPositions(
    aed::structure::SuffixTree::Node* node,
    float x,
    float y,
    std::unordered_map<aed::structure::SuffixTree::Node*, NodePosition>& pos,
    std::unordered_map<aed::structure::SuffixTree::Node*, float>& widths)
{
    pos[node] = {x, y};

    if (node->g.empty()) return;

    float totalWidth = widths[node];
    float startX = x - totalWidth / 2.0f;

    for (const auto& p : node->g) {
        if (p.second.tgt) {
            float w = widths[p.second.tgt];
            float childX = startX + w / 2.0f;
            assignPositions(p.second.tgt, childX, y + verticalSpacing, pos, widths);

            startX += w + (horizontalSpacing / 2.0f);
        }
    }
}


void TreeVisualizer::draw(sf::RenderWindow& window, aed::structure::SuffixTree& tree) {
    std::unordered_map<aed::structure::SuffixTree::Node*, float> widths;
    std::unordered_map<aed::structure::SuffixTree::Node*, NodePosition> positions;

    computeSubtreeWidth(&tree.tree.root, widths);

    // Centrar root en la ventana
    float centerX = window.getSize().x / 2.0f;
    assignPositions(&tree.tree.root, centerX, offsetY, positions, widths);
    
    // Dibujar aristas primero (para que queden detrás de los nodos)
    for (const auto& posPair : positions) {
        aed::structure::SuffixTree::Node* fromNode = posPair.first;
        const NodePosition& fromPos = posPair.second;
        
        for (const auto& transPair : fromNode->g) {
            if (transPair.second.tgt != nullptr) {
                auto it = positions.find(transPair.second.tgt);
                if (it != positions.end()) {
                    std::string label = getEdgeLabel(transPair.second, tree);
                    drawEdge(window, fromPos, it->second, label);
                }
            }
        }
    }
    
    // Dibujar nodos
    for (const auto& posPair : positions) {
        drawNode(window, posPair.second, posPair.first);
    }
}

void TreeVisualizer::drawNode(sf::RenderWindow& window, 
                              const NodePosition& pos, 
                              aed::structure::SuffixTree::Node* node) {
    // Círculo del nodo
    sf::CircleShape circle(nodeRadius);
    circle.setPosition(pos.x - nodeRadius, pos.y - nodeRadius);
    circle.setFillColor(getNodeColor(node));
    circle.setOutlineColor(sf::Color::Black);
    circle.setOutlineThickness(2);
    window.draw(circle);
    
    // Etiqueta del nodo
    if (fontLoaded) {
        std::string label = getNodeLabel(node);
        sf::Text text(label, font, 14);
        text.setFillColor(sf::Color::Black);
        text.setPosition(pos.x - nodeRadius/2, pos.y - 7);
        window.draw(text);
    }
}

void TreeVisualizer::drawEdge(sf::RenderWindow& window,
                              const NodePosition& from,
                              const NodePosition& to,
                              const std::string& label) {
    // Línea
    sf::Vertex line[] = {
        sf::Vertex(sf::Vector2f(from.x, from.y + nodeRadius), sf::Color::Black),
        sf::Vertex(sf::Vector2f(to.x, to.y - nodeRadius), sf::Color::Black)
    };
    window.draw(line, 2, sf::Lines);
    
    // Etiqueta de la arista (en el medio)
    if (fontLoaded && !label.empty()) {
        float midX = (from.x + to.x) / 2.0f;
        float midY = (from.y + to.y) / 2.0f;
        
        sf::Text text(label, font, 12);
        text.setFillColor(sf::Color::Blue);
        text.setPosition(midX - 10, midY - 10);
        
        // Fondo blanco para la etiqueta
        sf::RectangleShape bg(sf::Vector2f(text.getLocalBounds().width + 4, 
                                          text.getLocalBounds().height + 4));
        bg.setFillColor(sf::Color::White);
        bg.setPosition(midX - 12, midY - 12);
        window.draw(bg);
        window.draw(text);
    }
}

std::string TreeVisualizer::getNodeLabel(aed::structure::SuffixTree::Node* node) const {
    if (node == nullptr) return "?";
    
    // Mostrar colores del nodo
    std::ostringstream oss;
    bool first = true;
    for (size_t i = 0; i < aed::structure::SuffixTree::MAX_STRINGS; ++i) {
        if (node->colors.test(i)) {
            if (!first) oss << ",";
            oss << (i + 1);
            first = false;
        }
    }
    
    if (oss.str().empty()) {
        return "R";  // Root si no tiene colores
    }
    return oss.str();
}

std::string TreeVisualizer::getEdgeLabel(const aed::structure::SuffixTree::Transition& trans,
                                        const aed::structure::SuffixTree& tree) const {
    // Obtener el string de la transición
    std::string label = tree.substring_to_string(trans.sub);
    
    // Limitar longitud para que no sea muy largo
    if (label.length() > 10) {
        label = label.substr(0, 7) + "...";
    }
    
    return label;
}

sf::Color TreeVisualizer::getNodeColor(aed::structure::SuffixTree::Node* node) const {
    if (node == nullptr) return sf::Color::White;
    
    int colorCount = node->colors.count();
    
    if (colorCount == 0) {
        return sf::Color(211, 211, 211);  // LightGray - Sin colores
    } else if (colorCount == 1) {
        return sf::Color(144, 238, 144);  // LightGreen - Un solo color
    } else {
        return sf::Color(173, 216, 230);  // LightBlue - Múltiples colores
    }
}

