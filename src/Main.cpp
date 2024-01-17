#include "Component.h"
#include "Battery.h"
#include "LightBulb.h"
#include "Wire.h"
#include "DrawUtils.h"
#include "Interfaces.h"

#include <SFML/Graphics.hpp>

#include <iostream>

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

    Component* CreateShape(ICircuitBoardNavigator& navigator)
    {
        return mShape->CreateShape(navigator);
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

    Component* NewComponent(ICircuitBoardNavigator& navigator) const
    {
        assert(mSelectedComponent.has_value() && mSelectedComponent < mComponents.size());
        return mComponents[mSelectedComponent.value()]->CreateShape(navigator);
    }    

    void Draw(sf::RenderTarget& target)
    {
        mComponents[mSelectedComponent.value()]->Draw(target);
    }

private:
    std::optional<size_t> mSelectedComponent;
    std::vector<std::unique_ptr<ComponentPreviewCreator>> mComponents;
};

class CircuitBoard : public ICircuitBoardNavigator
{
public:
    CircuitBoard()
        : mSelectedPin(nullptr)
        , mGrid(10, 10)
        , mGridSpacing(25)
    {
        assert(mGrid.x > 1 && mGrid.y > 1);
        for (uint32_t index = 0; index < TotalPins(); index++)
        {
            mPins.emplace_back(index);
        }
        mSelectedPin = &mPins.at(0);
    }

    void AddComponent(Component* component)
    {
        mComponents.push_back(component);
    }

    ConnectionConnector CollectConnections(const Component* newComponent)
    {
        ConnectionConnector connectionConnector;
        for (Component* component : mComponents)
        {
            newComponent->CollectConnections(*component, connectionConnector);
        }
        return connectionConnector;
    }

    void UpdateSelectedPin(sf::Vector2f cursorWorldCoord)
    {
        float nearestGridX = std::round(cursorWorldCoord.x / mGridSpacing) * mGridSpacing;
        float nearestGridY = std::round(cursorWorldCoord.y / mGridSpacing) * mGridSpacing;

        nearestGridX = std::clamp(nearestGridX, 0.0f, (mGrid.x - 1) * mGridSpacing);
        nearestGridY = std::clamp(nearestGridY, 0.0f, (mGrid.y - 1) * mGridSpacing);

        uint32_t indexX = nearestGridX / mGridSpacing;
        uint32_t indexY = nearestGridY / mGridSpacing;

        mSelectedPin = &mPins.at(Get1DPinIndex(indexX, indexY));
    }

    void Draw(sf::RenderTarget& target)
    {
        for (uint32_t indexY = 0; indexY < mGrid.y; indexY++)
        {
            for (uint32_t indexX = 0; indexX < mGrid.x; indexX++)
            {
                sf::Vector2f position = sf::Vector2f(indexX, indexY) * mGridSpacing;
                DrawPoint(target, position, 4, sf::Color::Cyan);
            }
        }



        if (mSelectedPin)
        {
            DrawPoint(target, GetGridCoordinateFromPin(*mSelectedPin), 4, sf::Color::White);
        }
    }

private:
    // ICircuitBoardNavigator interface
    virtual Pin& GetSelectedPin()
    {
        return *mSelectedPin;
    }

    virtual Pin* GetSurroundingPin(Pin& pin, sf::Vector2i offset)  // rename to neahbor
    {
        sf::Vector2i newIndex = sf::Vector2i(Get2DPinIndex(pin.GetId()));
        newIndex += offset;
        if (newIndex.x < 0 || newIndex.x >= mGrid.x || newIndex.y < 0 || newIndex.y >= mGrid.y)
        {
            return nullptr;
        }
        return &mPins.at(Get1DPinIndex(static_cast<uint32_t>(newIndex.x), static_cast<uint32_t>(newIndex.y)));
    }

    virtual sf::Vector2f GetGridCoordinateFromPin(Pin& pin)
    {
        sf::Vector2u index = Get2DPinIndex(pin.GetId());
        return sf::Vector2f(index) * mGridSpacing;
    }

    uint32_t Get1DPinIndex(uint32_t xIndex, uint32_t yIndex)
    {
        return xIndex + yIndex * mGrid.x;
    }

