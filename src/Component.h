#pragma once

#include "DrawUtils.h"

class Component;
class Connection;

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

class Connector
{
public:
    Connector(Component* component)
        : mComponent(component)
    { }

    const sf::Vector2f& GetPosition() const { return mPosition; }
    void SetPosition(const sf::Vector2f position) { mPosition = position; }    
    void AddConnection(std::shared_ptr<Connection> connection) 
    { 
        mConnections.emplace_back(connection);
    }    

    std::vector<std::shared_ptr<Connection>>& GetConnections()
    {
        return mConnections;
    }    

    Component* GetComponent() { return mComponent; }

private:
    Component* mComponent;
    std::vector<std::shared_ptr<Connection>> mConnections;    
    sf::Vector2f mPosition;
};

class Connection
{
public:
    Connection(Connector* sourceConnector, Connector* targetConnector)
        : mSourceConnector(sourceConnector)
        , mTargetConnector(targetConnector)
    { }

    Connector* GetOppositeConnector(Connector* connector)
    {
        if (mSourceConnector == connector)
        {
            return mTargetConnector;
        }
        return mSourceConnector;
    }

    bool IsComponentConnected(Component* component)
    {
        return (component == mSourceConnector->GetComponent()) || (component == mTargetConnector->GetComponent());
    }

private:
    Connector* mSourceConnector;
    Connector* mTargetConnector;
};

class ConnectionConnector
{
public:
    ConnectionConnector()
        : mIsPlaceable(true)
    { }

    void Connect()
    {
        for (const auto& connectorPair : mConnectorPairs)
        {
            Connector* sourceConnector = connectorPair.first;
            Connector* targetConnector = connectorPair.second;

            auto connection = std::make_shared<Connection>(sourceConnector, targetConnector);
            sourceConnector->AddConnection(connection);
            targetConnector->AddConnection(connection);
        }
    }

    void SetIsPlaceable(bool value) { mIsPlaceable = false; }
    bool IsPlaceable() { return mIsPlaceable; }
    
    void AddConnectorPair(Connector* sourceConnector, Connector* targetConnector) 
    { 
        mConnectorPairs.push_back({ sourceConnector, targetConnector });
    }

    void Reset()
    {
        mConnectorPairs.clear();
        mIsPlaceable = true;
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

    void AddConnector()
    { 
        mConnectors.emplace_back(std::make_unique<Connector>(this));
    }
     
    const std::vector<std::unique_ptr<Connector>>& GetConnectors() const
    { 
        return mConnectors; 
    }

    void SetColor(const sf::Color& color) { mColor = color; }    

    void CollectConnections(Component& component, ConnectionConnector& outConnectionConnector) const
    {        
        for (const sf::Vector2f& pin0 : mPins)
        {
            for (const sf::Vector2f& pin1 : component.mPins)
            {
                if (pin0 != pin1)
                {
                    continue;
                }

                Connector* sourceConnector = GetConnectorAtPin(pin0);
                Connector* targetConnector = component.GetConnectorAtPin(pin1);

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

    virtual Connector* GetConnectorAtPin(sf::Vector2f pin) const
    {
        for (auto& connectorPtr : mConnectors)
        {
            sf::Vector2f position = connectorPtr->GetPosition();
            if (pin.x == position.x && pin.y == position.y)
            {
                return connectorPtr.get();
            }
        }
        return nullptr;
    }    

    virtual Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing) const = 0;
    virtual void Move(const sf::Vector2f& cursor) = 0;
    
    virtual void DrawComponent(sf::RenderTarget& target) = 0;
    virtual void DrawIcon(sf::RenderTarget& target, const sf::Transform& transform, const sf::FloatRect& localBounds) = 0;
    virtual void DebugDraw(sf::RenderTarget& target) { };

protected:
    std::vector<sf::Vector2f> mPins;
    std::vector<std::unique_ptr<Connector>> mConnectors;
    sf::Color mColor;

private:    
    std::vector<Node> mNodes;    
    Node* mSelectedNode{ nullptr };
    size_t mMaxNodes;
};