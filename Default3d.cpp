#include "Default3d.h"


const char* Default3d::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec3 norm;\n"
"layout (location = 2) in vec2 vt;\n"

"uniform mat4 perspective;\n"
"uniform mat4 camera;\n"
"uniform mat4 model;\n"

"out vec2 texCoord;\n"

"void main()\n"
"{\n"
"   gl_Position = perspective * camera * model * vec4(pos.x, pos.y, pos.z, 1.0);\n"
"	texCoord = vt;\n"
"}\0";
const char* Default3d::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"uniform sampler2D tex;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = texture(tex,texCoord);\n"
" } ";