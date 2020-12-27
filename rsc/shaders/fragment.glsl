#version 330 core

#define SQRT2 1.41421356273
#define SQRT3 1.73205080757
#define MAX_WAVE_CENTERS 20 

in vec3 modelPos;
in vec3 normal;
in vec3 toLight;

uniform int[512] perm;
uniform float time;
uniform vec3 toCamera;

uniform int effect;	// Chooses a perlin texture to apply.

uniform float persistence;
uniform int octaveCount;
uniform int octaveStart;
uniform float ringFreq;

uniform float minFreq;
uniform float maxFreq;
uniform float phaseSpeed;
uniform int waveCenters;

out vec4 fragColor;

/**
 *	Input a t in the range [0,1] and outputs
 *	a smoothed values also in the range [0,1].
 *	Uses the function 6t^5 - 15t^4 + 10t^3 
 */
float ease(float t)
{
	return ((6*t - 15)*t + 10)*t*t*t;
}

vec2 getGradient2D(int cornerValue)
{
	// return one of four gradient vectors.
	int v = cornerValue & 3;	
	if (v == 0)
		return vec2(SQRT2, 0);
	else if (v == 1)
		return vec2(0, SQRT2);
	else if (v == 2)
		return vec2(-SQRT2, 0);
	else 
		return vec2(0, -SQRT2);
}

vec3 getGradient3D(int cornerValue)
{
	// return one of eight gradient vectors.
	int v = cornerValue & 7;	
	if (v == 0)
		return vec3(SQRT3, 0, 0);
	else if (v == 1)
		return vec3(0, SQRT3, 0);
	else if (v == 2)
		return vec3(-SQRT3, 0, 0);
	else if (v == 3)
		return vec3(0, -SQRT3, 0);
	else if (v == 4)
		return vec3(0, 0, SQRT3);
	else if (v == 5)
		return vec3(0, 0, -SQRT3);
	else if (v == 6)
		return vec3(0, -SQRT3 / SQRT2, SQRT3 / SQRT2);
	else
		return vec3(0, SQRT3 / SQRT2, -SQRT3 / SQRT2);
}

/**
 *	Computes the 2D perlin noise.
 *	Returns a value in the range [0,1].
 */
float noise(vec2 vec)
{
	// Get the lower left corner of the grid.
	int xi = int(vec.x) & 255;
	int yi = int(vec.y) & 255;

	// Compute the vector pointing from the corner to the
	// given point. Place the fractional part of point in [0,1]^2.
	vec2 frac = vec - vec2(int(vec.x), int(vec.y));

	//if (frac.x < 0)
	//	frac.x += 1.0f;
	//if (frac.y < 0)
	//	frac.y += 1.0f;
	vec2 fracSign = (-sign(frac) + vec2(1.0)) * 0.5;
	frac += fracSign;

	vec2 topRight = frac - vec2(1.0f, 1.0f); 
	vec2 topLeft = frac - vec2(0.0f, 1.0f); 
	vec2 botRight = frac - vec2(1.0f, 0.0f); 
	vec2 botLeft = frac;

	// Get a value from permuation matrix for the four
	// corners of the grid cell. Take care to keep index in bounds.
	int valueTopRight = perm[ perm[xi + 1] + yi + 1 ];
	int valueTopLeft = perm[ perm[xi] + yi + 1 ];
	int valueBotRight = perm[ perm[xi + 1] + yi ];
	int valueBotLeft = perm[ perm[xi] + yi ];

	// Take the dot between the vector from corner to point and
	// the gradient vector of the corner.
	float dotTopRight = dot(topRight, getGradient2D(valueTopRight));
	float dotTopLeft = dot(topLeft, getGradient2D(valueTopLeft));
	float dotBotRight = dot(botRight, getGradient2D(valueBotRight));
	float dotBotLeft = dot(botLeft, getGradient2D(valueBotLeft));

	// Interpolate first vertically then horizontally.
	// First ease the fractional values to create a smooth
	// transistion between grids.
	float u = ease(frac.x);
	float v = ease(frac.y);
	float vert1 = mix(dotBotLeft, dotTopLeft, v);
	float vert2 = mix(dotBotRight, dotTopRight, v);
	float value = mix(vert1, vert2, u);

	// put in range [0,1]. -2 <= dotprod <= 2.
	value = (value + 2.0) * 0.25;
	return value;
}

/**
 *	Computes the 3D perlin noise.
 *	Returns a value in the range [0,1].
 */
