#include "DebugGraphics.h"

const char* HboxGraphics::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec3 color;\n"

"uniform mat4 perspective;\n"
"uniform mat4 camera;\n"
"uniform mat4 model;\n"

"out vec3 vert_color;\n"

"void main()\n"
"{\n"
"	vert_color = color;\n"
"   gl_Position = perspective * camera * model * vec4(pos.x, pos.y, pos.z, 1.0);\n"
"}\0";
const char* HboxGraphics::fragment_code = "#version 330 core\n"
"in vec3 vert_color;\n "

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = vec4(vert_color.x,vert_color.y,vert_color.z,1.);\n"
//"	FragColor = vec4(0.,0.,0.,1.);\n"
" } ";