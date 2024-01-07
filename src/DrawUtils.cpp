#include "DrawUtils.h"

void DrawPoint(sf::RenderTarget& target, sf::Vector2f position, float radius, sf::Color color)
{
    sf::CircleShape shape(radius);
    shape.setFillColor(color);
    shape.setPosition({ position.x - radius, position.y - radius });
    target.draw(shape);
}

void DrawFloatRect(sf::RenderTarget& target, const sf::FloatRect& rect, sf::Color color)
{
    sf::RectangleShape shape;
    shape.setPosition(rect.getPosition());
    shape.setSize(rect.getSize());
    shape.setFillColor({ 0, 0, 0, 0 });
    shape.setOutlineColor(color);
    shape.setOutlineThickness(-1);

    target.draw(shape);
}