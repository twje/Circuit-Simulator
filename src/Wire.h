#pragma once

#include "Component.h"

#include <iostream>

/*
class Wire : public Component
{
public:
    Wire(const sf::Vector2f& startPosition, float gridSpacing)
        : Component(2)
        , mGridSpacing(gridSpacing)
    { 
        GetNextNode(startPosition);
        GetNextNode(sf::Vector2f());

        AddConnector();
        AddConnector();
    }

    virtual Component* CreateShape(ICircuitBoardNavigator& navigator, const sf::Vector2f& cursor, float gridSpacing) const
    {
        auto component = new Wire(cursor, gridSpacing);
        component->UpdateComponent(cursor);
        return component;
    }

    virtual void Move(const sf::Vector2f& cursor) override
    {
        UpdateComponent(cursor);
    }

    virtual void DrawComponent(sf::RenderTarget& target)
    {
        sf::Vertex line[] = {
            GetNode(0).GetPosition(),
            GetNode(1).GetPosition()
        };

        line[0].color = mColor;
        line[1].color = mColor; 

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
        //for (auto& node : GetNodes())
        //{
        //    DrawPoint(target, node.GetPosition(), 10, sf::Color::Yellow);
        //}

        //for (auto connector : mConnectors)
        //{
        //    DrawPoint(target, connector.GetPosition(), 10, sf::Color::Yellow);
        //}

        //for (auto pin : mPins)
        //{
        //    DrawPoint(target, pin, 10, sf::Color::Yellow);
        //}
    };

private:
    void UpdateComponent(const sf::Vector2f& cursor)
    {
        UpdateNodePositions(cursor);
        UpdateConnectorPositions(cursor);
        UpdatePins(cursor);
    }

    void UpdateNodePositions(const sf::Vector2f& cursor)
    {
        sf::Vector2f offset;
        sf::Vector2f position = GetNode(0).GetPosition();

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
    }

    void UpdateConnectorPositions(const sf::Vector2f& cursor)
    {
        mConnectors[0]->SetPosition(GetNode(0).GetPosition());
        mConnectors[1]->SetPosition(GetNode(1).GetPosition());
    }

    void UpdatePins(const sf::Vector2f& cursor)
    {
        mPins.clear();

        sf::Vector2f position0 = GetNode(0).GetPosition();
        sf::Vector2f position1 = GetNode(1).GetPosition();
        if (position0 == position1)
        {
            return;
        }

        sf::Vector2f direction = (position0 - position1).normalized() * mGridSpacing;

        float steps = (position0 - position1).length() / mGridSpacing;
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
*/