#include "Dynamic3d.hpp"


const char* Dynamic3d::vertex_code = "\n"
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
"	position = (model * vec4(pos.x, pos.y, pos.z, 1.0)).xyz;"
"	normal = (model* vec4(norm.x, norm.y, norm.z, 0.0)).xyz;"
"   gl_Position = perspective * camera * vec4(position.x, position.y, position.z, 1.0);\n"
"	texCoord = vt;\n"
"}\0";
const char* Dynamic3d::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"in vec3 position;\n"
"in vec3 normal;\n"

"uniform sampler2D tex;\n"
"uniform vec4 atmosphere_color;\n" //alpha is atmosphere strength
"uniform vec4 light_color;\n"
"uniform vec3 light_position;\n"
"uniform float light_strength;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"
"   float a = atmosphere_color.w * (length(position));"

"   vec3 light_dir = (light_position - position);\n"
"	float diff = max(dot(normal, normalize(light_dir)), 0.0);\n"
"	diff = (diff*light_strength*light_strength)/(light_strength*light_strength+dot(light_dir, light_dir));\n"//strength scaling
"	vec4 tex_pixel_data = texture(tex,texCoord);\n"
"   if(tex_pixel_data.w < .2) discard;\n"
"	vec3 tex_color = (diff + .3) * tex_pixel_data.xyz;\n"
//apply atmospheric perspective
"	FragColor.xyz = (tex_color + atmosphere_color.xyz * a)/(1 + a);\n"

"	FragColor.w = tex_pixel_data.w;\n"
" } ";