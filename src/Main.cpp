#include "Component.h"
#include "Battery.h"
#include "LightBulb.h"
#include "Wire.h"
#include "DrawUtils.h"

#include <SFML/Graphics.hpp>

class ComponentPreviewCreator : public sf::Transformable
{
public:
    ComponentPreviewCreator(std::unique_ptr<Component> shape)
        : mShape(std::move(shape))
    { }

    void Draw(sf::RenderTarget& target)
    {
        // Compute local bounds
        float border = 2.0f;
        float padding = 1.0f;
        float totalBorderPadding = border + padding;
        sf::Vector2f size(100, 100);        
        sf::FloatRect localBounds(
            { totalBorderPadding , totalBorderPadding },
            { size.x - 2 * totalBorderPadding , size.y - 2 * totalBorderPadding }
        );
     
        // Draw background
        sf::RectangleShape background(size);
        background.setFillColor({ sf::Color::Magenta });
        background.setOutlineColor(sf::Color::Red);
        background.setOutlineThickness(-border);
        target.draw(background, getTransform());

        // Draw component
        mShape->DrawIcon(target, getTransform(), localBounds);
    }

    Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing)
    {
        return mShape->CreateShape(cursor, gridSpacing);
    }

private:
    std::unique_ptr<Component> mShape;
};

class ComponentPicker
{
public:
    void AddComponent(std::unique_ptr<Component> component)
    {
        if (mComponents.empty())
        {
            mSelectedComponent = 0;
        }
        mComponents.emplace_back(std::make_unique<ComponentPreviewCreator>(std::move(component)));
    }

    void StepForward() 
    { 
        if (!mComponents.empty())
        {
            if (mSelectedComponent.value() + 1 >= mComponents.size())
            {             
                mSelectedComponent = 0;
            }
            else
            {             
                mSelectedComponent = mSelectedComponent.value() + 1;
            }
        }
    }
    
    void StepBack() 
    { 
        if (!mComponents.empty())
        {
            if (mSelectedComponent.value() == 0)
            {             
                mSelectedComponent = mComponents.size() - 1;
            }
            else
            {             
                mSelectedComponent = mSelectedComponent.value() - 1;
            }
        }
    }

    Component* NewComponent(const sf::Vector2f& cursor, float gridSpacing)
    {
        assert(mSelectedComponent.has_value() && mSelectedComponent < mComponents.size());
        return mComponents[mSelectedComponent.value()]->CreateShape(cursor, gridSpacing);
    }    

    void Draw(sf::RenderTarget& target)
    {
        mComponents[mSelectedComponent.value()]->Draw(target);
    }

private:
    std::optional<size_t> mSelectedComponent;
    std::vector<std::unique_ptr<ComponentPreviewCreator>> mComponents;
};

class Application
{
public:
    Application()
        : mWindow(sf::VideoMode(sf::Vector2u(1600, 960), 32), "SFML works!", 
                                sf::Style::Default, sf::ContextSettings(0, 0, 8))        
    {
        mView = mWindow.getDefaultView();
        mHUDView = mWindow.getDefaultView();       

        mComponentPicker.AddComponent(std::make_unique<Wire>());
        mComponentPicker.AddComponent(std::make_unique<LightBulb>());
        mComponentPicker.AddComponent(std::make_unique<Battery>());
    }

    void Run()
    {
        bool isMiddleButtonPressed = false;        
        sf::Vector2i lastPanPosition;        
        
        Component* tempShape = nullptr;
        Node* selectedNode = nullptr;        

        while (mWindow.isOpen())
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(mWindow);
            float zoomIncrement = 0.0f;
            bool isLeftButtonJustReleased = false;
            bool createShape = false;

            sf::Event event;
            while (mWindow.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    mWindow.close();
                }

                // Pan
                if (event.type == sf::Event::MouseButtonPressed)
                {
                    if (event.mouseButton.button == sf::Mouse::Button::Middle)
                    {
                        lastPanPosition = mousePosition;
                        isMiddleButtonPressed = true;                        
                    }                                       
                }

                if (event.type == sf::Event::MouseButtonReleased)
                {
                    if (event.mouseButton.button == sf::Mouse::Button::Middle)
                    {
                        isMiddleButtonPressed = false;
                    }

                    if (event.mouseButton.button == sf::Mouse::Button::Left)
                    {
                        isLeftButtonJustReleased = true;
                    }
                }

                // Zoom
                if (event.type == sf::Event::MouseWheelScrolled)
                {
                    if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::Vertical)
                    {                                   
                        if (event.mouseWheelScroll.delta == 1)
                        {
                            zoomIncrement = mZoomSpeed;
                        }
                        else if (event.mouseWheelScroll.delta == -1)
                        {
                            zoomIncrement = -mZoomSpeed;
                        }   
                    }
                }

