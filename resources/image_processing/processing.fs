#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diff;
uniform int   processing_mode;
uniform float gamma;
uniform float filter[9];
uniform vec2 offset[9];

void main()
{    
    // standard
	if (processing_mode == 0)
	{
		FragColor = texture(diff, vTex);
	}

	// greyscale
	if (processing_mode == 1)
	{
		// Convert to greyscale using NTSC weightings
		float grey = dot(texture(diff, vTex).rgb, vec3(0.299, 0.587, 0.114));
		FragColor = vec4(grey, grey, grey, 1.0);
	}

	// inversion
	if (processing_mode == 2)
	{
		FragColor = vec4(vec3(1.0 - texture(diff, vTex)), 1.0);
	}

    // power-law(gamma) transformation
    if (processing_mode == 3)
	{
		// Convert to greyscale using NTSC weightings
        vec4 c = texture(diff, vTex);
		float r = pow(c.r, gamma);
        float g = pow(c.g, gamma);
        float b = pow(c.b, gamma);

		FragColor = vec4(r, g, b, 1.0);
	}

	// Sharpen
	if (processing_mode == 4)
	{
		vec3 sampleTex[9];
    	for(int i = 0; i < 9; i++)
        	sampleTex[i] = vec3(texture(diff, vTex + offset[i]));

    	vec3 sum = vec3(0.0);
    	for(int i = 0; i < 9; i++)
        	sum += sampleTex[i] * filter[i];

		FragColor = vec4(sum, 1.0);
	}

	
	
}