#include "CollisionVisualizer.hpp"

const char* CollisionVisualizer::vertex_code = "\n"
"#version 330 core\n"
"layout (location = 0) in vec3 pos;\n"

"uniform mat4 perspective;\n"
"uniform mat4 camera;\n"
"uniform mat4 model;\n"


"void main()\n"
"{\n"
"   gl_Position = perspective * camera *model*vec4(pos.x, pos.y, pos.z, 1.0);\n"
"}\0";

const char* CollisionVisualizer::fragment_code = "#version 330 core\n"
"uniform vec3 color;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"	FragColor.xyz = color;\n"
"	FragColor.w = 1.0;\n"
" } ";