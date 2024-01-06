#pragma once

#include "Component.h"

class Wire : public Component
{
public:
    Wire()
        : Component(2)
    { }

    virtual Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing) const
    {
        Component* tempShape = new Wire();
        tempShape->GetNextNode(cursor);
        tempShape->GetNextNode(cursor);

        return tempShape;
    }

    virtual void Move(const sf::Vector2f& cursor, float gridSpacing) override
    {
        Node* node = GetSelectedNode();
        if (node != nullptr)
        {
            node->SetPosition(cursor);
        }
    }

    virtual void DrawShape(sf::RenderTarget& target)
    {
        sf::Vertex line[] = {
            GetNode(0).GetPosition(),
            GetNode(1).GetPosition()
        };

        line[0].color = GetColor();
        line[1].color = GetColor();

        target.draw(line, 2, sf::PrimitiveType::Lines);
    }

    virtual void DrawIcon(sf::RenderTarget& target, const sf::Transform& transform, const sf::FloatRect& localBounds) override
    {
        sf::RenderStates states;
        states.transform = transform;

        float halfHeight = localBounds.top + localBounds.height / 2.0f;
        float startX = localBounds.left;
        float endX = localBounds.left + localBounds.width;

        sf::Vertex line[] = {
            sf::Vector2f(startX, halfHeight),
            sf::Vector2f(endX, halfHeight)
        };

        line[0].color = sf::Color::Cyan;
        line[1].color = sf::Color::Cyan;

        target.draw(line, 2, sf::PrimitiveType::Lines, states);
    }
};