#version 460

uniform sampler2D mainTex;
uniform sampler2DShadow shadowTex;
uniform sampler2D diffuseTex;
uniform mat4 modelMatrix 	= mat4(1.0f);

const int NUM_LIGHTS = 2;

struct Light{
 	vec3	lightPos;
	float	lightRadius;
	vec4	lightColour;
	vec3 	spotDir;
};

uniform Light spotLights[NUM_LIGHTS];
uniform Light dirLight;

uniform vec3 cameraPos;
uniform bool hasTexture;

in Vertex{ 
	vec4 colour;
	vec2 texCoord;
	vec4 shadowProj;
	vec3 normal;
	vec3 worldPos;
} IN;

out vec4 fragColor;

vec4 directionalLight(vec4 color){

	float shadow = 1.0; // New !
	
	if( IN . shadowProj . w > 0.0) { // New !
		shadow = textureProj ( shadowTex , IN . shadowProj ) * 0.5f;
	}

	vec3 lightDir = normalize(dirLight.lightPos);

    float intensity = smoothstep(0.1, 0.6, dot(lightDir, IN.normal));

    if (intensity > 0.95)
		color = vec4(color.xyz * 0.8, 1.0);
	else if (intensity > 0.6)
		color = vec4(color.xyz * 0.5,1.0);
	else if (intensity > 0.25)
		color = vec4(color.xyz * 0.2,1.0);
	else
		color = vec4(color.xyz * 0.1,1.0);

	shadow = clamp(shadow, 0.2, 1.0);
	color *= 2.0;
	return color * shadow;
}

vec4 spotLightCalc(vec4 color){
	for(int i = 0; i < NUM_LIGHTS; i++){

		float shadow = 1.0; // New !
	
		if( IN . shadowProj . w > 0.0) { // New !
			shadow = textureProj ( shadowTex , IN . shadowProj ) * 0.5f;
		}

		vec3 incident = normalize ( spotLights[i].lightPos - IN . worldPos );
		vec3 viewDir = normalize ( cameraPos - IN . worldPos );
		vec3 halfDir = normalize ( incident + viewDir );
		
		float theta = dot(incident, normalize(-spotLights[i].spotDir));
		float cutoff = cos(radians(spotLights[i].lightRadius));

		vec4 diffuse = texture ( diffuseTex , IN . texCoord );
		float lambert = max ( dot ( incident , IN.normal), 0.0f );
		float distance = length ( spotLights[i].lightPos - IN . worldPos );
		float attenuation = 1.0 - clamp ( distance / spotLights[i].lightRadius , 0.0 , 1.0);
		float specFactor = clamp ( dot ( halfDir , IN.normal ) ,0.0 ,1.0);
		specFactor = pow ( specFactor , 100.0 );
		vec3 surface = ( diffuse . rgb * color . rgb );

		if (theta > cutoff){
			vec3 result = surface * lambert * attenuation;
			result += (color.rgb * specFactor) * attenuation * 0.33;
			result += surface * 0.1f;
			result *= 10.0f;

			shadow = clamp(shadow, 0.2, 1.0);
			result *= shadow;
			return vec4(result, 1.0f);
		}
	}
}

void main()
{
	vec4 colour = IN.colour;

    if(hasTexture){
		colour *= texture(mainTex, IN.texCoord);
    }

	fragColor = directionalLight(colour);
	fragColor += spotLightCalc(colour);

}