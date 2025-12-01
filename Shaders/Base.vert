// ----------------------------------------------------------------
// From Game Programming in C++ by Sanjay Madhav
// Copyright (C) 2017 Sanjay Madhav. All rights reserved.
//
// Released under the BSD License
// See LICENSE.txt for full details.
// ----------------------------------------------------------------

// Request GLSL 3.3
#version 330

// Attribute 0 is position
layout (location = 0) in vec2 inPosition;

// Attribute 1 is texture coordinate
layout(location = 1) in vec2 inTexCoord;

uniform mat4 uWorldTransform;
uniform mat4 uOrthoProj;
uniform vec4 uColor;
uniform vec2 uCameraPos;

// (u0, v0, u1, v1) for current sprite frame
uniform vec4 uTexRect;

// Any vertex outputs (other than position)
out vec2 fragTexCoord;

void main()
{
    // Transform the vertex position to world space
    vec4 worldPos = uWorldTransform * vec4(inPosition, 0.0, 1.0);

    // Subtract the camera position in world space
    worldPos.xy -= uCameraPos;

    // Transform to clip space
    gl_Position = uOrthoProj * worldPos;

    // Pass the texture coordinate to the fragment shader
    // Scale and offset the input texture coordinate to map to the correct sprite
    fragTexCoord = inTexCoord * uTexRect.zw + uTexRect.xy;
}