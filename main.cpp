#include <SFML/Graphics.hpp>
#include <iostream>
#include <string>
#include <vector>
#include <numeric>
#include "include/SuffixTree.h"
#include "include/TreeVisualizer.h"

struct StepState {
    bool initialized = false;
    aed::structure::SuffixTree::ReferencePoint active_point = {nullptr,0,0};
    int i = 0;
    int sindex = 0;
    std::string s;
};

int main() {
    sf::RenderWindow window(sf::VideoMode(1400, 800), "Suffix Tree Visualizer");
    window.setFramerateLimit(60);

    aed::structure::SuffixTree tree;
    TreeVisualizer visualizer;
    visualizer.setPosition(700, 50);

    std::vector<std::string> stringsToAdd = {
        // "",
        "aaaa"
    };

    StepState step;

    sf::Font font;
    font.loadFromFile("../resources/arial.ttf");

    std::size_t currentStringIndex = 0;

    // ========= TEXTBOX PARA BUSCAR =========
    bool textboxActive = false;
    std::string textboxContent = "";
    std::string queryResult = "";

    sf::RectangleShape textboxRect(sf::Vector2f(300, 35));
    textboxRect.setFillColor(sf::Color(240,240,240));
    textboxRect.setOutlineColor(sf::Color::Black);
    textboxRect.setOutlineThickness(2);
    textboxRect.setPosition(20, 720);

    sf::Text textboxText("", font, 18);
    textboxText.setFillColor(sf::Color::Black);
    textboxText.setPosition(30, 725);

    sf::Text queryText("", font, 18);
    queryText.setFillColor(sf::Color::Black);
    queryText.setPosition(350, 725);

    // =======================================

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();

            // ========= INPUT DEL TEXTBOX =========
            if (textboxActive) {

                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode >= 32 && event.text.unicode <= 126) {
                        textboxContent += static_cast<char>(event.text.unicode);
                    }
                }

                if (event.type == sf::Event::KeyPressed) {

                    if (event.key.code == sf::Keyboard::BackSpace) {
                        if (!textboxContent.empty())
                            textboxContent.pop_back();
                    }

                    if (event.key.code == sf::Keyboard::Escape) {
                        textboxActive = false;
                    }

                    if (event.key.code == sf::Keyboard::Enter) {
                        // ========== REALIZAR CONSULTA ==========
                        if (!textboxContent.empty()) {
                            bool sub = tree.is_substring(textboxContent);
                            bool suf = tree.is_suffix(textboxContent);

                            queryResult =
                                "substring: " + std::string(sub ? "true" : "false") +
                                " | suffix: " + std::string(suf ? "true" : "false");
                        }
                    }
                }

                continue; // NO PROCESA ENTER PARA UKKONEN
            }


            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::C) {
                    tree = aed::structure::SuffixTree();
                    step = StepState();
                    currentStringIndex = 0;
                }

                if (event.key.code == sf::Keyboard::T) {
                    textboxActive = true;
                    textboxContent = "";
                    queryResult = "";
                }

                if (event.key.code == sf::Keyboard::Enter) {

                    if (currentStringIndex < stringsToAdd.size()) {

                        // ========= INICIALIZACIÓN LOCAL DE deploy_suffixes =========
                        if (!step.initialized) {

                            const std::string& str = stringsToAdd[currentStringIndex];
                            tree.last_index++;
                            step.sindex = tree.last_index;

                            step.s = str + aed::structure::SuffixTree::END_TOKEN;
                            tree.haystack[tree.last_index] = step.s;

                            step.active_point = {
                                &tree.tree.root,
                                step.sindex,
                                0
                            };

                            step.i = tree.get_starting_node(step.s, &step.active_point);
                            if (step.i == std::numeric_limits<int>::max()) {
                                std::cerr << "Error starting node\n";
                                return -1;
                            }

                            step.initialized = true;
                        }


                        if (step.i < step.s.size()) {
                            aed::structure::SuffixTree::MappedSubstring ki(
                                step.sindex,
                                step.active_point.pos,
                                step.i
                            );

                            step.active_point =
                                tree.update(step.active_point.node, ki);

                            ki.l = step.active_point.pos;

                            step.active_point =
                                tree.canonize(step.active_point.node, ki);

                            step.i++;

                        } else {
                            // ========= TERMINA LA CADENA =========
                            tree.colors_computed = false;
                            tree.compute_colors();
                            currentStringIndex++;

                            step = StepState();
                        }
                    }
                }
            }
        }



        window.clear(sf::Color::White);
        visualizer.draw(window, tree);

        // ==== INFO ORIGINAL ====
        sf::Text info("", font, 16);
        info.setFillColor(sf::Color::Black);
        info.setPosition(10, 10);

        std::string msg =
            "ENTER = siguiente paso Ukkonen\n"
            "C = limpiar\n"
            "T = activar caja de texto para buscar\n\n";

        if (currentStringIndex < stringsToAdd.size()) {
            msg += "Insertando: " + stringsToAdd[currentStringIndex] + "\n";
            msg += "Paso i = " + std::to_string(step.i) + " / "
                   + std::to_string(step.s.size()) + "\n";
        } else {
            msg += "Construccion terminada.\n";
        }

        info.setString(msg);
        window.draw(info);

        // ==== TEXTBOX ====
        window.draw(textboxRect);

        textboxText.setString(textboxContent +
            (textboxActive ? "_" : "")); // cursor visual
        window.draw(textboxText);

        queryText.setString(queryResult);
        window.draw(queryText);

        window.display();
    }

    return 0;
}



