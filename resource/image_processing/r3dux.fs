#version 330 core
in vec2 vTex;
out vec4 FragColor;

uniform sampler2D diff;
uniform int       processing_mode;
uniform vec2      offset[25]; // Texture coordinate offsets

// https://r3dux.org/2011/06/glsl-image-processing/

void main()
{    
    // Standard
	if (processing_mode == 0)
	{
		FragColor = texture2D(diff, vTex);
	}

	// Greyscale
	if (processing_mode == 1)
	{
		// Convert to greyscale using NTSC weightings
		float grey = dot(texture2D(diff, vTex).rgb, vec3(0.299, 0.587, 0.114));

		FragColor = vec4(grey, grey, grey, 1.0);
	}

	// Sepia tone
	if (processing_mode == 2)
	{
		// Convert to greyscale using NTSC weightings
		float grey = dot(texture2D(diff, vTex).rgb, vec3(0.299, 0.587, 0.114));

		// Play with these rgb weightings to get different tones.
		// (As long as all rgb weightings add up to 1.0 you won't lighten or darken the image)
		FragColor = vec4(grey * vec3(1.2, 1.0, 0.8), 1.0);
	}

	// Negative
	if (processing_mode == 3)
	{
		vec4 texMapColour = texture2D(diff, vTex);

		FragColor = vec4(1.0 - texMapColour.rgb, 1.0);
	}

	// Blur (gaussian)
	if (processing_mode == 4)
	{
		vec4 sample[25];

		for (int i = 0; i < 25; i++)
		{
			// Sample a grid around and including our texel
			sample[i] = texture(diff, vTex + offset[i]);
		}

		// Gaussian weighting:
		// 1  4  7  4 1
		// 4 16 26 16 4
		// 7 26 41 26 7 / 273 (i.e. divide by total of weightings)
		// 4 16 26 16 4
		// 1  4  7  4 1

    		FragColor = (
        	           (1.0  * (sample[0] + sample[4]  + sample[20] + sample[24])) +
	                   (4.0  * (sample[1] + sample[3]  + sample[5]  + sample[9] + sample[15] + sample[19] + sample[21] + sample[23])) +
	                   (7.0  * (sample[2] + sample[10] + sample[14] + sample[22])) +
	                   (16.0 * (sample[6] + sample[8]  + sample[16] + sample[18])) +
	                   (26.0 * (sample[7] + sample[11] + sample[13] + sample[17])) +
	                   (41.0 * sample[12])
	                   ) / 273.0;

	}

	// Blur (mean filter)
	if (processing_mode == 5)
	{
		FragColor = vec4(0.0);

		for (int i = 0; i < 25; i++)
		{
			// Sample a grid around and including our texel
			FragColor += texture(diff, vTex + offset[i]);
		}

		// Divide by the number of samples to get our mean
		FragColor /= 25;
	}

	// Sharpen
	if (processing_mode == 6)
	{
		vec4 sample[25];

		for (int i = 0; i < 25; i++)
		{
			// Sample a grid around and including our texel
			sample[i] = texture(diff, vTex + offset[i]);
		}

		// Sharpen weighting:
		// -1 -1 -1 -1 -1
		// -1 -1 -1 -1 -1
		// -1 -1 25 -1 -1
		// -1 -1 -1 -1 -1
		// -1 -1 -1 -1 -1

    		FragColor = 25.0 * sample[12];

		for (int i = 0; i < 25; i++)
		{
			if (i != 12)
				FragColor -= sample[i];
		}
	}

	// Dilate
	if (processing_mode == 7)
	{
		vec4 sample[25];
		vec4 maxValue = vec4(0.0);

		for (int i = 0; i < 25; i++)
		{
			// Sample a grid around and including our texel
			sample[i] = texture(diff, vTex + offset[i]);

			// Keep the maximum value		
			maxValue = max(sample[i], maxValue);
		}

		FragColor = maxValue;
	}

	// Erode
	if (processing_mode == 8)
	{
		vec4 sample[25];
		vec4 minValue = vec4(1.0);

		for (int i = 0; i < 25; i++)
		{
			// Sample a grid around and including our texel
			sample[i] = texture(diff, vTex + offset[i]);

			// Keep the minimum value		
			minValue = min(sample[i], minValue);
		}

		FragColor = minValue;
	}

	// Laplacian Edge Detection (very, very similar to sharpen filter - check it out!)
	if (processing_mode == 9)
	{
		vec4 sample[25];

		for (int i = 0; i < 25; i++)
		{
			// Sample a grid around and including our texel
			sample[i] = texture(diff, vTex + offset[i]);
		}

		// Laplacian weighting:
		// -1 -1 -1 -1 -1
		// -1 -1 -1 -1 -1
		// -1 -1 24 -1 -1
		// -1 -1 -1 -1 -1
		// -1 -1 -1 -1 -1

    		FragColor = 24.0 * sample[12];

		for (int i = 0; i < 25; i++)
		{
			if (i != 12)
				FragColor -= sample[i];
		}
	}
}