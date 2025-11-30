#ifndef TREE_VISUALIZER_H
#define TREE_VISUALIZER_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include "SuffixTree.h"

class TreeVisualizer {
public:
    TreeVisualizer();
    void draw(sf::RenderWindow& window, aed::structure::SuffixTree& tree);
    void setPosition(float x, float y) { offsetX = x; offsetY = y; }
    
private:
    struct NodePosition {
        float x, y;
        aed::structure::SuffixTree::Node* node;
    };
    
    float offsetX, offsetY;
    float nodeRadius;
    float horizontalSpacing;
    float verticalSpacing;
    
    sf::Font font;
    bool fontLoaded;
    
    float computeSubtreeWidth(aed::structure::SuffixTree::Node* node,
            std::unordered_map<aed::structure::SuffixTree::Node*, float>& widths);

    void assignPositions(
            aed::structure::SuffixTree::Node* node,
            float x,
            float y,
            std::unordered_map<aed::structure::SuffixTree::Node*, NodePosition>& pos,
            std::unordered_map<aed::structure::SuffixTree::Node*, float>& widths);
    
    void drawNode(sf::RenderWindow& window, 
                  const NodePosition& pos, 
                  aed::structure::SuffixTree::Node* node);
    
    void drawEdge(sf::RenderWindow& window,
                  const NodePosition& from,
                  const NodePosition& to,
                  const std::string& label);
    
    std::string getNodeLabel(aed::structure::SuffixTree::Node* node) const;
    std::string getEdgeLabel(const aed::structure::SuffixTree::Transition& trans,
                            const aed::structure::SuffixTree& tree) const;
    sf::Color getNodeColor(aed::structure::SuffixTree::Node* node) const;
};

#endif // TREE_VISUALIZER_H

