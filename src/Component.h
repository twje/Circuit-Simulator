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

    virtual void Move(const sf::Vector2f& cursor, float gridSpacing) = 0;
    virtual Component* CreateShape(const sf::Vector2f& cursor, float gridSpacing) const = 0;
    virtual void DrawIcon(sf::RenderTarget& target, const sf::Transform& transform, const sf::FloatRect& localBounds) = 0;

private:
    std::vector<Node> mNodes;
    Node* mSelectedNode{ nullptr };
    size_t mMaxNodes;
    sf::Color mColor;
};