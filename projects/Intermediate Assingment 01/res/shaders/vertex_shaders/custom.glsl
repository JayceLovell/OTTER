#version 440

// Include our common vertex shader attributes and uniforms
#include "../fragments/vs_common.glsl"

void main() {
	vec3 vert = inPosition;

	vert.z = sin(vert.x * 50.0 + u_Time) * 0.01;

    //gl_Position = u_ModelViewProjection * vec4(vert, 1.0);
	//gl_Position = u_ModelViewProjection * vec4(inPosition, 1.0);

	// Lecture 5
	// Pass vertex pos in world space to frag shader
	//outWorldPos = (u_Model * vec4(vert, 1.0)).xyz;
	outViewPos = (u_Model * vec4(inPosition, 1.0)).xyz + vert;

	gl_Position = u_ModelViewProjection * vec4(outViewPos,1.0);


	// Normals
	outNormal = mat3(u_NormalMatrix) * inNormal;

    // We use a TBN matrix for tangent space normal mapping
    vec3 T = normalize(vec3(mat3(u_NormalMatrix) * inTangent));
    vec3 B = normalize(vec3(mat3(u_NormalMatrix) * inBiTangent));
    vec3 N = normalize(vec3(mat3(u_NormalMatrix) * inNormal));
    mat3 TBN = mat3(T, B, N);

    // We can pass the TBN matrix to the fragment shader to save computation
    outTBN = TBN;

	// Pass our UV coords to the fragment shader
	outUV = inUV;

	///////////
	outColor = inColor;

}