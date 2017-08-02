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
	headerFile << "#include <tscore/types.h>\n"
		"#include <tscore/strings.h>\n"
		"#include <tscore/system/memory.h>\n"
		"#include <ostream>\n"
		"#include <istream>\n\n";

	//Namespace
	headerFile << "namespace ts { namespace rc {\n\n";

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
	for (const Schema::CompositeType& comptype : schema.getCompositeTypes())
	{
		//Begin struct
		headerFile << "struct " << comptype.name << "\n{\n";

		for (const Field& field : comptype.fields)
		{
			headerFile << format("    % %;\n", field.type.name, field.name);
		}

		//End struct
		headerFile << "};\n\n";
	}

	//Each schema has a corresponding loader class
	for (const Resource& rcs : schema)
	{
		if (flags & GENERATE_BUILDER)
		{
			generateBuilderClass(headerFile, rcs);
		}

		if (flags & GENERATE_LOADER)
		{
			generateLoaderClass(headerFile, rcs);
		}
	}

	headerFile << "}}\n";

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
	headerFile << "class " << className << "\n";
	headerFile << "{\n";

	//Private section
	headerFile << "private:\n\n";

	//Field descriptors
	generateFieldDescriptorTable(headerFile, rsc);

	//Private methods - store
	headerFile << "    ";
	headerFile << "template<typename Type> inline void store(FieldDescriptor field, const Type& value) { *reinterpret_cast<Type*>((byte*)m_data.pointer() + field) = value; }\n";
	headerFile << "\n";

	headerFile << "    void serializeBuffer(std::ostream& out, const MemoryBuffer& data)\n"
		"    {\n"
		"        Offset sz = (Offset)data.size();\n"
		"        out.write((const char*)&sz, sizeof(Offset));\n"
		"        out.write((const char*)data.pointer(), data.size());\n"
		"    }\n\n";

	headerFile << "    MemoryBuffer m_data;\n";

	//Foreach reference field
	for (const Field& field : rsc.getFields())
	{
		if (field.type.flags & TYPE_IS_REFERENCE)
		{
			//Generate reference buffers
			headerFile << format("    MemoryBuffer m_data_%;\n", field.name);
		}
	}

	headerFile << "\n";
	
	//Public section
	headerFile << "public:\n\n";

	//Constructor
	headerFile << "    ";
	headerFile << className << "()\n"
		"    {\n"
		"        m_data = MemoryBuffer(resource_size);\n"
		"    }\n\n";

	//Field accessors
	for (const Field& field : rsc.getFields())
	{
		generateFieldSetter(headerFile, field);
	}

	headerFile << "\n";

	//Builder method
	headerFile << ""
		"    void build(std::ostream& out)\n"
		"    {\n"
		"        Offset rootOffset = resource_size;\n";

	//Foreach reference field
	for (const Field& field : rsc.getFields())
	{
		if (field.type.flags & TYPE_IS_REFERENCE)
		{
			//Set references to buffer offsets
			headerFile << format("        store<Offset>(field_%, rootOffset);\n", field.name);
			headerFile << format("        rootOffset += 4 + (Offset)m_data_%.size();\n", field.name);			
		}
	}

	headerFile << ""
		"        out.write((const char*)&rootOffset, sizeof(Offset));\n"
		"        out.write((const char*)m_data.pointer(), m_data.size());\n";

	//Foreach reference field
	for (const Field& field : rsc.getFields())
	{
		if (field.type.flags & TYPE_IS_REFERENCE)
		{
			//Serialize reference buffers
			headerFile << format("        serializeBuffer(out, m_data_%);\n", field.name);
		}
	}

	//End builder method
	headerFile << "    }\n";

	//End class
	headerFile << "};\n\n";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CPPGenerator::generateLoaderClass(std::ostream& headerFile, const Resource& rsc)
{
	const String className = rsc.getName() + "Loader";

	//Class declaration
	headerFile << "class " << className << "\n";
	headerFile << "{\n";

	//Private section
	headerFile << "private:\n\n";

	//Field descriptors
	generateFieldDescriptorTable(headerFile, rsc);

	headerFile << "    ";
	headerFile << "MemoryBuffer m_data; ";
	headerFile << "\n";

	//Private methods - load/store
	headerFile << "    template<typename Type> inline Type load(FieldDescriptor field) const { return *reinterpret_cast<const Type*>((const byte*)m_data.pointer() + field); }\n";
	headerFile << "    template<typename Type> inline const Type* loadPtr(FieldDescriptor field) const { return reinterpret_cast<const Type*>((const byte*)m_data.pointer() + load<Offset>(field)); }\n";
	headerFile << endl;
	headerFile << "    template<typename Type> inline const Type* getArray(FieldDescriptor field) const { return (const Type*)(loadPtr<Offset>(field) + 1); }\n";
	headerFile << "    inline uint32 getArrayLength(FieldDescriptor field) const { return *loadPtr<Offset>(field); }\n";
	headerFile << endl;

	//Public section
	headerFile << "public:\n\n";

	//Constructor
	headerFile << "    ";
	headerFile << className << "(std::istream& in)\n"
	"    {\n"
	"    	 uint32 totalSize;\n"
	"    	 in.read((char*)&totalSize, sizeof(uint32));\n"
	"    	 m_data = MemoryBuffer(totalSize);\n"
	"    	 in.read((char*)m_data.pointer(), m_data.size());\n"
	"    }\n\n";

	//Field accessors
	for (const Field& field : rsc.getFields())
	{
		generateFieldGetter(headerFile, field);
	}

	//End class
	headerFile << "};\n\n";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

size_t CPPGenerator::generateFieldDescriptorTable(ostream& headerFile, const Resource& rsc)
{
	headerFile << "    typedef uint32 Offset;\n\n";

	//Enum declaration
	headerFile << "    enum FieldDescriptor : Offset\n";
	headerFile << "    {\n";

	uint32 descriptor = 0;

	//Field offset table
	for (const Field& field : rsc.getFields())
	{
		headerFile << "        ";
		headerFile << format("field_% = %,\n", field.name, descriptor);
		descriptor += field.type.size;
	}

	headerFile << "        ";
	headerFile << format("resource_size = %,\n", descriptor);

	//Close enum
	headerFile << "    };\n\n";

	//Return size of resource
	return descriptor;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Used in resource builder classes
void CPPGenerator::generateFieldSetter(ostream& headerFile, const Field& field)
{
	if (field.type.flags & TYPE_IS_REFERENCE)
	{
		//Strings require their own method signature
		if ((field.type.flags & TYPE_IS_STRING) == TYPE_IS_STRING)
		{
			//comment
			headerFile << format("    // String: %\n", field.name);
			headerFile << format("    void set_%(const String& value) { m_data_% = MemoryBuffer(value.c_str(), value.size() + 1); }\n", field.name, field.name);
		}
		else if ((field.type.flags & TYPE_IS_ARRAY) == TYPE_IS_ARRAY)
		{
			//comment
			headerFile << format("    // Array: %\n", field.name);
			headerFile << format("    void set_%(const %* value, uint32 valueLength) { m_data_% = MemoryBuffer(value, valueLength * sizeof(%)); }\n", field.name, field.type.name, field.name, field.type.name);
		}
	}
	//If field is not an array type
	else
	{
		//comment
		headerFile << format("    // Field: %\n", field.name);
		headerFile << format("    void set_%(% value) { store(field_%, value); }\n",
			field.name,
			field.type.name,
			field.name
		);
	}
}

// Used in resource loader classes
void CPPGenerator::generateFieldGetter(ostream& headerFile, const Field& field)
{
	if (field.type.flags & TYPE_IS_REFERENCE)
	{
		//Strings require their own method signature
		if ((field.type.flags & TYPE_IS_STRING) == TYPE_IS_STRING)
		{
			//comment
			headerFile << format("    // String: %\n", field.name);
			headerFile << format("    const char* get_%() const { return getArray<char>(field_%); }\n", field.name, field.name);
			headerFile << format("    uint32 length_%() const { return getArrayLength(field_%); }\n", field.name, field.name);
		}
		else if ((field.type.flags & TYPE_IS_ARRAY) == TYPE_IS_ARRAY)
		{
			//comment
			headerFile << format("    // Array: %\n", field.name);
			headerFile << format("    const %* get_%() const { return getArray<%>(field_%); }\n", field.type.name, field.name, field.type.name, field.name);
			headerFile << format("    uint32 length_%() const { return getArrayLength(field_%) / sizeof(%); }\n", field.name, field.name, field.type.name);
		}

	}
	// If field is not a reference type
	else
	{
		//comment
		headerFile << format("    // Field: %\n", field.name);

		headerFile << format("    % get_%() const { return load<%>(field_%); }\n",
			field.type.name,
			field.name,
			field.type.name,
			field.name
		);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
