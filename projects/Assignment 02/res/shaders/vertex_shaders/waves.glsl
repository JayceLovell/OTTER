#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

out vec3 color;
out vec2 texUV;

uniform mat4 MVP;

uniform sampler2D myTextureSampler;
//lecture 10
uniform float delta;

void main() {

	color = inColor;
	texUV = inUV;

	//lecture 10b
	vec3 vert = inPosition;
	//vert.y = texture(myTextureSampler, vertex_uv).r;

	//sin animation
	vert.y = sin(vert.x * 100.0 + delta) * 0.01;

	//To-do
	//Add transparency

	gl_Position = MVP * vec4(vert, 1.0);
	//gl_Position = MVP * vec4(vertex_pos, 1.0);

}

