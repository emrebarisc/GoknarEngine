#ifndef __NAVIGATION_TREE__
#define __NAVIGATION_TREE__

#include "Goknar/Core.h"

#include <memory>
#include <vector>

#include "Goknar/Geometry/Area.h"

struct GOKNAR_API NavigationNode
{
    NavigationNode() = default;
    ~NavigationNode() = default;

    Area area;
    
    int id{ -1 };
    
    float cost{ 1.f };

    std::vector<NavigationNode*> neighbours;
};

class GOKNAR_API NavigationTree
{
public:
    NavigationTree();
    NavigationTree(const NavigationTree&) = delete;
    NavigationTree(NavigationTree&&) noexcept = default;
    ~NavigationTree();

    NavigationTree& operator=(const NavigationTree&) = delete;
    NavigationTree& operator=(NavigationTree&&) noexcept = default;

    NavigationNode* AddNode(const Area& area);
    void RemoveNode(NavigationNode* node);
    void Clear();

    NavigationNode* GetRoot() const
    {
        return root;
    }

    void SetRoot(NavigationNode* node)
    {
        root = node;
    }

    const std::vector<std::unique_ptr<NavigationNode>>& GetNodes() const
    {
        return nodes_;
    }

    std::vector<std::unique_ptr<NavigationNode>>& GetNodes()
    {
        return nodes_;
    }

    NavigationNode* GetNodeById(int id) const;
    int GetNodeIndex(const NavigationNode* node) const;
    void EnsureNextNodeId(int nextNodeId);

private:
    NavigationNode* root{ nullptr };
    std::vector<std::unique_ptr<NavigationNode>> nodes_;
    int nextNodeId_{ 0 };
};

#endif
