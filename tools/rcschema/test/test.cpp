/*
	Resource Schema Generator conformance tester
*/

#include <tscore/types.h>
#include <sstream>
#include <iostream>

using namespace std;
using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper functions
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Assertion helper
void _assert(const char* func, const char* expr, bool eval)
{
	if (!eval)
	{
		cerr << "[" << func << "] Assertion failed: " << expr << endl;
		exit(-1);
	}
}

#define assert(expr) _assert(__FUNCTION__, #expr, (expr))

#define array_length(x) sizeof(x) / sizeof(x[0])

template<typename Type, size_t length>
void assertArrayEquals(Type(&arr)[length], const Type* arrPtr)
{
	for (int i = 0; i < length; i++)
	{
		assert(arr[i] == *(arrPtr + i));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generated headers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Test0.rcs.h"
#include "Test1.rcs.h"
#include "Test2.rcs.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test cases
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void test0()
{
	using namespace ts::rc;

	stringstream data(ios::binary | ios::out | ios::in);
	
	Test0Builder builder;
	builder.set_field0('\xff');	//byte
	builder.set_field1(true);	//bool
	builder.set_field2(-1);		//int16
	builder.set_field3(-1);		//int32
	builder.set_field4(-1);		//int64
	builder.set_field5(1);		//uint16
	builder.set_field6(1);		//uint32
	builder.set_field7(1);		//uint64
	builder.set_field8(1.0f);	//float32
	builder.set_field9(1.0);	//float64

	builder.build(data);
	assert(data.good());

	Test0Loader loader(data);
	assert(data.good());

	assert(loader.get_field0() == (byte)'\xff');
	assert(loader.get_field1() == true);
	assert(loader.get_field2() == -1);
	assert(loader.get_field3() == -1);
	assert(loader.get_field4() == -1);
	assert(loader.get_field5() == 1);
	assert(loader.get_field6() == 1);
	assert(loader.get_field7() == 1);
	assert(loader.get_field8() == 1.0f);
	assert(loader.get_field9() == 1.0);
}

void test1()
{
	using namespace ts::rc;

	stringstream data(ios::binary | ios::out | ios::in);

	uint32 array0[] = { 1, 3, 2, 5, 4 };
	float array1[] = { 0.5f, 0.5f, 0.5f };
	
	//Test builder
	Test1Builder builder;
	builder.set_s0("123abc");
	builder.set_array0(array0, array_length(array0));
	builder.set_array1(array1, array_length(array1));

	builder.build(data);
	assert(data.good());

	//Test loader
	Test1Loader loader(data);
	assert(data.good());

	assert(String(loader.get_s0()) == "123abc");
	assert(loader.length_array0() == array_length(array0));
	assert(loader.length_array1() == array_length(array1));
	assertArrayEquals(array0, loader.get_array0());
	assertArrayEquals(array1, loader.get_array1());
	
}

void test2()
{
	using namespace ts::rc;

	stringstream data(ios::binary | ios::out | ios::in);

	Vector3 pos = { 15.0f, 30.0f, 5.0f};
	Vector3 vel = { 4.0f, 3.0f, 0.0f };

	Matrix3x3 rot = {
		{ 1.0f, 0.0f, 0.0f },
		{ 0.0f, 1.0f, 0.0f },
		{ 0.0f, 0.0f, 1.0f }
	};

	Test2Builder builder;
	builder.set_position(pos);
	builder.set_velocity(vel);
	builder.set_rotation(rot);
	builder.build(data);
	assert(data.good());

	Test2Loader loader(data);
	assert(loader.get_position().x == pos.x);
	assert(loader.get_position().y == pos.y);
	assert(loader.get_position().z == pos.z);
	assert(loader.get_velocity().x == vel.x);
	assert(loader.get_velocity().y == vel.y);
	assert(loader.get_velocity().z == vel.z);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int main(int argc, char** argv)
{
	//Execute test cases
	test0();
	test1();
	test2();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
