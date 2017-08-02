/*
	Resource Schema class source
*/

#include "Schema.h"

#include <algorithm>

using namespace ts;
using namespace ts::rc;
using namespace std;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool isArrayTypeName(const String& typeName)
{
	const String ending = "[]";

	if (typeName.length() >= ending.length())
	{
		return (typeName.compare(typeName.length() - ending.length(), ending.length(), ending) == 0);
	}

	return false;
}

String getBaseTypeName(const String& typeName)
{
	//If is array type
	if (isArrayTypeName(typeName))
	{
		return typeName.substr(0, typeName.size() - 2);
	}

	//Otherwise return original type
	return typeName;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Field Set
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FieldSet::add(const String& name, const String& type)
{
	//If a field with this name does not already exist
	if (find(begin(), end(), name) == end())
	{
		TypeInfo info;

		//Lookup type information
		if (m_schema->getTypeInfo(type, info))
		{
			Field field;
			field.name = name;
			field.type = info;
			
			//Save field entry
			Base::push_back(field);

			return true;
		}
	}

	return false;
}

void FieldSet::remove(const String& name)
{
	Base::iterator it = find(Base::begin(), Base::end(), name);
	
	if (it != Base::end())
	{
		Base::erase(it);
	}
}

uint32 FieldSet::getSize() const
{
	uint32 size = 0;
	
	for (const Field& field : *this)
	{
		size += field.type.size;
	}

	return size;
}

uint32 FieldSet::getCount() const
{
	return (uint32)Base::size();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Schema resource methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Schema::isResource(const String& name) const
{
	return find(m_resources.begin(), m_resources.end(), name) != m_resources.end();
}

bool Schema::addResource(const Resource& rsc)
{
	auto it = find(m_resources.begin(), m_resources.end(), rsc.getName());

	if (it == m_resources.end())
	{
		m_resources.push_back(rsc);
		return true;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Schema type methods
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define DEF_TYPE_ENTRY(typeName, typeSize, typeFlags) make_pair(typeName, TypeInfo(typeName, typeSize, typeFlags))

void Schema::initPrebuiltTypes()
{
	m_types = TypeMap(
	{
		DEF_TYPE_ENTRY("byte", 1, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("bool", 1, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("int16", 2, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("int32", 4, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("int64", 8, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("uint16", 2, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("uint32", 4, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("uint64", 8, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("float32", 4, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("float64", 8, TYPE_IS_PRIMITIVE),
		DEF_TYPE_ENTRY("string", 4, TYPE_IS_STRING),
	});
}

bool Schema::isType(const String& typeName) const
{
	return m_types.find(getBaseTypeName(typeName)) != m_types.end();
}

bool Schema::getTypeInfo(const String& _typeName, TypeInfo& info) const
{
	String baseTypeName = _typeName;
	bool typeIsReference = false;

	//If given type is an array type
	if (isArrayTypeName(_typeName))
	{
		//Get base type without array brackets
		baseTypeName = getBaseTypeName(_typeName);
		typeIsReference = true;
	}

	//Find base type
	auto it = m_types.find(baseTypeName);

	//If base type was found
	if (it != m_types.end())
	{
		//Get TypeInfo structure
		info = it->second;

		//If input type is a reference type
		if (typeIsReference)
		{
			//Then if base type is a primitive type
			if ((info.flags & TYPE_IS_PRIMITIVE) == TYPE_IS_PRIMITIVE)
			{
				info.flags = TYPE_IS_ARRAY;
			}
			//Otherwise if base type is reference type
			else if ((info.flags & TYPE_IS_REFERENCE) == TYPE_IS_REFERENCE)
			{
				//return error - cannot have references to references
				return false;
			}
		}

		return true;
	}

	return false;
}

bool Schema::defineType(const String& typeName, const FieldSet& fields)
{
	//Type name must not be an already defined resource or type
	if (isType(typeName) || isResource(typeName))
	{
		return false;
	}

	//Must not be an array typename
	if (isArrayTypeName(typeName))
	{
		return false;
	}

	//Fields in custom data types must not be composed of reference types
	for (const Field& field : fields)
	{
		if (field.type.flags & TYPE_IS_REFERENCE)
		{
			return false;
		}
	}

	TypeInfo info;
	info.flags = TYPE_IS_PRIMITIVE;
	info.size = fields.getSize();
	info.name = typeName;

	//Add type entry
	m_types.insert(make_pair(typeName, info));

	//Add field set to composite type list
	CompositeType compType;
	compType.fields = fields;
	compType.name = typeName;
	m_compositeTypes.push_back(compType);

	return true;
}

bool Schema::defineEnum(const String& typeName, const EnumSet& enums)
{
	//Type name must not be an already defined resource or type
	if (isType(typeName) || isResource(typeName))
	{
		return false;
	}

	//Must not be an array typename
	if (isArrayTypeName(typeName))
	{
		return false;
	}

	TypeInfo info;
	info.flags = TYPE_IS_PRIMITIVE;
	info.size = sizeof(uint32);
	info.name = typeName;

	//Add type entry
	m_types.insert(make_pair(typeName, info));

	//Add enum type
	EnumType enumType;
	enumType.name = typeName;
	enumType.enums = enums;
	m_enumTypes.push_back(enumType);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
