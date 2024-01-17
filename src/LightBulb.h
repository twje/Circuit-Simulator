#pragma once

#include "Component.h"

#include <vector>
#include <iostream>
#include <unordered_map>

class LightBulb : public Component
{
public:
    LightBulb() = default;
    LightBulb(ICircuitBoardNavigator* navigator)
        : Component(navigator, 2)
    { 
        GetNextNode();
        GetNextNode();
        
        AddComponentPin(true);   // 0
        AddComponentPin(true);   // 1
        AddComponentPin(true);   // 2
        AddComponentPin(true);   // 3
        AddComponentPin(false);  // 4

        mDirectionsMap[0] = { -1, 0 };
        mDirectionsMap[1] = { 0, -1 };
        mDirectionsMap[2] = { 1, 0 };
        mDirectionsMap[3] = { 0, 1 };
    }

    virtual Component* CreateShape(ICircuitBoardNavigator* navigator) const
    {
        auto component = new LightBulb(navigator);
        component->UpdateComponent();
        return component;
    }

    virtual void Move() override
    {
        UpdateComponent();
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

private:
    void UpdateComponent()
    {              
        // Clamp to circuit board
        Pin* selectedPin = &GetCircuitBoardPinAtCursor();
        for (const auto& pair : mDirectionsMap)
        {
            if (!GetNeighborCircuitBoardPin(*selectedPin, pair.second))
            {
                selectedPin = GetNeighborCircuitBoardPin(*selectedPin, -pair.second);
                assert(selectedPin);
            }
        }
        
        // Associate circuit board pins
        for (const auto& pair : mDirectionsMap)
        {
            Pin* circuitBoardPin = GetNeighborCircuitBoardPin(*selectedPin, pair.second);
            AssociateComponentWithCircuitBoardPin(pair.first, circuitBoardPin);
        } 
        AssociateComponentWithCircuitBoardPin(4, selectedPin);

        // Update node positions
        GetNode(0).SetPosition(GetCircuitBoardPinPosition(4));
        GetNode(1).SetPosition(GetCircuitBoardPinPosition(0));
    }
   
private:
    std::unordered_map<uint32_t, sf::Vector2i> mDirectionsMap;
};