#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fNormal;
out vec4 fPosEye;
out vec2 fTexCoords;
out vec4 fragPosLightSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform	mat3 normalMatrix;
uniform mat4 lightSpaceTrMatrix;
uniform vec3 cameraPosWorld;


out vec3 fNormalWorld;
out vec3 viewDirWorld;

void main() 
{
	//compute eye space coordinates

	fPosEye = view * model * vec4(vPosition, 1.0f);
	fNormal = normalize(normalMatrix * vNormal);
	fTexCoords = vTexCoords;
	fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);

	mat3 normalMatrixSkybox = transpose(inverse(mat3(model))); // Extract 3x3, inverse, and transpose
	fNormalWorld = normalize(normalMatrixSkybox * vNormal); // Transform and normalize
	viewDirWorld = normalize((model * vec4(vPosition,1)).xyz - cameraPosWorld);
}
