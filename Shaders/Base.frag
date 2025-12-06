// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE.txt for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// This corresponds to the output color to the color buffer
out vec4 outColor;
uniform vec4 uColor;

// This is used for the texture sampling
uniform sampler2D uTexture;

// This is used for texture blending
uniform float uTextureFactor;

// Global alpha multiplier
uniform float uGlobalAlpha;

// Circle mode
uniform float uIsCircle;

// Tex coord input from vertex shader
in vec2 fragTexCoord;

void main()
{
    if (uIsCircle > 0.5) {
        // Radial gradient for blurred circle (Gaussian-like soft particle)
        float dist = distance(fragTexCoord, vec2(0.5, 0.5));
        // 0.5 is the radius. We want it to fade out before hitting the corners.
        // Gaussian falloff: exp(-k * dist^2)
        // Adjust k to control sharpness. 
        float alpha = exp(-dist * dist * 20.0);
        
        // Hard cut at 0.5 to ensure it doesn't bleed into corners if exp doesn't reach 0
        if (dist > 0.5) {
            alpha = 0.0;
        }
        
        outColor = uColor * alpha;
        outColor.a *= uGlobalAlpha;
    } else {
        vec4 texColor = texture(uTexture, fragTexCoord);
        outColor = mix(uColor, texColor, uTextureFactor);
        outColor.a *= uGlobalAlpha;
    }
}
