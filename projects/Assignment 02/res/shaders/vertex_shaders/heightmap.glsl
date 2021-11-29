#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

out vec3 color;
out vec2 texUV;

uniform sampler2D myTextureSampler;

void main() {
	
	color = inColor;
	texUV = inUV;

	//Lec 10b
	vec3 vert = inPosition;

	vert.y = texture(myTextureSampler,inUV).r;

	// sin animation
	//vert.y = sin(vert.x * 5.0 + uTime) * 0.2;

	gl_Position = u_ModelViewProjection * vec4(vert,1.0);

	//gl_Position = MVP * vec4(vertex_pos, 1.0);
}