/*
	Resource Schema Generator core header
*/

#pragma once

#include <string>
#include <ostream>
#include <istream>
#include <vector>
#include <memory>

/*
	Macros
*/
#define RCS_BEGIN_DATA __pragma(pack(push, 1))
#define RCS_END_DATA   __pragma(pack(pop))
#define RCS_DATA_STRUCT __declspec(align(1)) struct
#define RCS_SEALED final

namespace rc
{
	// Basic types
	using OffsetType = uint32_t;
	using SizeType   = uint32_t;

	// Type trait : type A can be cast to type B
	template<typename A, typename B>
	struct IsCastable
	{
		static const bool value = std::is_base_of<A, B>::value;
	};

	// Type trait : type A is a basic type
	template<typename A>
	struct IsBasicType
	{
		static const bool value = std::is_pod<A>::value;
	};
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Ref class:

		Wrapper class around a relative offset
	*/
	template<typename Type>
	class Ref
	{
	private:

		OffsetType offset;

	public:

		//Implicit downcast to basic offset
		inline operator OffsetType() const { return offset; }
		inline OffsetType get() const { return offset; }

		//Implicit upcast from basic offset
		inline Ref(OffsetType o = 0) :
			offset(o)
		{}

		template<typename OtherType, typename = std::enable_if<IsCastable<OtherType, Type>::value>::type>
		inline Ref(const Ref<OtherType>& other) :
			Ref((OffsetType)other)
		{}

		template<typename OtherType, typename = std::enable_if<IsCastable<OtherType, Type>::value>::type>
		inline Ref<Type>& operator=(const Ref<OtherType>& other)
		{
			offset = other.offset;
			return *this;
		}

		inline bool null() const { return offset == 0; }
	};

	//Void Ref
	using VoidRef = Ref<uint8_t>;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Pointer utils:

		Provides a set of helper functions for manipulating pointers and managing multiple indirections
	*/
	class Utils
	{
	private:

		/*
			Get pointer at given offset

			Offset can be negative
		*/
		template<typename Type>
		static Type* pointer(uint8_t* getPtr, int32_t offset = 0)
		{
			return reinterpret_cast<Type*>(getPtr + offset);
		}

		template<typename Type>
		static const Type* pointer(const uint8_t* getPtr, int32_t offset = 0)
		{
			return reinterpret_cast<const Type*>(getPtr + offset);
		}

	public:

		/*
			Set a field value at a given field offset
		*/
		template<typename Type>
		static inline void storeField(uint8_t* root, OffsetType fieldOffset, const Type& fieldValue)
		{
			loadField<Type>(root, fieldOffset) = fieldValue;
		}

		/*
			Get a field value from a given field offset
		*/
		template<typename Type>
		static inline Type& loadField(uint8_t* root, OffsetType fieldOffset)
		{
			return *pointer<Type>(root, fieldOffset);
		}

		template<typename Type>
		static inline const Type& loadField(const uint8_t* root, OffsetType fieldOffset)
		{
			return *pointer<Type>(root, fieldOffset);
		}

		/*
			Set a pointer offset value at a given field
		*/
		static inline void storePointer(uint8_t* root, OffsetType fieldOffset, OffsetType offsetValue)
		{
			//Make pointer value relative to field
			loadField<OffsetType>(root, fieldOffset) = (offsetValue - fieldOffset);
		}

		/*
			Get an absolute pointer from a given field
		*/
		template<typename Type = uint8_t>
		static inline const Type* loadPointer(const uint8_t* root, OffsetType fieldOffset)
		{
			//Load pointer offset from field
			int32_t pointerOffset = *pointer<int32_t>(root, fieldOffset);
			//Calculate absolute address from pointer offset
			return pointer<Type>(root, pointerOffset + fieldOffset);
		}

