#pragma once
#include "Graphics.h"

#ifndef PUPPET_GRAPHICS_NODRAW
#define PUPPET_GRAPHICS_NODRAW
/*
struct null_grobj: public GraphicsObject{
	null_grobj(Eigen::Matrix4f pos) :GraphicsObject() {}
};

class NoDraw :public Graphics<null_grobj> {
	virtual void drawGrobj(const null_grobj& grobj) const override {};
};

const char* NoDraw::vertex_code = "";
const char* NoDraw::fragment_code = "";
*/
#endif