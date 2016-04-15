/*
	File system module
*/

#pragma once

#include "filecore.h"

#include <C3E\core\strings.h>
#include <C3E\core\corecommon.h>
#include <C3E\core\memory.h>

#include <fstream>

namespace C3E
{

	///////////////////////////////////////////////////////////////////////////////////////
	//File system class - factory for creating file interfaces
	///////////////////////////////////////////////////////////////////////////////////////

	class C3E_CORE_API FileSystem
	{
	public:

		///////////////////////////////////////////////////////////////////////////////////////

		//file tag
		class IFile {};

		///////////////////////////////////////////////////////////////////////////////////////

		//Abstraction of rapidxml node types
		enum XMLnodeType
		{
			xml_document,
			xml_element,
			xml_data,
			xml_cdata,
			xml_comment,
			xml_declaration,
			xml_doctype,
			xml_pi
		};

		class IXMLnode
		{
		public:

			class iterator : public std::iterator<std::bidirectional_iterator_tag, IXMLnode*, std::ptrdiff_t, IXMLnode*>
			{
			private:

				IXMLnode* m_node = nullptr;

			public:

				iterator() : m_node(nullptr) {}
				iterator(IXMLnode* n) : m_node(n) {}

				inline reference operator *() { return m_node; }

				inline pointer operator->() { return m_node; }

				inline iterator& operator++()
				{
					m_node = m_node->GetNextNode();
					return *this;
				}

				inline iterator operator++(int)
				{
					iterator tmp = *this;
					++(*this);
					return tmp;
				}

				inline iterator& operator--()
				{
					m_node = m_node->GetPreviousNode();
					return *this;
				}

				inline iterator operator--(int)
				{
					iterator tmp = *this;
					++(*this);
					return tmp;
				}

				inline bool operator==(const iterator& from) { return m_node == from.m_node; }

				inline bool operator!=(const iterator& from) { return m_node != from.m_node; }
			};

			struct Attribute
			{
				const char* name = nullstr();
				const char* value = nullstr();
			};

			virtual void AddNode(IXMLnode*) = 0;
			virtual IXMLnode* GetNode(const char*) = 0;

			virtual void DestroyAllNodes() = 0;

			virtual IXMLnode* GetNextNode() = 0;
			virtual IXMLnode* GetPreviousNode() = 0;

			virtual iterator begin() = 0;
			virtual iterator end() = 0;

			virtual void AddAttribute(const char*, const char* = "") = 0;
			virtual Attribute GetAttribute(const char*) = 0;

			virtual const char* Name() = 0;
			virtual const char* Value() = 0;

			virtual void SetName(const char* n) = 0;
			virtual void SetValue(const char* v) = 0;

			virtual ~IXMLnode() {}
		};

		///////////////////////////////////////////////////////////////////////////////////////

		class IXMLfile : public IFile
		{
		public:

			virtual IXMLnode* AllocateNode(XMLnodeType = xml_element) = 0;
			virtual IXMLnode* AllocateNode(const char*, const char* = "", XMLnodeType = xml_element) = 0;

			virtual IXMLnode* Root() = 0;

			//Read
			virtual void Parse() = 0;
			//Write
			virtual void Save() = 0;

			virtual ~IXMLfile() {}
		};

		///////////////////////////////////////////////////////////////////////////////////////

		FileSystem();
		~FileSystem();

		std::unique_ptr<IXMLfile> OpenXMLfile(const char*);

		static bool FileExists(const char*);
		static bool DirectoryExists(const char*);

		static std::string GetContainingFolder(const std::string& file)
		{
			using namespace std;

			auto pos = file.find_last_of('\\');
			
			if (pos == string::npos) return "";

			return file.substr(0, pos);
		}

		static std::string GetFileExtension(const std::string& file)
		{
			using namespace std;

			auto pos = file.find_last_of('.');

			if (pos == string::npos) return "";

			return file.substr(pos, file.size() - pos);
		}
	};

	///////////////////////////////////////////////////////////////////////////////////////

}