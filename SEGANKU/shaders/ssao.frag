#version 330 core

in vec2 texCoord; // TODO: do we need this in range [0,1] ?
layout(location = 0) out vec4 outColor;

const int RANDOM_VECTOR_ARRAY_SIZE = 128;
const float SAMPLE_RADIUS = 10; // TODO: play with this value

uniform sampler2D screenColorTexture; // the whole rendered screen
uniform sampler2D viewPosTexture; // interpolated vertex positions in view space

// TODO PROVIDE:
uniform mat4 projMat;
uniform vec3 randomUnitVectors[RANDOM_VECTOR_ARRAY_SIZE];

void main()
{
	vec3 screenColor = texture(screenColorTexture, texCoord).rgb;
    vec3 viewPos = texture(viewPosTexture, texCoord).xyz;

    float AO = 0.0;

	// sample random points to compare depths around the view space position.
	// the more sampled points lie in front of the actual depth at the sampled position,
	// the higher the probability of the surface point to be occluded.
    for (int i = 0; i < RANDOM_VECTOR_ARRAY_SIZE; ++i) {

		// take a random sample point.
        vec3 samplePos = viewPos + randomUnitVectors[i];

		// project sample point onto near clipping plane
		// to find the depth value (i.e. actual surface geometry)
		// at the given view space position for which to compare depth
        vec4 offset = vec4(samplePos, 1.0);
        offset = projMat * offset; // project onto near clipping plane
        offset.xy /= offset.w; // perform perspective divide
        offset.xy = offset.xy * 0.5 + vec2(0.5); // transform to [0,1] range
        float sampleActualSurfaceDepth = texture(viewPosTexture, offset.xy).z;

		// compare depth of random sampled point to actual depth at sampled xy position:
		// the function step(edge, value) returns 1 if value > edge, else 0
		// thus if the random sampled point's depth is greater (lies behind) of the actual surface depth at that point,
		// the probability of occlusion increases.
		// note: if the actual depth at the sampled position is too far off from the depth at the fragment position,
		// i.e. the surface has a sharp ridge/crevice, it doesnt add to the occlusion, to avoid artifacts.
        if (abs(viewPos.z - sampleActualSurfaceDepth) < SAMPLE_RADIUS) {
            AO += step(sampleActualSurfaceDepth, samplePos.z);
        }
    }

	// normalize the ratio of sampled points lying behind the surface to a probability in [0,1]
	// the occlusion factor should make the color darker, not lighter, so we invert it.
    AO = 1.0 - AO/(float)RANDOM_VECTOR_ARRAY_SIZE;

    outColor = screenColor * vec4(pow(AO, 2.0)); // TODO: play with this function
}
