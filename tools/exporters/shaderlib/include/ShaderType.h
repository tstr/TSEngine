/*
	Shader Type System
*/

#pragma once

#include <tscore/ptr.h>
#include <tscore/strings.h>
#include <tscore/types.h>

namespace ts
{
	class MemberType;
	class TypeContext;

	struct StructMemberInfo
	{
		String typeName;
		String name;
		String semantic;
		uint32 arraySize;

		StructMemberInfo() : arraySize(1) {}
	};

	class StructMember
	{
	private:

		const MemberType* m_type;
		String m_name;
		String m_semantic;
		uint32 m_arraySize;

	public:

		StructMember() : m_type(nullptr), m_arraySize(1) {}

		StructMember(
			const MemberType* type,
			const String& name,
			const String& semantic,
			uint32 arraysize,
			uint32 offset
		)
			: m_type(type), m_name(name),
			m_arraySize(arraysize), m_semantic(semantic)
		{}

		StructMember(const TypeContext* types, const StructMemberInfo& info);

		const MemberType* type() const { return m_type; }

		uint32 arraySize() const { return m_arraySize; }

		String semantic() const { return m_semantic; }

		String name() const { return m_name; }
		void setName(const String& name) { m_name = name; }
	};

	class MemberType
	{
	private:

		std::string m_name;
		std::vector<StructMember> m_members;

		bool m_isBasic;
		uint32 m_size;

		MemberType(const String& name, size_t size) :
			m_name(name),
			m_size(size),
			m_isBasic(true)
		{}

		MemberType(const String& name, const std::vector<StructMember>& members) :
			m_name(name),
			m_members(members),
			m_isBasic(false)
		{}

	public:

		friend class TypeContext;

		using MemberList = std::vector<StructMember>;
		using MemberIterator = MemberList::const_iterator;

		bool isBasic() const { return m_isBasic; }
		bool isComposite() const { return !isBasic(); }

		const char* name() const { return m_name.c_str(); }

		MemberIterator membersBegin() const { return m_members.begin(); }
		MemberIterator membersEnd() const { return m_members.end(); }

		const MemberList& getMembers() const { return m_members; }
		StructMember getMember(size_t i) const { return m_members.at(i); }
		size_t getMemberCount() const { return m_members.size(); }

		int operator<(const MemberType& other)
		{
			return this->m_name < other.m_name;
		}

		bool operator==(const MemberType& other)
		{
			return this->m_name == other.m_name;
		}

		size_t size() const
		{
			size_t sz = 0;

			if (isBasic())
			{
				sz = m_size;
			}
			else
			{
				for (const StructMember& var : m_members)
				{
					sz += var.type()->size() * var.arraySize();
				}
			}

			return sz;
		}
	};

	class TypeContext
	{
	private:

		using TypeRef = std::unique_ptr<MemberType>;

		std::vector<TypeRef> m_typeSet;

		void defineBasicTypes();

	public:

		TypeContext()
		{
			defineBasicTypes();
		}

		bool isType(const std::string& name) const;
		const MemberType* type(const std::string& name) const;

		const MemberType* defineType(const std::string& name, const std::vector<StructMemberInfo>& memberDescs);
	};
}
