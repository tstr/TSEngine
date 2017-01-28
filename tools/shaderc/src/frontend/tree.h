/*
	Simple tree class
*/

#pragma once

#include <tscore/types.h>
#include <tscore/system/memory.h>

template<
	typename value_t,
	typename = is_default_constructible<value_t>::type
>
class Tree
{
public:

	typedef ts::uint32 Integer;
	typedef ts::uint32 NodeId;

private:

	struct Node
	{
		//Value of node
		value_t value;
		
		//Indices of child nodes
		std::vector<NodeId> children;
		
		Node() :
			value(value_t())
		{}

		Node(const value_t& v) :
			value(v)
		{}

		Node(value_t&& v) :
			value(v)
		{}
	};

	std::vector<Node> m_nodes;

	bool validateId(NodeId id) const
	{
		bool expr = (id <= m_nodes.size() && (id > 0));

		//if (!expr) throw logic_error("Invalid tree node id");

		return expr;
	}

	bool getNode(NodeId id, Node& node) const
	{
		if (!validateId(id))
			return false;

		node = m_nodes.at((size_t)id - 1);

		return true;
	}
	
	bool setNode(NodeId id, Node node)
	{
		if (!validateId(id))
			return false;

		m_nodes.at((size_t)id - 1) = node;

		return true;
	}

public:
	
	NodeId allocNode(const value_t& value)
	{
		m_nodes.push_back(Node(value));
		return (NodeId)(m_nodes.size());
	}

	NodeId allocNode(value_t&& value)
	{
		m_nodes.push_back(Node(value));
		return (NodeId)(m_nodes.size());
	}
	
	void linkNode(NodeId parentId, NodeId childId)
	{
		Node parent;
		getNode(parentId, parent);
		parent.children.push_back(childId);
		setNode(parentId, parent);
	}
	
	//Returns true if a node has no children
	bool isNodeLeaf(NodeId node) const
	{
		Node n;

		if (getNode(node, n))
			return n.children.empty();
		else
			return false;
	}

	//Returns true if a node has at least one child
	bool isNodeBranch(NodeId node) const
	{
		return !isNodeLeaf(node);
	}

	//Returns true if the node exists
	bool isNode(NodeId node) const
	{
		return validateId(node);
	}

	std::vector<NodeId> getChildren(NodeId id) const
	{
		Node n;
		getNode(id, n);
		return n.children;
	}
	
	Integer getChildCount(NodeId id) const
	{
		Node n;
		getNode(id, n);
		return (Integer)n.children.size();
	}
	
	NodeId getChild(NodeId id, Integer idx) const
	{
		Node n;
		getNode(id, n);
		return n.children.at(idx);
	}

	bool getNodeValue(NodeId id, value_t& value) const
	{
		Node n;

		if (!this->getNode(id, n))
			return false;

		value = n.value;
		return true;
	}
};
