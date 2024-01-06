#pragma once

#include "Component.h"

class LightBulb : public Component
{
public:
    LightBulb()
        : Component(2)
    { }

    virtual Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing) const
    {
        Component* tempShape = new LightBulb();
        tempShape->GetNextNode(cursor);
        Node* node = tempShape->GetNextNode(cursor);
        sf::Vector2f position = node->GetPosition();
        node->SetPosition({ position.x + gridSpacing, position.y });

        return tempShape;
    }

    virtual void Move(const sf::Vector2f& cursor, float gridSpacing) override
    {
        GetNode(0).SetPosition(cursor);
        GetNode(1).SetPosition({ cursor.x + gridSpacing, cursor.y });
    }

    virtual void DrawShape(sf::RenderTarget& target)
    {
        const sf::Vector2f& position1 = GetNode(0).GetPosition();
        const sf::Vector2f& position2 = GetNode(1).GetPosition();
        float radius = (position1 - position2).length();

        sf::Vertex line[] = { position1, position2 };

        line[0].color = GetColor();
        line[1].color = GetColor();

        sf::CircleShape circle(radius);
        circle.setFillColor({ 0, 0, 0, 0 });
        circle.setOutlineColor(GetColor());
        circle.setOutlineThickness(1.0f);
        circle.setPosition(position1 - sf::Vector2f(radius, radius));

        target.draw(line, 2, sf::PrimitiveType::Lines);
        target.draw(circle);
    }

    virtual void DrawIcon(sf::RenderTarget& target, const sf::Transform& transform, const sf::FloatRect& localBounds) override
    {
        sf::RenderStates states;
        states.transform = transform;

        float halfHeight = localBounds.top + localBounds.height / 2.0f;
        float halfWidth = localBounds.left + localBounds.width / 2.0f;
        float radius = halfWidth;

        sf::CircleShape circle(radius);
        circle.setFillColor(sf::Color::Blue);
        circle.setPosition(sf::Vector2f(halfHeight, halfWidth) - sf::Vector2f(radius, radius));

        target.draw(circle, states);
    }
};