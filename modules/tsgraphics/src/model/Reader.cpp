/*
	Model reader
*/

#include "Reader.h"

#include "tsgraphics/schemas/Model.rcs.h"

using namespace ts;
using namespace tsr;

ModelReader::ModelReader()
{
	ModelBuilder b;
	b.set_vertexStride(1);
	b.set_indexData(b.createArray<uint32>({ 0 }));
	b.set_vertexData(b.createArray<byte>({ 1, 3, 4, 5}));
	b.build();
}
