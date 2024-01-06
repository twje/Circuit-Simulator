#include <SFML/Graphics.hpp>

#include <iostream>

class Component;

void DrawPoint(sf::RenderTarget& target, sf::Vector2f position, float radius, sf::Color color)
{
    sf::CircleShape shape(radius);
    shape.setFillColor(color);
    shape.setPosition({ position.x - radius, position.y - radius });
    target.draw(shape);
}

class Node
{
public:
    Node(Component* parent, const sf::Vector2f& position)
        : mParent(parent)
        , mPosition(position)
    { }

    void SetPosition(const sf::Vector2f& position)
    {
        mPosition = position;
    }

    const sf::Vector2f& GetPosition() const { return mPosition; }

public:
    Component* mParent;
    sf::Vector2f mPosition;
};

class Component
{
public:    
    virtual ~Component() = default;

    Component(size_t maxNodes)
        : mMaxNodes(maxNodes)
        , mColor(sf::Color::Green)
    {
        mNodes.reserve(maxNodes);        
    }    

    Node* GetNextNode(const sf::Vector2f& position)
    {
        if (mNodes.size() == mMaxNodes)
        {
            return nullptr;
        }

        Node node(this, position);
        mNodes.push_back(node);

        mSelectedNode = &mNodes[mNodes.size() - 1];
        return mSelectedNode;
    }

    Node* GetSelectedNode() { return mSelectedNode; }    
    const Node& GetNode(size_t index) const { return mNodes[index]; }
    const sf::Color& GetColor() { return mColor; }

    void SetColor(const sf::Color& color) { mColor = color; }

    void DrawNodes(sf::RenderTarget& target)
    {
        for (const Node& node : mNodes)
        {
            DrawPoint(target, node.GetPosition(), 3.0f, sf::Color::Blue);
        }
    }

    virtual Component* CreateShape(const sf::Vector2f& cursor) const = 0;
    virtual void DrawShape(sf::RenderTarget& target) = 0;
    virtual void DrawIcon(sf::RenderTarget& target, const sf::Transform& transform, const sf::FloatRect& localBounds) = 0;

private:
    std::vector<Node> mNodes;
    Node* mSelectedNode{ nullptr };
    size_t mMaxNodes;
    sf::Color mColor;
};

class Wire : public Component
{
public:
    Wire()
        : Component(2)
    { }

    virtual Component* CreateShape(const sf::Vector2f& cursor) const
    {
        Component* tempShape = new Wire();
        tempShape->GetNextNode(cursor);
        tempShape->GetNextNode(cursor);

        return tempShape;
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
};

class LightBulb : public Component
{
public:
    LightBulb()
        : Component(2)
    { }

    virtual Component* CreateShape(const sf::Vector2f& cursor) const
    {
        Component* tempShape = new LightBulb();
        tempShape->GetNextNode(cursor);
        tempShape->GetNextNode(cursor);

        return tempShape;
    }

    virtual void DrawShape(sf::RenderTarget& target)
    {
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
};

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

    Component* CreateShape(const sf::Vector2f& cursor)
    {
        return mShape->CreateShape(cursor);
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

    Component* NewComponent(const sf::Vector2f& cursor)
    {
        assert(mSelectedComponent.has_value() && mSelectedComponent < mComponents.size());
        return mComponents[mSelectedComponent.value()]->CreateShape(cursor);
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
                tempShape = mComponentPicker.NewComponent(mCursor);
                selectedNode = tempShape->GetSelectedNode();
            }

            if (selectedNode != nullptr)
            {
                selectedNode->SetPosition(mCursor);
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