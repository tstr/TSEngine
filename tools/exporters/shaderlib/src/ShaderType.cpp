/*
	Shader Type System source
*/

#include "ShaderType.h"

using namespace std;
using namespace ts;

StructMember::StructMember(const TypeContext* types, const StructMemberInfo& varInfo)
{
	m_type = types->type(varInfo.typeName);
	m_name = varInfo.name;
	m_semantic = varInfo.semantic;
	m_arraySize = varInfo.arraySize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void TypeContext::defineBasicTypes()
{
	const char* basicTypes[] = {
		"int", "uint", "float", "bool", "double",
		"float2", "float3", "float4",
		"double2", "double3", "double4",
		"float2x2", "float3x3", "float4x4",
		"matrix"
	};

	const size_t ALIGN = 16;

	m_typeSet.emplace_back(new MemberType("int", 4));
	m_typeSet.emplace_back(new MemberType("uint", 4));
	m_typeSet.emplace_back(new MemberType("float", 4));
	m_typeSet.emplace_back(new MemberType("scalar", 4));
	m_typeSet.emplace_back(new MemberType("bool", 4));
	m_typeSet.emplace_back(new MemberType("double", 8));
	m_typeSet.emplace_back(new MemberType("float2", 8));
	m_typeSet.emplace_back(new MemberType("float3", 12));
	m_typeSet.emplace_back(new MemberType("float4", 16));
	m_typeSet.emplace_back(new MemberType("vector", 16));
	m_typeSet.emplace_back(new MemberType("double2", 16));
	m_typeSet.emplace_back(new MemberType("double3", 24));
	m_typeSet.emplace_back(new MemberType("double4", 32));
	m_typeSet.emplace_back(new MemberType("float2x2", 16));
	m_typeSet.emplace_back(new MemberType("float3x3", 32));
	m_typeSet.emplace_back(new MemberType("float4x4", 64));
	m_typeSet.emplace_back(new MemberType("matrix", 64));
}

bool TypeContext::isType(const std::string& name) const
{
	return type(name) != nullptr;
}

const MemberType* TypeContext::type(const std::string& name) const
{
	using namespace std;

	auto it = find_if(m_typeSet.begin(), m_typeSet.end(),
		[&](const TypeRef& type) {
		return name == type->name();
	});

	if (it == m_typeSet.end())
	{
		return nullptr;
	}
	else
	{
		return it->get();
	}
}

const MemberType* TypeContext::defineType(const std::string& name, const std::vector<StructMemberInfo>& memberDescs)
{
	//Type is already defined
	if (isType(name))
	{
		return nullptr;
	}

	std::vector<StructMember> memberList(memberDescs.size());

	for (size_t i = 0; i < memberDescs.size(); i++)
	{
		const StructMemberInfo& varInfo = memberDescs.at(i);

		StructMember var(this, varInfo);

		if (var.type() == nullptr)
		{
			return nullptr;
		}

		memberList.at(i) = var;
	}

	MemberType* ptr = new MemberType(name, memberList);
	m_typeSet.push_back(TypeRef(ptr));
	return ptr;
}
