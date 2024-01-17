#pragma once

#include "DrawUtils.h"
#include "Interfaces.h"


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

class Pin
{
public:
    Pin(uint32_t id)
        : mId(id)
    { }

    uint32_t GetId() { return mId; }

private:
    uint32_t mId;
};

class ComponentPin
{
public:
    ComponentPin(uint32_t id, bool isConnectable)
        : mPin(id)
        , mTemporaryConnectionPin(nullptr)
        , mIsConnectable(isConnectable)
    { }

    // Getters
    Pin* GetTemporaryConnectionPin() { return mTemporaryConnectionPin; }
    bool IsConnectable() { return mIsConnectable; }

    // Setters
    void SetTemporaryConnectionPin(Pin* pin) { mTemporaryConnectionPin = pin; }

private:
    Pin mPin;
    Pin* mTemporaryConnectionPin;
    bool mIsConnectable;
};

class Component
{
public:
    Component() = default;
    Component(ICircuitBoardNavigator* navigator, size_t maxNodes)
        : mNavigator(navigator)
        , mMaxNodes(maxNodes)
        , mColor(sf::Color::Green)
    {
        mNodes.reserve(maxNodes);
    }
    
    virtual ~Component() = default;

    Node* GetNextNode()
    {
        if (mNodes.size() == mMaxNodes)
        {
            return nullptr;
        }
        
        sf::Vector2f position = mNavigator->GetGridCoordinateFromPin(mNavigator->GetSelectedPin());
        Node node(this, position);
        mNodes.push_back(node);

        mSelectedNode = &mNodes[mNodes.size() - 1];
        return mSelectedNode;
    }

    Node* GetSelectedNode() { return mSelectedNode; }
    Node& GetNode(size_t index) { return mNodes[index]; }
    std::vector<Node>& GetNodes() { return mNodes; }

    void SetColor(const sf::Color& color) { mColor = color; }    

    void CollectConnections(Component& component, ConnectionConnector& outConnectionConnector) const
    {   
        /*
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
        */
    }     

    virtual Connector* GetConnectorAtPin(sf::Vector2f pin) const
    {
        return nullptr;

        /*
        for (auto& connectorPtr : mConnectors)
        {
            sf::Vector2f position = connectorPtr->GetPosition();
            if (pin.x == position.x && pin.y == position.y)
            {
                return connectorPtr.get();
            }
        }
        return nullptr;
        */
    }

    virtual Component* CreateShape(ICircuitBoardNavigator* navigator) const = 0;
    
    // 
    virtual void Move() = 0;
    
    // Draing
    virtual void DrawComponent(sf::RenderTarget& target) = 0;
    virtual void DrawIcon(sf::RenderTarget& target, const sf::Transform& transform, const sf::FloatRect& localBounds) = 0;
    virtual void DebugDraw(sf::RenderTarget& target) { };

protected:
    void AddComponentPin(bool connectable)
    {
        uint32_t pinId = mPins.size();
        mPins.push_back(ComponentPin(pinId, connectable));
    }

    ComponentPin& GetComponentPin(uint32_t componentPinId)
    { 
        return mPins.at(componentPinId);
    }

    void AssociateComponentWithCircuitBoardPin(uint32_t componentPinId, Pin* circuitBoardPin)
    {
        mPins.at(componentPinId).SetTemporaryConnectionPin(circuitBoardPin);
    }

    const sf::Vector2f& GetCircuitBoardPinPosition(uint32_t componentPinId)
    {
        Pin* temporaryConnectionPin = GetComponentPin(componentPinId).GetTemporaryConnectionPin();
        return mNavigator->GetGridCoordinateFromPin(*temporaryConnectionPin);
    }

    Pin* GetNeighborCircuitBoardPin(Pin& circuitBoardPin, const sf::Vector2i& neighborOffset)
    {
        return mNavigator->GetSurroundingPin(circuitBoardPin, neighborOffset);
    }

    Pin& GetCircuitBoardPinAtCursor()
    {
        return mNavigator->GetSelectedPin();
    }

    sf::Color mColor;

private:
    ICircuitBoardNavigator* mNavigator;
    std::vector<ComponentPin> mPins;
    std::vector<Node> mNodes;    
    Node* mSelectedNode{ nullptr };
    size_t mMaxNodes;
};