		template<typename Type = uint8_t>
		static inline Type* loadPointer(uint8_t* root, OffsetType fieldOffset)
		{
			//Load pointer offset from field
			int32_t pointerOffset = *pointer<int32_t>(root, fieldOffset);
			//Calculate absolute address from pointer offset
			return pointer<Type>(root, pointerOffset + fieldOffset);
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/*
		Type is a field

		Load field directly
	*/
	template<typename Type>
	struct IndirectLoad
	{
		using ReturnType = Type&;
		using ConstReturnType = const Type&;

		inline ConstReturnType operator()(const uint8_t* data, OffsetType dataOffsetInBytes = 0)
		{
			return Utils::loadField<Type>(data, dataOffsetInBytes);
		}

		inline ReturnType operator()(uint8_t* data, OffsetType dataOffsetInBytes = 0)
		{
			return Utils::loadField<Type>(data, dataOffsetInBytes);
		}
	};

	/*
		Type is a reference to a field

		Load pointer from field
	*/
	template<typename Type>
	struct IndirectLoad<Ref<Type>>
	{
		using ReturnType = Type&;
		using ConstReturnType = const Type&;

		inline ConstReturnType operator()(const uint8_t* data, OffsetType dataOffsetInBytes)
		{
			return *Utils::loadPointer<Type>(data, dataOffsetInBytes);
		}

		inline ReturnType operator()(uint8_t* data, OffsetType dataOffsetInBytes)
		{
			return *Utils::loadPointer<Type>(data, dataOffsetInBytes);
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Resource view class

		[ SizeType : length in bytes ]
		[ byte     :   element 0     ]
	*/
	class ResourceView
	{
	private:

		SizeType m_byteSize;

		//Data views can only be used as a reference type.
		//ie. they cannot be constructed, only cast from a raw pointer
		ResourceView() {}
		ResourceView(const ResourceView&) = delete;
		ResourceView(ResourceView&&) = delete;

	protected:

		//Size of data - in bytes
		inline SizeType rcsize() const { return m_byteSize; }

		//Pointer to data
		inline const uint8_t* rcptr() const { return (const uint8_t*)(&m_byteSize + 1); }
		inline uint8_t* rcptr() { return (uint8_t*)(&m_byteSize + 1); }

	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Array view class

		[ SizeType : length in bytes ]
		[ Type     :   element 0     ]
		[ Type     :   element 0     ]
		[ .......................... ]
		[ Type     :   element N     ]
	*/
	template<typename Type>
	class ArrayView : public ResourceView
	{
	public:
		
		using ElementType       = typename IndirectLoad<Type>::ReturnType;
		using ConstElementType  = typename IndirectLoad<Type>::ConstReturnType;

		//Array length
		inline SizeType length() const { return ResourceView::rcsize() / sizeof(Type); }
		inline SizeType size() const { return length(); }

		//Pointer to array data
		inline Type* data() { return reinterpret_cast<Type*>(ResourceView::rcptr()); }
		inline const Type* data() const { return reinterpret_cast<const Type*>(ResourceView::rcptr()); }

		//Get array element
		inline ConstElementType at(OffsetType index) { return IndirectLoad<Type>()((uint8_t*)rcptr(), index * sizeof(Type)); }
		inline ConstElementType at(OffsetType index) const { return IndirectLoad<Type>()((const uint8_t*)rcptr(), index * sizeof(Type)); }

		inline ConstElementType operator[](OffsetType index) { return at(index); }
		inline ConstElementType operator[](OffsetType index) const { return at(index); }

		//todo: use proper iterators to handle indirections
		template<typename = std::enable_if<std::is_same<Type, std::remove_reference<ElementType>::type>::value>::type>
		std::vector<Type> toVector() const { return std::vector<Type>(data(), data() + length()); }
	};

	class StringView : public ArrayView<char>
	{
	public:

		char* str() { return data(); }
		const char* str() const { return data(); }

		std::string stdStr() const { return std::string(data()); }
		void stdStr(std::string& str) const { str = data(); }
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Buffer class
	*/
	class ResourceBuffer : private std::vector<uint8_t>
	{
	public:
		
		using Base = std::vector<uint8_t>;
		
		//Constructors
		ResourceBuffer() {}
		inline ResourceBuffer(SizeType initialSize)
		{
			this->resize(initialSize);
		}
		
		template<typename Type>
		inline ResourceBuffer(const Type* data, SizeType dataSize)
		{
			this->resize(dataSize);
			memcpy((void*)this->pointer(), (const void*)data, dataSize);
		}
		
		inline ResourceBuffer(ResourceBuffer&& other) { *this = other; }
		inline ResourceBuffer(const ResourceBuffer& other) { *this = other; }
		
		//Operators
		ResourceBuffer& operator=(const ResourceBuffer& other) { ((Base&)*this) = ((const Base&)(other)); return *this; }
		ResourceBuffer& operator=(ResourceBuffer&& other) { ((Base&)*this) = ((Base&&)(other)); return *this; }
		
		/*
			Resize the buffer
		*/
		inline void resize(SizeType size)
		{
			Base::resize(size);
		}

		/*
			Extend the buffer
		*/
		inline void extend(SizeType size)
		{
			Base::resize(Base::size() + size);
		}
		
		/*
			Get pointer to data
		*/
		inline const uint8_t* pointer() const { return Base::data(); }
		inline uint8_t* pointer() { return Base::data(); }

		/*
			Gets size of the buffer
		*/
		inline SizeType size() const
		{
			return (SizeType)Base::size();
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Resource Stream class
	*/
	class ResourceStream
	{
	private:

		//Internal buffer
		ResourceBuffer m_streamBuf;

	public:

		ResourceStream() {}

		ResourceStream(const ResourceStream&) = delete;

		ResourceStream(ResourceStream&& other)
		{
			std::swap(m_streamBuf, other.m_streamBuf);
		}

		ResourceStream& operator=(ResourceStream& other)
		{
			std::swap(m_streamBuf, other.m_streamBuf);
			return *this;
		}

		//Push binary data into the pool
		inline OffsetType write(const void* bytes, SizeType bytesLength)
		{
			//Get current offset
			OffsetType o = m_streamBuf.size();

			//Grow data buffer
			m_streamBuf.extend(bytesLength);

			//Copy new data
			memcpy(m_streamBuf.pointer() + o, bytes, bytesLength);

			return o;
		}

		//Widen pool
		inline OffsetType widen(SizeType bytesWidth)
		{
			//Get current offset
			OffsetType o = m_streamBuf.size();

			//Grow data buffer
			m_streamBuf.extend(bytesWidth);

			return o;
		}

		//Get pointers
		uint8_t* pointer() { return m_streamBuf.pointer(); }
		const uint8_t* pointer() const { return m_streamBuf.pointer(); }

		//Get size of stream data
		SizeType size() const { return m_streamBuf.size(); }

		//Check if stream buffer is empty
		bool isEmpty() const { return size() == 0; }
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/*
		Resource builder class
	*/
	class ResourceBuilder
	{
	private:

		/*
			Size of the resource header

			[ header: SizeType size ]
		*/
		static const SizeType s_headerSize = sizeof(SizeType);

		//Parent resource builder
		ResourceBuilder* m_parent = nullptr;
		//Stream output
		ResourceStream m_stream;
		//Offset in stream to resource header
		OffsetType m_rootOffset;

		//uint32_t m_parentDepth;
		//uint32_t m_parentOffset;

		//Assert that ResourceBuilder::build() has not been called yet
		void assertNotIsBuilt()
		{
			if (isBuilt())
			{
				throw std::exception("ResourceBuiler::build() has already been called");
			}
		}

		/*
			Offset to start of resource data (excluding header)
		*/
		SizeType baseOffset() const
		{
			return m_rootOffset + s_headerSize;
		}

	protected:

		/*
			Get pointers to data portion of the resource (excluding header)
		*/
		inline const uint8_t* rcptr() const { return m_stream.pointer() + baseOffset(); }
		inline uint8_t* rcptr() { return m_stream.pointer() + baseOffset(); }

	public:

		/*
			Construct an optionally nested resource builder
		*/
		ResourceBuilder(ResourceBuilder* parent, SizeType resourceSize) :
			m_parent(parent)
		{
			//If parent is specified then take stream ownership
			if (m_parent != nullptr)
			{
				m_stream = std::move(m_parent->m_stream);
			}

			//Resource size must have space for header
			resourceSize += s_headerSize;

			m_rootOffset = m_stream.widen(resourceSize);
		}

		~ResourceBuilder()
		{
			if (isNested())
			{
				//Return stream ownership to parent
				std::swap(m_stream, m_parent->m_stream);
				//Reset parent pointer
				m_parent = nullptr;
			}
		}

		//Not copyable or moveable
		explicit ResourceBuilder(ResourceBuilder&&) = delete;
		explicit ResourceBuilder(const ResourceBuilder&) = delete;

		//Test if resource builder owns stream
		bool isRoot() const { return m_parent == nullptr; }
		bool isNested() const { return !isRoot(); }
		
		//Check if ResourceBuilder::build() has been called
		bool isBuilt() const { return m_stream.isEmpty(); }

		/*
			Allocate an array of basic types
		*/
		template<typename Type, typename = std::enable_if<IsBasicType<Type>::value>::type>
		Ref<ArrayView<Type>> createArray(const Type* data, SizeType dataLength)
		{
			//Assert that the resource can be modified
			assertNotIsBuilt();

			const SizeType dataSizeInBytes = dataLength * sizeof(Type);
			
			//Push array size
			OffsetType root = m_stream.write((const void*)&dataSizeInBytes, sizeof(SizeType));
			//Make array root offset relative to resource base
			root -= baseOffset();

			//Push array data
			m_stream.write((const void*)data, dataSizeInBytes);
			
			//Return array offset
			return (Ref<ArrayView<Type>>)root;
		}

		/*
			Allocate an array of reference types
		*/
		template<typename Type>
		Ref<ArrayView<Ref<Type>>> createArrayOfRefs(const Ref<Type>* data, SizeType dataLength)
		{
			//Assert that the resource can be modified
			assertNotIsBuilt();

			const SizeType dataSizeInBytes = dataLength * sizeof(OffsetType);
			
			//Push array size
			OffsetType root = m_stream.write((const void*)&dataSizeInBytes, sizeof(SizeType));
			//Make array root offset relative to resource base
			root -= baseOffset();

			//Push pointers
			for (SizeType i = 0; i < dataLength; i++)
			{
				//Reserve space for pointer
				OffsetType cur = m_stream.widen(sizeof(OffsetType));
				//Make reference relative to resource base
				cur -= baseOffset();

				//Write pointer
				Utils::storePointer(this->rcptr(), cur, (OffsetType)data[i]);
			}
			
			//Return array offset
			return (Ref<ArrayView<Ref<Type>>>)root;
		}

		/*
			Allocate array from std::vector
		*/
		template<typename Type>
		inline Ref<ArrayView<Type>> createArray(const std::vector<Type>& data)
		{
			return this->createArray<Type>(&data[0], (SizeType)data.size());
		}

		/*
			Allocate array of reference types from std::vector
		*/
		template<typename Type>
		inline Ref<ArrayView<Ref<Type>>> createArrayOfRefs(const std::vector<Ref<Type>>& data)
		{
			return this->createArrayOfRefs<Type>(&data[0], (SizeType)data.size());
		}

		/*
			Allocate array from given iterators
		*/
		template<typename Iterator, typename ValueType = std::iterator_traits<Iterator>::value_type>
		inline Ref<ArrayView<ValueType>> createArray(Iterator beg, Iterator end)
		{
			return createArray(std::vector<ValueType>(beg, end));
		}

		/*
			Allocate array of strings from std::vector of strings
		*/
		Ref<ArrayView<Ref<StringView>>> createArrayOfStrings(const std::vector<std::string>& data)
		{
			//Fill vector of stringview references
			std::vector<Ref<StringView>> strs;
			strs.reserve(data.size());
			for (const std::string& str : data)
			{
				//Allocate string
				strs.push_back(this->createString(str));
			}
			//Allocate array
			//return this->createArray<Ref<StringView>>(strs);
			return this->createArrayOfRefs<StringView>(strs);
		}

		/*
			Allocate a string
		*/
		inline Ref<StringView> createString(const char* str)
		{
			return createArray<char>(str, (SizeType)strlen(str) + 1);
		}

		inline Ref<StringView> createString(const std::string& str)
		{
			return createArray<char>(str.c_str(), (SizeType)str.size() + 1);
		}

		/*
			Allocate a raw buffer
		*/
		inline VoidRef createBuffer(const uint8_t* data, SizeType dataSize)
		{
			return (VoidRef)createArray<uint8_t>(data, dataSize);
		}

		inline VoidRef createBuffer(const ResourceBuffer& buffer)
		{
			return createBuffer(buffer.pointer(), buffer.size());
		}
		
		/*
			Finalize resource, returns offset to resource in stream
		*/
		Ref<ResourceView> build()
		{
			//Store size of resource data (excluding header)
			Utils::storeField(m_stream.pointer(), m_rootOffset, m_stream.size() - baseOffset());

			if (isRoot())
			{
				return 0;
			}
			else
			{
				//Get resource offset relative to parent resource data
				OffsetType roffset = m_rootOffset - m_parent->baseOffset();

				//Return stream ownership to parent
				std::swap(m_stream, m_parent->m_stream);
				//Reset parent pointer
				m_parent = nullptr;

				return roffset;
			}
		}

		/*
			Finalize resource and serialize to output
		*/
		Ref<ResourceView> build(std::ostream& out)
		{
			//Finalize resource
			auto root = build();

			//Serialize resource contents to output
			out.write((const char*)m_stream.pointer() + m_rootOffset, m_stream.size() - m_rootOffset);

			return root;
		}
	};
	
	/*
		Resource loader class
	*/
	class ResourceLoader
	{
	private:

		ResourceBuffer m_data;
		bool m_success;

	public:

		ResourceLoader() {}

		ResourceLoader(std::istream& in) :
			m_success(in.good())
		{
			load(in);
		}
		
		/*
			Read in binary data from stream
		*/
		void load(std::istream& in)
		{
			//Remember read ptr
			size_t pos = in.tellg();

			//Peek header
			SizeType totalSize = 0;
			in.read((char*)&totalSize, sizeof(SizeType));
			
			//Reset read ptr
			in.seekg(pos);

			totalSize += sizeof(SizeType);

			//Read entire resource
			m_data.resize(totalSize);
			in.read((char*)m_data.pointer(), totalSize);

			m_success = in.good();
		}

		/*
			Get loader state
		*/
		bool success() const { return m_success; }
		bool fail() const { return !m_success; }
		operator bool() const { return success(); }
		
		/*
			Deserialize resource of given type
		*/
		template<typename View, typename = std::enable_if<std::is_base_of<ResourceView, View>::value>::type>
		View& deserialize()
		{
			return *reinterpret_cast<View*>(m_data.pointer());
		}
		
		template<typename View, typename = std::enable_if<std::is_base_of<ResourceView, View>::value>::type>
		const View& deserialize() const
		{
			return *reinterpret_cast<const View*>(m_data.pointer());
		}
	};

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}
