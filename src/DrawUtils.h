#pragma once

#include <SFML/Graphics.hpp>

void DrawPoint(sf::RenderTarget& target, sf::Vector2f position, float radius, sf::Color color);
void DrawFloatRect(sf::RenderTarget& target, const sf::FloatRect& rect, sf::Color color);