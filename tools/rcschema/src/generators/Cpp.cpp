/*
	C++ code generator source
*/

#include "Cpp.h"

#include <tscore/pathutil.h>
#include <iostream>
#include <fstream>

using namespace std;
using namespace ts;
using namespace ts::rc;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool CPPGenerator::generate(const Schema& schema, const Path& outputDir, uint32 flags)
{
	const String headerName = schema.getName() + ".rcs.h";
	
	Path headerFilePath;

	if (!isDirectory(outputDir))
	{
		cerr << "ERROR: output directory \"" << outputDir.str() << "\" does not exist\n";
		return false;
	}

	headerFilePath = outputDir;
	headerFilePath.addDirectories(headerName);

	fstream headerFile(headerFilePath.str(), ios::out);

	if (headerFile.fail())
	{
		cerr << "ERROR: cannot create file: \"" << headerFilePath.str() << "\"\n";
		return false;
	}

	/*
		Generate header file
	*/

	//Header comment at top of file
	headerFile << format("/*\n"
		"    Machine generated file.\n\n"
		"    %\n" // File name
		"*/\n\n",
		headerName);

	headerFile << "#pragma once\n\n";

	//Include files
	headerFile << "#include <rcschema.h>\n\n";

	//Declare namespaces
	uint32 numNamespaces = 0;
	if (!schema.getNamespace().empty())
	{
		for (const String& name : split(schema.getNamespace(), "."))
		{
			headerFile << "namespace " << name << " { ";
			numNamespaces++;
		}

		headerFile << "\n\n";
	}

	//User defined enums
	for (const Schema::EnumType& enumtype : schema.getEnumTypes())
	{
		//Begin enum declaration
		headerFile << "enum " << enumtype.name << "\n{\n";

		for (const String& value : enumtype.enums)
		{
			headerFile << format("    %,\n", value);
		}

		//End enum
		headerFile << "};\n\n";
	}


	//User defined data types
	if (!schema.getCompositeTypes().empty())
	{
		headerFile << "RCS_BEGIN_DATA\n\n";

		for (const Schema::CompositeType& comptype : schema.getCompositeTypes())
		{
			//Begin struct
			headerFile << "RCS_DATA_STRUCT " << comptype.name << "\n{\n";

			for (const Field& field : comptype.fields)
			{
				headerFile << format("    % %;\n", translateFieldType(field), field.name);
			}

			//End struct
			headerFile << "};\n\n";
		}

		headerFile << "RCS_END_DATA\n\n";
	}

	//Forward declare loader classes
	for (const Resource& rsc : schema)
	{
		if (flags & GENERATE_LOADER)
		{
			headerFile << "class " << rsc.getName() << ";\n";
		}
	}

	headerFile << "\n";

	//Each resource has a corresponding builder/resource class
	for (const Resource& rcs : schema)
	{
		headerFile << "///////////////////////////////////////////////////////////////////////////////////////\n\n";

		if (flags & GENERATE_LOADER)
		{
			generateLoaderClass(headerFile, rcs);
		}

		if (flags & GENERATE_BUILDER)
		{
			generateBuilderClass(headerFile, rcs);
		}
	}

	headerFile << "///////////////////////////////////////////////////////////////////////////////////////\n\n";

	//End namespace declarations
	for (uint32 i = 0; i < numNamespaces; i++)
	{
		headerFile << "}";
	}
	headerFile << "\n\n";

	//Force save and close file
	headerFile.flush();
	headerFile.close();

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPPGenerator::generateBuilderClass(std::ostream& headerFile, const Resource& rsc)
{
	const String className = rsc.getName() + "Builder";

	//Class declaration
	headerFile << "class " << className << " RCS_SEALED : public ::rc::ResourceBuilder\n";
	headerFile << "{\n";

	//Private section
	headerFile << "private:\n\n";
	
	//Field offsets
	generateFieldTable(headerFile, rsc.getFields());

	//Public section
	headerFile << "public:\n\n";

	//Constructor
	headerFile << "    " << className << "() : ResourceBuilder(nullptr, total_fields_size) {}\n";
	headerFile << "    template<typename Builder> " << className << "(Builder& parent) : ResourceBuilder(&parent, total_fields_size) {}\n\n";
	//headerFile << "    template<typename BuilderType, typename = std::enable_if<std::is_base_of<::rc::ResourceBuilder, BuilderType>::value>::type>" << className << "(BuilderType& parent) : ResourceBuilder((::rc::ResourceBuilder&)parent, total_fields_size) {}\n\n";

	//Field accessors
	for (const Field& field : rsc.getFields())
	{
		generateFieldSetter(headerFile, field);
	}

	//End class
	headerFile << "};\n\n";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPPGenerator::generateLoaderClass(std::ostream& headerFile, const Resource& rsc)
{
	const String className = rsc.getName();
	
	//Class declaration
	headerFile << "class " << className << " RCS_SEALED : public ::rc::ResourceView\n";
	headerFile << "{\n";

	//Private section
	headerFile << "private:\n\n";

	//Field offsets
	generateFieldTable(headerFile, rsc.getFields());

	//Public section
	headerFile << "public:\n\n";

	//Field accessors
	for (const Field& field : rsc.getFields())
	{
		generateFieldGetter(headerFile, field);
	}

	//End class
	headerFile << "};\n\n";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint32 CPPGenerator::generateFieldTable(ostream& headerFile, const FieldSet& fields)
{
	//Enum declaration
	headerFile << "    enum FieldTable : ::rc::OffsetType\n";
	headerFile << "    {\n";

	uint32 descriptor = 0;

	//Field offset table
	for (const Field& field : fields)
	{
		headerFile << "        ";
		headerFile << format("field_% = %,\n", field.name, descriptor);

		if (field.isArray)
		{
			descriptor += sizeof(uint32);
		}
		else
		{
			descriptor += field.type.size;
		}
	}

	headerFile << "        ";
	headerFile << format("total_fields_size = %,\n", descriptor);

	//Close enum
	headerFile << "    };\n\n";

	//Return size of resource
	return descriptor;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Used in resource builder classes
void CPPGenerator::generateFieldSetter(ostream& headerFile, const Field& field)
{
	//If field is reference
	if (field.type.flags & TYPE_IS_REFERENCE || field.isArray)
	{
		//comment
		headerFile << format("    // Reference: % %\n", field.type.name + ((field.isArray) ? "[]" : ""), field.name);
		headerFile << format("    auto& set_%(::rc::Ref<%> p) { ::rc::Utils::storePointer(rcptr(), field_%, p); return *this; }\n", field.name, translateFieldType(field), field.name);
	}
	//Otherwise if field is primitive
	else
	{
		//comment
		headerFile << format("    // Field: % %\n", field.type.name + ((field.isArray) ? "[]" : ""), field.name);
		headerFile << format("    auto& set_%(% value) { ::rc::Utils::storeField(rcptr(), field_%, value); return *this; }\n", field.name, translateFieldType(field), field.name);
	}
}

// Used in resource loader classes
void CPPGenerator::generateFieldGetter(ostream& headerFile, const Field& field)
{
	const String cppType(translateFieldType(field));

	//If field is reference
	if (field.type.flags & TYPE_IS_REFERENCE || field.isArray)
	{
		//comment
		headerFile << format("    // Reference: % %\n", field.type.name + ((field.isArray) ? "[]" : ""), field.name);
		headerFile << format("    const %& %() const { return *::rc::Utils::loadPointer<%>(rcptr(), field_%); }\n", cppType, field.name, cppType, field.name);
		headerFile << format("    bool has_%() const { return ::rc::Utils::loadField<::rc::OffsetType>(rcptr(), field_%) != 0; }\n", field.name, field.name);
	}
	//Otherwise if field is primitive
	else
	{
		//comment
		headerFile << format("    // Field: % %\n", field.type.name + ((field.isArray) ? "[]" : ""), field.name);
		headerFile << format("    % %() const { return ::rc::Utils::loadField<%>(rcptr(), field_%); }\n", cppType, field.name, cppType, field.name);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

String CPPGenerator::translateFieldType(const Field& field) const
{
	String cppType(field.type.name);

	//Aliases for prebuilt types
	static map<String, String> typeAliases = 
	{
		//Basic types
		{ "byte", "uint8_t" },
		{ "bool", "bool"},
		{ "int16", "int16_t" },
		{ "int32", "int32_t" },
		{ "int64", "int64_t" },
		{ "uint16", "uint16_t" },
		{ "uint32", "uint32_t" },
		{ "uint64", "uint64_t" },
		{ "float32", "float"},
		{ "float64", "double"},
		//Reference types
		{ "string", "::rc::StringView" }
	};

	//Lookup type
	auto it = typeAliases.find(field.type.name);

	//If a value was found in the table
	if (it != typeAliases.end())
	{
		cppType = it->second;
	}

	//If field is an array of types
	if (field.isArray)
	{
		//If basic type is a reference
		if (field.type.flags & TYPE_IS_REFERENCE)
		{
			//Array of references
			cppType = format("::rc::ArrayView<::rc::Ref<%>>", cppType);
		}
		//Otherwise if it is a primitive
		else
		{
			cppType = format("::rc::ArrayView<%>", cppType);
		}
	}

	return cppType;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
