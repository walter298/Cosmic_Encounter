#pragma once

#include "novalis/RenderTypes.h"

template<typename Obj, std::invocable<const Obj&> Func> 
void setRenderLayout(std::vector<Obj>& objects, Func widthGetter,
	int x, int y, int spacing) 
{
	int objX = x;

	for (auto& obj : objects) {
		obj.setRenPos(objX, y);
		objX += widthGetter(obj) + spacing;
	}
}