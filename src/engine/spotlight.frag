#version 130

in vec2 TexCoord;

uniform sampler2D backgroundTexture;  // Background texture
uniform sampler2D normalMap;          // Normal map texture

uniform vec2 lightPos;               // Light position in screen space
uniform float lightRadius;           // Radius of the light
uniform float lightIntensity;        // Intensity of the light
uniform vec4 lightColor;             // Spotlight color
uniform vec4 ambientColor;           // Ambient light color

// Function to compute attenuation
float computeAttenuation(float distance, float radius) {
    return max(0.0, 1.0 - (distance / radius));
}

void main() {
    // Compute light direction and distance
    vec2 fragPos = gl_FragCoord.xy;
    
    // Compute the direction of light (use absolute difference in positions for a consistent light direction)
    vec2 lightDir =  lightPos - fragPos; // Keep the direction vector as is
    float distance = length(lightDir);  // Calculate distance from the fragment to the light
    float attenuation = computeAttenuation(distance, lightRadius);

    // Normalize the light direction
    lightDir = normalize(lightDir);

    // Fetch normal from the normal map
    vec3 normal = texture2D(normalMap, TexCoord).rgb;
    normal = normalize(normal * 2.0 - 1.0); // Convert from [0,1] to [-1,1]

    // Compute diffuse lighting
    float diffuse = max(dot(normal.xy, lightDir), 0.0);

    // Combine light effect with attenuation and diffuse
    vec4 lightEffect = lightColor * lightIntensity * diffuse * attenuation;

    // Fetch background color
    vec4 backgroundColor = texture2D(backgroundTexture, TexCoord);

    // Final fragment color: Ambient + Light Effect
    gl_FragColor = ambientColor + backgroundColor * lightEffect;
}
