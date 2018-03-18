#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diff;
uniform int   processing_mode;
uniform float gamma;

void main()
{    
    // standard
	if (processing_mode == 0)
	{
		FragColor = texture2D(diff, vTex);
	}

	// greyscale
	if (processing_mode == 1)
	{
		// Convert to greyscale using NTSC weightings
		float grey = dot(texture2D(diff, vTex).rgb, vec3(0.299, 0.587, 0.114));

		FragColor = vec4(grey, grey, grey, 1.0);
	}

    // gray-scale transformation
    // power-law(gamma) transformation
    if (processing_mode == 2)
	{
		// Convert to greyscale using NTSC weightings
        vec4 c = texture2D(diff, vTex);
		float r = pow(c.r, gamma);
        float g = pow(c.g, gamma);
        float b = pow(c.b, gamma);

		FragColor = vec4(r, g, b, 1.0);
	}
	
}