float noise(vec3 vec)
{
	// Get the lower back left corner of the cube.
	int xi = int(floor(vec.x)) & 255;
	int yi = int(floor(vec.y)) & 255;
	int zi = int(floor(vec.z)) & 255;

	// Compute the vector pointing from the corner to the
	// given point. Place the fractional part of point in [0,1]^3.
	vec3 frac = vec - floor(vec);

	vec3 frontTopRight = frac - vec3(1.0f, 1.0f, 1.0f); 
	vec3 frontTopLeft = frac - vec3(0.0f, 1.0f, 1.0f); 
	vec3 frontBotRight = frac - vec3(1.0f, 0.0f, 1.0f); 
	vec3 frontBotLeft = frac - vec3(0.0f, 0.0f, 1.0f);

	vec3 backTopRight = frac - vec3(1.0f, 1.0f, 0.0f); 
	vec3 backTopLeft = frac - vec3(0.0f, 1.0f, 0.0f); 
	vec3 backBotRight = frac - vec3(1.0f, 0.0f, 0.0f); 
	vec3 backBotLeft = frac - vec3(0.0f, 0.0f, 0.0f);

	// Get a value from permuation matrix for the eight
	// corners of the grid cell. Take care to keep index in bounds.
	int valueFrontTopRight = perm[ perm[ perm[xi + 1] + yi + 1 ] + zi + 1];
	int valueFrontTopLeft = perm[ perm[ perm[xi] + yi + 1 ] + zi + 1];
	int valueFrontBotRight = perm[ perm[ perm[xi + 1] + yi ] + zi + 1];
	int valueFrontBotLeft = perm[ perm[ perm[xi] + yi ] + zi + 1];

	int valueBackTopRight = perm[ perm[ perm[xi + 1] + yi + 1 ] + zi];
	int valueBackTopLeft = perm[ perm[ perm[xi] + yi + 1 ] + zi];
	int valueBackBotRight = perm[ perm[ perm[xi + 1] + yi ] + zi];
	int valueBackBotLeft = perm[ perm[ perm[xi] + yi ] + zi];

	// Take the dot between the vector from corner to point and
	// the gradient vector of the corner.
	float dotFrontTopRight = dot(frontTopRight, getGradient3D(valueFrontTopRight));
	float dotFrontTopLeft = dot(frontTopLeft, getGradient3D(valueFrontTopLeft));
	float dotFrontBotRight = dot(frontBotRight, getGradient3D(valueFrontBotRight));
	float dotFrontBotLeft = dot(frontBotLeft, getGradient3D(valueFrontBotLeft));

	float dotBackTopRight = dot(backTopRight, getGradient3D(valueBackTopRight));
	float dotBackTopLeft = dot(backTopLeft, getGradient3D(valueBackTopLeft));
	float dotBackBotRight = dot(backBotRight, getGradient3D(valueBackBotRight));
	float dotBackBotLeft = dot(backBotLeft, getGradient3D(valueBackBotLeft));

	// Interpolate first vertically then horizontally.
	// First ease the fractional values to create a smooth
	// transistion between grids.
	float u = ease(frac.x);
	float v = ease(frac.y);
	float w = ease(frac.z);
	//float u = (frac.x);
	//float v = (frac.y);
	//float t = (frac.z);
	float frontVert1 = mix(dotFrontBotLeft, dotFrontTopLeft, v);
	float frontVert2 = mix(dotFrontBotRight, dotFrontTopRight, v);
	float frontHorz = mix(frontVert1, frontVert2, u);
	float backVert1 = mix(dotBackBotLeft, dotBackTopLeft, v);
	float backVert2 = mix(dotBackBotRight, dotBackTopRight, v);
	float backHorz = mix(backVert1, backVert2, u);
	float value = mix(backHorz, frontHorz, w);

	// put in range [0,1]. -3 <= dotprod <= 3.
	value = (value + 3.0) * 0.16667;
	return value;
}

/**
 * Computes the instantaneous rate of change of the noise function at
 * the given point.
 */
vec3 diffNoise(vec3 vec)
{
	float h = 0.0001;
	// Use the centered difference formula to compute the partial
	// derivative at each component.
	vec3 x1 = vec + vec3(h,0,0);
	vec3 x2 = vec + vec3(-h,0,0);
	float dx = (noise(x1) - noise(x2))/(2*h);

	vec3 y1 = vec + vec3(0,h,0);
	vec3 y2 = vec + vec3(0,-h,0);
	float dy = (noise(y1) - noise(y2))/(2*h);

	vec3 z1 = vec + vec3(0,0,h);
	vec3 z2 = vec + vec3(0,0,-h);
	float dz = (noise(z1) - noise(z2))/(2*h);

	return vec3(dx, dy, dz);
}

