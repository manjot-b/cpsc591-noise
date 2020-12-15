#version 330 core

#define SQRT2 1.41421356273

in vec2 modelPos;

uniform int[512] perm;

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

vec2 getGradientVector(int cornerValue)
{
	// return one of four gradient vectors.
	int v = cornerValue % 4;	
	if (v == 0)
		return vec2(SQRT2, 0);
	else if (v == 1)
		return vec2(0, SQRT2);
	else if (v == 2)
		return vec2(-SQRT2, 0);
	else 
		return vec2(0, -SQRT2);
}

/**
 *	Computes the 2d perlin noise.
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
	float dotTopRight = dot(topRight, getGradientVector(valueTopRight));
	float dotTopLeft = dot(topLeft, getGradientVector(valueTopLeft));
	float dotBotRight = dot(botRight, getGradientVector(valueBotRight));
	float dotBotLeft = dot(botLeft, getGradientVector(valueBotLeft));

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

vec4 grass()
{
	// offset so we are not at (0,0). Noise function does not
	// behave correctly near the origin.
	float offset = 25;
	float persistence = 7/16.0;
	int octaves = 8;
	float total = 0;
	for (int i = 1; i < octaves; i++)
	{
		float freq = pow(2, i);
		float amp = pow(persistence, i);
		total += noise(modelPos * freq + offset) * amp;
	}

	vec4 green = vec4(0.133, 0.545, 0.133, 0.0);
	vec4 brown = vec4(0.545, 0.271, 0.075, 0.0);
	vec4 final;

	// The persistance from the first octave will contribute
	// the most to the total noise.
	if (total < persistence * 0.80)
		final = brown * total;
	else
		final = green * total;
	return final;
}

void main()
{
	fragColor = grass();
}
