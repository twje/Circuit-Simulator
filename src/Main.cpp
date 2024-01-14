#include "Component.h"
#include "Battery.h"
#include "LightBulb.h"
#include "Wire.h"
#include "DrawUtils.h"

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

    Component* NewComponent(const sf::Vector2f& cursor, float gridSpacing) const
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

class CircuitBoard
{
public:
    CircuitBoard()
        : mPinCount(100, 100)
    { }

    ConnectionConnector CollectConnections(const Component* newComponent)
    {
        ConnectionConnector connectionConnector;
        for (Component* component : mComponents)
        {
            newComponent->CollectConnections(*component, connectionConnector);
        }
        return connectionConnector;
    }

    void AddComponent(Component* component)
    {
        mComponents.push_back(component);
    }

    Component* GetLastAddedComponent()
    {
        return mComponents.back();
    }

    void Draw(sf::RenderTarget& target)
    {
        for (Component* component : mComponents)
        {
            component->DrawComponent(target);
        }
    }

    const sf::Vector2i& GetPinCount() { return mPinCount; }

private:
    std::vector<Component*> mComponents;
    sf::Vector2i mPinCount;
};

class CircuitBoardManipulator
{
public:
    CircuitBoardManipulator(CircuitBoard& circuitBoard)
        : mCircuitBoard(circuitBoard)
    { }

    void CreateComponent(const ComponentPicker& componentPicker, const sf::Vector2f& cursor, float gridSpacing)
    {        
        mNewComponent = componentPicker.NewComponent(cursor, gridSpacing);
        mCntPin = mNewComponent->GetSelectedNode();
    }

    void MoveComponent(const sf::Vector2f& cursor)
    {        
        if (mNewComponent != nullptr)
        {
            mNewComponent->Move(cursor);
            mConnectionConnector = mCircuitBoard.CollectConnections(mNewComponent);
        }
    }

    Component* TryPlaceComponent(const sf::Vector2f& cursor)
    {        
        if (mNewComponent != nullptr)
        {
            mPrvPin = mNewComponent->GetSelectedNode();
            mCntPin = mNewComponent->GetNextNode(cursor);

            if (mCntPin == nullptr)
            {
                if (mConnectionConnector.IsPlaceable())
                {
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

        if (mNewComponent == nullptr)
        {
            return mCircuitBoard.GetLastAddedComponent();
        }
        return nullptr;
    }    

    void Draw(sf::RenderTarget& target)
    {
        if (mNewComponent)
        {
            mNewComponent->DrawComponent(target);
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
    ViewController(sf::RenderWindow& window, sf::View& view, sf::FloatRect bounds)
        : mWindow(window)
        , mBounds(bounds)
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

    void DrawDebug()
    {
        sf::RectangleShape shape;
        shape.setPosition(mBounds.getPosition());
        shape.setSize(mBounds.getSize());
        shape.setFillColor(sf::Color::Transparent);
        shape.setOutlineColor(sf::Color::Red);
        shape.setOutlineThickness(-5);

        mWindow.draw(shape);
    }

    const sf::FloatRect& GetBounds() const { return mBounds; }

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
    sf::FloatRect mBounds;
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
        : mWindow(sf::VideoMode(sf::Vector2u(1600, 960), 32), "SFML works!", 
                                sf::Style::Default, sf::ContextSettings(0, 0, 8))        
    {
        mView = mWindow.getDefaultView();
        mHUDView = mWindow.getDefaultView();       

        mComponentPicker.AddComponent(std::make_unique<Wire>(sf::Vector2f(), mGridSpacing));
        mComponentPicker.AddComponent(std::make_unique<LightBulb>(mGridSpacing));
        mComponentPicker.AddComponent(std::make_unique<Battery>());

        mCircuitBoardManipulator = std::make_unique<CircuitBoardManipulator>(mCircuitBoard);
        
        sf::FloatRect bounds(sf::Vector2f(), 
                             sf::Vector2f(mCircuitBoard.GetPinCount()) * mGridSpacing);

        mViewController = std::make_unique<ViewController>(mWindow, mView, bounds);
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

            mCursor = GetNearestGridCoordinate(mousePosition);

            // Manipulate component
            if (createShape)
            {
                mCircuitBoardManipulator->CreateComponent(mComponentPicker, mCursor, mGridSpacing);
            }

            mCircuitBoardManipulator->MoveComponent(mCursor);

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
            
            const sf::FloatRect& bounds = mViewController->GetBounds();
            sf::FloatRect viewBounds = sf::FloatRect(mView.getCenter() - mView.getSize() / 2.0f,
                                                     mView.getSize());

            for (float gridX = 0; gridX <= bounds.getSize().x; gridX += mGridSpacing)
            {
                for (float gridY = 0; gridY <= bounds.getSize().y; gridY += mGridSpacing)
                {
                    sf::Vector2f point(gridX, gridY);                    
                    if (viewBounds.contains(point))
                    {
                        DrawPoint(mWindow, point, 3.0f, sf::Color::Green);
                    }                    
                }
            }

            mViewController->DrawDebug();
            mCircuitBoard.Draw(mWindow);
            mCircuitBoardManipulator->Draw(mWindow);
            DrawPoint(mWindow, mCursor, 3.0f, sf::Color::Yellow);
            
            // DRAW BOUNDs            
            mWindow.setView(mHUDView);
            mComponentPicker.Draw(mWindow);

            mWindow.display();           
        }
    }

private:
    sf::Vector2f GetGridCoordinateFromPinId(const sf::Vector2f& pinId)
    {
        const sf::FloatRect& bounds = mViewController->GetBounds();

        // Convert pinId back to world coordinates
        sf::Vector2f worldCoord;
        worldCoord.x = pinId.x * mGridSpacing + bounds.left;
        worldCoord.y = pinId.y * mGridSpacing + bounds.top;

        // Ensure the world coordinates are clamped within the bounds
        worldCoord.x = std::clamp(worldCoord.x, bounds.left, bounds.left + bounds.width);
        worldCoord.y = std::clamp(worldCoord.y, bounds.top, bounds.top + bounds.height);

        return worldCoord;
    }

    sf::Vector2f GetPinId(sf::Vector2i screenCoord)
    {
        sf::Vector2f nearestGridCoord = GetNearestGridCoordinate(screenCoord);        
        const sf::FloatRect& bounds = mViewController->GetBounds();
                
        return (nearestGridCoord - bounds.getPosition()) / mGridSpacing;
    }

    sf::Vector2f GetNearestGridCoordinate(sf::Vector2i screenCoord)
    {
        const sf::FloatRect& bounds = mViewController->GetBounds();
        sf::Vector2f worldMousePosition = mViewController->MapPixelToCoords(screenCoord);

        float nearestGridX = std::round(worldMousePosition.x / mGridSpacing) * mGridSpacing;
        float nearestGridY = std::round(worldMousePosition.y / mGridSpacing) * mGridSpacing;

        // Clamping nearestGridX and nearestGridY within bounds
        nearestGridX = std::clamp(nearestGridX, bounds.left, bounds.left + bounds.width);
        nearestGridY = std::clamp(nearestGridY, bounds.top, bounds.top + bounds.height);

        return { nearestGridX, nearestGridY };
    }

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