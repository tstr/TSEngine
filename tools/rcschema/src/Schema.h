/*
	Resource Schema class
*/

#pragma once

#include <tscore/strings.h>
#include <vector>
#include <map>

namespace ts
{
	namespace rc
	{
		using std::istream;

		class Schema;

		enum ETypeFlags
		{
			TYPE_IS_REFERENCE = 1,
			TYPE_IS_PRIMITIVE = 2,

			TYPE_IS_ARRAY     = 4 | TYPE_IS_REFERENCE,
			TYPE_IS_STRING    = 6 | TYPE_IS_REFERENCE,
		};

		struct TypeInfo
		{
			String name;
			uint32 size;
			ETypeFlags flags;

			explicit TypeInfo(const String& typeName = "", uint32 size = 0, ETypeFlags flags = TYPE_IS_PRIMITIVE) :
				name(typeName),
				size(size),
				flags(flags)
			{}
		};

		/*
			Field struct
		*/
		struct Field
		{
			String name;
			TypeInfo type;

			bool operator==(const Field& rhs) const { return name == rhs.name; }
			bool operator==(const String& rhs) const { return name == rhs; }
		};

		/*
			Field Set class
		*/
		class FieldSet : private std::vector<Field>
		{
		private:

			//Parent schema
			Schema* m_schema;

		public:

			//Typedefs
			using Base = std::vector<Field>;
			using Iterator = typename Base::const_iterator;

			//Constructor
			explicit FieldSet(Schema* schema = nullptr) : m_schema(schema) {}
			FieldSet(const FieldSet& rhs) : m_schema(rhs.m_schema), Base((const Base&)rhs) {}

			//Iterators
			Iterator begin() const { return Base::cbegin(); }
			Iterator end() const { return Base::cend(); }

			//Parent schema
			Schema* getSchema() { return m_schema; }

			//Add/remove fields by name
			bool add(const String& name, const String& typeName);
			void remove(const String& name);

			//Get total size in bytes
			uint32 getSize() const;
			//Get number of fields
			uint32 getCount() const;

			//Clears all fields
			using Base::clear;
		};

		/*
			Resource class
		*/
		class Resource
		{
		public:

			//Constructors
			Resource() = default;
			Resource(Schema* schema, const String& name) :
				m_schema(schema),
				m_name(name),
				m_fields(schema)
			{}
			
			Resource(const Resource& r) :
				m_schema(r.m_schema),
				m_name(r.m_name),
				m_fields(r.m_fields)
			{}

			Resource(Resource&& rhs) = default;
			~Resource() = default;

			//Operators
			inline bool operator==(const Resource& r) const { return m_name == r.m_name; }
			inline bool operator==(const String& r) const { return m_name == r; }

			inline Resource& operator=(const Resource& r)
			{
				m_schema = r.m_schema;
				m_name = r.m_name;
				m_fields = r.m_fields;
				return *this;
			}

			//Schema property
			Schema* getSchema() const { return m_schema; }

			//Name property
			String getName() const { return m_name; }
			void setName(const String& name) { m_name = name; }

			//Field set
			FieldSet& getFields() { return m_fields; }
			const FieldSet& getFields() const { return m_fields; }

			void clear()
			{
				m_name.clear();
				m_fields.clear();
			}

		private:

			//Parent schema
			Schema* m_schema;
			//Resource name
			String m_name;
			//Resource fields
			FieldSet m_fields;
		};

		/*
			Resource Schema class
		*/
		class Schema
		{
		public:
			
			struct CompositeType
			{
				String name;
				FieldSet fields;
			};

			using CompositeTypeList = std::vector<CompositeType>;
			using ResourceSet = std::vector<Resource>;
			
			//Constructors
			Schema() { initPrebuiltTypes(); }
			Schema(const String& name) : m_name(name) { initPrebuiltTypes(); }
			
			//Operators
			inline Schema& operator=(const Schema& s) { m_name = s.m_name; m_resources = s.m_resources; m_types = s.m_types; return *this; }

			//Resource iterators
			ResourceSet::iterator begin() { return m_resources.begin(); }
			ResourceSet::iterator end() { return m_resources.end(); }
			ResourceSet::const_iterator begin() const { return m_resources.cbegin(); }
			ResourceSet::const_iterator end() const { return m_resources.cend(); }

			const ResourceSet& getResourceSet() const { return m_resources; }
			const CompositeTypeList& getCompositeTypes() const { return m_compositeTypes; }

			//Resource methods
			bool isResource(const String& name) const;
			bool addResource(const Resource& rsc);

			//Type methods
			bool isType(const String& typeName) const;
			bool getTypeInfo(const String& typeName, TypeInfo& info) const;
			//Define a custom type
			bool defineType(const String& typeName, const FieldSet& fieldSet);
			
			//Name property
			String getName() const { return m_name; }
			void setName(const String& name) { m_name = name; }

			void clear()
			{
				m_name.clear();
				m_resources.clear();
				m_types.clear();
				m_compositeTypes.clear();
			}

		private:

			void initPrebuiltTypes();

			using TypeMap = std::map<String, TypeInfo>;

			String m_name;
			ResourceSet m_resources;
			TypeMap m_types;
			CompositeTypeList m_compositeTypes;
		};
	}
}