                // Shapes
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left))
                {
                    mComponentPicker.StepBack();
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right))
                {
                    mComponentPicker.StepForward();
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space))
                {
                    createShape = true;
                }
            }            

            if (zoomIncrement != 0)
            {
                sf::Vector2f mouseWorldBeforeZoom = mWindow.mapPixelToCoords(mousePosition, mView);
                mView.zoom(1.0f / mZoomFactor);  // Reset zoom factor to 1.0f
                mZoomFactor = Clamp(mZoomFactor + zoomIncrement, mZoomMin, mZoomMax);
                mView.zoom(mZoomFactor);

                sf::Vector2f mouseWorldAfterZoom = mWindow.mapPixelToCoords(mousePosition, mView);
                sf::Vector2f adjustment = mouseWorldBeforeZoom - mouseWorldAfterZoom;
                mView.move(adjustment);
            }

            if (isMiddleButtonPressed)
            {          
                // Convert the last pan position and current mouse position to world coordinates
                sf::Vector2f worldLastPanPosition = mWindow.mapPixelToCoords(lastPanPosition, mView);
                sf::Vector2f worldMousePosition = mWindow.mapPixelToCoords(mousePosition, mView);
                
                // Calculate the pan offset in world coordinates
                sf::Vector2f panOffsetWorld = worldLastPanPosition - worldMousePosition;
                mView.move(panOffsetWorld);

                // Update the last pan position (in screen coordinates)
                lastPanPosition = mousePosition;               
            }

            sf::Vector2f worldMousePosition = mWindow.mapPixelToCoords(mousePosition, mView);

            float nearestGridX = std::round(worldMousePosition.x / mGridSpacing) * mGridSpacing;
            float nearestGridY = std::round(worldMousePosition.y / mGridSpacing) * mGridSpacing;

            mCursor.x = nearestGridX;
            mCursor.y = nearestGridY;

            if (createShape)
            {
                tempShape = mComponentPicker.NewComponent(mCursor, mGridSpacing);
                selectedNode = tempShape->GetSelectedNode();
            }

            if (tempShape != nullptr)
            {
                tempShape->Move(mCursor, mGridSpacing);
            }

            if (isLeftButtonJustReleased)
            {
                if (tempShape != nullptr)
                {
                    selectedNode = tempShape->GetNextNode(mCursor);
                    if (selectedNode == nullptr)
                    {
                        tempShape->SetColor(sf::Color::White);
                        mShapes.push_back(tempShape);
                        tempShape = nullptr;
                    }
                }
                else
                {
                    selectedNode = nullptr;
                }
            }

            mWindow.setView(mView);
            mWindow.clear();
            
            sf::Vector2f viewSize = mView.getSize();
            sf::Vector2f topLeftWorld = mWindow.mapPixelToCoords({ 0, 0 }, mView);

            topLeftWorld.x = std::floor(topLeftWorld.x / mGridSpacing) * mGridSpacing;
            topLeftWorld.y = std::floor(topLeftWorld.y / mGridSpacing) * mGridSpacing;

            for (size_t gridX = 0; gridX < viewSize.x + mGridSpacing; gridX += mGridSpacing)
            {
                for (size_t gridY = 0; gridY < viewSize.y + mGridSpacing; gridY += mGridSpacing)
                {
                    DrawPoint(mWindow, { gridX + topLeftWorld.x, gridY + topLeftWorld.y }, 3.0f, sf::Color::Green);
                }
            }
            
            for (Component* shape : mShapes)
            {
                shape->DrawShape(mWindow);
                shape->DrawNodes(mWindow);
            }

            if (tempShape != nullptr)
            {
                tempShape->DrawShape(mWindow);
                tempShape->DrawNodes(mWindow);
            }

            DrawPoint(mWindow, mCursor, 3.0f, sf::Color::Yellow);
            
            mWindow.setView(mHUDView);                      
            mComponentPicker.Draw(mWindow);

            mWindow.display();           
        }
    }

private:
    float Clamp(float value, float minValue, float maxValue) 
    {
        return std::max(minValue, std::min(value, maxValue));
    }

    sf::RenderWindow mWindow;    
    sf::View mView;
    sf::View mHUDView;

    float mZoomFactor = 1.0f;
    float mZoomSpeed = 0.1f;
    float mZoomMin = 0.3f;
    float mZoomMax = 1.7f;
    float mGridSpacing = 70.0f;
    sf::Vector2f mCursor;
    std::vector<Component*> mShapes;
    ComponentPicker mComponentPicker;
};

int main()
{
    Application app;
    app.Run();

    return 0;
}