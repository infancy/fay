#version 330 core
out vec4 FragColor;

// TODO: remove
uniform sampler2D Albedo;
uniform bool bAlbedo;

float near = 1.0; 
float far  = 300.0;

void main()
{   
    // from camera space to screen space

    // opengl-style
    // z_NDC = (f * z + n * z + 2 * n * f) / (z * (f - n));
    // z_SCREEN = (zp + 1) / 2; // [-1, 1] -> [0, 1]
    // linearize depth
    // float z = gl_FragCoord.z * 2.0 - 1.0; // back to NDC 
    // float depth = (2.0 * near * far) / (z * (far - near) - far - near);
    // depth *= -1.0;
    //depth /= far;

    // d3d-style
    // z_NDC = f(z - n) / z(f - n)
    // z_SCREEN = z_NDC
    float depth = far * near / (far - gl_FragCoord.z * (far - near));

    FragColor = vec4(vec3(depth), 1.0);
}