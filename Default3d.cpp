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
"out vec3 rel_position;\n"
"out vec3 normal;"

"void main()\n"
"{\n"
"	position = ( model * vec4(pos.x, pos.y, pos.z, 1.0)).xyz;"
"	normal = (model *  vec4(norm.x, norm.y, norm.z, 0.0)).xyz;"
"   rel_position = (camera * vec4(position.x, position.y, position.z, 1.0)).xyz;"
"   gl_Position = perspective *vec4(rel_position.x, rel_position.y, rel_position.z, 1.0);\n"
"	texCoord = vt;\n"
"}\0";

const char* Default3d::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"in vec3 position;\n"
"in vec3 rel_position;\n"
"in vec3 normal;\n"

"uniform sampler2D tex;\n"

"uniform vec4 overlay_color;\n"

"uniform vec4 atmosphere_color;\n" //alpha is atmosphere strength
"uniform float ambient_light;\n"
"uniform float white_reduction;\n"
"uniform vec4 light_color;\n"
"uniform vec3 light_position;\n"
"uniform float light_strength;\n"


"uniform vec4 light_color_1;\n"
"uniform vec3 light_position_1;\n"
"uniform float light_strength_1;\n"
"uniform vec4 light_color_2;\n"
"uniform vec3 light_position_2;\n"
"uniform float light_strength_2;\n"
"uniform vec4 light_color_3;\n"
"uniform vec3 light_position_3;\n"
"uniform float light_strength_3;\n"

"out vec4 FragColor;\n"

"void main()\n"
"{\n"

"   float a = 1-exp(-atmosphere_color.w * length(rel_position));\n"
"	float diff = 0;\n"
"   vec3 light_dir = (light_position - position);\n"
"	diff += (max(dot(normal, normalize(light_dir)), 0.0)*light_strength*light_strength)/(light_strength*light_strength+dot(light_dir, light_dir));\n"//strength scaling

"   light_dir = (light_position_1 - position);\n"
"	diff += (max(dot(normal, normalize(light_dir)), 0.0)*light_strength_1*light_strength_1)/(light_strength_1*light_strength_1+dot(light_dir, light_dir));\n"
"   light_dir = (light_position_2 - position); \n"
"	diff += (max(dot(normal, normalize(light_dir)), 0.0)*light_strength_2*light_strength_2)/(light_strength_2*light_strength_2+dot(light_dir, light_dir));\n"
"   light_dir = (light_position_3 - position); \n"
"	diff += (max(dot(normal, normalize(light_dir)), 0.0)*light_strength_3*light_strength_3)/(light_strength_3*light_strength_3+dot(light_dir, light_dir));\n"

"	vec3 tex_color_raw = texture(tex,texCoord).xyz;\n"
"	vec3 tex_color = (diff + ambient_light*(1-white_reduction*length(tex_color_raw))) * texture(tex,texCoord).xyz;\n"
//apply atmospheric perspective
"	FragColor.xyz = (tex_color*(1-overlay_color.w) + overlay_color.xyz*overlay_color.w)*(1 - a) + atmosphere_color.xyz * a;\n"
"	FragColor.w = texture(tex,texCoord).w;\n"
" } ";