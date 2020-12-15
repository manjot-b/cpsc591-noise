#version 330 core

#define SQRT2 1.41421356273

in vec2 modelPos;

uniform int[256] perm;

out vec4 fragColor;

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
	int xi = int(vec.x);	
	int yi = int(vec.y);	

	// Compute the vector pointing from the corner to the
	// given point. Place the fractional part of point in [0,1]^2.
	vec2 frac = vec - vec2(float(xi), float(yi));

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
	int valueTopRight = perm[ (perm[(xi + 1) % 256] + yi + 1) % 256];
	int valueTopLeft = perm[ (perm[xi % 256] + yi + 1) % 256];
	int valueBotRight = perm[ (perm[(xi + 1) % 256] + yi) % 256];
	int valueBotLeft = perm[ (perm[xi % 256] + yi) % 256];

	// Take the dot between the vector from corner to point and
	// the gradient vector of the corner.
	float dotTopRight = dot(topRight, getGradientVector(valueTopRight));
	float dotTopLeft = dot(topLeft, getGradientVector(valueTopLeft));
	float dotBotRight = dot(botRight, getGradientVector(valueBotRight));
	float dotBotLeft = dot(botLeft, getGradientVector(valueBotLeft));

	// Interpolate first vertically then horizontally.
	float vert1 = mix(dotBotLeft, dotTopLeft, frac.y);
	float vert2 = mix(dotBotRight, dotTopRight, frac.y);
	float value = mix(vert1, vert2, frac.x);

	// put in range [0,1]. -2 <= dotprod <= 2.
	value = (value + 2.0) * 0.25;
	return value;
}

void main()
{
	// offset so we are not at (0,0). Noise function does not
	// behave correctly near the origin.
	float offset = 25;
	float persistence = 6/16.0;
	int octaves = 8;
	float total = 0;
	for (int i = 1; i < octaves; i++)
	{
		float freq = pow(2, i);
		float amp = pow(persistence, i);
		total += noise(modelPos * freq + offset) * amp;
	}
	fragColor = vec4(1.0f, 1.0f, 1.0f, 0.0f) * total;
	//fragColor = vec4(1.0f, 1.0f, 1.0f, 0.0f);
}
