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
"out vec3 position;\n"
"out vec3 normal;"

"void main()\n"
"{\n"
"	position = (camera * model * vec4(pos.x, pos.y, pos.z, 1.0)).xyz;"
"	normal = (camera * model *  vec4(norm.x, norm.y, norm.z, 0.0)).xyz;"
"   gl_Position = perspective* vec4(position.x, position.y, position.z, 1.0);\n"
"	texCoord = vt;\n"
"}\0";

const char* Default3d::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"in vec3 position;\n"
"in vec3 normal;\n"

"uniform sampler2D tex;\n"
"uniform vec4 atmosphere_color;\n" //alpha is atmosphere strength
"uniform vec3 light_color;\n"
"uniform vec3 light_position;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"   float a = atmosphere_color.w * (length(position));"

"   vec3 light_dir = (light_position - position);\n"
"	light_dir = (light_dir*5)/(5+dot(light_dir, light_dir));"
"	float diff = max(dot(normal, light_dir), 0.0);\n"
"	vec3 tex_color = (diff + .2) * texture(tex,texCoord).xyz;\n"
//apply atmospheric perspective
"	FragColor.xyz = (tex_color + atmosphere_color.xyz * a)/(1 + a);\n"
"	FragColor.w = texture(tex,texCoord).w;\n"
" } ";