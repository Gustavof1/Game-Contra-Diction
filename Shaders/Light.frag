#version 330
out vec4 outColor;
uniform vec4 uColor;
uniform sampler2D uTexture;
uniform float uTextureFactor;
uniform float uGlobalAlpha;

in vec2 fragTexCoord;
in vec2 fragWorldPos;

#define MAX_LIGHTS 16
uniform vec2 uLightPositions[MAX_LIGHTS];
uniform float uLightRadii[MAX_LIGHTS];
uniform vec3 uLightColors[MAX_LIGHTS];
uniform int uNumLights;
uniform vec3 uAmbientLight;

// Flashlight uniforms
uniform bool uFlashlightActive;
uniform vec2 uFlashlightPos;
uniform vec2 uFlashlightDir;
uniform float uFlashlightAngle; // In radians (half angle)

void main()
{
    vec4 texColor = texture(uTexture, fragTexCoord);
    vec4 baseColor = mix(uColor, texColor, uTextureFactor);
    
    vec3 lighting = uAmbientLight;
    
    for (int i = 0; i < uNumLights; i++)
    {
        float dist = distance(fragWorldPos, uLightPositions[i]);
        float radius = uLightRadii[i];
        
        if (dist < radius)
        {
            float intensity = 1.0 - (dist / radius);
            intensity = intensity * intensity; // Quadratic falloff looks nicer
            lighting += uLightColors[i] * intensity;
        }
    }

    if (uFlashlightActive)
    {
        vec2 toFrag = fragWorldPos - uFlashlightPos;
        float dist = length(toFrag);
        if (dist > 0.0) // Avoid division by zero
        {
            vec2 dir = normalize(toFrag);
            float angle = acos(dot(dir, uFlashlightDir));
            
            if (angle < uFlashlightAngle)
            {
                float radius = 600.0; // Flashlight range
                if (dist < radius)
                {
                    float intensity = 1.0 - (dist / radius);
                    // Soft edge for the cone
                    float edgeSoftness = 0.1;
                    float spotEffect = smoothstep(uFlashlightAngle, uFlashlightAngle - edgeSoftness, angle);
                    
                    lighting += vec3(1.0, 1.0, 0.9) * intensity * spotEffect * 1.5;
                }
            }
        }
    }
    
    lighting = clamp(lighting, 0.0, 1.0);
    
    outColor = vec4(baseColor.rgb * lighting, baseColor.a * uGlobalAlpha);
}
