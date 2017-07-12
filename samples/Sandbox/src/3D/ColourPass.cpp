/*
	Colour Pass class source
*/

#include "ColourPass.h"

using namespace ts;
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////

ColourPass::ColourPass(GraphicsContext* context) :
	Pass(context)
{
	this->initDisplayTarget();
}

void ColourPass::add(GraphicsContext::ItemID item, const SDrawCommand command)
{
	this->setItemCommand(item, command);
}

HDrawCmd ColourPass::get(GraphicsContext::ItemID item)
{
	return this->getItemCommand(item);
}

//////////////////////////////////////////////////////////////////////////////////////////