    sf::Vector2u Get2DPinIndex(uint32_t index)
    {
        uint32_t xIndex = index % mGrid.y;
        uint32_t yIndex = index / mGrid.y;
        return { xIndex, yIndex };
    }

    uint32_t TotalPins()
    {
        return mGrid.x * mGrid.y;
    }

    std::vector<Component*> mComponents;
    Pin* mSelectedPin;
    std::vector<Pin> mPins;
    sf::Vector2i mGrid;
    float mGridSpacing;
};

class CircuitBoardManipulator
{
public:
    CircuitBoardManipulator(CircuitBoard& circuitBoard)
        : mCircuitBoard(circuitBoard)
    { }

    void CreateComponent(ICircuitBoardNavigator& navigator, const ComponentPicker& componentPicker)
    {        
        mNewComponent = componentPicker.NewComponent(navigator);
        mCntPin = mNewComponent->GetSelectedNode();
    }

    void MoveComponent()
    {        
        if (mNewComponent != nullptr)
        {
            mNewComponent->Move();
            mConnectionConnector = mCircuitBoard.CollectConnections(mNewComponent);
        }
    }

    Component* TryPlaceComponent(const sf::Vector2f& cursor)
    {        
        Component* placedComponent = nullptr;

        if (mNewComponent != nullptr)
        {
            mPrvPin = mNewComponent->GetSelectedNode();
            mCntPin = mNewComponent->GetNextNode(cursor);

            if (mCntPin == nullptr)
            {
                if (mConnectionConnector.IsPlaceable())
                {
                    placedComponent = mNewComponent;
                    mConnectionConnector.Connect();                                        
                    mCircuitBoard.AddComponent(mNewComponent);
                    mNewComponent = nullptr;                    
                }
                else
                {
                    mCntPin = mPrvPin;
                }
            }
        }

        return placedComponent;
    }    

    void Draw(sf::RenderTarget& target)
    {
        if (mNewComponent)
        {
            mNewComponent->DrawComponent(target);
            mNewComponent->DebugDraw(target);
        }
    }    

    void SetComponentColor(const sf::Color& color)
    {
        if (mNewComponent)
        {
            mNewComponent->SetColor(color);
        }
    }

    bool IsManipulatingComponent() 
    { 
        return mNewComponent != nullptr;  
    }
    
    bool IsComponentPlaceable() 
    { 
        return mConnectionConnector.IsPlaceable(); 
    }

private:
    CircuitBoard& mCircuitBoard;
    ConnectionConnector mConnectionConnector;
    Component* mNewComponent{ nullptr };
    Node* mPrvPin{ nullptr };
    Node* mCntPin{ nullptr };
};



class ViewController
{
public:
    ViewController(sf::RenderWindow& window, sf::View& view)
        : mWindow(window)
        , mView(view)
        , mZoomFactor(1.0f)
        , mZoomSpeed(0.1f)
        , mZoomMin(0.3f)
        , mZoomMax(1.7f)
    { }

    void StartPan(const sf::Vector2i& mousePosition)
    {
        mLastPanPosition = mousePosition;
    }

    void UpdatePan()
    {        
        sf::Vector2i mousePosition = sf::Mouse::getPosition(mWindow);

        // Convert the last pan position and current mouse position to world coordinates
        sf::Vector2f worldLastPanPosition = mWindow.mapPixelToCoords(mLastPanPosition, mView);
        sf::Vector2f worldMousePosition = mWindow.mapPixelToCoords(mousePosition, mView);

        // Calculate the pan offset in world coordinates
        sf::Vector2f panOffsetWorld = worldLastPanPosition - worldMousePosition;
        mView.move(panOffsetWorld);

        // Update the last pan position (in screen coordinates)
        mLastPanPosition = mousePosition;     
    }

    void ZoomIn() { Zoom(-mZoomSpeed); }
    void ZoomOut() { Zoom(mZoomSpeed); }

