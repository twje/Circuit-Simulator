#pragma once

#include "Component.h"

#include <iostream>

class Wire : public Component
{
public:
    Wire(float gridSpacing)
        : Component(2)
        , mGridSpacing(gridSpacing)
    { }

    virtual Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing) const
    {
        // Create shape
        Wire* component = new Wire(gridSpacing);
        component->GetNextNode(cursor);
        component->GetNextNode(cursor);

        // Create connectors
        component->AddConnector({ cursor, true });
        component->AddConnector({ cursor, true });

        // Create pins
        component->InitPins();

        return component;
    }

    virtual void Move(const sf::Vector2f& cursor) override
    {
        if (GetSelectedNode() != nullptr)
        {
            sf::Vector2f offset;
            sf::Vector2f position = GetNode(0).GetPosition(); // Start node

            float hortLength = cursor.x - position.x;
            float vertLength = cursor.y - position.y;

            // Hort movement
            if (std::abs(vertLength) > std::abs(hortLength))
            {
                offset.y = vertLength;
            }
            // Vert movement
            else 
            {
                offset.x = hortLength;
            }
            
            GetNode(1).SetPosition(position + offset);
            mConnectors[1].SetPosition(position + offset);
            InitPins();
        }
    }

    virtual bool IsConnector(sf::Vector2f cursor) 
    { 
        sf::Vector2f position1 = GetNode(0).GetPosition();
        sf::Vector2f position2 = GetNode(1).GetPosition();
        sf::Vector2f direction = (position1 - position2).normalized() * mGridSpacing;

        float steps = (position1 - position2).length() / mGridSpacing;
        for (float step = 0; step <= steps; step++)
        {
            sf::Vector2f pin = position1 + (direction * step);
            if (cursor.x == pin.x && cursor.y == pin.y)
            {
                return true;
            }
        }
        return false;
    }

    virtual bool IsConnectorConnectable(sf::Vector2f cursor) 
    { 
        for (Node& node : GetNodes())
        {            
            if (cursor.x == node.GetPosition().x && cursor.y == node.GetPosition().y)
            {
                return true;
            }
        }
        return false;
    };

    virtual sf::FloatRect GetGlobalBounds(float gridSpacing) override
    {
        sf::Vector2f position1 = GetNode(0).GetPosition();
        sf::Vector2f position2 = GetNode(1).GetPosition();
        float halfGridSpacing = gridSpacing / 2.0f;

        // Hort line
        if (position1.y == position2.y)
        {
            if (position1.x > position2.x)
            {
                SwapVectors(position1, position2);
            }

            float left = position1.x;
            float top = position1.y - halfGridSpacing;
            float right = position2.x;
            float bottom = position2.y + halfGridSpacing;

            return { { left, top }, { right - left, bottom - top } };
        }
        // Vert line
        else
        {
            if (position1.y < position2.y)
            {
                SwapVectors(position1, position2);
            }
        }

        return { };
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

        sf::Vector2f position1 = GetNode(0).GetPosition();
        sf::Vector2f position2 = GetNode(1).GetPosition();
        if (position1 == position2)
        {
            return;
        }
        
        sf::Vector2f direction = (position2 - position1).normalized() * mGridSpacing;

        float steps = (position1 - position2).length() / mGridSpacing;
        for (float step = 0; step <= steps; step++)
        {
            sf::Vector2f pin = position1 + (direction * step);
            mPins.push_back(pin);
        }
    }

    void SwapVectors(sf::Vector2f& vector1, sf::Vector2f& vector2)
    {
        sf::Vector2f temp = vector1;
        vector1 = vector2;
        vector2 = temp;
    }

    float mGridSpacing;    
};