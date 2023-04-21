#include "Default2d.hpp"


const char* Default2d::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"
"layout (location = 1) in vec2 vt;\n"


"uniform mat4 position_matrix;\n"

"out vec2 texCoord;\n"

"void main()\n"
"{\n"
"   gl_Position = position_matrix * vec4(pos.x, pos.y, 0, 1.0);\n"
"	texCoord = vt;\n"
"}\0";
const char* Default2d::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"uniform sampler2D tex;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor = texture(tex,texCoord);\n"
//"	FragColor = vec4(0.,0.,0.,1.);\n"
" } ";