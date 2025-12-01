#version 330
layout (location = 0) in vec2 inPosition;
layout (location = 1) in vec2 inTexCoord;

uniform mat4 uWorldTransform;
uniform mat4 uOrthoProj;
uniform vec2 uCameraPos;
uniform vec4 uTexRect;
uniform float uTime;
uniform float uIsVegetation;

out vec2 fragTexCoord;
out vec2 fragWorldPos;

void main()
{
    vec4 pos = vec4(inPosition, 0.0, 1.0);
    
    if (uIsVegetation > 0.5) {
        // Wiggle effect
        // 0.5 is bottom, -0.5 is top. We want 0 movement at bottom.
        float factor = (0.5 - inPosition.y); 
        pos.x += sin(uTime * 3.0 + uWorldTransform[3][0] * 0.1) * factor * 0.2; 
        // uWorldTransform[3][0] is the translation X component, to make different grasses wiggle differently
    }

    vec4 worldPos = uWorldTransform * pos;
    fragWorldPos = worldPos.xy;
    worldPos.xy -= uCameraPos;
    gl_Position = uOrthoProj * worldPos;
    fragTexCoord = inTexCoord * uTexRect.zw + uTexRect.xy;
}
