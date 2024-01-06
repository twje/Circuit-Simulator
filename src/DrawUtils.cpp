#include "DrawUtils.h"

void DrawPoint(sf::RenderTarget& target, sf::Vector2f position, float radius, sf::Color color)
{
    sf::CircleShape shape(radius);
    shape.setFillColor(color);
    shape.setPosition({ position.x - radius, position.y - radius });
    target.draw(shape);
}