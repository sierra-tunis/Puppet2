#include "ZMapper.h"


const char* ZMapper::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec3 norm;\n"

"uniform float room_id;\n"

"uniform mat4 camera;\n"
	
"uniform mat4 ZClip;"

"out vec4 Zdata;\n"

"void main()\n"
"{\n"
"	vec4 position = ZClip * camera * vec4(pos.x, pos.y, pos.z, 1.0);\n"//no model matrix since model does not move
"   gl_Position = position;\n"
"	Zdata = vec4(0.5-position.z/2.,norm.x/2.+.5,norm.z/2.+.5,room_id);"//red is Z height, green is y slope, blue is x slope, alpha is room_id (for now)
"}\0";

const char* ZMapper::fragment_code = "#version 330 core\n"
"in vec4 Zdata;\n "

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = Zdata;\n"
" } ";
uint8_t ZMapper::last_room_id_ = 1;