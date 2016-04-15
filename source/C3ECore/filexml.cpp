/*
	FileSystem module
*/

#include "pch.h"

#include <C3E\filesystem\filexml.h>
#include <fstream>
#include <sstream>

//XML parsing library
#include "rapidxml\rapidxml.hpp"
#include "rapidxml\rapidxml_utils.hpp"
#include "rapidxml\rapidxml_print.hpp"
#include "rapidxml\rapidxml_iterators.hpp"

using namespace std;
using namespace C3E;
using namespace rapidxml;

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//XML file class
////////////////////////////////////////////////////////////////////////////////////////////////////////////

//File node
class XMLnode : public FileSystem::IXMLnode
{
private:

	FileSystem::IXMLfile* m_file = nullptr;
	xml_node<>* m_node = nullptr;
	xml_document<>* m_document = nullptr;

public:

	XMLnode(xml_node<>*, FileSystem::IXMLfile*);

	~XMLnode() {}

	static XMLnode* upcast(IXMLnode* ptr) { return (static_cast<XMLnode*>(ptr)); }

	/////////////////////////////////////////////////////////////////////////////

	void AddNode(FileSystem::IXMLnode* pnode) override
	{
		auto n = static_cast<XMLnode*>(pnode);
		m_node->append_node(n->GetRawNode());
	}

	void DestroyAllNodes() override
	{
		m_node->remove_all_nodes();
	}

	FileSystem::IXMLnode* GetNextNode() override;
	FileSystem::IXMLnode* GetPreviousNode() override;

	FileSystem::IXMLnode* GetNode(const char*) override;

	/////////////////////////////////////////////////////////////////////////////

	void AddAttribute(const char* i, const char* v = "") override
	{
		m_document->allocate_string(i);

		if (v == "")
		{
			m_node->append_attribute(m_document->allocate_attribute(m_document->allocate_string(i)));
		}
		else
		{
			m_node->append_attribute(
				m_document->allocate_attribute(
					m_document->allocate_string(i),
					m_document->allocate_string(v)
				)
			);
		}
	}

	Attribute GetAttribute(const char* name) override
	{
		xml_attribute<>* a = m_node->first_attribute(name);

		Attribute A;

		if (!a) return A;

		A.name = a->name();
		A.value = a->value();

		return A;
	}

	const char* Name() override
	{
		return (m_node->name());
	}

	const char* Value() override
	{
		return (m_node->value());
	}

	void SetName(const char* n) override
	{
		m_node->name(n);
	}

	void SetValue(const char* v) override
	{
		m_node->value(v);
	}

	xml_node<>* GetRawNode() { return m_node; }

	/////////////////////////////////////////////////////////////////////////////

	iterator begin() override;
	iterator end() override;

	/////////////////////////////////////////////////////////////////////////////
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

class XMLfile : public FileSystem::IXMLfile
{
private:

	string m_filename;

	unique_ptr<file<char>> m_filebuffer;
	unique_ptr<XMLnode> m_root;
	
public:

	xml_document<> m_document;
	vector<unique_ptr<XMLnode>> m_nodes;

	/////////////////////////////////////////////////////////////////////////////

	XMLfile(const char* fn) :
		m_filename(fn)
	{
		m_root.reset(new XMLnode(&m_document, this));

		if (FileSystem::FileExists(fn))
		{
			m_filebuffer.reset(new file<char>(fn));
			Parse();
		}
	}

	~XMLfile()
	{
		for (auto& i : m_nodes)
		{
			i.reset();
		}

		m_document.clear();
	}

	/////////////////////////////////////////////////////////////////////////////
	//XML write operations
	/////////////////////////////////////////////////////////////////////////////

	FileSystem::IXMLnode* AllocateNode(FileSystem::XMLnodeType t = FileSystem::xml_element)
	{
		unique_ptr<XMLnode> newnode(new XMLnode(m_document.allocate_node((node_type)t), this));
		m_nodes.push_back(move(newnode));

		return m_nodes.back().get();
	}

