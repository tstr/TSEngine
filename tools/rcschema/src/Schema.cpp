/*
	Resource Schema class source
*/

#include "Schema.h"

#include <unordered_set>
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

bool FieldSet::add(const String& name, const String& type, bool isArray)
{
    static const unordered_set<String> reservedNames({"rcptr", "rcsize"});
    
    //Check if field name is not reserved
    if (reservedNames.find(name) == reservedNames.end())
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
                field.isArray = isArray;
                
                //Save field entry
                Base::push_back(field);

                return true;
            }
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

void Schema::declareResource(const String& name)
{
	auto it = find(m_resources.begin(), m_resources.end(), name);

	//If resource does not already exist
	if (it == m_resources.end())
	{
		m_resources.push_back(Resource(nullptr, name));
	}
}

bool Schema::addResource(const Resource& rsc)
{
	auto it = find(m_resources.begin(), m_resources.end(), rsc.getName());

	//If resource entry does not exist
	if (it == m_resources.end())
	{
		//Insert new entry
		m_resources.push_back(rsc);
		return true;
	}
	else
	{
		//If resource was predeclared then overwrite entry
		if (!it->isComplete())
		{
			*it = rsc;
			return true;
		}
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
		DEF_TYPE_ENTRY("string", 4, TYPE_IS_REFERENCE),
	});
}

bool Schema::isType(const String& typeName) const
{
	return (m_types.find(getBaseTypeName(typeName)) != m_types.end()) || isResource(typeName);
}

bool Schema::getTypeInfo(const String& typeName, TypeInfo& info) const
{
	//If typename is a resource
	if (isResource(typeName))
	{
		info.flags = TYPE_IS_RESOURCE;
		info.name = typeName;
		info.size = sizeof(uint32);
	}
	else
	{
		//Find type name
		auto it = m_types.find(typeName);

		//If type was not found
		if (it == m_types.end())
		{
			return false;
		}

		//Get TypeInfo structure from table
		info = it->second;
	}

	return true;
}

bool Schema::defineType(const String& typeName, const FieldSet& fields)
{
	//Type name must not be an already defined resource or type
	if (isType(typeName))
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