/**
 * Create turbulence at the given point. The higher the persistance
 * the more turbulence is generated. The higher the octaves the smoother
 * the final turbulance will be.
 */
float turbulence(vec3 vec, float persistence, int octaves, int start, float offset)
{
	// offset so we are not at (0,0). Noise function does not
	// behave correctly near the origin.
	float total = 0;
	for (int i = start; i < octaves; i++)
	{
		float freq = pow(2, i);
		float amp = pow(persistence, i);
		total += noise(modelPos * freq + offset) * amp;
	}
	return total;
}

vec4 grass()
{
	vec3 green = vec3(0.133, 0.545, 0.133);
	vec3 brown = vec3(0.545, 0.271, 0.075);
	vec3 final;

	float turbulence = turbulence(modelPos, persistence, octaveCount, octaveStart, 25);
	// The persistance from the first octave will contribute
	// the most to the total noise.
	if (turbulence < persistence * 0.75)
		final = brown * turbulence;
		//final = mix(brown, green, turbulence);
	else
		final = green * turbulence;
	return vec4(final, 1);
}

vec4 wood()
{
	// Add some turbulance to the distance so that we get curvy rings.
	float turbulence = turbulence(modelPos, persistence, octaveCount, octaveStart, 25);
	float dist = length(modelPos.xz) + turbulence;
	float value = (cos(dist * ringFreq) + 1 ) * 0.5f;
	value = pow(value, 3);

	vec3 light = vec3(0.2941, 0.2118, 0.1294);
	vec3 dark = vec3(0.1686, 0.1176, 0.0863);
	return vec4(mix(light, dark, value), 1);
}

vec3 waves(vec3 vec)
{
	vec3[MAX_WAVE_CENTERS] centers;
	float[MAX_WAVE_CENTERS] freqs;

	for (int i = 0; i < waveCenters; i++)
	{
		centers[i] = normalize(diffNoise(i * vec3(100,0,0)));
		freqs[i] = noise(vec2(i+0.12, i+0.91)) * (maxFreq - minFreq) / maxFreq + minFreq;
	}

	vec3 displacement = vec3(0);

	for (int i = 0; i < waveCenters; i++)
	{
		vec3 toPoint = vec - centers[i];
		displacement += normalize(toPoint) * cos(length(toPoint)*freqs[i] - time*phaseSpeed);
	}
	return displacement;
}

void main()
{
	vec3 unitToCamera = normalize(toCamera);
	vec3 unitToLight = normalize(toLight);
	vec3 unitNormal = normalize(normal);

	vec3 ambient = vec3(0.4);

	float diffuseBrightness = max(dot(unitNormal, unitToLight), 0);
	vec3 diffuse = vec3(1.0) * diffuseBrightness;

	vec3 specular = vec3(0);

	vec4 textureCol = vec4(1);

	if (effect == 0)	// terrain
	{
		textureCol = grass();
	}
	else if (effect == 1)	// wood
	{
		textureCol = wood();
	}
	else if (effect == 2)	// water
	{
		textureCol = vec4(0.1, 0.5, 0.8, 0.5);	// blue
		// Add some noise to the normal vector but keep it cyclic.
		vec3 waveNormal = normalize(unitNormal + waves(modelPos));
		diffuseBrightness = max(dot(waveNormal, unitToLight), 0);
		diffuse = vec3(1.0) * diffuseBrightness;

		float shininess = 32;
		float specularCoeff = 0.5;
		vec3 refl = 2 * dot(unitToLight, waveNormal) * waveNormal - unitToLight;
		refl = normalize(refl);
		float specularFactor = max(dot(refl, unitToCamera), 0);
		float dampedFactor = pow(specularFactor, shininess);
		specular = dampedFactor * specularCoeff * vec3(1.0);
	}
	else if (effect == 3) // black white noise
	{
		float n = turbulence(modelPos, persistence, octaveCount, octaveStart, 25);
		textureCol = vec4(n,n,n,1);
	}

	vec4 totalLight = vec4(ambient + diffuse , 1);
	fragColor = textureCol * totalLight + vec4(specular,0);
}