    sf::Vector2f MapPixelToCoords(sf::Vector2i pixel)
    {        
        return mWindow.mapPixelToCoords(pixel, mView);
    }

private:
    void Zoom(float zoomSpeed)
    {
        sf::Vector2i mousePosition = sf::Mouse::getPosition(mWindow);

        // Zoom 
        sf::Vector2f mouseWorldBeforeZoom = mWindow.mapPixelToCoords(mousePosition, mView);
        mView.zoom(1.0f / mZoomFactor);  // Reset zoom factor to 1.0f
        mZoomFactor = Clamp(mZoomFactor + zoomSpeed, mZoomMin, mZoomMax);
        mView.zoom(mZoomFactor);

        // Ensure same pixel is zoomed in/out on
        sf::Vector2f mouseWorldAfterZoom = mWindow.mapPixelToCoords(mousePosition, mView);
        sf::Vector2f adjustment = mouseWorldBeforeZoom - mouseWorldAfterZoom;
        mView.move(adjustment);
    }

    float Clamp(float value, float minValue, float maxValue)
    {
        return std::max(minValue, std::min(value, maxValue));
    }

    sf::RenderWindow& mWindow;
    sf::View& mView;
    float mZoomFactor;
    float mZoomSpeed;
    float mZoomMin;
    float mZoomMax;
    sf::Vector2i mLastPanPosition;    
};

class Application
{
public:
    Application()
        : mWindow(sf::VideoMode(sf::Vector2u(1600, 960), 32), "SFML works!")        
    {
        

        mView = mWindow.getDefaultView();
        mHUDView = mWindow.getDefaultView();       

        //mComponentPicker.AddComponent(std::make_unique<Wire>(sf::Vector2f(), mGridSpacing));
        mComponentPicker.AddComponent(std::make_unique<LightBulb>(mCircuitBoard));
        //mComponentPicker.AddComponent(std::make_unique<Battery>());

        mCircuitBoardManipulator = std::make_unique<CircuitBoardManipulator>(mCircuitBoard);
        mViewController = std::make_unique<ViewController>(mWindow, mView);
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
                        mViewController->StartPan(mousePosition);
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
                            mViewController->ZoomIn();
                        }
                        else if (event.mouseWheelScroll.delta == -1)
                        {
                            mViewController->ZoomOut();                            
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

            if (isMiddleButtonPressed)
            {
                mViewController->UpdatePan();
            }

            //mCursor = GetNearestGridCoordinate(mousePosition);

            // Manipulate component
            if (createShape)
            {
                mCircuitBoardManipulator->CreateComponent(mCircuitBoard, mComponentPicker);
            }

            mCircuitBoardManipulator->MoveComponent();

            if (mCircuitBoardManipulator->IsManipulatingComponent())
            {
                if (mCircuitBoardManipulator->IsComponentPlaceable())
                {
                    mCircuitBoardManipulator->SetComponentColor(sf::Color::Magenta);
                }
                else
                {
                    mCircuitBoardManipulator->SetComponentColor(sf::Color::Cyan);
                }
            }

            if (isLeftButtonJustReleased)
            {
                if (Component* component = mCircuitBoardManipulator->TryPlaceComponent(mCursor))
                {
                    component->SetColor(sf::Color::White);
                }
            }

            mWindow.setView(mView);
            mWindow.clear();
                        
            sf::FloatRect viewBounds = sf::FloatRect(mView.getCenter() - mView.getSize() / 2.0f,
                                                     mView.getSize());
            
            
            mCircuitBoard.Draw(mWindow);
            mCircuitBoardManipulator->Draw(mWindow);
            DrawPoint(mWindow, mCursor, 3.0f, sf::Color::Yellow);
            
            sf::Vector2f cursorWorldCoord = mViewController->MapPixelToCoords(mousePosition);
            mCircuitBoard.UpdateSelectedPin(cursorWorldCoord);
            mCircuitBoard.Draw(mWindow);

            // DRAW BOUNDs            
            mWindow.setView(mHUDView);
            mComponentPicker.Draw(mWindow);

            mWindow.display();                 
        }
    }

private:   
    ComponentPicker mComponentPicker;
    CircuitBoard mCircuitBoard;
    std::unique_ptr<CircuitBoardManipulator> mCircuitBoardManipulator;
    std::unique_ptr<ViewController> mViewController;

    sf::RenderWindow mWindow;    
    sf::View mView;
    sf::View mHUDView;

    float mGridSpacing = 70.0f;
    sf::Vector2f mCursor;
};

int main()
{
    Application app;
    app.Run();

    return 0;
}