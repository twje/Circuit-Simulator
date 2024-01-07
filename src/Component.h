#pragma once

#include "DrawUtils.h"

class Component;

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

class Connection
{

};

class Connector
{
public:
    Connector(const sf::Vector2f position, bool isConnectable)
        : mPosition(position)
        , mIsConnectable(isConnectable)
        , mColor(sf::Color::Red)
    { }

    const sf::Vector2f& GetPosition() const { return mPosition; }
    const sf::Color& GetColor() const { return mColor; }
    
    void SetPosition(const sf::Vector2f position) { mPosition = position; }
    void SetColor(const sf::Color& color) { mColor = color; }

private:
    sf::Vector2f mPosition;
    sf::Color mColor;
    bool mIsConnectable;
};

class ConnectionConnector
{
public:
    ConnectionConnector()
        : mIsPlaceable(true)
    { }

    void SetIsPlaceable(bool value) { mIsPlaceable = false; }
    bool IsPlaceable() { return mIsPlaceable; }
    
    void AddConnectorPair(Connector* sourceConnector, Connector* targetConnector) 
    { 
        mConnectorPairs.push_back({ sourceConnector, targetConnector });
    }

private:
    bool mIsPlaceable;
    std::vector<std::pair<Connector*, Connector*>> mConnectorPairs;
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
    Node& GetNode(size_t index) { return mNodes[index]; }
    std::vector<Node>& GetNodes() { return mNodes; }

    void AddConnector(Connector&& connector) 
    { 
        mConnectors.emplace_back(std::move(connector));
    }    

    virtual bool IsConnector(sf::Vector2f cursor) { return false; }
    virtual bool IsConnectorConnectable(sf::Vector2f cursor) { return false; };    

    virtual Connector* GetConnector(sf::Vector2f pin)
    {
        for (Connector& connector : GetConnectors())
        {
            sf::Vector2f position = connector.GetPosition();
            if (pin.x == position.x && pin.y == position.y)
            {
                return &connector;
            }
        }
        return nullptr;
    }

    std::vector<Connector>& GetConnectors() { return mConnectors; }

    virtual sf::FloatRect GetGlobalBounds(float gridSpacing) { return { }; }

    const sf::Color& GetColor() { return mColor; }
    void SetColor(const sf::Color& color) { mColor = color; }    

    void CollectConnections(Component& component, ConnectionConnector& outConnectionConnector)
    {        
        for (const sf::Vector2f& pin0 : mPins)
        {
            for (const sf::Vector2f& pin1 : component.mPins)
            {
                if (pin0 != pin1)
                {
                    continue;
                }

                Connector* sourceConnector = GetConnector(pin0);
                Connector* targetConnector = component.GetConnector(pin1);

                if (sourceConnector == nullptr || targetConnector == nullptr)
                {
                    outConnectionConnector.SetIsPlaceable(false);
                }
                else
                {
                    outConnectionConnector.AddConnectorPair(sourceConnector, targetConnector);
                }
            }
        }
    }    

    std::vector<sf::Vector2f> GetContactPins(Component& component)
    {
        std::vector<sf::Vector2f> contactPins;

        for (const sf::Vector2f& pin0 : mPins)
        {
            for (const sf::Vector2f& pin1 : component.mPins)
            {
                if (pin0 == pin1)
                {
                    contactPins.push_back(pin0);
                }
            }
        }

        return contactPins;
    }

    /*bool IsPinContact(Component& component)
    {
        for (const sf::Vector2f& pin0 : mPins)
        {
            for (const sf::Vector2f& pin1 : component.mPins)
            {
                if (pin0 == pin1)
                {
                    return true;
                }
            }
        }
        return false;
    }*/

    void DrawNodes(sf::RenderTarget& target)
    {
        for (const Node& node : mNodes)
        {
            DrawPoint(target, node.GetPosition(), 3.0f, sf::Color::Blue);
        }
    }

    void DrawConnectors(sf::RenderTarget& target)
    {
        for (const Connector& connector : mConnectors)
        {
            DrawPoint(target, connector.GetPosition(), 3.0f, connector.GetColor());
        }
    }

    virtual void DrawShape(sf::RenderTarget& target) = 0;

    virtual Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing) const = 0;
    virtual void Move(const sf::Vector2f& cursor) = 0;
    virtual void DrawIcon(sf::RenderTarget& target, const sf::Transform& transform, const sf::FloatRect& localBounds) = 0;

    virtual void DebugDraw(sf::RenderTarget& target) { };

protected:
    std::vector<sf::Vector2f> mPins;
    std::vector<Connector> mConnectors;

private:    
    std::vector<Node> mNodes;    

    Node* mSelectedNode{ nullptr };
    size_t mMaxNodes;
    sf::Color mColor;
};