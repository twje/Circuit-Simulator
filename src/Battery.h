#pragma once

#include "Component.h"

/*
class Battery : public Component
{
public:
    Battery()
        : Component(2)
    { }

    virtual Component* CreateShape(ICircuitBoardNavigator& navigator, const sf::Vector2f& cursor, float gridSpacing) const
    {
        Component* component = new Battery();
        component->GetNextNode(cursor);
        component->GetNextNode(cursor);

        return component;
    }

    virtual void Move(const sf::Vector2f& cursor) override
    {
        Node* node = GetSelectedNode();
        if (node != nullptr)
        {
            node->SetPosition(cursor);
        }
    }

    virtual void DrawComponent(sf::RenderTarget& target)
    {
        const sf::Vector2f& position1 = GetNode(0).GetPosition();
        const sf::Vector2f& position2 = GetNode(1).GetPosition();
        sf::Vector2f size = position2 - position1;

        sf::RectangleShape rectangle(size);
        rectangle.setPosition(position1);
        rectangle.setFillColor({ 0, 0, 0, 0 });
        rectangle.setOutlineColor(mColor);
        rectangle.setOutlineThickness(-1.f);

        target.draw(rectangle);
    }

    virtual void DrawIcon(sf::RenderTarget& target, const sf::Transform& transform, const sf::FloatRect& localBounds) override
    {
        sf::RenderStates states;
        states.transform = transform;

        sf::RectangleShape rectangle;
        rectangle.setPosition({ localBounds.left, localBounds.top });
        rectangle.setSize({ localBounds.width, localBounds.height });
        rectangle.setFillColor(sf::Color::Blue);

        target.draw(rectangle, states);
    }
};
*/