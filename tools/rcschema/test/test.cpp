/*
	Resource Schema Generator conformance tester
*/

#include <tscore/types.h>
#include <tscore/strings.h>
#include <sstream>
#include <iostream>

using namespace std;
using namespace ts;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Generated headers
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Test0.rcs.h"
#include "Test1.rcs.h"
#include "Test2.rcs.h"
#include "Test3.rcs.h"

using namespace test;

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

template<typename Type, size_t expectLength>
void assertArrayEquals(Type(&expectArr)[expectLength], const rc::ArrayView<Type>& actualArr)
{
	assert(expectLength == actualArr.length());

	for (int i = 0; i < expectLength; i++)
	{
		assert(expectArr[i] == actualArr.at(i));
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Test cases
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void test0()
{
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

	//Test loader
	rc::ResourceLoader loader(data);
	assert(loader.success());

	Test0& reader = loader.deserialize<Test0>();
	
	assert(reader.field0() == (byte)'\xff');
	assert(reader.field1() == true);
	assert(reader.field2() == -1);
	assert(reader.field3() == -1);
	assert(reader.field4() == -1);
	assert(reader.field5() == 1);
	assert(reader.field6() == 1);
	assert(reader.field7() == 1);
	assert(reader.field8() == 1.0f);
	assert(reader.field9() == 1.0);
}

void test1()
{
	stringstream data(ios::binary | ios::out | ios::in);

	uint32 array0[] = { 1, 3, 2, 5, 4 };
	byte array1[] = { 0, 3, 127, 255, 1 };

	vector<String> arrayOfStrings = { "abc", "123", "def", "456", "*&^$$£%$7637GGyugy" };

	//Test builder
	Test1Builder builder;
	builder.set_str(builder.createString("123abc"));
	builder.set_array0(builder.createArray(array0, array_length(array0)));
	builder.set_array1(builder.createArray(array1, array_length(array1)));
	builder.set_strArray(builder.createArrayOfStrings(arrayOfStrings));

	builder.build(data);
	assert(data.good());

	//Test loader
	rc::ResourceLoader loader(data);
	assert(loader.success());

	Test1& reader = loader.deserialize<Test1>();

	assert(String(reader.str().data()) == "123abc");
	assertArrayEquals(array0, reader.array0());
	assertArrayEquals(array1, reader.array1());
	
	for (uint32 i = 0; i < reader.strArray().length(); i++)
	{
		assert(arrayOfStrings[i] == reader.strArray().at(i).data());
	}
}

void test2()
{
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
	builder.set_flag(FLAG_2);
	
	builder.build(data);
	assert(data.good());

	//Test loader
	rc::ResourceLoader loader(data);
	assert(loader.success());

	Test2& reader = loader.deserialize<Test2>();

	assert(reader.position().x == pos.x);
	assert(reader.position().y == pos.y);
	assert(reader.position().z == pos.z);
	
	assert(reader.velocity().x == vel.x);
	assert(reader.velocity().y == vel.y);
	assert(reader.velocity().z == vel.z);
	
	assert(reader.flag() == FLAG_2);
}

void test3()
{
	// Test binary tree
	{
		stringstream data(ios::binary | ios::out | ios::in);

		/*
			  4
			/   \
		   2     6
				/
			   5
		*/

		BinaryTreeBuilder tBuilder;
		NodeBuilder rootNode(tBuilder);

		{
			//Build left child of root node
			NodeBuilder LNode(rootNode);
			LNode.set_value(2);
			rootNode.set_nodeLeft(LNode.build());

			//Build right child of root node
			NodeBuilder RNode(rootNode);
			RNode.set_value(6);
			{
				//Build left child of right child of root node
				RNode.set_nodeLeft(NodeBuilder(RNode).set_value(5).build());
			}
			rootNode.set_nodeRight(RNode.build());
		}
		//Build root node
		tBuilder.set_rootNode(rootNode.set_value(4).build());
		//Build tree
		tBuilder.build(data);

		assert(data.good());

		rc::ResourceLoader loader(data);
		assert(loader.success());

		BinaryTree& tree = loader.deserialize<BinaryTree>();

		assert(tree.rootNode().value() == 4);
		assert(tree.rootNode().nodeLeft().value() == 2);
		assert(tree.rootNode().nodeRight().value() == 6);
		assert(tree.rootNode().nodeRight().nodeLeft().value() == 5);
	}

	////////////////////////////////////////////////////////////////////////////

	// Test tree
	{
		stringstream data(ios::binary | ios::out | ios::in);

		TreeBuilder t;
		TreeNodeBuilder root(t);

		//Create array of nodes
		rc::Ref<TreeNode> nodes[] =
		{
			TreeNodeBuilder(root).set_value(1).build(),
			TreeNodeBuilder(root).set_value(2).build(),
			TreeNodeBuilder(root).set_value(3).build(),
			TreeNodeBuilder(root).set_value(4).build(),
			TreeNodeBuilder(root).set_value(5).build()
		};

		//Set array of nodes to root
		root.set_nodes(root.createArrayOfRefs<TreeNode>(nodes, array_length(nodes)));
		//Set root value
		root.set_value(11);
		//Set root node
		t.set_root(root.build());
		//Build tree
		t.build(data);

		rc::ResourceLoader loader(data);
		assert(loader.success());

		Tree& tree = loader.deserialize<Tree>();
		
		assert(tree.has_root() == true);
		assert(tree.root().nodes().at(0).has_nodes() == false);

		assert(tree.root().value() == 11);

		assert(tree.root().nodes().length() == 5);
		
		assert(tree.root().nodes().at(0).value() == 1);
		assert(tree.root().nodes().at(1).value() == 2);
		assert(tree.root().nodes().at(2).value() == 3);
		assert(tree.root().nodes().at(3).value() == 4);
		assert(tree.root().nodes().at(4).value() == 5);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
	//Execute test cases
	test0();
	test1();
	test2();
	test3();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
