#include <SFML/Graphics.hpp>

#include <iostream>

class Shape;

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
    Node(Shape* parent, const sf::Vector2f& position)
        : mParent(parent)
        , mPosition(position)
    { }

    void SetPosition(const sf::Vector2f& position)
    {
        mPosition = position;
    }

    const sf::Vector2f& GetPosition() const { return mPosition; }

public:
    Shape* mParent;
    sf::Vector2f mPosition;
};

class Shape
{
public:    
    virtual ~Shape() = default;

    Shape(size_t maxNodes)
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
        return &mNodes[mNodes.size() - 1];
    }

    size_t NodeCount() const { return mNodes.size(); }
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

    virtual void DrawShape(sf::RenderTarget& target) = 0;

private:
    std::vector<Node> mNodes;
    size_t mMaxNodes;
    sf::Color mColor;
};

class Line : public Shape
{
public:
    Line()
        : Shape(2)
    { }

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
};

class Box : public Shape
{
public:
    Box()
        : Shape(2)
    { }

    virtual void DrawShape(sf::RenderTarget& target)
    {
        const sf::Vector2f& position1 = GetNode(0).GetPosition();
        const sf::Vector2f& position2 = GetNode(1).GetPosition();
        sf::Vector2f size = position2 - position1;

        sf::RectangleShape rectangle(size);
        rectangle.setPosition(position1);        
        rectangle.setFillColor({ 0, 0, 0, 0 });
        rectangle.setOutlineColor(GetColor());
        rectangle.setOutlineThickness(-1.f);

        target.draw(rectangle);
    }
};

class Circle : public Shape
{
public:
    Circle()
        : Shape(2)
    { }

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
};

class Curve : public Shape
{
public:
    Curve()
        : Shape(3)
    { }

    virtual void DrawShape(sf::RenderTarget& target)
    {
        // Can only draw line from first to second
        if (NodeCount() < 3)
        {
            const sf::Vector2f& position1 = GetNode(0).GetPosition();
            const sf::Vector2f& position2 = GetNode(1).GetPosition();

            sf::Vertex line[] = { position1, position2 };

            line[0].color = GetColor();
            line[1].color = GetColor();

            target.draw(line, 2, sf::PrimitiveType::Lines);
        }

        if (NodeCount() == 3)
        {
            const sf::Vector2f& position1 = GetNode(0).GetPosition();
            const sf::Vector2f& position2 = GetNode(1).GetPosition();
            const sf::Vector2f& position3 = GetNode(2).GetPosition();

            sf::Vertex line1[] = { position1, position2 };
            line1[0].color = GetColor();
            line1[1].color = GetColor();

            sf::Vertex line2[] = { position2, position3 };
            line2[0].color = GetColor();
            line2[1].color = GetColor();
            
            target.draw(line1, 2, sf::PrimitiveType::Lines);
            target.draw(line2, 2, sf::PrimitiveType::Lines);

            // Bezier 
            sf::Vector2f op = position1;
            sf::Vector2f np = op;
            for (float t = 0; t < 1.0f; t += 0.01f)
            {
                np = (1 - t) * (1 - t) * position1 + 2 * (1 - t) * t * position2 + t * t * position3;
                sf::Vertex line[] = { op, np };
                line[0].color = GetColor();
                line[1].color = GetColor();
                target.draw(line, 2, sf::PrimitiveType::Lines);
                op = np;
            }
        }
    }
};

enum class ShapeType
{
    NONE = 0,
    LINE = 1,
    BOX = 2,
    CIRCLE = 3,
    CURVE = 4
};

class Application
{
public:
    Application()
        : mWindow(sf::VideoMode(sf::Vector2u(1600, 960), 32), "SFML works!", 
                                sf::Style::Default, sf::ContextSettings(0, 0, 8))
    {
        mView = mWindow.getDefaultView();
    }

    void Run()
    {
        bool isMiddleButtonPressed = false;        
        sf::Vector2i lastPanPosition;        
        
        Shape* tempShape = nullptr;
        Node* selectedNode = nullptr;
        ShapeType createShape = ShapeType::NONE;

        while (mWindow.isOpen())
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(mWindow);
            float zoomIncrement = 0.0f;
            bool isLeftButtonJustReleased = false;

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
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L))
                {
                    createShape = ShapeType::LINE;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::B))
                {
                    createShape = ShapeType::BOX;
                }

                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::C))
                {
                    createShape = ShapeType::CIRCLE;
                }
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S))
                {
                    createShape = ShapeType::CURVE;
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

            if (createShape != ShapeType::NONE)
            {
                switch (createShape)
                {
                    case ShapeType::LINE:
                    {
                        tempShape = new Line();
                        selectedNode = tempShape->GetNextNode(mCursor);
                        selectedNode = tempShape->GetNextNode(mCursor);
                        break;
                    }
                    case ShapeType::BOX:
                    {
                        tempShape = new Box();
                        selectedNode = tempShape->GetNextNode(mCursor);
                        selectedNode = tempShape->GetNextNode(mCursor);
                        break;
                    }
                    case ShapeType::CIRCLE:
                    {
                        tempShape = new Circle();
                        selectedNode = tempShape->GetNextNode(mCursor);
                        selectedNode = tempShape->GetNextNode(mCursor);
                        break;
                    }
                    case ShapeType::CURVE:
                    {
                        tempShape = new Curve();
                        selectedNode = tempShape->GetNextNode(mCursor);
                        selectedNode = tempShape->GetNextNode(mCursor);
                        break;
                    }
                }
                createShape = ShapeType::NONE;
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
            
            for (Shape* shape : mShapes)
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
    float mZoomFactor = 1.0f;
    float mZoomSpeed = 0.1f;
    float mZoomMin = 0.3f;
    float mZoomMax = 1.7f;
    float mGridSpacing = 70.0f;
    sf::Vector2f mCursor;
    std::vector<Shape*> mShapes;
};

int main()
{
    Application app;
    app.Run();

    return 0;
}