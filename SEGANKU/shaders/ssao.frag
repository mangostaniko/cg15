#version 330 core

in vec2 texCoord;
layout(location = 0) out vec4 outColor;

const int RANDOM_VECTOR_ARRAY_MAX_SIZE = 32; // reference uses 64 [increase for better quality]
const float SAMPLE_RADIUS = 1.5f; // reference uses 1.5 [causes issues, currently not used]

uniform sampler2D screenColorTexture; // the whole rendered screen
uniform sampler2D viewPosTexture; // interpolated vertex positions in view space

uniform mat4 projMat;

// we use a uniform buffer object for better performance
layout (std140) uniform RandomVectors
{
    vec3 randomVectors[RANDOM_VECTOR_ARRAY_MAX_SIZE];
};

void main()
{

	vec3 screenColor = texture(screenColorTexture, texCoord).rgb;
    vec3 viewPos = texture(viewPosTexture, texCoord).xyz;

    float AO = 0.0f;

	// sample random points to compare depths around the view space position.
	// the more sampled points lie in front of the actual depth at the sampled position,
	// the higher the probability of the surface point to be occluded.
    for (int i = 0; i < RANDOM_VECTOR_ARRAY_MAX_SIZE; ++i) {

		// take a random sample point.
        vec3 samplePos = viewPos + randomVectors[i];

		// project sample point onto near clipping plane
		// to find the depth value (i.e. actual surface geometry)
		// at the given view space position for which to compare depth
        vec4 offset = vec4(samplePos, 1.0f);
        offset = projMat * offset; // project onto near clipping plane
        offset.xy /= offset.w; // perform perspective divide
        offset.xy = offset.xy * 0.5f + vec2(0.5f); // transform to [0,1] range
        float sampleActualSurfaceDepth = texture(viewPosTexture, offset.xy).z-0.1f;

		// compare depth of random sampled point to actual depth at sampled xy position:
		// the function step(edge, value) returns 1 if value > edge, else 0
		// thus if the random sampled point's depth is greater (lies behind) of the actual surface depth at that point,
		// the probability of occlusion increases.
		// note: in the reference, if actual depth at sampled position is too far off
		// from depth at fragment position, it doesnt add to occlusion, to avoid artifacts.
		// however, here this check causes the sky to disappear, thus it is disabled.
//		float distance = abs(viewPos.z - sampleActualSurfaceDepth);
//      if (distance < SAMPLE_RADIUS) {
			AO += step(sampleActualSurfaceDepth, samplePos.z);
//      }
    }

	// normalize the ratio of sampled points lying behind the surface to a probability in [0,1]
	AO = AO / float(RANDOM_VECTOR_ARRAY_MAX_SIZE);

	// mix screen color with black color depending on occlusion factor
	AO = AO*AO*1.5;
	vec3 mixedColor = (1.0f-AO) * vec3(0.0f) + AO * screenColor;

	outColor = vec4(vec3(mixedColor), 1.0f);
}
