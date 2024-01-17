#pragma once

#include<SFML/Graphics.hpp>

class Pin;
class ComponentPreviewCreator;

class ICircuitBoardNavigator
{
public:
    virtual Pin* GetSurroundingPin(Pin& pin, sf::Vector2i offset) = 0;
    virtual Pin& GetSelectedPin() = 0;
    virtual sf::Vector2f GetGridCoordinateFromPin(Pin& pin) = 0;
};

class IComponentPickerObserver
{
public:
    virtual void OnCreateNewComponent(ComponentPreviewCreator* componentFactory) = 0;
};
