#include "Component.h"
#include "Battery.h"
#include "LightBulb.h"
#include "Wire.h"
#include "DrawUtils.h"
#include "Interfaces.h"

#include <SFML/Graphics.hpp>

#include <iostream>

class ComponentFactory : public sf::Transformable
{
public:
    ComponentFactory(std::unique_ptr<Component> shape)
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

    Component* CreateShape(ICircuitBoardNavigator* navigator)
    {
        return mShape->CreateShape(navigator);
    }

private:
    std::unique_ptr<Component> mShape;
};

class ComponentPicker
{
public:
    void Subscribe(IComponentPickerObserver* observer)
    {
        mObservers.push_back(observer);
    }

    void AddComponent(std::unique_ptr<Component> component)
    {
        if (mFactories.empty())
        {
            mSelectedComponent = 0;
        }
        mFactories.emplace_back(std::make_unique<ComponentFactory>(std::move(component)));
    }

    void StepForward() 
    { 
        if (!mFactories.empty())
        {
            if (mSelectedComponent.value() + 1 >= mFactories.size())
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
        if (!mFactories.empty())
        {
            if (mSelectedComponent.value() == 0)
            {             
                mSelectedComponent = mFactories.size() - 1;
            }
            else
            {             
                mSelectedComponent = mSelectedComponent.value() - 1;
            }
        }
    }

    void CreateNewComponent()
    {
        assert(mSelectedComponent.has_value() && mSelectedComponent < mFactories.size());
        ComponentFactory* factory = mFactories[mSelectedComponent.value()].get();

        for (IComponentPickerObserver* observer : mObservers)
        {
            observer->OnCreateNewComponent(factory);
        }
    }

    void Draw(sf::RenderTarget& target)
    {
        mFactories[mSelectedComponent.value()]->Draw(target);
    }

private:
    std::optional<size_t> mSelectedComponent;
    std::vector<std::unique_ptr<ComponentFactory>> mFactories;
    std::vector<IComponentPickerObserver*> mObservers;
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

        for (Component* component : mComponents)
        {
            component->DrawComponent(target);
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
    void SetCircuitBoard(CircuitBoard* circuitBoard)
    {
        mCircuitBoard = circuitBoard;
    }

    void CreateComponent(Component* newComponent)
    {        
        mNewComponent = newComponent;
        mCntPin = mNewComponent->GetSelectedNode();
    }

    void MoveComponent()
    {        
        assert(mCircuitBoard);
        if (mNewComponent != nullptr)
        {
            mNewComponent->Move();
            mConnectionConnector = mCircuitBoard->CollectConnections(mNewComponent);
        }
    }

    Component* TryPlaceComponent()
    {        
        assert(mCircuitBoard);
        Component* placedComponent = nullptr;

        if (mNewComponent != nullptr)
        {
            mPrvPin = mNewComponent->GetSelectedNode();
            mCntPin = mNewComponent->GetNextNode();

            if (mCntPin == nullptr)
            {
                if (mConnectionConnector.IsPlaceable())
                {
                    placedComponent = mNewComponent;
                    mConnectionConnector.Connect();                                        
                    mCircuitBoard->AddComponent(mNewComponent);
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
    ConnectionConnector mConnectionConnector;
    CircuitBoard* mCircuitBoard{ nullptr };
    Component* mNewComponent{ nullptr };
    Node* mPrvPin{ nullptr };
    Node* mCntPin{ nullptr };
};

class CircuitBoardController : public IComponentPickerObserver
{
public:
    CircuitBoardController()
    {
        mCircuitBoardManipulator.SetCircuitBoard(&mCircuitBoard);
    }

    void Update(sf::Vector2f cursorWorldCoord)
    {
        mCircuitBoard.UpdateSelectedPin(cursorWorldCoord);

        mCircuitBoardManipulator.MoveComponent();

        if (mCircuitBoardManipulator.IsManipulatingComponent())
        {
            if (mCircuitBoardManipulator.IsComponentPlaceable())
            {
                mCircuitBoardManipulator.SetComponentColor(sf::Color::Magenta);
            }
            else
            {
                mCircuitBoardManipulator.SetComponentColor(sf::Color::Cyan);
            }
        }
    }

    void TryPlaceComponent()
    {
        if (Component* component = mCircuitBoardManipulator.TryPlaceComponent())
        {
            component->SetColor(sf::Color::White);
        }
    }

    void Draw(sf::RenderTarget& target)
    {
        mCircuitBoard.Draw(target);
        mCircuitBoardManipulator.Draw(target);
    }

    // IComponentPickerObserver interface
    virtual void OnCreateNewComponent(ComponentFactory* factory) override
    {
        Component* newComponent = factory->CreateShape(&mCircuitBoard);
        mCircuitBoardManipulator.CreateComponent(newComponent);
    }

private:
    CircuitBoard mCircuitBoard;
    CircuitBoardManipulator mCircuitBoardManipulator;
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

        mViewController = std::make_unique<ViewController>(mWindow, mView);
        mComponentPicker.Subscribe(&mCircuitBoardController);
        mComponentPicker.AddComponent(std::make_unique<LightBulb>());
    }

    void Run()
    {
        bool isMiddleButtonPressed = false;

        while (mWindow.isOpen())
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(mWindow);            
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

            if (createShape)
            {
                mComponentPicker.CreateNewComponent();
            }

            sf::Vector2f cursorWorldCoord = mViewController->MapPixelToCoords(mousePosition);
            mCircuitBoardController.Update(cursorWorldCoord);

            if (isLeftButtonJustReleased)
            {
                mCircuitBoardController.TryPlaceComponent();
            }
            
            mWindow.setView(mView);
            mWindow.clear();
                        
            mCircuitBoardController.Draw(mWindow);

            // DRAW BOUNDs
            mWindow.setView(mHUDView);
            mComponentPicker.Draw(mWindow);

            mWindow.display();                 
        }
    }

private:   
    CircuitBoardController mCircuitBoardController;
    ComponentPicker mComponentPicker;    
    std::unique_ptr<ViewController> mViewController;

    sf::RenderWindow mWindow;    
    sf::View mView;
    sf::View mHUDView;    
};

int main()
{
    Application app;
    app.Run();

    return 0;
}