	FileSystem::IXMLnode* AllocateNode(const char* n, const char* v = "", FileSystem::XMLnodeType t = FileSystem::xml_element)
	{
		unique_ptr<XMLnode> newnode(new XMLnode(m_document.allocate_node((node_type)t, m_document.allocate_string(n), m_document.allocate_string(v)), this));
		m_nodes.push_back(move(newnode));

		return m_nodes.back().get();
	}

	//Save an XML file
	void Save()
	{
		ofstream file(m_filename, ios_base::trunc);
		file << m_document;
		file.close();
	}

	/////////////////////////////////////////////////////////////////////////////
	//XML read operations
	/////////////////////////////////////////////////////////////////////////////

	XMLnode* GetNodePtr(xml_node<>* rawnode)
	{
		for (unique_ptr<XMLnode>& i : m_nodes)
		{
			if ((intptr_t)i->GetRawNode() == (intptr_t)rawnode)
				return i.get();
		}

		return 0;
	}

	FileSystem::IXMLnode* Root()
	{
		return m_root.get();
	}

	void Parse()
	{
		try
		{
			m_document.parse<rapidxml::parse_full>(m_filebuffer->data());
		}
		catch (const parse_error& e)
		{
			stringstream buffer;
			buffer << "Error parsing XML file: '" << m_filename << "'.\n"
				<< e.what();

			cerr << buffer.str();

			throw exception(buffer.str().c_str());

			return;
		}

		function<void(xml_node<char>*)> copy_nodes = [&copy_nodes, this](xml_node<char>* _node)
		{
			if (_node == nullptr) return;
			if (_node->first_node() == nullptr) return;

			node_iterator<char> begin(_node);
			node_iterator<char> end;

			for_each(begin, end, [&copy_nodes, this](xml_node<char>& _node)
			{
				unique_ptr<XMLnode> newnode(new XMLnode(&_node, this));
				this->m_nodes.push_back(move(newnode));
				copy_nodes(&_node);
			});
		};

		copy_nodes(&m_document);
	}

	static XMLfile* upcast(IXMLfile* f) { return (static_cast<XMLfile*>(f)); }

	/////////////////////////////////////////////////////////////////////////////
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////

XMLnode::XMLnode(xml_node<>* node, FileSystem::IXMLfile* file)
{
	m_file = file;
	m_node = node;
	m_document = &XMLfile::upcast(m_file)->m_document;
}

FileSystem::IXMLnode* XMLnode::GetNextNode()
{
	return XMLfile::upcast(m_file)->GetNodePtr(m_node->next_sibling());
	
}

FileSystem::IXMLnode* XMLnode::GetPreviousNode()
{
	return XMLfile::upcast(m_file)->GetNodePtr(m_node->previous_sibling());
}

FileSystem::IXMLnode* XMLnode::GetNode(const char* name)
{
	return XMLfile::upcast(m_file)->GetNodePtr(m_node->first_node(name, strlen(name), false));
}

/////////////////////////////////////////////////////////////////////////////

//iterators

FileSystem::IXMLnode::iterator XMLnode::begin()
{
	return iterator(XMLfile::upcast(m_file)->GetNodePtr(m_node->first_node()));
}

FileSystem::IXMLnode::iterator XMLnode::end()
{
	return iterator(nullptr);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//File system methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////

FileSystem::FileSystem()
{

}

FileSystem::~FileSystem()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

unique_ptr<FileSystem::IXMLfile> FileSystem::OpenXMLfile(const char* fn)
{
	try
	{
		return (unique_ptr<FileSystem::IXMLfile>(new XMLfile(fn)));
	}
	catch (const parse_error& e)
	{
		ConsolePrintline((string)"XML parser error: " + e.what());
	}

	return (unique_ptr<FileSystem::IXMLfile>());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FileSystem::FileExists(const char* fn)
{
	ifstream stream(fn);
	return stream.good();
}

bool FileSystem::DirectoryExists(const char* fn)
{
	return (FILE_ATTRIBUTE_DIRECTORY == GetFileAttributesA(fn));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////