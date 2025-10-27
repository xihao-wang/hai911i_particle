#version 430

#extension GL_EXT_geometry_shader4 : enable
#extension GL_EXT_gpu_shader4 : enable

layout ( points ) in;

// Shader de passage : passe de point vers point
layout ( triangle_strip, max_vertices = 4 ) out;
// Shader utile : passe de point vers triangles
// layout ( triangle_strip, max_vertices = 4 ) out;

// Les matrices peuvent être réutilisées
uniform mat4 mvp;

// Variables d'entrées : des tableaux
in vec3 initialVertPos[];

// Variables de sorties : des éléments uniques
out vec2 UV;

uniform vec3 camRight;
uniform vec3 camUp;



//Geometry Shader entry point
void main(void) {
	// Taille d'une particule
	float scale = 0.05;

	vec3 C =initialVertPos[0];
	vec3 R = normalize(camRight) * scale;
    vec3 U = normalize(camUp)    * scale;

	vec3 corners[4];
    vec2 uvs[4];

	corners[0] = C - R - U;
	uvs[0] = vec2(0.0,0.0);
	corners[1] = C + R - U;
	uvs[1] = vec2(0.0,1.0);
	corners[2] = C - R + U;
	uvs[2] = vec2(1.0,0.0);
	corners[3] = C + R + U;
	uvs[3] = vec2(1.0,1.0);
	
	for(int i=0;i<4;i++){
		gl_Position = mvp * vec4(corners[i],1.0);
		UV = uvs[i];
		EmitVertex();

	}
	// gl_Position = gl_in[0].gl_Position;
	// EmitVertex();
	EndPrimitive();
}