#version 330 core

in vec2 texCoord;
layout(location = 0) out vec4 outColor;

uniform sampler2D ssaoTexture; // the ssao factor for each fragment
uniform sampler2D screenColorTexture; // the whole rendered screen

uniform float blurEnabled; // whether to use blurring (glsl has no bool type so we check > 0.5)

float offsets[5] = float[](-1.5f, -0.5f, 0.25f, 1.5f, 2.5f); // shifted a bit, somehow looks nicer
float kernel5[25] = float[](.003f, .013f, .022f, .013f, .003f,
                            .013f, .059f, .097f, .059f, .013f,
                            .022f, .097f, .159f, .097f, .022f,
                            .013f, .059f, .097f, .059f, .013f,
                            .003f, .013f, .022f, .013f, .003f);

float kernel3[9] = float[](.125f, .250f, .125f,
                           .250f, .500f, .250f,
                           .125f, .250f, .125f);

void main()
{

	vec3 screenColor = texture(screenColorTexture, texCoord).rgb;
	float AO = 0.0f;

	if (blurEnabled > 0.5) {

		// blur the ssao factors using gaussian kernel
		// note: the filter kernel could be seperated into two passes to save 15 iterations for each fragment
		for (int i = 0; i < 5; ++i) {
			for (int j = 0; j < 5; ++j) {
				vec2 tc = texCoord;
				tc.x = texCoord.x + offsets[j] / textureSize(ssaoTexture, 0).x;
				tc.y = texCoord.y + offsets[i] / textureSize(ssaoTexture, 0).y;
				AO += texture(ssaoTexture, tc).r * kernel5[i+5*j];
			}
		}
	}
	else {
		AO = texture(ssaoTexture, texCoord).r;
	}

	outColor = vec4(AO * screenColor, 1);

}
