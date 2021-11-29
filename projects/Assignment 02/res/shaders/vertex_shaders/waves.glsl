#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

uniform float delta;
uniform sampler2D myTextureSampler;

void main() {

	outColor = inColor;
	outUV = inUV;

	//lecture 10b
	vec3 vert = inPosition;
	//vert.y = texture(myTextureSampler, vertex_uv).r;

	//sin animation
	vert.y = sin(vert.x * 100.0 + delta) * 0.01;

	//To-do
	//Add transparency

	gl_Position = u_ModelViewProjection * vec4(vert, 1.0);
	//gl_Position = MVP * vec4(vertex_pos, 1.0);

}

