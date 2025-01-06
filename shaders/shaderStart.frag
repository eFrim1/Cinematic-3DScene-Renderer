#version 410 core

in vec3 fNormal;
in vec4 fPosEye;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

in vec3 fNormalWorld;
in vec3 viewDirWorld;

out vec4 fColor;

//lighting
uniform	vec3 lightDir;
uniform	vec3 lightColor;
uniform vec3 pointLightPos;

//texture
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;
uniform samplerCube skybox;
uniform samplerCube pointShadowMap;
float skyboxSpecularStrength = 0.5f;
vec3 skyboxSpecular;

vec3 ambient;
vec3 diffuse;
vec3 specular;
uniform float specularStrength = 0.5f;
uniform float shininess = 32.0f;
uniform float ambientStrength = 0.2f;

uniform float far_plane;

uniform bool showFog;
uniform bool pointShadow;


void computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(fNormal);
	vec3 lightDirN = normalize(lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 halfVector = normalize(lightDirN + viewDirN);
	ambient = ambientStrength * lightColor;
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;

	vec3 frag_reft = -reflect(viewDirWorld, fNormalWorld);
	vec3 colorFromSkybox = vec3(texture(skybox, frag_reft));
	skyboxSpecular += skyboxSpecularStrength * colorFromSkybox;
}

float computeShadow()
{
	float bias = 0.005f;
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	if(normalizedCoords.z > 1.0f)
		return 0.0f;
	normalizedCoords = normalizedCoords * 0.5f + 0.5f;
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;
	float currentDepth = normalizedCoords.z;
	float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

	return shadow;
}

float LinearizeDepth(float depth)
{
	float z = depth * 2.0 - 1.0; // Back to NDC
	return (2.0 * 1.0 * far_plane) / (far_plane + 1.0 - z * (far_plane - 1.0));
}

float computePointShadow() {
	vec3 fragToLight = fPosEye.xyz - pointLightPos;
	float closestDepth = texture(pointShadowMap, fragToLight).r; // Sample depth from cube map
	//closestDepth *= far_plane;
	float currentDepth = length(fragToLight); // Distance to light
	float bias = 0.05f; // Adjust for acne prevention
	float shadow = (currentDepth - bias > closestDepth) ? 1.0f : 0.0f;
	//fColor = vec4(vec3(LinearizeDepth(closestDepth) / far_plane), 1.0);
	return shadow;
}

float computeFog()
{
	float fogDensity;

	if(showFog)
		fogDensity = 0.25f;
	else
		fogDensity = 0.0f;

	float fragmentDistance = length(fPosEye);
	float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

	return clamp(fogFactor, 0.0f, 1.0f);
}


void main() 
{
	computeLightComponents();
	
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;

	skyboxSpecular *= texture(specularTexture, fTexCoords).r;

	float shadow = computeShadow();
	if(pointShadow){
		shadow = shadow-computePointShadow();
	}

	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow)*specular + skyboxSpecular, 1.0f);
    
	// Calculate fog
	float fogFactor = computeFog();
	vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);

	//vec4(color, 1.0f);
	fColor = mix(fogColor, vec4(color, 1.0f), fogFactor);
}
