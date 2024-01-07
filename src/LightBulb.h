#pragma once

#include "Component.h"

#include <vector>

class LightBulb : public Component
{
public:
    LightBulb(float gridSpacing)
        : Component(2)
        , mGridSpacing(gridSpacing)
        , mDirections({ {-gridSpacing, 0}, {0, -gridSpacing}, {gridSpacing, 0}, {0, gridSpacing} })
    { 
        GetNextNode(sf::Vector2f());
        GetNextNode(sf::Vector2f());
        
        AddConnector(Connector(sf::Vector2f(), true));
        AddConnector(Connector(sf::Vector2f(), true));
        AddConnector(Connector(sf::Vector2f(), true));
        AddConnector(Connector(sf::Vector2f(), true));
    }

    virtual Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing) const
    {
        auto component = new LightBulb(gridSpacing);
        component->UpdateComponent(cursor);
        return component;
    }

    virtual void Move(const sf::Vector2f& cursor) override
    {
        UpdateComponent(cursor);
    }

    virtual void DrawComponent(sf::RenderTarget& target)
    {
        // Draw shape
        const sf::Vector2f& position1 = GetNode(0).GetPosition();
        const sf::Vector2f& position2 = GetNode(1).GetPosition();
        float radius = (position1 - position2).length();

        sf::Vertex line[] = { position1, position2 };

        line[0].color = mColor;
        line[1].color = mColor;

        sf::CircleShape circle(radius);
        circle.setFillColor({ 0, 0, 0, 0 });
        circle.setOutlineColor(mColor);
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
    void UpdateComponent(const sf::Vector2f& cursor)
    {
        UpdateNodePositions(cursor);
        UpdateConnectorPositions(cursor);
        UpdatePins(cursor);
    }

    void UpdateNodePositions(const sf::Vector2f& cursor)
    {
        GetNode(0).SetPosition(cursor);
        GetNode(1).SetPosition({ cursor.x + mGridSpacing, cursor.y });
    }

    void UpdateConnectorPositions(const sf::Vector2f& cursor)
    {        
        for (size_t i = 0; i < mDirections.size(); ++i) 
        {
            mConnectors[i].SetPosition(cursor + mDirections[i]);
        }
    }

    void UpdatePins(const sf::Vector2f& cursor)
    {
        mPins.clear();        
        for (const auto& dir : mDirections)
        {
            mPins.push_back(cursor + dir);
        }
        mPins.push_back(GetNode(0).GetPosition());
    }

    float mGridSpacing;
    std::vector<sf::Vector2f> mDirections;
};