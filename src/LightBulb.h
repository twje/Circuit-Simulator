#pragma once

#include "Component.h"

#include <vector>

class LightBulb : public Component
{
public:
    LightBulb(float gridSpacing)
        : Component(2)
        , mGridSpacing(gridSpacing)
    { }

    virtual Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing) const
    {
        sf::Vector2f left{ -gridSpacing, 0.0f };
        sf::Vector2f top{ 0.0f, -gridSpacing };
        sf::Vector2f right{ gridSpacing, 0.0f };
        sf::Vector2f bottom{ 0.0f, +gridSpacing };

        // Create shape
        LightBulb* component = new LightBulb(gridSpacing);
        component->GetNextNode(cursor);
        Node* node = component->GetNextNode(cursor);
        sf::Vector2f position = node->GetPosition();
        node->SetPosition(position + right);

        // Create connectors
        component->AddConnector({ cursor + left, true });
        component->AddConnector({ cursor + top, true });
        component->AddConnector({cursor + right, true });
        component->AddConnector({cursor + bottom, true });

        // Create pins
        component->InitPins();

        return component;
    }

    virtual void Move(const sf::Vector2f& cursor) override
    {
        sf::Vector2f left{ -mGridSpacing, 0.0f };
        sf::Vector2f top{ 0.0f, -mGridSpacing };
        sf::Vector2f right{ mGridSpacing, 0.0f };
        sf::Vector2f bottom{ 0.0f, mGridSpacing };

        // Update node positions
        GetNode(0).SetPosition(cursor);
        GetNode(1).SetPosition({ cursor.x + mGridSpacing, cursor.y });

        // Update connector positions
        mConnectors[0].SetPosition(cursor + left);
        mConnectors[1].SetPosition(cursor + top);
        mConnectors[2].SetPosition(cursor + right);
        mConnectors[3].SetPosition(cursor + bottom);

        // Update pins
        InitPins();
    }

    virtual sf::FloatRect GetGlobalBounds(float gridSpacing) override
    {
        sf::Vector2f position1 = GetNode(0).GetPosition();
        sf::Vector2f position2 = GetNode(1).GetPosition();
        float radius = (position1 - position2).length();
    
        float left = position1.x - radius;
        float top = position1.y - radius;
        float right = position1.x + radius;
        float bottom = position1.y + radius;

        return { { left, top }, { right - left, bottom - top } };
    }

    virtual void DrawShape(sf::RenderTarget& target)
    {
        // Draw shape
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

    virtual void DebugDraw(sf::RenderTarget& target)
    {
        for (const sf::Vector2f& pin : mPins)
        {
            DrawPoint(target, pin, 10, sf::Color::Yellow);
        }        
    };

private:
    void InitPins()
    {
        mPins.clear();

        sf::Vector2f position = GetNode(0).GetPosition();
        sf::Vector2f left{ -mGridSpacing, 0.0f };
        sf::Vector2f top{ 0.0f, -mGridSpacing };
        sf::Vector2f right{ mGridSpacing, 0.0f };
        sf::Vector2f bottom{ 0.0f, +mGridSpacing };               

        mPins.push_back(position);
        mPins.push_back(position + left);
        mPins.push_back(position + top);
        mPins.push_back(position + right);
        mPins.push_back(position + bottom);
    }

    float mGridSpacing;
};