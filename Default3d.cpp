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
"	position = ( model * vec4(pos.x, pos.y, pos.z, 1.0)).xyz;"
"	normal = (model *  vec4(norm.x, norm.y, norm.z, 0.0)).xyz;"
"   gl_Position = perspective * camera *vec4(position.x, position.y, position.z, 1.0);\n"
"	texCoord = vt;\n"
"}\0";

const char* Default3d::fragment_code = "#version 330 core\n"
"in vec2 texCoord;\n "
"in vec3 position;\n"
"in vec3 normal;\n"

"uniform sampler2D tex;\n"

"uniform vec4 overlay_color;\n"

"uniform vec4 atmosphere_color;\n" //alpha is atmosphere strength
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
"   float a = atmosphere_color.w * (length(position));"
"	float diff = 0;"
"   vec3 light_dir = (light_position - position);\n"
"	diff += (max(dot(normal, normalize(light_dir)), 0.0)*light_strength*light_strength)/(light_strength*light_strength+dot(light_dir, light_dir));\n"//strength scaling

"   light_dir = (light_position_1 - position);\n"
"	diff += (max(dot(normal, normalize(light_dir)), 0.0)*light_strength_1*light_strength_1)/(light_strength_1*light_strength_1+dot(light_dir, light_dir));\n"
"   light_dir = (light_position_2 - position); \n"
"	diff += (max(dot(normal, normalize(light_dir)), 0.0)*light_strength_2*light_strength_2)/(light_strength_2*light_strength_2+dot(light_dir, light_dir));\n"
"   light_dir = (light_position_3 - position); \n"
"	diff += (max(dot(normal, normalize(light_dir)), 0.0)*light_strength_3*light_strength_3)/(light_strength_3*light_strength_3+dot(light_dir, light_dir));\n"

"	vec3 tex_color = (diff + .3) * texture(tex,texCoord).xyz;\n"
//apply atmospheric perspective
"	FragColor.xyz = (tex_color + atmosphere_color.xyz * a)/(1 + a)*(1-overlay_color.w) + overlay_color.xyz*overlay_color.w;\n"
"	FragColor.w = texture(tex,texCoord).w;\n"
" } ";