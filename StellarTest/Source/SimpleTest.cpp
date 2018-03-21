#include "Graphics/Color.h"

using namespace Graphics;

void main()
{
	Color a(0.5f, 0.5f, 0.5f);
	auto b = a.R11G11B10F(false);
	auto c = b;
}